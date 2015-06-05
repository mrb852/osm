/*
 * Trivial Filesystem (GFS).
 *
 * Copyright (C) 2003 Juha Aatrokoski, Timo Lilja,
 *   Leena Salmela, Teemu Takanen, Aleksi Virtanen.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "kernel/kmalloc.h"
#include "kernel/assert.h"
#include "vm/pagepool.h"
#include "drivers/gbd.h"
#include "fs/vfs.h"
#include "fs/gfs.h"
#include "lib/libc.h"
#include "lib/bitmap.h"

/**@name Trivial Filesystem (GFS)
 *
 * This module contains implementation for GFS.
 *
 * @{
 */

/* Data structure used internally by GFS filesystem. This data structure
   is used by gfs-functions. it is initialized during gfs_init(). Also
   memory for the buffers is reserved _dynamically_ during init.

   Buffers are used when reading/writing system or data blocks from/to
   disk.
*/
typedef struct {
  /* Total number of blocks of the disk */
  uint32_t       totalblocks;

  /* Pointer to gbd device performing gfs */
  gbd_t          *disk;

  /* lock for mutual exclusion of fs-operations (we support only
     one operation at a time in any case) */
  semaphore_t    *lock;

  /* Buffers for read/write operations on disk. */
  gfs_inode_t    *buffer_inode;   /* buffer for inode blocks */
  bitmap_t       *buffer_bat;     /* buffer for allocation block */
  gfs_direntry_t *buffer_md;      /* buffer for directory block */
} gfs_t;


/**
 * Initialize trivial filesystem. Allocates 1 page of memory dynamically for
 * filesystem data structure, gfs data structure and buffers needed.
 * Sets fs_t and gfs_t fields. If initialization is succesful, returns
 * pointer to fs_t data structure. Else NULL pointer is returned.
 *
 * @param Pointer to gbd-device performing gfs.
 *
 * @return Pointer to the filesystem data structure fs_t, if fails
 * return NULL.
 */
fs_t * gfs_init(gbd_t *disk)
{
  uint32_t addr;
  gbd_request_t req;
  char name[GFS_VOLUMENAME_MAX];
  fs_t *fs;
  gfs_t *gfs;
  int r;
  semaphore_t *sem;

  if(disk->block_size(disk) != GFS_BLOCK_SIZE)
    return NULL;

  /* check semaphore availability before memory allocation */
  sem = semaphore_create(1);
  if (sem == NULL) {
    kprintf("gfs_init: could not create a new semaphore.\n");
    return NULL;
  }

  addr = pagepool_get_phys_page();
  if(addr == 0) {
    semaphore_destroy(sem);
    kprintf("gfs_init: could not allocate memory.\n");
    return NULL;
  }
  addr = ADDR_PHYS_TO_KERNEL(addr);      /* transform to vm address */


  /* Assert that one page is enough */
  KERNEL_ASSERT(PAGE_SIZE >= (3*GFS_BLOCK_SIZE+sizeof(gfs_t)+sizeof(fs_t)));

  /* Read header block, and make sure this is gfs drive */
  req.block = 0;
  req.sem = NULL;
  req.buf = ADDR_KERNEL_TO_PHYS(addr);   /* disk needs physical addr */
  r = disk->read_block(disk, &req);
  if(r == 0) {
    semaphore_destroy(sem);
    pagepool_free_phys_page(ADDR_KERNEL_TO_PHYS(addr));
    kprintf("gfs_init: Error during disk read. Initialization failed.\n");
    return NULL;
  }

  if(((uint32_t *)addr)[0] != GFS_MAGIC) {
    semaphore_destroy(sem);
    pagepool_free_phys_page(ADDR_KERNEL_TO_PHYS(addr));
    return NULL;
  }

  /* Copy volume name from header block. */
  stringcopy(name, (char *)(addr+4), GFS_VOLUMENAME_MAX);

  /* fs_t, gfs_t and all buffers in gfs_t fit in one page, so obtain
     addresses for each structure and buffer inside the allocated
     memory page. */
  fs  = (fs_t *)addr;
  gfs = (gfs_t *)(addr + sizeof(fs_t));
  gfs->buffer_inode = (gfs_inode_t *)((uint32_t)gfs + sizeof(gfs_t));
  gfs->buffer_bat = (bitmap_t *)((uint32_t)gfs->buffer_inode +
                        GFS_BLOCK_SIZE);
  gfs->buffer_md   = (gfs_direntry_t *)((uint32_t)gfs->buffer_bat +
    GFS_BLOCK_SIZE);

  gfs->totalblocks = MIN(disk->total_blocks(disk), 8*GFS_BLOCK_SIZE);
  gfs->disk        = disk;

  /* save the semaphore to the gfs_t */
  gfs->lock = sem;

  fs->internal = (void *)gfs;
  stringcopy(fs->volume_name, name, VFS_NAME_LENGTH);

  fs->unmount = gfs_unmount;
  fs->open    = gfs_open;
  fs->close   = gfs_close;
  fs->create  = gfs_create;
  fs->remove  = gfs_remove;
  fs->read    = gfs_read;
  fs->write   = gfs_write;
  fs->getfree  = gfs_getfree;

  return fs;
}

