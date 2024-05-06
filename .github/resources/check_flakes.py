import argparse
import logging
from decimal import getcontext, Decimal, ROUND_UP
from pathlib import Path
from typing import Dict, Set

from junitparser import JUnitXml, TestSuite
import pandas as pd
import numpy as np
import seaborn as sns
import xml.etree.ElementTree as ET

EWM_ALPHA = 1
EWM_ADJUST = False
HEATMAP_FIGSIZE = (100, 50)
test_counts = {}

def parse_input_files(junit_files: str, test_history_csv: str):
    if junit_files:
        df = parse_junit_to_df(Path(junit_files))
    else:
        df = pd.read_csv(
            test_history_csv,
            index_col="timestamp",
            parse_dates=["timestamp"],
        )
    return df.sort_index()


def calc_fliprate(testruns: pd.Series) -> pd.DataFrame:
    """Calculate test result fliprate from given test results series"""
    if len(testruns) < 2:
        return pd.DataFrame({'flip_rate': [0.0], 'consecutive_failures': [0]})

    first = True
    previous = None
    flips = 0
    consecutive_failures = 0
    possible_flips = len(testruns) - 1
    print("calc_fliprate")
    print(testruns)
    for _, val in testruns.items():
        if first:
            first = False
            previous = val
            continue
        if val != previous:
            flips += 1
        if val != "pass":
            consecutive_failures += 1
        else:
            consecutive_failures = 0
        previous = val

    flip_rate = flips / possible_flips

    return pd.DataFrame({'flip_rate': [flip_rate], 'consecutive_failures': [consecutive_failures]})



def non_overlapping_window_fliprate(testruns: pd.Series, window_size: int) -> pd.Series:
    """Calculate flip rate for non-overlapping run windows"""
    # Apply calc_fliprate directly to the selected rows
    testruns_last = testruns.iloc[-window_size:]
    fliprate_groups = calc_fliprate(testruns_last)
    print("fliprate_groups")
    print(fliprate_groups)
    return fliprate_groups.reset_index(drop=True)

def calculate_n_days_fliprate_table(testrun_table: pd.DataFrame, days: int, window_count: int) -> pd.DataFrame:
    """Select given history amount and calculate fliprates for given n day windows.

    Return a table containing the results.
    """
    data = testrun_table[testrun_table.index >= (testrun_table.index.max() - pd.Timedelta(days=days * window_count))]

    fliprates = data.groupby([pd.Grouper(freq=f"{days}D"), "test_identifier"])["test_status"].apply(calc_fliprate)

    fliprate_table = fliprates.rename("flip_rate").reset_index(drop=True)
    fliprate_table["flip_rate_ewm"] = (
        fliprate_table.groupby("test_identifier")["flip_rate"]
        .ewm(alpha=EWM_ALPHA, adjust=EWM_ADJUST)
        .mean()
        .droplevel("test_identifier")
    )

    return fliprate_table[fliprate_table.flip_rate != 0]

def calculate_n_runs_fliprate_table(testrun_table: pd.DataFrame, window_size: int) -> pd.DataFrame:
    """Calculate fliprates for given n run window and select m of those windows
    Return a table containing the results.
    """
    print("all tests:")
    print(testrun_table)
    #testrun_table = testrun_table.iloc[-window_size:]
    # Apply non_overlapping_window_fliprate to each group in testrun_table
    fliprates = testrun_table.groupby("test_identifier")["test_status"].apply(
        lambda x: non_overlapping_window_fliprate(x, window_size)
    )

    # Convert fliprates Series of DataFrames to a DataFrame
    fliprate_table = fliprates.reset_index()
    print("fliprate_table")
    print(fliprate_table)
    # Rename the columns in fliprate_table
    fliprate_table = fliprate_table.rename(columns={"flip_rate": "flip_rate", "consecutive_failures": "consecutive_failures"})
    print("fliprate table final:")
    print(fliprate_table)

    # Calculate the EWMA of flip rates for each test identifier
    fliprate_table["flip_rate_ewm"] = (
        fliprate_table.groupby("test_identifier")["flip_rate"]
        .ewm(alpha=EWM_ALPHA, adjust=EWM_ADJUST)
        .mean()
        .droplevel("test_identifier")
    )

    # Rename the index level to "window"
    fliprate_table = fliprate_table.rename(columns={"level_1": "window"})

    # Filter out rows where flip_rate is not zero
    fliprate_table = fliprate_table[fliprate_table.flip_rate != 0]

    return fliprate_table

