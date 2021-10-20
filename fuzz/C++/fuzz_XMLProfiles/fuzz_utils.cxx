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

extern "C" int delete_file(
        const char* pathname)
{
    int ret = unlink(pathname);
    if (ret == -1)
    {
        warn("failed to delete \"%s\"", pathname);
    }

    free((void*)pathname);

    return ret;
}

extern "C" char* buf_to_file(
        const uint8_t* buf,
        size_t size)
{
    char* pathname = strdup("/dev/shm/fuzz-XXXXXX");
    if (pathname == NULL)
    {
        return NULL;
    }

    int fd = mkstemp(pathname);
    if (fd == -1)
    {
        warn("mkstemp(\"%s\")", pathname);
        free(pathname);
        return NULL;
    }

    size_t pos = 0;
    while (pos < size)
    {
        int nbytes = write(fd, &buf[pos], size - pos);
        if (nbytes <= 0)
        {
            if (nbytes == -1 && errno == EINTR)
            {
                continue;
            }
            warn("write");
            goto err;
        }
        pos += nbytes;
    }

    if (close(fd) == -1)
    {
        warn("close");
        goto err;
    }

    return pathname;

err:
    delete_file(pathname);
    return NULL;
}