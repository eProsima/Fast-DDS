// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "fuzz_utils.hpp"

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
