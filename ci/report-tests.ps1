param(
    [string] $test_result_dir,
    [string] $target_platform
)

# Artifact path used by Buildkite (drop the initial C:\)
$formatted_test_result_dir = (Split-Path -Path "$test_result_dir" -NoQualifier).Substring(1)

if (Test-Path "$test_result_dir\index.html" -PathType Leaf) {
    # The Unreal Engine produces a mostly undocumented index.html/index.json as the result of running a test suite, for now seems mostly
    # for internal use - but it's an okay visualisation for test results, so we fix it up here to display as a build artifact in CI
    # (replacing local dependencies in the html by CDNs or correcting paths)

    $replacement_strings = @()
    $replacement_strings += @('/bower_components/font-awesome/css/font-awesome.min.css', 'https://cdnjs.cloudflare.com/ajax/libs/font-awesome/4.7.0/css/font-awesome.min.css')
    $replacement_strings += @('/bower_components/twentytwenty/css/twentytwenty.css', 'https://cdnjs.cloudflare.com/ajax/libs/mhayes-twentytwenty/1.0.0/css/twentytwenty.min.css')
    $replacement_strings += @('/bower_components/featherlight/release/featherlight.min.css', 'https://cdnjs.cloudflare.com/ajax/libs/featherlight/1.7.13/featherlight.min.css')
    $replacement_strings += @('/bower_components/bootstrap/dist/css/bootstrap.min.css', 'https://cdnjs.cloudflare.com/ajax/libs/twitter-bootstrap/3.3.7/css/bootstrap.min.css')
    $replacement_strings += @('/bower_components/jquery/dist/jquery.min.js', 'https://cdnjs.cloudflare.com/ajax/libs/jquery/3.1.1/jquery.min.js')
    $replacement_strings += @('/bower_components/jquery.event.move/js/jquery.event.move.js', 'https://cdnjs.cloudflare.com/ajax/libs/mhayes-twentytwenty/1.0.0/js/jquery.event.move.min.js')
    $replacement_strings += @('/bower_components/jquery_lazyload/jquery.lazyload.js', 'https://cdnjs.cloudflare.com/ajax/libs/jquery_lazyload/1.9.7/jquery.lazyload.min.js')
    $replacement_strings += @('/bower_components/twentytwenty/js/jquery.twentytwenty.js', 'https://cdnjs.cloudflare.com/ajax/libs/mhayes-twentytwenty/1.0.0/js/jquery.twentytwenty.min.js')
    $replacement_strings += @('/bower_components/clipboard/dist/clipboard.min.js', 'https://cdnjs.cloudflare.com/ajax/libs/clipboard.js/1.5.16/clipboard.min.js')
    $replacement_strings += @('/bower_components/anchor-js/anchor.min.js', 'https://cdnjs.cloudflare.com/ajax/libs/anchor-js/3.2.2/anchor.min.js')
    $replacement_strings += @('/bower_components/featherlight/release/featherlight.min.js', 'https://cdnjs.cloudflare.com/ajax/libs/featherlight/1.7.13/featherlight.min.js')
    $replacement_strings += @('/bower_components/bootstrap/dist/js/bootstrap.min.js', 'https://cdnjs.cloudflare.com/ajax/libs/twitter-bootstrap/3.3.7/js/bootstrap.min.js')
    $replacement_strings += @('/bower_components/dustjs-linkedin/dist/dust-full.min.js', 'https://cdnjs.cloudflare.com/ajax/libs/dustjs-linkedin/2.7.5/dust-full.min.js')
    $replacement_strings += @('/bower_components/numeral/min/numeral.min.js', 'https://cdnjs.cloudflare.com/ajax/libs/numeral.js/2.0.4/numeral.min.js')

    for ($i = 0; $i -lt $replacement_strings.length; $i = $i + 2) {
        $first = $replacement_strings[$i]
        $second = $replacement_strings[$i + 1]
        ((Get-Content -Path "$test_result_dir\index.html" -Raw) -Replace $first, $second) | Set-Content -Path "$test_result_dir\index.html"
    }

    # %5C is the escape code for a backslash \, needed to successfully reach the artifact from the serving site
    ((Get-Content -Path "$test_result_dir\index.html" -Raw) -Replace "index.json", "$($formatted_test_result_dir.Replace("\","%5C"))%5Cindex.json") | Set-Content -Path "$test_result_dir\index.html"

    Write-Output "Test results in a nicer format can be found <a href='artifact://$formatted_test_result_dir\index.html'>here</a>.`n" | Out-File "$gdk_home/annotation.md"

    Get-Content "$gdk_home/annotation.md" | buildkite-agent annotate `
        --context "unreal-gdk-test-artifact-location"  `
        --style info
}
else {
    $error_msg = "The Unreal Editor crashed while running tests, see the test-gdk annotation for logs (or the tests.log buildkite artifact)."
    Write-Error $error_msg
    buildkite-agent artifact upload "$test_result_dir\*"
    Throw $error_msg
}

