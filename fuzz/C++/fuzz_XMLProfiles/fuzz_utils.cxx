#include "fuzz_utils.h"

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

extern "C" int ignore_stdout(
        void)
{
    int fd = open("/dev/null", O_WRONLY);
    if (fd == -1)
    {
        warn("open(\"/dev/null\") failed");
        return -1;
    }

    int ret = 0;
    if (dup2(fd, STDOUT_FILENO) == -1)
    {
        warn("failed to redirect stdout to /dev/null\n");
        ret = -1;
    }

    if (close(fd) == -1)
    {
        warn("close");
        ret = -1;
    }

    return ret;
}
