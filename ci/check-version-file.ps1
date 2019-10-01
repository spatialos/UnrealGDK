# Expects $gdk_home
param(
    $pinned_branches = @("master", "release", "feature/ci-use-latest-engine-fake-master")
)

pushd "$($gdk_home)\ci"
    $branch_name = (Get-Item -Path env:BUILDKITE_BRANCH).Value
    if ($pinned_branches.Contains($branch_name)) {
        $unreal_version = Get-Content -Path "unreal-engine.version" -Raw

        if ($unreal_version.StartsWith("HEAD ")) {
            Throw "Branches $($pinned_branches -join ", ") are required to have a pinned engine version. (I.e. of the form UnrealEngine-<commit hash>)"
        }
    }
popd