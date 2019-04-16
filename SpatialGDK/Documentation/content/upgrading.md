<%(TOC)%>
# Keep your GDK up to date

To use the SpatialOS GDK for Unreal, you need two git repositories:<br>

* [The SpatialOS Unreal Engine fork.](https://github.com/improbableio/UnrealEngine)
* [The GDK](https://github.com/spatialos/UnrealGDK)<br>

It is good practice to always develop your game on the latest version of the software by regularly updating it. This ensures you benefit from the most up-to-date functionality. Whenever you update your GDK software, you **must** also update your SpatialOS Unreal Engine fork software. If you don't, you may get errors from them being out of synch.

We recommend that you update your version of the GDK and SpatialOS Unreal Engine fork every week.

## Step 1: Ensure you're on the release branches.

If you followed our [get started]({{urlRoot}}/content/get-started/introduction) guide, you have these repositories cloned on your computer.<br>

* Your `UnrealGDK` repository should have the `release` branch checked out.<br>
* Your `UnrealEngine` repository should have the branch ending with `-SpatialOSUnrealGDK-release`checked out.<br>

You can determine which branch is checked out by following the instructions below:<br>

1. In a terminal of your choice, change directory to the root of the repository.<br>
1. Run `git status`.
This should return `On *-SpatialOSUnrealGDK-release` in your `UnrealEngine` repository and `On release` in your `UnrealGDK` repository.<br>
If it returns a different branch, run `git checkout <branch-name>` to check out the branch that you want.

## Step 2: Update your Unreal Engine fork and GDK.

Before you begin, read the release notes on the releases page of the [`UnrealGDK` GitHub](https://github.com/spatialos/UnrealGDK/releases) so you understand the changes that you're about to download.

To update your Unreal Engine fork and GDK to the latest version, complete the following steps:

1. In a terminal, change directory to the root of `UnrealEngine`.
1. Run `git pull` to update your Unreal Engine.
1. In a terminal, change directory to the root of `UnrealGDK`.
1. Run `git pull` to update your GDK.
1. Open **File Explorer**, navigate to the root directory of the Unreal GDK repository, and then double-click **`Setup.bat`**. You might be prompted to sign into your SpatialOS account if you have not signed in yet.
1. In **File Explorer**, navigate to the `<GameRoot>` directory that contains your project's `.uproject` file.<br>
Right-click on your `.uproject` file and select **Generate Visual Studio project files**.

You are now on the latest GDK and the latest SpatialOS Unreal Engine fork.

Be sure to join the community on our <a href="https://forums.improbable.io" data-track-link="Join Forums Clicked|product=Docs" target="_blank">forums</a> or on <a href="https://discord.gg/vAT7RSU" data-track-link="Join Discord Clicked|product=Docs|platform=Win|label=Win" target="_blank">Discord</a>. We announce GDK versions there.

-----

_2019-04-15 Page added with editorial review_