def get_top_fliprates(fliprate_table: pd.DataFrame, top_n: int, precision: int) -> Dict[str, tuple[Decimal, int]]:
    """Return the top n highest scoring test identifiers and their scores

    Look at the last calculation window for each test from the fliprate table
    and return the top n highest scoring test identifiers along with their scores
    and corresponding consecutive failures.
    """
    context = getcontext()
    context.prec = precision
    context.rounding = ROUND_UP
    last_window_values = fliprate_table.groupby("test_identifier").last()

    if top_n != 0:
        top_fliprates_ewm = last_window_values.nlargest(top_n, "flip_rate_ewm")
    else :
        top_fliprates_ewm = last_window_values.nlargest(len(last_window_values), "flip_rate_ewm")

    # Create a dictionary with test_identifier as keys and a tuple of flip_rate_ewm and consecutive_failures as values
    results = {}
    for test_id, row in top_fliprates_ewm.iterrows():
        results[test_id] = (Decimal(row["flip_rate_ewm"]), row["consecutive_failures"])

    return results


def parse_junit_suite_to_df(suite: TestSuite) -> list:
    """Parses Junit TestSuite results to a test history dataframe"""
    dataframe_entries = []
    time = suite.attrib.get('timestamp')

    for testcase in suite.findall('.//testcase'):
        test_identifier = testcase.attrib.get('classname') + "::" + testcase.attrib.get('name')

        # Update test count
        test_counts[test_identifier] = test_counts.get(test_identifier, 0) + 1

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

def parse_junit_to_df(folderpath: Path) -> pd.DataFrame:
    """Read JUnit test result files to a test history dataframe"""
    dataframe_entries = []

    for filepath in folderpath.glob("*.xml"):
        xml = ET.parse(filepath)
        root = xml.getroot()
        for suite in root.findall('.//testsuite'):
            dataframe_entries += parse_junit_suite_to_df(suite)


    if dataframe_entries:
        df = pd.DataFrame(dataframe_entries)
        df["timestamp"] = pd.to_datetime(df["timestamp"])
        df = df.set_index("timestamp")
        return df
    else:
        raise RuntimeError(f"No Junit files found from path {folderpath}")

def create_md_summary(results):
    """Create Markdown summary from results."""
    # Test summary
    summary = '## Flaky tests\n'

    # Table header
    summary += '|#|Flaky tests|Fliprate score %|Consecutive failures|\n'
    summary += '|-|-|-|-|\n'
    i = 1
    for test_name, (flip_rate_ewm, consecutive_failures) in results.items():
        summary += f'| {i} | {test_name} | {round(flip_rate_ewm, 2)*100} | {consecutive_failures} |\n'
        i += 1

    return summary

def main():
    """Print out top flaky tests and their fliprate scores.
    Also generate seaborn heatmaps visualizing the results if wanted.
    """

    logging.basicConfig(format="%(message)s", level=logging.INFO)

    parser = argparse.ArgumentParser()
    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument("--junit-files", help="Path for a folder with JUnit xml test history files", type=str)
    group.add_argument("--test-history-csv", help="Path for precomputed test history csv", type=str)
    parser.add_argument(
        "--grouping-option",
        choices=["days", "runs"],
        help="flip rate calculation method - days or runs",
        required=True,
    )
    parser.add_argument(
        "--window-size",
        type=int,
        help="flip rate calculation window size",
        required=True,
    )
    parser.add_argument(
        "--window-count",
        type=int,
        help="flip rate calculation window count (history size)",
        required=True,
    )
    parser.add_argument(
        "--top-n",
        type=int,
        help="amount of unique tests and scores to print out",
        required=False,
    )
    parser.add_argument(
        "--precision, -p",
        type=int,
        help="Precision of the flip rate score, default is 4",
        default=4,
        dest="decimal_count",
    )
    parser.add_argument(
        '-o',
        '--output-file',
        type=str,
        required=True,
        help='Path to output file.'
    )
    args = parser.parse_args()
    precision = args.decimal_count
    if args.top_n :
        top_n = args.top_n
    else :
        top_n = 0

    df = parse_input_files(args.junit_files, args.test_history_csv)

    if args.grouping_option == "days":
        fliprate_table = calculate_n_days_fliprate_table(df, args.window_size, args.window_count)
    else:
        fliprate_table = calculate_n_runs_fliprate_table(df, args.window_size)

    top_flip_rates = get_top_fliprates(fliprate_table, top_n, precision)

    if not top_flip_rates:
        logging.info("No flaky tests.")
        return

    logging.info(
        f"\nTop {top_n} flaky tests based on latest window fliprate score",
    )
    for test_id, (flip_rate_ewm, consecutive_failures) in top_flip_rates.items():
        logging.info(f"{test_id} --- flip_rate_ewm: {flip_rate_ewm} --- consecutive failures: {consecutive_failures}")

    print('::set-output name=top_flip_rates::{}'.format(', '.join(top_flip_rates)))
    with open('flaky_tests_report.txt', 'w') as f:
        for test in top_flip_rates:
            f.write('%s\n' % test)

    summary = create_md_summary(top_flip_rates)

    if args.output_file != '':
        with open(args.output_file, 'a') as file:
            file.write(summary)


if __name__ == "__main__":
    main()
