# GDK for Unreal Release Process

This document outlines the process for releasing a version of the GDK for Unreal and all associated projects.

> This process will need to be reviewed once we hit beta.

## Terminology
* **Release version** is the version of the SpatialOS GDK for Unreal that you are releasing by performing the steps in this document.
* **Previous version** is the latest version of the SpatialOS GDK for Unreal that is currently released to customers. You can find out what this version is [here](https://github.com/spatialos/UnrealGDK/releases).

## Pre-Validation

### Create the `UnrealGDK` release candidate
1. Notify `#dev-unreal-internal` that you intend to commence a release. Ask if anyone `@here` knows of any blocking defects in code or docs that should be resolved prior to commencement of the release process.
1. `git clone` the [UnrealGDK](https://github.com/spatialos/UnrealGDK).
1. `git checkout master`
1. `git pull`
1. Using `git log`, take note of the latest commit hash.
1. `git checkout -b x.y.z-rc` in order to create release candidate branch.
1. Open `CHANGELOG.md`, which is in the root of the repository, and put the release version and planned date of release in a `##` block. Move the `Unreleased` section above this.
    - Look at the previous release versions in the changelog to see how this should be done.
1. Commit your changes to `CHANGELOG.md`.
1. `git push --set-upstream origin x.y.z-rc` to push the branch.
1. Announce the branch and the commit hash it uses in the `#unreal-gdk-release` channel.

### Create the `improbableio/UnrealEngine` release candidate
1. `git clone` the [improbableio/UnrealEngine](https://github.com/improbableio/UnrealEngine).
1. `git checkout 4.xx-SpatialOSUnrealGDK`
1. `git pull`
1. Using `git log`, take note of the latest commit hash.
1. `git checkout -b 4.xx-SpatialOSUnrealGDK-x.y.z-rc` in order to create release candidate branch.
1. `git push --set-upstream origin 4.xx-SpatialOSUnrealGDK-x.y.z-rc` to push the branch.
1. Announce the branch and the commit hash it uses in the `#unreal-gdk-release` channel.

### Create the `UnrealGDKThirdPersonShooter` release candidate
1. `git clone` the [UnrealGDKThirdPersonShooter](https://github.com/spatialos/UnrealGDKThirdPersonShooter).
1. `git checkout master`
1. `git pull`
1. Using `git log`, take note of the latest commit hash.
1. `git checkout -b x.y.z-rc` in order to create release candidate branch.
1. `git push --set-upstream origin x.y.z-rc` to push the branch.
1. Announce the branch and the commit hash it uses in the #unreal-gdk-release channel.

### Create the `UnrealGDKTestSuite` release candidate
1. `git clone` the [UnrealGDKTestSuite](https://github.com/spatialos/UnrealGDKTestSuite).
1. `git checkout master`
1. `git pull`
1. Using `git log`, take note of the latest commit hash.
1. `git checkout -b x.y.z-rc` in order to create release candidate branch.
1. `git push --set-upstream origin x.y.z-rc` to push the branch.
1. Announce the branch and the commit hash it uses in the #unreal-gdk-release channel.

### Serve docs locally
It is vital that you test using the docs for the release version that you are about to publish, not with the currently live docs that relate to the previous version.
1. `improbadoc serve <path to x.y.z-rc-x docs>`

## Build your release candidate engine
1. Open http://localhost:8080/reference/1.0/content/get-started/dependencies.
1. Uninstall all dependencies listed on this page so that you can accurately validate our installation steps.
1. If you have one, delete your local clone of `UnrealEngine`.
1. Follow the installation steps on http://localhost:8080/reference/1.0/content/get-started/dependencies.
1. When you clone the `UnrealEngine`, be sure to checkout `x.y.z-rc-x` so you're building the release version.

## Implementing fixes

If at any point in the below validation steps you encounter a blocker, you must fix that defect prior to releasing.

The workflow for this is:

1. Raise a bug ticket in JIRA detailing the blocker.
1. `git checkout x.y.z-rc`
1. `git pull`
1. `git checkout -b bugfix/UNR-xxx`
1. Fix the defect.
1. `git commit`, `git push -u origin HEAD`, target your PR at `x.y.z-rc`.
1. When the PR is merged, `git checkout x.y.z-rc`, `git pull` and re-test the defect to ensure you fixed it.
1. Notify #unreal-gdk-release that the release candidate has been updated.
1. **Judgment call**: If the fix was isolated, continue the validation steps from where you left off. If the fix was significant, restart testing from scratch. Consult the rest of the team if you are unsure which to choose.

## Validation (Multiserver Shooter tutorial)
1. Follow these steps: http://localhost:8080/reference/1.0/content/get-started/tutorial, bearing in mind the following caveats:
* When you clone `UnrealGDKThirdPersonShooter`, be sure to checkout the release candidate branch, so you're working with the release version.
* When you clone the GDK into the `Plugins` folder, be sure to checkout the release candidate branch, so you're working with the release version.
* Your release candidate is cut from `master` so all steps in the tutorial that require you to make code changes to the project **do not need to be executed**, as cross-server RPCs are already implemented on `master`. Later in this release process you will `git rebase` into `tutorial`, so that users can still follow the tutorial on that branch. The steps that you should skip are:
  * [Replicate health changes](http://localhost:8080/reference/1.0/content/get-started/tutorial#replicate-health-changes)
  * [Deploy the project locally (again)](http://localhost:8080/reference/1.0/content/get-started/tutorial#deploy-the-project-locally-again)
  * [Enable cross server RPCs](http://localhost:8080/reference/1.0/content/get-started/tutorial#enable-cross-server-rpcs)
  * [Deploy the project locally (last time)](http://localhost:8080/reference/1.0/content/get-started/tutorial#deploy-the-project-locally-last-time)
2. Launch a local SpatialOS deployment, then a standalone server-worker, and then connect two standalone clients to it. To do this:
* In your file browser, click `UnrealGDKThirdPersonShooter\LaunchSpatial.bat` in order to run it.
* In your file browser, click `UnrealGDKThirdPersonShooter\LaunchServer.bat` in order to run it.
* In your file browser, click `UnrealGDKThirdPersonShooter\LaunchClient.bat` in order to run it.
* Run the same script again in order to launch the second client
* Run and shoot eachother with the clients as a smoke test.
* Open the `UE4 Console` and enter the command `open 127.0.0.1`. The desired effect is that the client disconnect and then re-connects to the map. If you can continue to play after executing the command then you've succesfully tested client travel.
3. On a seperate Windows PC, launch a local SpatialOS deployment, then a standalone server-worker, and then on your original Windows PC, connect two standalone clients to it. To do this:
* Ensure that both machines are on the same network.
* On your the machine you're going to run the server on, follow the setup steps listed with caveats in step 1.
* Still on your server machine, run `UnrealGDKThirdPersonShooter\LaunchSpatial.bat`
* Still on your server machine, run `UnrealGDKThirdPersonShooter\LaunchServer.bat`.
* Still on your server machine, discover your local IP address by following these [steps](https://lifehacker.com/5833108/how-to-find-your-local-and-external-ip-address).
* On the machine you're going to run your clients on, open `UnrealGDKThirdPersonShooter\LaunchClient.bat` in your code editor of choice and:
  * Change `127.0.0.1`to the local IP of your server machine.
  * Add `-OverrideSpatialNetworking` after the ip address
  * Add `-NetDriverOverrides=/Script/SpatialGDK.SpatialNetDriver` after that
  * Add `+useExternalIpForBridge true` after that
  * Your final script shoudl look somethisn like: `@echo off
call "%~dp0ProjectPaths.bat"
"%UNREAL_HOME%\Engine\Binaries\Win64\UE4Editor.exe" "%~dp0%PROJECT_PATH%\%GAME_NAME%.uproject" 172.16.120.76 -game -log -OverrideSpatialNetworking -NoLogToSpatial -windowed -ResX=1280 -ResY=720  +workerType UnrealClient -NetDriverOverrides=/Script/SpatialGDK.SpatialNetDriver +useExternalIpForBridge true`
* Save your changes.
* In your file browser, click `UnrealGDKThirdPersonShooter\LaunchClient.bat` in order to run it.
* Run the same script again in order to launch the second client
* Run and shoot eachother with the clients as a smoke test.
* You can now turn off your server machine.

## Validation (GDK Starter Template)
1. Follow these steps: http://localhost:8080/reference/1.0/content/get-started/gdk-template, bearing in mind the following caveat:
* When you clone the GDK into the `Plugins` folder, be sure to checkout the release candidate branch, so you're working with the release version.

## Validation (UnrealGDKTestSuite)
1. Follow these steps: https://github.com/spatialos/UnrealGDKTestSuite/blob/release/README.md. All tests must pass.

## Validation (Playtest)
1. Follow these steps: https://brevi.link/unreal-release-playtests. All tests must pass.

## Validation (Docs)
1. Upload docs to docs-testing using Improbadoc.
1. Validate that Improbadoc reports no linting errors.
1. Read the docs for five minutes to ensure nothing looks broken.

## Release

All of the above tests **must** have passed and there must be no outstanding blocking issues before you start this, the release phase.

When merging the following PRs, you need to enable `Allow merge commits` option on the repos and choose `Create a merge commit` from the dropdown in the pull request UI to merge the branch, then disable `Allow merge commits` option on the repos once the release process is complete. You need to be an admin to perform this.

1. In `UnrealGDK`, create `premerge-x.y.z-rc` from `x.y.z-rc` (select `x.y.z-rc` in the branch dropdown, then type the name of the new branch to create. If there's already a branch with that name, delete it first).
1. In `UnrealGDK`, merge `x.y.z-rc` into `release` using GitHub PR (use `Update branch` button to merge `release` into `x.y.z-rc`).
1. Use the GitHub Release UI to tag the commit you just made to as `x.y.z`.<br/>
Copy the latest release notes from `CHANGELOG.md` and paste them into the release description field.
1. In `improbableio/UnrealEngine`, create `premerge-4.xx-SpatialOSUnrealGDK-x.y.z-rc` from `4.xx-SpatialOSUnrealGDK-x.y.z-rc`.
1. In `improbableio/UnrealEngine`, merge `4.xx-SpatialOSUnrealGDK-x.y.z-rc` into `4.xx-SpatialOSUnrealGDK-release` using GitHub PR (use `Update branch` button to merge `4.xx-SpatialOSUnrealGDK-release` into `4.xx-SpatialOSUnrealGDK-x.y.z-rc`).
1. Use the GitHub Release UI to tag the commit you just made as `4.xx-SpatialOSUnrealGDK-x.y.z`.<br/>
1. In `UnrealGDKThirdPersonShooter`, create `premerge-x.y.z-rc` from `x.y.z-rc`.
1. In `UnrealGDKThirdPersonShooter`, merge `x.y.z-rc` into `release` using GitHub PR (use `Update branch` button to merge `release` into `x.y.z-rc`).
1. Use the GitHub Release UI to tag the commit you just made to as `x.y.z`.
1. In `UnrealGDKThirdPersonShooter`, `git rebase` `release` into `tutorial`.
1. In `UnrealGDKThirdPersonShooter`, `git rebase` `release` into `tutorial-complete`.
1. In `UnrealGDKTestSuite`, create `premerge-x.y.z-rc` from `x.y.z-rc`.
1. In `UnrealGDKTestSuite`, merge `x.y.z-rc` into `release` using GitHub PR (use `Update branch` button to merge `release` into `x.y.z-rc`).
1. Use the GitHub Release UI to tag the commit you just made to as `x.y.z`.
1. Publish the docs to live using Improbadoc commands listed [here](https://improbableio.atlassian.net/wiki/spaces/GBU/pages/327485360/Publishing+GDK+Docs).
1. Update the [roadmap](https://github.com/spatialos/UnrealGDK/pull/900), moving the release from **Planned** to **Released**, and linking to the release.
1. Announce the release in:

* Forums
* Discord (`#unreal`, do not `@here`)
* Slack (`#releases`)
* Email (`spatialos-announce@`)

Congratulations, you've done the release!

## Clean up

There are potentially changes that were merged into the release candidate branches that aren't merged into the corresponding development (master) branches yet. Because there are some race conditions involved, a GDK engineer should be in charge of doing this.

indirect-merge from `A` into `B` (`premerge-x.y.z-rc` into `master`, except `improbableio/UnrealEngine` repo where this is `premerge-4.xx-SpatialOSUnrealGDK-x.y.z-rc` into `4.xx-SpatialOSUnrealGDK`) is defined as:
1. Make sure there is no `merge-A-into-B` branch created already, and delete it if it does.
1. Create branch `merge-A-into-B` from `B` (`git checkout B`, `git checkout -b merge-A-into-B`).
1. Merge the one before last commit of `A` into `merge-A-into-B` (`git merge A~`, `git push origin merge-A-into-B`). The reason why the one before last commit is chosen is because 
1. Merge `merge-A-into-B` into `B` using GitHub PR. Make sure you use `Create a merge commit` option.

Use the above definition to perform the following:
1. `UnrealGDK` repo: indirect-merge `premerge-x.y.z-rc` into `master`.
1. `improbableio/UnrealEngine` repo: indirect-merge `premerge-4.xx-SpatialOSUnrealGDK-x.y.z-rc` into `4.xx-SpatialOSUnrealGDK`.
1. `UnrealGDKThirdPersonShooter` repo: (skip this step if `premerge-x.y.z-rc` has no new commits compared to `master`) indirect-merge `premerge-x.y.z-rc` into `master`.
1. `UnrealGDKTestSuite` repo: (skip this step if `premerge-x.y.z-rc` has no new commits compared to `master`) indirect-merge `premerge-x.y.z-rc` into `master`.
1. Delete the `premerge-` branches.

## Appendix

<details>
  <summary>Forum Post Template</summary>

 We are happy to announce the release of version x.y.z of the SpatialOS GDK for Unreal.

Please see the full release notes on GitHub:

Unreal GDK - https://github.com/spatialos/UnrealGDK/releases/tag/x.y.z<br/>
Corresponding fork of Unreal Engine - https://github.com/improbableio/UnrealEngine/releases/tag/4.xx-SpatialOSUnrealGDK-x.y.z<br/>
Corresponding version of the Third Person Shooter Project - https://github.com/spatialos/UnrealGDKThirdPersonShooter/releases/tag/x.y.z <br/>

</details>
