# GDK for Unreal Release Process

This document outlines the process for releasing a version of the GDK for Unreal and all associated projects.

## Terminology
* **GDK release version** is the version of the SpatialOS GDK for Unreal that you are releasing by performing the steps in this document. It's [semantically versioned](https://semver.org/) and looks like `x.y.z`.
* **Previous GDK version** is the version of the SpatialOS GDK for Unreal that is currently at HEAD of the `release` branch. You can find out what this version is [here](https://github.com/spatialos/UnrealGDK/releases).

### Create the `UnrealGDK` release candidate
1. Notify `#dev-unreal-internal` that you intend to commence a release. Ask if anyone `@here` knows of any blocking defects in code or documentation that should be resolved prior to commencement of the release process.
1. If nobody objects to the release, navigate to [unrealgdk-release](https://buildkite.com/improbable/unrealgdk-release/) and select the New Build button.
1. In the Message field type "Releasing [GDK release version]".
1. The "Commit" field is prepopulated with `HEAD`, leave it as is.
1. The "Branch" field is prepopulated with `master`. Leave it as is if you're executing a major or minor release, change it to `release` if you're executing a patch release.
1. Select "Create Build".
1. Wait about 20 seconds for `imp-ci steps run --from .buildkite/release.steps.yaml` to pass and then select "Configure your release."
1. In the "UnrealGDK component release version" enter the GDK release version.
1. The "UnrealEngine source branches" field should be prepopulated with the source branches of the latest fully supported and legacy supported Unreal Engine versions. If this is the case, select Continue.<br>
If this is not the case, select the button with an X at the upper-right corner of the form, and then select Cancel. Then, on the UnrealGDK's `master` branch at [`.buildkite/release.steps.yaml#L32`](https://github.com/spatialos/UnrealGDK/blob/master/.buildkite/release.steps.yaml#L32), update the default branches to the latest, merge that change and restart this release process.


**Documentation**
1. Notify @techwriters in [#docs](https://improbable.slack.com/archives/C0TBQAB5X) that they may publish the new version of the docs.

**Announce**
1. Announce the release in:

* Forums
* Discord (`#unreal`, do not `@here`)
* Slack (`#releases`)
* Email (`unreal-interest@`)

Congratulations, you've completed the release process!

## Clean up

1. Delete all `rc` branches.

## Appendix

<details>
  <summary>Forum Post Template</summary>

 We are happy to announce the release of version x.y.z of the SpatialOS GDK for Unreal.

Please see the full release notes on GitHub:

Unreal GDK - https://github.com/spatialos/UnrealGDK/releases/tag/x.y.z<br/>
Corresponding Unreal Engine versions - https://github.com/improbableio/UnrealEngine/releases/tag/4.xx-SpatialOSUnrealGDK-x.y.z<br/>
                                     - https://github.com/improbableio/UnrealEngine/releases/tag/4.xx-SpatialOSUnrealGDK-x.y.z<br/>
Corresponding version of the Example Project - https://github.com/spatialos/UnrealGDKExampleProject/releases/tag/x.y.z <br/>

</details>
