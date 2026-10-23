// Compiles the tinyply implementation for the tests in a translation unit of
// its own, so that tests.cpp includes the headers as a regular client would.
#define TINYPLY_AS_LIBRARY

#include "tinyply.h"