# Upload artifacts to Buildkite, capture output to extract artifact ID in the Slack message generation
# Command format is the results of Powershell weirdness, likely related to the following:
# https://stackoverflow.com/questions/2095088/error-when-calling-3rd-party-executable-from-powershell-when-using-an-ide
$upload_output = & cmd /c 'buildkite-agent 2>&1' artifact upload "$test_result_dir\*" | Out-String

# Artifacts are assigned an ID upon upload, so grab IDs from upload process output to build the artifact URLs
Try {
    $test_results_id = (Select-String -Pattern "[^ ]* $($formatted_test_result_dir.Replace("\","\\"))\\index.html" -InputObject $upload_output -CaseSensitive).Matches[0].Value.Split(" ")[0]
    $test_log_id = (Select-String -Pattern "[^ ]* $($formatted_test_result_dir.Replace("\","\\"))\\tests.log" -InputObject $upload_output -CaseSensitive).Matches[0].Value.Split(" ")[0]
}
Catch {
    Write-Error "Failed to parse artifact ID from the buildkite uploading output: $upload_output"
    Throw $_
}
$test_results_url = "https://buildkite.com/organizations/$env:BUILDKITE_ORGANIZATION_SLUG/pipelines/$env:BUILDKITE_PIPELINE_SLUG/builds/$env:BUILDKITE_BUILD_ID/jobs/$env:BUILDKITE_JOB_ID/artifacts/$test_results_id"
$test_log_url = "https://buildkite.com/organizations/$env:BUILDKITE_ORGANIZATION_SLUG/pipelines/$env:BUILDKITE_PIPELINE_SLUG/builds/$env:BUILDKITE_BUILD_ID/jobs/$env:BUILDKITE_JOB_ID/artifacts/$test_log_id"

# Read the test results
$results_path = Join-Path -Path $test_result_dir -ChildPath "index.json"
$results_json = Get-Content $results_path -Raw
$test_results_obj = ConvertFrom-Json $results_json
$tests_passed = $test_results_obj.failed -eq 0

# Build Slack attachment
$total_tests_succeeded = $test_results_obj.succeeded + $test_results_obj.succeededWithWarnings
$total_tests_run = $total_tests_succeeded + $test_results_obj.failed
$slack_attachment = [ordered]@{
    fallback = "Find the test results at $test_results_url"
    color    = $(if ($tests_passed) { "good" } else { "danger" })
    fields   = @(
        @{
            value = "*$env:ENGINE_COMMIT_HASH* $(Split-Path $test_result_dir -Leaf)"
            short = $True
        }
        @{
            value = "Passed $total_tests_succeeded / $total_tests_run tests."
            short = $True
        }
    )
    actions  = @(
        @{
            type  = "button"
            text  = ":bar_chart: Test results"
            url   = "$test_results_url"
            style = "primary"
        }
        @{
            type  = "button"
            text  = ":page_with_curl: Test log"
            url   = "$test_log_url"
            style = "primary"
        }
    )
}

$slack_attachment | ConvertTo-Json | Set-Content -Path "$test_result_dir\slack_attachment_$env:BUILDKITE_STEP_ID.json"

buildkite-agent artifact upload "$test_result_dir\slack_attachment_$env:BUILDKITE_STEP_ID.json"

# Count the number of SpatialGDK tests in order to report this
$num_gdk_tests = 0
Foreach ($test in $test_results_obj.tests) {
    if ($test.fulltestPath.Contains("SpatialGDK.")) {
        $num_gdk_tests += 1
    }
}

# Count the number of Project (functional) tests in order to report this
$num_project_tests = 0
Foreach ($test in $test_results_obj.tests) {
    if ($test.fulltestPath.Contains("Project.")) {
        $num_project_tests += 1
    }
}

# Define and upload test summary JSON artifact for longer-term test metric tracking (see upload-test-metrics.sh)
$test_summary = [pscustomobject]@{
    time                       = Get-Date -UFormat %s
    build_url                  = "$env:BUILDKITE_BUILD_URL"
    platform                   = "$target_platform"
    unreal_engine_commit       = "$env:ENGINE_COMMIT_HASH"
    passed_all_tests           = $tests_passed
    tests_duration_seconds     = $test_results_obj.totalDuration
    num_tests                  = $total_tests_run
    num_gdk_tests              = $num_gdk_tests
    num_project_tests          = $num_project_tests
    test_result_directory_name = Split-Path $test_result_dir -Leaf
}
$test_summary | ConvertTo-Json -Compress | Set-Content -Path "$test_result_dir\test_summary_$env:BUILDKITE_STEP_ID.json"

buildkite-agent artifact upload "$test_result_dir\test_summary_$env:BUILDKITE_STEP_ID.json"

# Fail this build if any tests failed
if (-Not $tests_passed) {
    $fail_msg = "$($test_results_obj.failed) tests failed. Logs for these tests are contained in the tests.log artifact."
    Write-Log $fail_msg
    Throw $fail_msg
}
Write-Log "All tests passed!"
