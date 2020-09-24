# GDK for Unreal Release Process

This document outlines the process for releasing a version of the GDK for Unreal and all associated projects.

## Terminology
* **GDK release version** is the version of the SpatialOS GDK for Unreal that you are releasing by performing the steps in this document. It's [semantically versioned](https://semver.org/) and looks like `x.y.z`.
* **Previous GDK version** is the version of the SpatialOS GDK for Unreal that is currently at HEAD of the `release` branch. You can find out what this version is [here](https://github.com/spatialos/UnrealGDK/releases).

## Release
1. Notify `#dev-unreal-internal` that you intend to commence a release. Ask if anyone `@here` knows of any blocking defects in code or documentation that should be resolved prior to commencement of the release process.
1. Notify `@techwriters` in #docs that they may commence their [CHANGELOG review process](https://improbableio.atlassian.net/l/c/4FsZzbHk).
1. If nobody objects to the release, navigate to [unrealgdk-release](https://buildkite.com/improbable/unrealgdk-release/) and select the New Build button.
1. In the Message field type "Releasing [GDK release version]".
1. The "Commit" field is prepopulated with `HEAD`, leave it as is.
1. The "Branch" field is prepopulated with `master`. Leave it as is. This determines which version of the unrealgdk-release pipeline is run.
1. Select "Create Build".
1. Wait about 20 seconds for `imp-ci steps run --from .buildkite/release.steps.yaml` to pass and then select "Configure your release."
1. In the "UnrealGDK component release version" field enter the GDK release version.
1. The "UnrealGDK source branch" field is prepopulated with `master`. Leave it as is if you're executing a major or minor release, change it to `release` if you're executing a patch release.
1. The "UnrealEngine source branches" field should be prepopulated with the source branches of the latest fully supported and legacy supported Unreal Engine versions. If you're executing a patch release you'll need to suffix each branch with `-release`.<details> <summary>Wrong prepopulated branches?</summary> If the prepopulated branches are wrong, select the button with an X at the upper-right corner of the form, and then select "Cancel" to stop this build of unrealgdk-release. Then, on the UnrealGDK's `master` branch at [`.buildkite/release.steps.yaml#L32`](https://github.com/spatialos/UnrealGDK/blob/master/.buildkite/release.steps.yaml#L32), update the default branches to the latest, merge that change and restart this release process </details>
1. Select "Continue" and move onto the next step.
1. Wait for the "Prepare the release" step to run, it takes about 20 minutes, maybe grab a coffee?
1. Once the "Prepare the release" step has passed the "Build & upload all UnrealEngine release candidates" step will commence.<br> While those builds run, take a look at the top of the build page, where you'll notice a new [annotation](https://buildkite.com/docs/agent/v3/cli-annotate): "your human labour is now required to complete the tasks listed in the PR descriptions and unblock the pipeline to resume the release."<br>Click through to the PRs using the links in the annotations and follow the steps. Come back when you're done.
1. As soon as the "Build & upload all UnrealEngine release candidates" step has passed, select "Run all tests".
1. Once all test have passed, all PRs are approved and all tasks listed in the PR descriptions are complete, select "Unblock the release". This will trigger "Release `ci/release.sh`".
1. When "Release `ci/release.sh`" is complete, the unrealgdk-release pipeline will pass.<br>
Take a look at the top of the build page, where you'll notice a new [annotation](https://buildkite.com/docs/agent/v3/cli-annotate):<br>
"Release Successful!"<br>
"Release hash: [hash]"<br>
"Draft release: [url]"
1. Open every draft release link and click publish on each one.
1. Notify China Infra team [sync the release branch to Gitee](https://buildkite.com/improbable/platform-copybara) and add the release tags manually.
1. Notify @techwriters in [#docs](https://improbable.slack.com/archives/C0TBQAB5X) that they may publish the new version of the docs.
1. Announce the release in:

* Forums
* Discord (`#unreal`, do not `@here`)
* Slack (`#releases`)
* Email (`unreal-interest@`)

Congratulations, you've completed the release process!

**Update public roadmap and known issues**
1. Create a new column on the [public roadmap](https://github.com/spatialos/UnrealGDK/projects/1) and link the release to it.
1. Move any [known issues](https://github.com/spatialos/UnrealGDK/projects/2) that were fixed in this release into the "Fixed" column and close them.

## Clean up

1. Delete all `-rc` branches.

## Appendix

<details>
  <summary>Forum Post Template</summary>

 We are happy to announce the release of version [GDK release version] of the SpatialOS GDK for Unreal.

Please see the full release notes on GitHub:

Unreal GDK - https://github.com/spatialos/UnrealGDK/releases/tag/x.y.z<br/>

Corresponding Unreal Engine versions:
- https://github.com/improbableio/UnrealEngine/releases/tag/4.xx-SpatialOSUnrealGDK-x.y.z<br/>
- https://github.com/improbableio/UnrealEngine/releases/tag/4.xx-SpatialOSUnrealGDK-x.y.z<br/>

Corresponding version of the Example Project - https://github.com/spatialos/UnrealGDKExampleProject/releases/tag/x.y.z <br/>

</details>