/**
 * Unmounts gfs filesystem from gbd device. After this GFS-driver and
 * gbd-device are no longer linked together. Implements
 * fs.unmount(). Waits for the current operation(s) to finish, frees
 * reserved memory and returns OK.
 *
 * @param fs Pointer to fs data structure of the device.
 *
 * @return VFS_OK
 */
int gfs_unmount(fs_t *fs)
{
  gfs_t *gfs;

  gfs = (gfs_t *)fs->internal;

  semaphore_P(gfs->lock); /* The semaphore should be free at this
    point, we get it just in case something has gone wrong. */

  /* free semaphore and allocated memory */
  semaphore_destroy(gfs->lock);
  pagepool_free_phys_page(ADDR_KERNEL_TO_PHYS((uint32_t)fs));
  return VFS_OK;
}

/**
 * Opens file. Implements fs.open(). Reads directory block of gfs
 * device and finds given file. Returns file's inode block number or
 * VFS_NOT_FOUND, if file not found.
 *
 * @param fs Pointer to fs data structure of the device.
 * @param filename Name of the file to be opened.
 *
 * @return If file found, return inode block number as fileid, otherwise
 * return VFS_NOT_FOUND.
 */
int gfs_open(fs_t *fs, char *filename)
{
  gfs_t *gfs;
  gbd_request_t req;
  uint32_t i;
  int r;

  gfs = (gfs_t *)fs->internal;

  semaphore_P(gfs->lock);

  req.block     = GFS_DIRECTORY_BLOCK;
  req.buf       = ADDR_KERNEL_TO_PHYS((uint32_t)gfs->buffer_md);
  req.sem       = NULL;
  r = gfs->disk->read_block(gfs->disk,&req);
  if(r == 0) {
    /* An error occured during read. */
    semaphore_V(gfs->lock);
    return VFS_ERROR;
  }

  for(i=0;i < GFS_MAX_FILES;i++) {
    if(stringcmp(gfs->buffer_md[i].name, filename) == 0) {
      semaphore_V(gfs->lock);
      return gfs->buffer_md[i].inode;
    }
  }

  semaphore_V(gfs->lock);
  return VFS_NOT_FOUND;
}


/**
 * Closes file. Implements fs.close(). There is nothing to be done, no
 * data strucutures or similar are reserved for file. Returns VFS_OK.
 *
 * @param fs Pointer to fs data structure of the device.
 * @param fileid File id (inode block number) of the file.
 *
 * @return VFS_OK
 */
int gfs_close(fs_t *fs, int fileid)
{
  fs = fs;
  fileid = fileid;

  return VFS_OK;
}


/**
 * Creates file of given size. Implements fs.create(). Checks that
 * file name doesn't allready exist in directory block.Allocates
 * enough blocks from the allocation block for the file (1 for inode
 * and then enough for the file of given size). Reserved blocks are zeroed.
 *
 * @param fs Pointer to fs data structure of the device.
 * @param filename File name of the file to be created
 * @param size Size of the file to be created
 *
 * @return If file allready exists or not enough space return VFS_ERROR,
 * otherwise return VFS_OK.
 */
