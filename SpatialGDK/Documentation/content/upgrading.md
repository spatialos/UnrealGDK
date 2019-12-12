
# Keep your GDK up to date

To use the SpatialOS GDK for Unreal, you need software from two git repositories:<br>

* [The SpatialOS Unreal Engine fork](https://github.com/improbableio/UnrealEngine)
* [The GDK](https://github.com/spatialos/UnrealGDK)<br>
You download both of these as part of the [Get started]({{urlRoot}}/content/get-started/introduction) steps. <br/>
To ensure you benefit from the most up-to-date functionality, always develop your game on the latest version of the software by regularly updating it. Whenever you update your GDK software, you **must** also update your SpatialOS Unreal Engine fork software. If you don't, you might get errors from them being out of synch.

We recommend that you update your version of the GDK and SpatialOS Unreal Engine fork every week.  To do this, follow the steps below.

## Step 1: Ensure you're on the release branches

If you followed our [Get started]({{urlRoot}}/content/get-started/introduction) guide, you have these repositories cloned on your computer.<br>

* Your `UnrealEngine` repository should have the branch ending with `-SpatialOSUnrealGDK-release`checked out.<br>
* Your `UnrealGDK` repository should have the `release` branch checked out.<br>

You can find out which branch you have checked out by following the instructions below:<br>

1. In a terminal of your choice, change directory to the root of the repository.<br>
1. Run `git status`.
This should return `On branch *-SpatialOSUnrealGDK-release` in your `UnrealEngine` repository and `On branch release` in your `UnrealGDK` repository.<br>
If it returns a different branch, run `git checkout <branch-name>` to check out the branch that you want.

> For more information about the different GDK branches and their maturity, see the [Versioning scheme page]({{urlRoot}}/content/pricing-and-support/versioning-scheme).

## Step 2: Update your Unreal Engine fork and plugin

Before you begin, read the release notes on the releases page of the [`UnrealGDK` GitHub](https://github.com/spatialos/UnrealGDK/releases) so you understand the changes that you're about to download.

To update your Unreal Engine fork and GDK to the latest version, complete the following steps:

1. In a terminal, change directory to the root of `UnrealEngine`.
1. Run `git pull` to update your Unreal Engine.
1. In a terminal, change directory to the root of `UnrealGDK`.
1. Run `git pull` to update your GDK.
1. Open **File Explorer**, navigate to the root directory of the Unreal GDK repository, and then double-click **`Setup.bat`**. You might be prompted to sign into your SpatialOS account if you have not signed in yet.
1. In **File Explorer**, navigate to the `<GameRoot>` directory that contains your project's `.uproject` file.<br>
Right-click on your `.uproject` file and select **Generate Visual Studio project files**.
1. In **File Explorer**, navigate to `<GameRoot>\Content\Spatial`and delete `SchemaDatabase.uasset`. This is necessary because some GDK upgrades change how we handle schema, and this sometimes invalidates previously generated schema.

You are now on the latest GDK and the latest SpatialOS Unreal Engine fork.

## Optional: upgrade your clang version 

If you're also upgrading your Unreal Engine version, you need to ensure you have the up to date version of Linux cross-compilation (clang), for SpatialOS to be able to build targeting Linux (for cloud deployments). 

1. Check [which version of clang](https://docs.unrealengine.com/en-US/Platforms/Linux/GettingStarted/index.html) corresponds to your Engine version and download it
1. Ensure the `LINUX_MULTIARCH_ROOT` environment variable is set to the new clang folder
1. Run `GenerateProjectFiles.bat` in the engine
1. Build the Engine
1. In your project, right click on .uproject file and `Generate Project Files` again
1. Build the project


Be sure to join the community on our <a href="https://forums.improbable.io" data-track-link="Join Forums Clicked|product=Docs" target="_blank">forums</a> or on <a href="https://discord.gg/vAT7RSU" data-track-link="Join Discord Clicked|product=Docs|platform=Win|label=Win" target="_blank">Discord</a>. We announce GDK versions there.


<br/>------<br/>
_2019-07-31 Page updated with limited editorial review_
