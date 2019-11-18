param(
    [string] $test_result_dir
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
        $second = $replacement_strings[$i+1]
        ((Get-Content -Path "$test_result_dir\index.html" -Raw) -Replace $first, $second) | Set-Content -Path "$test_result_dir\index.html"
    }

    # %5C is the escape code for a backslash \, needed to successfully reach the artifact from the serving site
    ((Get-Content -Path "$test_result_dir\index.html" -Raw) -Replace "index.json", "$($formatted_test_result_dir.Replace("\","%5C"))%5Cindex.json") | Set-Content -Path "$test_result_dir\index.html"

    echo "Test results in a nicer format can be found <a href='artifact://$formatted_test_result_dir\index.html'>here</a>.`n" | Out-File "$gdk_home/annotation.md"

    Get-Content "$gdk_home/annotation.md" | buildkite-agent annotate `
        --context "unreal-gdk-test-artifact-location"  `
        --style info
}

## Read the test results, and pass/fail the build accordingly 
$results_path = Join-Path -Path $test_result_dir -ChildPath "index.json"
$results_json = Get-Content $results_path -Raw

$results_obj = ConvertFrom-Json $results_json

Write-Log "Test results are displayed in a nicer form in the artifacts (index.html / index.json)"

if ($results_obj.failed -ne 0) {
    $fail_msg = "$($results_obj.failed) tests failed. Logs for these tests are contained in the tests.log artifact."
    Write-Log $fail_msg
    Throw $fail_msg
}

Write-Log "All tests passed!"