int gfs_create(fs_t *fs, char *filename, int size)
{
  gfs_t *gfs = (gfs_t *)fs->internal;
  gbd_request_t req;
  uint32_t i;
  uint32_t numblocks = (size + GFS_BLOCK_SIZE - 1)/GFS_BLOCK_SIZE;
  int index = -1;
  int r;

  semaphore_P(gfs->lock);

  if(numblocks > (GFS_BLOCK_SIZE / 4 - 1)) {
    semaphore_V(gfs->lock);
    return VFS_ERROR;
  }

  /* Read directory block. Check that file doesn't allready exist and
     there is space left for the file in directory block. */
  req.block = GFS_DIRECTORY_BLOCK;
  req.buf = ADDR_KERNEL_TO_PHYS((uint32_t)gfs->buffer_md);
  req.sem = NULL;
  r = gfs->disk->read_block(gfs->disk, &req);
  if(r == 0) {
    /* An error occured. */
    semaphore_V(gfs->lock);
    return VFS_ERROR;
  }

  for(i=0;i<GFS_MAX_FILES;i++) {
    if(stringcmp(gfs->buffer_md[i].name, filename) == 0) {
      semaphore_V(gfs->lock);
      return VFS_ERROR;
    }

    if(gfs->buffer_md[i].inode == 0) {
      /* found free slot from directory */
      index = i;
    }
  }

  if(index == -1) {
    /* there was no space in directory, because index is not set */
    semaphore_V(gfs->lock);
    return VFS_ERROR;
  }

  stringcopy(gfs->buffer_md[index].name,filename, GFS_FILENAME_MAX);

  /* Read allocation block and... */
  req.block = GFS_ALLOCATION_BLOCK;
  req.buf = ADDR_KERNEL_TO_PHYS((uint32_t)gfs->buffer_bat);
  req.sem = NULL;
  r = gfs->disk->read_block(gfs->disk, &req);
  if(r==0) {
    /* An error occured. */
    semaphore_V(gfs->lock);
    return VFS_ERROR;
  }


  /* ...find space for inode... */
  gfs->buffer_md[index].inode = bitmap_findnset(gfs->buffer_bat,
                                  gfs->totalblocks);
  if((int)gfs->buffer_md[index].inode == -1) {
    semaphore_V(gfs->lock);
    return VFS_ERROR;
  }

  /* ...and the rest of the blocks. Mark found block numbers in
     inode.*/
  gfs->buffer_inode->filesize = size;
  for(i=0; i<numblocks; i++) {
    gfs->buffer_inode->block[i] = bitmap_findnset(gfs->buffer_bat,
                                    gfs->totalblocks);
    if((int)gfs->buffer_inode->block[i] == -1) {
      /* Disk full. No free block found. */
      semaphore_V(gfs->lock);
      return VFS_ERROR;
    }
  }

  /* Mark rest of the blocks in inode as unused. */
  while(i < (GFS_BLOCK_SIZE / 4 - 1))
    gfs->buffer_inode->block[i++] = 0;

  req.block = GFS_ALLOCATION_BLOCK;
  req.buf   = ADDR_KERNEL_TO_PHYS((uint32_t)gfs->buffer_bat);
  req.sem   = NULL;
  r = gfs->disk->write_block(gfs->disk, &req);
  if(r==0) {
    /* An error occured. */
    semaphore_V(gfs->lock);
    return VFS_ERROR;
  }

  req.block = GFS_DIRECTORY_BLOCK;
  req.buf   = ADDR_KERNEL_TO_PHYS((uint32_t)gfs->buffer_md);
  req.sem   = NULL;
  r = gfs->disk->write_block(gfs->disk, &req);
  if(r==0) {
    /* An error occured. */
    semaphore_V(gfs->lock);
    return VFS_ERROR;
  }

  req.block = gfs->buffer_md[index].inode;
  req.buf   = ADDR_KERNEL_TO_PHYS((uint32_t)gfs->buffer_inode);
  req.sem   = NULL;
  r = gfs->disk->write_block(gfs->disk, &req);
  if(r==0) {
    /* An error occured. */
    semaphore_V(gfs->lock);
    return VFS_ERROR;
  }

  /* Write zeros to the reserved blocks. Buffer for allocation block
     is no longer needed, so lets use it as zero buffer. */
  memoryset(gfs->buffer_bat, 0, GFS_BLOCK_SIZE);
  for(i=0;i<numblocks;i++) {
    req.block = gfs->buffer_inode->block[i];
    req.buf   = ADDR_KERNEL_TO_PHYS((uint32_t)gfs->buffer_bat);
    req.sem   = NULL;
    r = gfs->disk->write_block(gfs->disk, &req);
    if(r==0) {
      /* An error occured. */
      semaphore_V(gfs->lock);
      return VFS_ERROR;
    }
  }

  semaphore_V(gfs->lock);
  return VFS_OK;
}

