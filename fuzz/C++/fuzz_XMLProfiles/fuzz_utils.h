// Helpers functions for fuzz targets.

#ifndef FUZZ_UTILS_H_
#define FUZZ_UTILS_H_

#include <stddef.h>
#include <stdint.h>

// Redirect stdout to /dev/null. Useful to ignore output from verbose fuzz
// target functions.
//
// Return 0 on success, -1 otherwise.
extern "C" int ignore_stdout(
        void);


#endif  // FUZZ_UTILS_H_