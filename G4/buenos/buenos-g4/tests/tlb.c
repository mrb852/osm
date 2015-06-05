#include "tests/lib.h"

// A small program that triggers the TLB Store exception
int main() {

  int MAX = 100;
  void* arr[MAX];
  int i = 0;

  for (; i < MAX; ++i) {
    arr[i] = malloc(200);

    // This is needed. Otherwise the compiler will be too clever and filter out the arr
    // for being unused
    printf("index: %d, %p\n", i, &(arr[i]));
  }

  printf("\nWILL FAIL NOW\n\n\n");
  arr[MAX] = malloc(200);

  return 0;
}
