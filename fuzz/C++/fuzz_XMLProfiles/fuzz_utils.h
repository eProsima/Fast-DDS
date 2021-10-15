// Helpers functions for fuzz targets.

#ifndef FUZZ_UTILS_H_
#define FUZZ_UTILS_H_

#include <stddef.h>
#include <stdint.h>

// Redirect stdout to /dev/null. Useful to ignore output from verbose fuzz
// target functions.
//
// Return 0 on success, -1 otherwise.
extern "C" int ignore_stdout(void);

// Delete the file passed as argument and free the associated buffer. This
// function is meant to be called on buf_to_file return value.
//
// Return 0 on success, -1 otherwise.
extern "C" int delete_file(const char *pathname);

// Write the data provided in buf to a new temporary file. This function is
// meant to be called by LLVMFuzzerTestOneInput() for fuzz targets that only
// take file names (and not data) as input.
//
// Return the path of the newly created file or NULL on error. The caller should
// eventually free the returned buffer (see delete_file).
extern "C" char *buf_to_file(const uint8_t *buf, size_t size);

#endif  // FUZZ_UTILS_H_