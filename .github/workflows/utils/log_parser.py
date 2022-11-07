# Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

"""Script to parse the colcon test output file."""

import re
import os

from tomark import Tomark

# Python summary file saved in GITHUB_STEP_SUMMARY environment variable
SUMMARY_FILE = os.getenv('GITHUB_STEP_SUMMARY')
LOG_FILE = 'log/latest_test/fastrtps/stdout_stderr.log'

# Save the lines with failed tests
saved_lines = []
with open(LOG_FILE, 'r') as file:
    for line in reversed(file.readlines()):
        saved_lines.append(line)
        if (re.search('.*The following tests FAILED:.*', line)):
            break

# Exit if no test failed
if (not saved_lines):
    exit(0)

failed_tests = []
for test in saved_lines:
    if (re.search(r'\d* - .* \(.+\)', test)):
        split_test = test.split()
        failed_tests.insert(
            0,
            dict({
                'ID': split_test[0],
                'Name': split_test[2],
                'Type': test[test.find('(')+1:test.find(')')]
            }))

# Convert python dict to markdown table
md_table = Tomark.table(failed_tests)
print(md_table)

# Save table of failed test to GitHub action summary file
with open(SUMMARY_FILE, 'a') as file:
    file.write(f'\n{md_table}')