/**
 * Removes given file. Implements fs.remove(). Frees blocks allocated
 * for the file and directory entry.
 *
 * @param fs Pointer to fs data structure of the device.
 * @param filename file to be removed.
 *
 * @return VFS_OK if file succesfully removed. If file not found
 * VFS_NOT_FOUND.
 */
int gfs_remove(fs_t *fs, char *filename)
{
  gfs_t *gfs = (gfs_t *)fs->internal;
  gbd_request_t req;
  uint32_t i;
  int index = -1;
  int r;

  semaphore_P(gfs->lock);

  /* Find file and inode block number from directory block.
     If not found return VFS_NOT_FOUND. */
  req.block = GFS_DIRECTORY_BLOCK;
  req.buf = ADDR_KERNEL_TO_PHYS((uint32_t)gfs->buffer_md);
  req.sem = NULL;
  r = gfs->disk->read_block(gfs->disk, &req);
  if(r == 0) {
    /* An error occured. */
    semaphore_V(gfs->lock);
    return VFS_ERROR;
  }

  for(i=0;i<GFS_MAX_FILES;i++) {
    if(stringcmp(gfs->buffer_md[i].name, filename) == 0) {
      index = i;
      break;
    }
  }
  if(index == -1) {
    semaphore_V(gfs->lock);
    return VFS_NOT_FOUND;
  }

  /* Read allocation block of the device and inode block of the file.
     Free reserved blocks (marked in inode) from allocation block. */
  req.block = GFS_ALLOCATION_BLOCK;
  req.buf = ADDR_KERNEL_TO_PHYS((uint32_t)gfs->buffer_bat);
  req.sem = NULL;
  r = gfs->disk->read_block(gfs->disk, &req);
  if(r == 0) {
    /* An error occured. */
    semaphore_V(gfs->lock);
    return VFS_ERROR;
  }

  req.block = gfs->buffer_md[index].inode;
  req.buf = ADDR_KERNEL_TO_PHYS((uint32_t)gfs->buffer_inode);
  req.sem = NULL;
  r = gfs->disk->read_block(gfs->disk, &req);
  if(r == 0) {
    /* An error occured. */
    semaphore_V(gfs->lock);
    return VFS_ERROR;
  }


  bitmap_set(gfs->buffer_bat,gfs->buffer_md[index].inode,0);
  i=0;
  while(gfs->buffer_inode->block[i] != 0 &&
          i < (GFS_BLOCK_SIZE / 4 - 1)) {
    bitmap_set(gfs->buffer_bat,gfs->buffer_inode->block[i],0);
    i++;
  }

  /* Free directory entry. */
  gfs->buffer_md[index].inode   = 0;
  gfs->buffer_md[index].name[0] = 0;

  req.block = GFS_ALLOCATION_BLOCK;
  req.buf   = ADDR_KERNEL_TO_PHYS((uint32_t)gfs->buffer_bat);
  req.sem   = NULL;
  r = gfs->disk->write_block(gfs->disk, &req);
  if(r == 0) {
    /* An error occured. */
    semaphore_V(gfs->lock);
    return VFS_ERROR;
  }

  req.block = GFS_DIRECTORY_BLOCK;
  req.buf   = ADDR_KERNEL_TO_PHYS((uint32_t)gfs->buffer_md);
  req.sem   = NULL;
  r = gfs->disk->write_block(gfs->disk, &req);
  if(r == 0) {
    /* An error occured. */
    semaphore_V(gfs->lock);
    return VFS_ERROR;
  }

  semaphore_V(gfs->lock);
  return VFS_OK;
}


