param(
    [string] $project_clone_path
)

pushd $project_clone_path
    $clone_proc = Start-Process -Wait -PassThru -NoNewWindow "git" -ArgumentList @(`
            "clone", `
            "https://github.com/spatialos/UnrealGDKExampleProject"
        )
popd