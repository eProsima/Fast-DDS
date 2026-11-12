#!/usr/bin/env python3
# Copyright 2026 Fast DDS contributors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""Run remote type discovery in two isolated, concurrent processes."""

import os
import subprocess
import sys
import time


def main():
    """Run independent discovery roles and reject failures in either child process."""
    executable = sys.argv[1]
    # Keep UDP RTPS ports in range and avoid a fixed domain across concurrent tests.
    domain = str(100 + os.getpid() % 40)
    discover = subprocess.Popen([executable, "discover", domain])
    announce = None
    try:
        time.sleep(1)
        announce = subprocess.Popen([executable, "announce", domain])
        announce_status = announce.wait(timeout=60)
        discover_status = discover.wait(timeout=60)
        return 0 if announce_status == 0 and discover_status == 0 else 1
    finally:
        for process in (announce, discover):
            if process is not None and process.poll() is None:
                process.terminate()
                try:
                    process.wait(timeout=5)
                except subprocess.TimeoutExpired:
                    process.kill()
                    process.wait()


if __name__ == "__main__":
    sys.exit(main())
