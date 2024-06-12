# Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
Script to analyze and publish flaky tests from JUnit test history files.
"""
import argparse
import os
import re
from datetime import datetime
from decimal import getcontext, Decimal, ROUND_UP
from typing import Dict

from junitparser import TestSuite
import pandas as pd
import xml.etree.ElementTree as ET


class FlakyTestsPublisher:
    """
    Class to analyze and publish flaky tests from JUnit test history files.
    """
    # Regex pattern to match the timestamp part of the filename
    _timestamp_rstr = r'\d{4}-\d{2}-\d{2}T\d{2}-\d{2}-\d{2}'

    def __init__(self, junit_files_dir: str, window_size: int, delete_old_files: bool = True):
        """
        Args:
            junit_files_dir (str): Path to the directory containing the JUnit test history files.
            window_size (int): Number of test runs to consider for the fliprate calculation.
            delete_old_files (bool): Whether to delete old files taking window_size into account.
        """
        self._flip_rates = {}
        self._window_size = window_size
        self._junit_files = FlakyTestsPublisher._find_test_reports(junit_files_dir)

        if delete_old_files:
            self._delete_old_files()

    def analyze(self) -> int:
        """
        Analyze the test history files and calculate the fliprate for each test.

        Returns:
            int: Number of flaky tests found.
        """
        df = FlakyTestsPublisher._get_test_results(self._junit_files)
        fliprate_table = FlakyTestsPublisher._calculate_fliprate_table(df, self._window_size)
        self._flip_rates = FlakyTestsPublisher._get_top_fliprates(fliprate_table, 0, 4)
        return len(self._flip_rates)

    def publish_summary(self, output_file: str):
        """
        Publish the summary of the flaky tests found to a markdown file.

        Args:
            output_file (str): Path to the output file.
        """
        # Test summary
        summary = '## Flaky tests\n'

        # Table header
        # TODO(eduponz): Add a column for the failures/runs ratio
        summary += '|#|Flaky tests|Fliprate score %|Consecutive failures|Consecutive passes|Total failures|\n'
        summary += '|-|-|-|-|-|\n'

        i = 1
        for test_name, (flip_rate, consecutive_failures, consecutive_passes, failures) in self._flip_rates.items():
            summary += f'| {i} | {test_name} | {round(flip_rate, 2)*100} | {consecutive_failures} | {consecutive_passes} | {failures} |\n'
            i += 1

        with open(output_file, 'w') as file:
            file.write(summary)

    @staticmethod
    def _find_test_reports(directory):
        """
        Find all JUnit test history files in the given directory.

        Args:
            directory (str): Path to the directory containing the JUnit test history files.

        Returns:
            list: List of paths to the JUnit test history files.
        """
        pattern = re.compile(r'^test_report.*' + FlakyTestsPublisher._timestamp_rstr + r'\.xml$')

        files = os.listdir(directory)
        matched_files = [
            os.path.join(directory, f) for f in files if pattern.match(f)
        ]
        matched_files.sort(key=FlakyTestsPublisher._extract_timestamp)

        return matched_files

    @staticmethod
    def _extract_timestamp(file_path):
        """
        Extract the timestamp from the filename.

        Args:
            file_path (str): Path to the file.

        Returns:
            datetime: Timestamp extracted from the filename.
        """
        filename = os.path.basename(file_path)
        timestamp_str = re.search(FlakyTestsPublisher._timestamp_rstr, filename).group()
        return datetime.strptime(timestamp_str, '%Y-%m-%dT%H-%M-%S')

    def _delete_old_files(self):
        """
        Delete old files taking window_size into account.
        """
        if len(self._junit_files) > self._window_size:
            # Calculate the number of files to delete
            files_to_delete = self._junit_files[:-self._window_size]

            # Update the list of kept files
            self._junit_files = self._junit_files[-self._window_size:]

            # Delete the older files
            for file in files_to_delete:
                print(f'Deleting old file: {file}')
                os.remove(file)

    @staticmethod
    def _get_test_results(junit_files: list):
        """
        Parse JUnit test history files and return a DataFrame with the test results.

        Args:
            junit_files (list): List of paths to the JUnit test history files.

        Returns:
            pd.DataFrame: DataFrame with the test results.

        Raises:
            RuntimeError: If no valid JUnit files are found.
        """
        dataframe_entries = []

        for junit_file in junit_files:
            xml = ET.parse(junit_file)
            root = xml.getroot()
            for suite in root.findall('.//testsuite'):
                dataframe_entries += FlakyTestsPublisher._junit_suite_to_df(suite)

        if dataframe_entries:
            df = pd.DataFrame(dataframe_entries)
            df["timestamp"] = pd.to_datetime(df["timestamp"])
            df = df.set_index("timestamp")
            return df.sort_index()
        else:
            raise RuntimeError(f"No Junit files found in {junit_files}")

    @staticmethod
    def _junit_suite_to_df(suite: TestSuite) -> list:
        """
        Convert a JUnit test suite to a list of DataFrame entries.

        Args:
            suite (TestSuite): JUnit test suite.

        Returns:
            list: List of DataFrame entries.
        """
        dataframe_entries = []
        time = suite.attrib.get('timestamp')

        for testcase in suite.findall('.//testcase'):
            test_identifier = testcase.attrib.get('name')

            status = testcase.attrib.get('status')

            # Convert status to "pass" if it's "passed"
            if status == "passed":
                test_status = "pass"
            else:
                test_status = status
                if test_status == "skipped":
                    continue

            dataframe_entries.append(
                {
                    "timestamp": time,
                    "test_identifier": test_identifier,
                    "test_status": test_status,
                }
            )
        return dataframe_entries

    @staticmethod
    def _calculate_fliprate_table(testrun_table: pd.DataFrame, window_size: int) -> pd.DataFrame:
        """
        Calculate the fliprate for each test in the testrun_table.

        Args:
            testrun_table (pd.DataFrame): DataFrame with the test results.
            window_size (int): Number of test runs to consider for the fliprate calculation.

        Returns:
            pd.DataFrame: DataFrame with the fliprates for each test.
        """
        # Apply non_overlapping_window_fliprate to each group in testrun_table
        fliprates = testrun_table.groupby("test_identifier")["test_status"].apply(
            lambda x: FlakyTestsPublisher._non_overlapping_window_fliprate(x, window_size)
        )

        # Convert fliprates Series of DataFrames to a DataFrame
        fliprate_table = fliprates.reset_index()

        # Rename the index level to "window"
        fliprate_table = fliprate_table.rename(columns={"level_1": "window"})

        # Filter out rows where flip_rate is zero
        # TODO(eduponz): I'm seeing tests with 5 consecutive failures that are not showing here.
        #                Seems it's because they are not flaky, they are just always failing.
        fliprate_table = fliprate_table[fliprate_table.flip_rate != 0]

        return fliprate_table

    @staticmethod
    def _non_overlapping_window_fliprate(testruns: pd.Series, window_size: int) -> pd.Series:
        """
        Calculate the fliprate for a test in a non-overlapping window.

        Args:
            testruns (pd.Series): Series with the test results.
            window_size (int): Number of test runs to consider for the fliprate calculation.

        Returns:
            pd.Series: Series with the fliprate for the test.
        """
        # Apply _calc_fliprate directly to the last <window_size> selected rows
        testruns_last = testruns.iloc[-window_size:]
        fliprate_groups = FlakyTestsPublisher._calc_fliprate(testruns_last)

        return fliprate_groups.reset_index(drop=True)

    @staticmethod
    def _calc_fliprate(testruns: pd.Series) -> pd.DataFrame:
        """
        Calculate the fliprate for a test.

        Args:
            testruns (pd.Series): Series with the test results.

        Returns:
            pd.DataFrame: DataFrame with the fliprate, consecutive failures
                          and consecutive passes for the test.
        """
        if len(testruns) < 2:
            return pd.DataFrame(
                {
                    'flip_rate': [0.0],
                    'consecutive_failures': [0],
                    'consecutive_passes': [0],
                    'failures': [0],
                }
            )

        first = True
        previous = None
        flips = 0
        consecutive_failures = 0
        consecutive_passes = 0
        failures = 0
        possible_flips = len(testruns) - 1

        for _, val in testruns.items():
            if first:
                first = False
                previous = val
                continue

            if val != previous:
                flips += 1

            if val != "pass":
                consecutive_failures += 1
                consecutive_passes = 0
                failures += 1
            else:
                consecutive_failures = 0
                consecutive_passes += 1

            previous = val

        flip_rate = flips / possible_flips

        return pd.DataFrame(
            {
                'flip_rate': [flip_rate],
                'consecutive_failures': [consecutive_failures],
                'consecutive_passes': [consecutive_passes],
                'failures': [failures],
            }
        )

    @staticmethod
    def _get_top_fliprates(fliprate_table: pd.DataFrame, top_n: int, precision: int) -> Dict[str, tuple[Decimal, int, int, int]]:
        """
        Get the top N fliprates from the fliprate_table.

        Args:
            fliprate_table (pd.DataFrame): DataFrame with the fliprates.
            top_n (int): Number of top fliprates to get.
            precision (int): Number of decimal places to round the fliprates.

        Returns:
            Dict[str, tuple[Decimal, int, int, int]]: Dictionary with the top fliprates
                and their consecutive and total failures.
        """
        context = getcontext()
        context.prec = precision
        context.rounding = ROUND_UP
        last_window_values = fliprate_table.groupby("test_identifier").last()

        if top_n != 0:
            top_fliprates = last_window_values.nlargest(top_n, "flip_rate")
        else :
            top_fliprates = last_window_values.nlargest(len(last_window_values), "flip_rate")

        # Create a dictionary with test_identifier as keys and a tuple of flip_rate and consecutive_failures as values
        results = {}
        for test_id, row in top_fliprates.iterrows():
            results[test_id] = (
                Decimal(row["flip_rate"]),
                int(row["consecutive_failures"]),
                int(row["consecutive_passes"]),
                int(row["failures"])
            )

        return results


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument(
        "--junit-files",
        help="Path for a folder with JUnit xml test history files",
        type=str
    )
    parser.add_argument(
        "--window-size",
        type=int,
        help="flip rate calculation window size",
        required=True,
    )
    parser.add_argument(
        '-d',
        '--delete-old-files',
        action='store_true',
        help='Delete old files taking window size into account.'
    )
    parser.add_argument(
        '-o',
        '--output-file',
        type=str,
        required=False,
        help='Path to output file.'
    )
    args = parser.parse_args()

    publisher = FlakyTestsPublisher(args.junit_files, args.window_size, args.delete_old_files)
    flaky_test_count = publisher.analyze()

    if args.output_file:
        publisher.publish_summary(args.output_file)

    ret = 0 if flaky_test_count == 0 else 1
    exit(ret)
