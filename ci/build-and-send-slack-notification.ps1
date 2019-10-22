# Send a Slack notification with a link to the build.

# Download previously uploaded slack attachments that were generated for each testing step
New-Item -ItemType Directory -Path "$PSScriptRoot/slack_attachments"
& buildkite-agent artifact download "*slack_attachment_*.json" "$PSScriptRoot/slack_attachments"

$attachments = @()
$all_steps_passed = $true
Get-ChildItem -Recurse "$PSScriptRoot/slack_attachments" -Filter *.json | Foreach-Object {
    $attachment = Get-Content -Path $_.FullName | Out-String | ConvertFrom-Json
    if ($attachment.color -eq "danger") {
        all_steps_passed = $false
    }
    $attachments += $attachment
}

# Build text for slack message
if ($env:BUILDKITE_NIGHTLY_BUILD -eq "true") {
    $build_description = ":night_with_stars: Nightly build of *GDK for Unreal*"
} else {
    $build_description = "*GDK for Unreal* build by $env:BUILDKITE_BUILD_CREATOR"
}
if ($all_steps_passed) {
    $build_result = "passed testing"
} else {
    $build_result = "failed testing"
}
$slack_text = $build_description + " " + $build_result + "."

# Read Slack webhook secret from the vault and extract the Slack webhook URL from it.
$slack_webhook_secret = "$(imp-ci secrets read --environment=production --buildkite-org=improbable --secret-type=slack-webhook --secret-name=unreal-gdk-slack-web-hook)"
$slack_webhook_url = $slack_webhook_secret | ConvertFrom-Json | %{$_.url}

$json_message = [ordered]@{
    text = "$slack_text"
    attachments= @(
            @{
                fallback = "Find the build at $build_url"
                color = $(if ($all_steps_passed) {"good"} else {"danger"})
                fields = @(
                        @{
                            title = "Build message"
                            value = "$env:BUILDKITE_MESSAGE".Substring(0, [System.Math]::Min(64, "$env:BUILDKITE_MESSAGE".Length))
                            short = "true"
                        }
                        @{
                            title = "GDK branch"
                            value = "$env:BUILDKITE_BRANCH"
                            short = "true"
                        }
                    )
                actions = @(
                        @{
                            type = "button"
                            text = ":github: GDK commit"
                            url = "https://github.com/spatialos/UnrealGDK/commit/$env:BUILDKITE_COMMIT"
                            style = "primary"
                        }
                        @{
                            type = "button"
                            text = ":buildkite: BK build"
                            url = "$env:BUILDKITE_BUILD_URL"
                            style = "primary"
                        }
                    )
            }
        )
    }

# Add attachments from other build steps
foreach ($attachment in $attachments) {
    $json_message.attachments += $attachment
} 

$json_request = $json_message | ConvertTo-Json -Depth 10

Invoke-WebRequest -UseBasicParsing "$slack_webhook_url" -ContentType "application/json" -Method POST -Body "$json_request"
