#include "kernel/read.h"
#include "drivers/device.h"
#include "drivers/gcd.h"
#include "kernel/panic.h"
#include "lib/libc.h"
#include "kernel/assert.h"

int syscall_read(int filehandle, void *buffer, int length) {

  filehandle = filehandle;

  device_t *dev;
  gcd_t *gcd;

  /* Find system console (first tty) */
  dev = device_get(YAMS_TYPECODE_TTY, 0);
  KERNEL_ASSERT(dev != NULL);

  gcd = (gcd_t *)dev->generic_device;
  KERNEL_ASSERT(gcd != NULL);

  char b[length];

  int counter = 0;
  while(counter < length) {
    char one[1];
    counter += gcd->read(gcd, one, 1);

    if (one[0] == '\r') {
      counter--;
      break;
    }
    b[counter - 1] = one[0];
  }

  b[counter] = '\0';
  snprintf(buffer, length, b);

  return counter;
}
