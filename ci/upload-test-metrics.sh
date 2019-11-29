#!/bin/bash
set -euo pipefail

# Fetch the test summary artifacts uploaded earlier
mkdir "./test_summaries"
buildkite-agent artifact download "*test_summary*.json" "./test_summaries"

# Define target upload location
PROJECT="holocentroid-aimful-6523579"
DATASET="UnrealGDK"
TABLE="ci_metrics"

# Make sure that the gcp secret is always removed
GCP_SECRET="$(mktemp)"
function cleanup {
  rm -rf "${GCP_SECRET}"
}
trap cleanup EXIT

# Fetch Google credentials so that we can upload the metrics to the GCS bucket.
imp-ci secrets read --environment=production --buildkite-org=improbable \
    --secret-type=gce-key-pair --secret-name=qa-unreal-gce-service-account \
    --write-to=${GCP_SECRET}
gcloud auth activate-service-account --key-file "${GCP_SECRET}"

# Upload metrics
for json_file in ./test_summaries/*.json; do
    cat "${json_file}" | bq --project_id "${PROJECT}" --dataset_id "${DATASET}" insert "${TABLE}"
done