/**
 * Reads at most bufsize bytes from file to the buffer starting from
 * the offset. bufsize bytes is always read if possible. Returns
 * number of bytes read. Buffer size must be atleast bufsize.
 * Implements fs.read().
 *
 * @param fs  Pointer to fs data structure of the device.
 * @param fileid Fileid of the file.
 * @param buffer Pointer to the buffer the data is read into.
 * @param bufsize Maximum number of bytes to be read.
 * @param offset Start position of reading.
 *
 * @return Number of bytes read into buffer, or VFS_ERROR if error
 * occured.
 */
int gfs_read(fs_t *fs, int fileid, void *buffer, int bufsize, int offset)
{
  gfs_t *gfs = (gfs_t *)fs->internal;
  gbd_request_t req;
  int b1, b2;
  int read=0;
  int r;

  semaphore_P(gfs->lock);

  /* fileid is blocknum so ensure that we don't read system blocks
     or outside the disk */
  if(fileid < 2 || fileid > (int)gfs->totalblocks) {
    semaphore_V(gfs->lock);
    return VFS_ERROR;
  }

  req.block = fileid;
  req.buf   = ADDR_KERNEL_TO_PHYS((uint32_t)gfs->buffer_inode);
  req.sem   = NULL;
  r = gfs->disk->read_block(gfs->disk, &req);
  if(r == 0) {
    /* An error occured. */
    semaphore_V(gfs->lock);
    return VFS_ERROR;
  }

  /* Check that offset is inside the file */
  if(offset < 0 || offset > (int)gfs->buffer_inode->filesize) {
    semaphore_V(gfs->lock);
    return VFS_ERROR;
  }

  /* Read at most what is left from the file. */
  bufsize = MIN(bufsize,((int)gfs->buffer_inode->filesize) - offset);

  if(bufsize==0) {
    semaphore_V(gfs->lock);
    return 0;
  }

  /* first block to be read from the disk */
  b1 = offset / GFS_BLOCK_SIZE;

  /* last block to be read from the disk */
  b2 = (offset+bufsize-1) / GFS_BLOCK_SIZE;

  /* Read blocks from b1 to b2. First and last are
     special cases because whole block might not be written
       to the buffer. */
  req.block = gfs->buffer_inode->block[b1];
  req.buf   = ADDR_KERNEL_TO_PHYS((uint32_t)gfs->buffer_bat);
  req.sem   = NULL;
  r = gfs->disk->read_block(gfs->disk, &req);
  if(r == 0) {
    /* An error occured. */
    semaphore_V(gfs->lock);
    return VFS_ERROR;
  }

  /* Count the number of the bytes to be read from the block and
     written to the buffer from the first block. */
  read = MIN(GFS_BLOCK_SIZE - (offset % GFS_BLOCK_SIZE),bufsize);
  memcopy(read,
      buffer,
      (const uint32_t *)(((uint32_t)gfs->buffer_bat) +
         (offset % GFS_BLOCK_SIZE)));

  buffer = (void *)((uint32_t)buffer + read);
  b1++;
  while(b1 <= b2) {
    req.block = gfs->buffer_inode->block[b1];
    req.buf   = ADDR_KERNEL_TO_PHYS((uint32_t)gfs->buffer_bat);
    req.sem   = NULL;
    r = gfs->disk->read_block(gfs->disk, &req);
    if(r == 0) {
      /* An error occured. */
      semaphore_V(gfs->lock);
      return VFS_ERROR;
    }

    if(b1 == b2) {
      /* Last block. Whole block might not be read.*/
      memcopy(bufsize - read,
        buffer,
        (const uint32_t *)gfs->buffer_bat);
      read += (bufsize - read);
    } else {
      /* Read whole block */
      memcopy(GFS_BLOCK_SIZE,
        buffer,
        (const uint32_t *)gfs->buffer_bat);
      read += GFS_BLOCK_SIZE;
      buffer = (void *)((uint32_t)buffer + GFS_BLOCK_SIZE);
    }
    b1++;
  }

  semaphore_V(gfs->lock);
  return read;
}

