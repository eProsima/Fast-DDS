# Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
"""
Script to test that Discovery Server with security is properly disabled.
The test expects the discovery server to fail when security is enabled.
"""

import argparse
import os
import subprocess
import sys
import time


class ParseOptions():
    """Parse arguments."""

    def __init__(self):
        """Object constructor."""
        self.args = self.__parse_args()

    def __parse_args(self):
        """
        Parse the input arguments.

        :return: A dictionary containing the arguments parsed.
        """
        parser = argparse.ArgumentParser(
            formatter_class=argparse.ArgumentDefaultsHelpFormatter,
            add_help=True,
            description=(
                'Script to test that Discovery Server with security is disabled.'),
        )
        parser.add_argument(
            '-ds',
            '--ds-server',
            required=True,
            type=str,
            help='Path to the discovery server executable.'
        )
        parser.add_argument(
            '-DS',
            '--xml-ds',
            required=True,
            type=str,
            help='Path to the xml configuration file with security enabled.'
        )
        parser.add_argument(
            '-i',
            '--server-id',
            type=int,
            default=0,
            help='Discovery server identifier.'
        )
        parser.add_argument(
            '-t',
            '--timeout',
            type=int,
            default=5,
            help='Maximum time to wait for server to fail (seconds).'
        )

        return parser.parse_args()


def run(args):
    """
    Run the discovery server with security and expect it to fail.

    :param args: The input parameters.

    :return: Exit code (0 if test passed, 1 if test failed).
    """
    script_dir = os.path.dirname(os.path.realpath(__file__))

    # Validate discovery server executable
    if not os.path.isfile(args.ds_server):
        print(f'ERROR: Discovery server executable does not exist: {args.ds_server}')
        return 1

    if not os.access(args.ds_server, os.X_OK):
        print(f'ERROR: Discovery server executable lacks execution permissions: {args.ds_server}')
        return 1

    # Validate XML file
    xml_file_ds = os.path.join(script_dir, args.xml_ds)
    if not os.path.isfile(xml_file_ds):
        print(f'ERROR: XML configuration file does not exist: {xml_file_ds}')
        return 1

    # Build discovery server command
    ds_command = [
        args.ds_server,
        '42',
        '--xml-file', xml_file_ds,
        '--server-id', str(args.server_id)
    ]

    print('Testing that Discovery Server with security is disabled...')
    print(f'Command: {" ".join(ds_command)}')

    # Start discovery server
    ds_proc = subprocess.Popen(
        ds_command,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE
    )

    # Wait for the process to exit or timeout
    try:
        stdout, stderr = ds_proc.communicate(timeout=args.timeout)
        return_code = ds_proc.returncode
        
        # Decode output
        stdout_str = stdout.decode('utf-8', errors='replace') if stdout else ''
        stderr_str = stderr.decode('utf-8', errors='replace') if stderr else ''
        combined_output = stdout_str + stderr_str

        # Check if server failed as expected
        if return_code != 0:
            # Check for expected security-related error message
            security_keywords = [
                'security',
                'disabled',
                'pro',
                'not available',
                'not supported',
                'discovery server with security',
                'fast dds pro'
            ]

            found_security_error = any(
                keyword in combined_output.lower() 
                for keyword in security_keywords
            )
            
            if found_security_error:
                print('Discovery Server failed with security-related error as expected')
                print(f'  Error message: {combined_output.strip()[:300]}...')
                return 0
            else:
                print('FAIL: Discovery Server failed but not due to security restriction')
                print(f'  Return code: {return_code}')
                print(f'  stdout: {stdout_str}')
                print(f'  stderr: {stderr_str}')
                print('  Expected error message about security being disabled/not available')
                return 1
        else:
            print('FAIL: Discovery Server started successfully when it should have failed')
            print(f'  stdout: {stdout_str}')
            print(f'  stderr: {stderr_str}')
            
            # Kill the server if it's still running
            if ds_proc.poll() is None:
                ds_proc.kill()
                ds_proc.wait()

            return 1
            
    except subprocess.TimeoutExpired:
        print('FAIL: Discovery Server did not exit within timeout period')
        print('  This suggests the server started successfully when it should have failed')
        
        # Kill the server
        ds_proc.kill()
        try:
            stdout, stderr = ds_proc.communicate(timeout=2)
            stdout_str = stdout.decode('utf-8', errors='replace') if stdout else ''
            stderr_str = stderr.decode('utf-8', errors='replace') if stderr else ''
            print(f'  stdout: {stdout_str}')
            print(f'  stderr: {stderr_str}')
        except:
            pass

        return 1


if __name__ == '__main__':
    # Parse arguments
    args = ParseOptions()

    # Run test
    exit_code = run(args.args)
    
    sys.exit(exit_code)
