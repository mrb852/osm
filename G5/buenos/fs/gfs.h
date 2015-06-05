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

#ifndef FS_GFS_H
#define FS_GFS_H

#include "drivers/gbd.h"
#include "fs/vfs.h"
#include "lib/libc.h"
#include "lib/bitmap.h"

/* In GFS block size is 512. This will affect to various other
   features of GFS e.g. maximum file size. */
#define GFS_BLOCK_SIZE 512

/* Magic number found on each gfs filesystem's header block. */
#define GFS_MAGIC 0xD00F

/* Block numbers for system blocks */
#define GFS_HEADER_BLOCK      0
#define GFS_ALLOCATION_BLOCK  1
#define GFS_DIRECTORY_BLOCK   2

/* Names are limited to 16 characters */
#define GFS_VOLUMENAME_MAX    16
#define GFS_FILENAME_MAX      16

/*
   Maximum number of block pointers in one inode. Block pointers
   are of type uint32_t and one pointer "slot" is reserved for
   file size.
*/
#define GFS_BLOCKS_MAX ((GFS_BLOCK_SIZE/sizeof(uint32_t))-1)

/* Maximum file size. 512-byte Inode can store 127 blocks for a file.
   512*127=65024 */
#define GFS_MAX_FILESIZE (GFS_BLOCK_SIZE*GFS_BLOCKS_MAX)

/* File inode block. Inode contains the filesize and a table of blocknumbers
   allocated for the file. In GFS files can't have more blocks than fits in
   block table of the inode block.

   One 512 byte block can hold 128 32-bit integers. Therefore the table
   size is limited to 127 and filesize to 127*512=65024.
*/
typedef struct {
  /* filesize in bytes */
  uint32_t filesize;

  /* block numbers allocated for this file, zero
     means unused block. */
  uint32_t block[GFS_BLOCKS_MAX];
} gfs_inode_t;


/* Master directory block entry. If inode is zero, entry is
   unused (free). */
typedef struct {
  /* File's inode block number. */
  uint32_t inode;

  /* File name */
  char     name[GFS_FILENAME_MAX];
} gfs_direntry_t;

#define GFS_MAX_FILES (GFS_BLOCK_SIZE/sizeof(gfs_direntry_t))

/* functions */
fs_t* gfs_init(gbd_t *disk);

int gfs_unmount(fs_t *fs);
int gfs_open(fs_t *fs, char *filename);
int gfs_close(fs_t *fs, int fileid);
int gfs_create(fs_t *fs, char *filename, int size);
int gfs_remove(fs_t *fs, char *filename);
int gfs_read(fs_t *fs, int fileid, void *buffer, int bufsize, int offset);
int gfs_write(fs_t *fs, int fileid, void *buffer, int datasize, int offset);
int gfs_getfree(fs_t *fs);

#endif    /* FS_GFS_H */