/**
 * Write at most datasize bytes from buffer to the file starting from
 * the offset. datasize bytes is always written if possible. Returns
 * number of bytes written. Buffer size must be atleast datasize.
 * Implements fs.read().
 *
 * @param fs  Pointer to fs data structure of the device.
 * @param fileid Fileid of the file.
 * @param buffer Pointer to the buffer the data is written from.
 * @param datasize Maximum number of bytes to be written.
 * @param offset Start position of writing.
 *
 * @return Number of bytes written into buffer, or VFS_ERROR if error
 * occured.
 */
int gfs_write(fs_t *fs, int fileid, void *buffer, int datasize, int offset)
{
  gfs_t *gfs = (gfs_t *)fs->internal;
  gbd_request_t req;
  int b1, b2;
  int written=0;
  int r;

  semaphore_P(gfs->lock);

  /* fileid is blocknum so ensure that we don't read system blocks
     or outside the disk */
  if(fileid < 2 || fileid > (int)gfs->totalblocks) {
    semaphore_V(gfs->lock);
    return VFS_ERROR;
  }

  req.block = fileid;
  req.buf   = ADDR_KERNEL_TO_PHYS((uint32_t)gfs->buffer_inode);
  req.sem   = NULL;
  r = gfs->disk->read_block(gfs->disk, &req);
  if(r == 0) {
    /* An error occured. */
    semaphore_V(gfs->lock);
    return VFS_ERROR;
  }

  /* check that start position is inside the disk */
  if (offset < 0 || offset > (int)gfs->buffer_inode->filesize) {
    semaphore_V(gfs->lock);
    return VFS_ERROR;
  }

  if (datasize + offset > (int)gfs->buffer_inode->filesize) {
    gfs->buffer_inode->filesize = datasize + offset;
    r = gfs->disk->write_block(gfs->disk, &req);
    if(r==0) {
      /* An error occured. */
      semaphore_V(gfs->lock);
      return VFS_ERROR;
    }
  }


  req.block = GFS_ALLOCATION_BLOCK;
  req.buf = ADDR_KERNEL_TO_PHYS((uint32_t)gfs->buffer_bat);
  req.sem = NULL;
  r = gfs->disk->read_block(gfs->disk, &req);
  if(r == 0) {
    /* An error occured. */
    semaphore_V(gfs->lock);
    return VFS_ERROR;
  }

  uint32_t numblocks = (datasize + offset + GFS_BLOCK_SIZE - 1)/GFS_BLOCK_SIZE;

  uint32_t i;
  for(i=0; i<numblocks; i++) {
    if (gfs->buffer_inode->block[i] == 0)
    {
      gfs->buffer_inode->block[i] = bitmap_findnset(gfs->buffer_bat,
                                    gfs->totalblocks);
      if((int)gfs->buffer_inode->block[i] == -1) {
        /* Disk full. No free block found. */
        semaphore_V(gfs->lock);
        return VFS_ERROR;
      }
    }
  }

  req.block = fileid;
  req.buf   = ADDR_KERNEL_TO_PHYS((uint32_t)gfs->buffer_inode);
  req.sem   = NULL;

  r = gfs->disk->write_block(gfs->disk, &req);
  if(r==0) {
    /* An error occured. */
    semaphore_V(gfs->lock);
    return VFS_ERROR;
  }

  if(datasize==0) {
    semaphore_V(gfs->lock);
    return 0;
  }

  /* first block to be written into */
  b1 = offset / GFS_BLOCK_SIZE;

  /* last block to be written into */
  b2 = (offset+datasize-1) / GFS_BLOCK_SIZE;
  // b2 = numblocks;

  /* Write data to blocks from b1 to b2. First and last are special
     cases because whole block might not be written. Because of possible
     partial write, first and last block must be read before writing.

     If we write less than block size or start writing in the middle
     of the block, read the block firts. Buffer for allocation block
     is used because it is not needed (for allocation block) in this
     function. */
  written = MIN(GFS_BLOCK_SIZE - (offset % GFS_BLOCK_SIZE),datasize);

  if(written < GFS_BLOCK_SIZE) {
    req.block = gfs->buffer_inode->block[b1];
    req.buf   = ADDR_KERNEL_TO_PHYS((uint32_t)gfs->buffer_bat);
    req.sem   = NULL;
    r = gfs->disk->read_block(gfs->disk, &req);
    if(r == 0) {
      /* An error occured. */
      semaphore_V(gfs->lock);
      return VFS_ERROR;
    }
  }

  memcopy(written,
    (uint32_t *)(((uint32_t)gfs->buffer_bat) +
       (offset % GFS_BLOCK_SIZE)),
    buffer);

  req.block = gfs->buffer_inode->block[b1];
  req.buf   = ADDR_KERNEL_TO_PHYS((uint32_t)gfs->buffer_bat);
  req.sem   = NULL;
  r = gfs->disk->write_block(gfs->disk, &req);
  if(r == 0) {
    /* An error occured. */
    semaphore_V(gfs->lock);
    return VFS_ERROR;
  }

  buffer = (void *)((uint32_t)buffer + written);
  b1++;
  while(b1 <= b2) {

    if(b1 == b2) {
      /* Last block. If partial write, read the block first.
         Write anyway always to the beginning of the block */
      if((datasize - written) < GFS_BLOCK_SIZE) {
        req.block = gfs->buffer_inode->block[b1];
        req.buf   = ADDR_KERNEL_TO_PHYS((uint32_t)gfs->buffer_bat);
        req.sem   = NULL;
        r = gfs->disk->read_block(gfs->disk, &req);
        if(r == 0) {
          /* An error occured. */
          semaphore_V(gfs->lock);
          return VFS_ERROR;
        }
      }

      memcopy(datasize - written,
        (uint32_t *)gfs->buffer_bat,
          buffer);
      written = datasize;
    } else {
      /* Write whole block */
      memcopy(GFS_BLOCK_SIZE,
        (uint32_t *)gfs->buffer_bat,
        buffer);
      written += GFS_BLOCK_SIZE;
      buffer = (void *)((uint32_t)buffer + GFS_BLOCK_SIZE);
    }

    req.block = gfs->buffer_inode->block[b1];
    req.buf   = ADDR_KERNEL_TO_PHYS((uint32_t)gfs->buffer_bat);
    req.sem   = NULL;
    r = gfs->disk->write_block(gfs->disk, &req);
    if(r == 0) {
      /* An error occured. */
      semaphore_V(gfs->lock);
      return VFS_ERROR;
    }

    b1++;
  }

  semaphore_V(gfs->lock);
  return written;
}

/**
 * Get number of free bytes on the disk. Implements fs.getfree().
 * Reads allocation blocks and counts number of zeros in the bitmap.
 * Result is multiplied by the block size and returned.
 *
 * @param fs Pointer to the fs data structure of the device.
 *
 * @return Number of free bytes.
 */
int gfs_getfree(fs_t *fs)
{
  gfs_t *gfs = (gfs_t *)fs->internal;
  gbd_request_t req;
  int allocated = 0;
  uint32_t i;
  int r;

  semaphore_P(gfs->lock);

  req.block = GFS_ALLOCATION_BLOCK;
  req.buf = ADDR_KERNEL_TO_PHYS((uint32_t)gfs->buffer_bat);
  req.sem = NULL;
  r = gfs->disk->read_block(gfs->disk, &req);
  if(r == 0) {
    /* An error occured. */
    semaphore_V(gfs->lock);
    return VFS_ERROR;
  }

  for(i=0;i<gfs->totalblocks;i++) {
    allocated += bitmap_get(gfs->buffer_bat,i);
  }

  semaphore_V(gfs->lock);
  return (gfs->totalblocks - allocated)*GFS_BLOCK_SIZE;
}

/** @} */
