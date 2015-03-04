/*
	Source: Arne Andersson and Mikkel Thorup, A pragmatic implementation
	of monotone priority queues, Worldwide Web Document (1997)

	Warning: This method is not portable; in some machines the order of
	bits in the mask should be the opposite.
	
	Author: Jyrki Katajainen © 1998, 2015
*/

#include <limits.h>

struct mask {
  unsigned long int significant : 52;
	unsigned int exponent : 11; 
	unsigned int sign : 1;
};

union doubleandmask { 
	double as_double; 
	struct mask as_mask; 
};

/* Takes a 32-bit word and finds the leftmost 1-bit */

int leftmost_one(unsigned int n) {
	if (n == 0) {
		return INT_MIN;
	}
	union doubleandmask fm; 
	fm.as_double = (double) n; 
	return (fm.as_mask.exponent - 1023); 
}

#ifdef UNITTEST_LEFTMOST_ONE

#include <assert.h> // assert macro
#include <math.h> // ilogb
#include <stdio.h> // printf

int main() {
	unsigned int const bound = 10;//00000;
	for (unsigned int i = 0; i != bound; ++i) {
		assert(ilogb((double) i) == leftmost_one(i));
	}
}

#endif
