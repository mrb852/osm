#include "kernel/write.h"
#include "drivers/device.h"
#include "drivers/gcd.h"
#include "kernel/panic.h"
#include "lib/libc.h"
#include "kernel/assert.h"

int syscall_write(int filehandle, void *buffer, int length) {

  filehandle = filehandle;

  device_t *dev;
  gcd_t *gcd;


  /* Find system console (first tty) */
  dev = device_get(YAMS_TYPECODE_TTY, 0);
  KERNEL_ASSERT(dev != NULL);

  gcd = (gcd_t *)dev->generic_device;
  KERNEL_ASSERT(gcd != NULL);

  int lengthWritten = gcd->write(gcd, buffer, length);
  lengthWritten += gcd->write(gcd, (void *)"\n", 1);

  return lengthWritten +  1;
}
