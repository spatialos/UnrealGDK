<%(TOC)%>
# Keeping your GDK up to date

The SpatialOS GDK for Unreal consists of two git repositories:<br>

* [The GDK](https://github.com/spatialos/UnrealGDK)<br>
* [The SpatialOS Unreal Engine fork.](https://github.com/improbableio/UnrealEngine)

## Step 1: Ensure you're on the release branches.

If you've followed our [get started]({{urlRoot}}/content/get-started/introduction) guide, you will already have these repositories cloned on your computer.<br>

* Your `UnrealGDK` repository should have the `release` branch, checked out.<br>
* Your `UnrealEngine` repository should have the `4.20-SpatialOSUnrealGDK-release` branch, checked out.<br>

You can determine which branch is checked out by:<br>

1. In a terminal of your choice, change directory to the root of the repository.<br>
1. Run `git status`.
1. This command should return `On release` in your `UnrealGDK` repository and `On 4.20-SpatialOSUnrealGDK-release` in your `UnrealEngine` repository.<br>
If you're on a different branch, `git checkout` the branch that you want.

## Step 2: Upgrade your GDK and your Unreal Engine fork.

<%(Callout type="alert" message="Whenever you upgrade your GDK you **must** also upgrade your Unreal Engine fork. If you don't, errors can be caused by the two repositories being out of sync.")%>

It's good practice to always develop your game on the latest version of the GDK. To upgrade to latest:

1. Read the release notes on the releases page of the [`UnrealGDK` GitHub](https://github.com/spatialos/UnrealGDK/releases) so you understand the changes you're about to download.
1. In a terminal, change directory to the root of `UnrealGDK`.
1. `git pull`
1. In a terminal, change directory to the root of `UnrealEngine`.
1. `git pull`
1. Open **File Explorer**, navigate to the root directory of the Unreal GDK repository, and double-click **`Setup.bat`**. You may be prompted to sign into your SpatialOS account if you have not already.
1. In **File Explorer**, navigate to `<GameRoot>`, the directory containing your project's `.uproject` file and `Source` directory.<br>
Right-click on your `.uproject` file and select **Generate Visual Studio project files**.

You are now on the latest GDK.

Be sure to join the community on our <a href="https://forums.improbable.io" data-track-link="Join Forums Clicked|product=Docs" target="_blank">forums</a> or on <a href="https://discord.gg/vAT7RSU" data-track-link="Join Discord Clicked|product=Docs|platform=Win|label=Win" target="_blank">Discord</a>. We announce GDK versions there.