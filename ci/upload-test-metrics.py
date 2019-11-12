import glob
import json
import sys
from google.cloud import bigquery

# Define target upload location
PROJECT = "venator-unsepulcher-3029873"
DATASET = "benchmarks"
TABLE = "records"

# Initialize bigquery client
client = bigquery.Client(project=PROJECT)
table_ref = client.dataset(DATASET).table(TABLE)
table = client.get_table(table_ref)

# Read rows from files that have been generated per Buildkite step
def parse_json(file_path):
    with open(file_path) as json_file:
        return json.load(json_file)

rows_to_insert = [parse_json(summary_file) for summary_file in glob.glob("ci/test_summaries/*")]

# Upload rows
errors = client.insert_rows(table, rows_to_insert)

# Handle errors
for error in errors:
    print(f"Error inserting row at index {error['index']}: {error['errors']}", file=sys.stderr)

assert errors == []
