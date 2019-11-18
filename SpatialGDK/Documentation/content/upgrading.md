<%(TOC)%>
# Keep your GDK up to date

To use the SpatialOS GDK for Unreal, you need software from two git repositories:<br>

* [The SpatialOS Unreal Engine fork](https://github.com/improbableio/UnrealEngine)
* [The GDK](https://github.com/spatialos/UnrealGDK)<br>

You download both of these as part of the [Get started]({{urlRoot}}/content/get-started/introduction) steps. <br/>

To ensure you benefit from the most up-to-date functionality, always develop your game on the latest version of the software by regularly updating it. Whenever you update your GDK software, you **must** also update your SpatialOS Unreal Engine fork software. If you don't, you'll encounter errors caused by them being out of sync.

We recommend that you update your version of the GDK and SpatialOS Unreal Engine fork every week.  To do this, follow the steps below.

## Step 1: Decide which branch you want to use.

We publish the GDK and its corresponding SpatialOS Unreal Engine fork on two release branches: `release` and `preview`. The most stable version is `release`, the most up to date is `preview`. You must choose which one you want to use.

> For more information about the different GDK branches and their maturity, see the [Versioning scheme page]({{urlRoot}}/content/pricing-and-support/versioning-scheme).

## Step 2: Check the corresponding Unreal Engine version.

The GDK only supports one version of Unreal Engine at a time. You can tell which version this is by reading the release notes on the [releases page](https://github.com/spatialos/UnrealGDK/releases) of the `UnrealGDK`. Each release states: "The corresponding Engine version is: x.xx-SpatialOSUnrealGDK" (where `x.xx` is the Unreal Engine version of the fork.)

## Step 3: Ensure you're on the branch you want

If you followed our [Get started]({{urlRoot}}/content/get-started/introduction) guide, you already have these repositories cloned on your computer.<br>

You can find out which branch you have checked out by following the instructions below:<br>

1. In a terminal of your choice, change directory to the root of the repository.<br>
1. Run `git status`.<br>

    Your `UnrealEngine` repository should have a branch starting with the correct Unreal Engine version and ending with `-SpatialOSUnrealGDK-release` or `-SpatialOSUnrealGDK-preview` checked out.<br>

    Your `UnrealGDK` repository should have the `release` or `preview` branch checked out.<br>
1. If `git status` returns a different branch, run `git checkout <branch-name>` to check out the branch that you want.

## Step 4: Update your Unreal Engine fork and plugin

Before you update, read the release notes on the [releases page](https://github.com/spatialos/UnrealGDK/releases) of the `UnrealGDK` GitHub so you understand the changes that you're about to download. Pay close attention to the **Breaking Changes** section, as these changes may require you to make changes to your project.

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

## Step 5: Update your clang version, rebuild your Engine and your Project

This step is only required if you checked out an `UnrealEngine` branch with a new Unreal Engine version, for example, if you moved from `4.22-SpatialOSUnrealGDK-release` to `4.23-SpatialOSUnrealGDK-release`.

1. Download and install the [version of clang](https://docs.unrealengine.com/en-US/Platforms/Linux/GettingStarted/index.html) that corresponds to your Engine version.
1. Run `Setup.bat`, which is located in the root directory of the `UnrealEngine` repository.
1. Run `GenerateProjectFiles.bat`, which is in the same root directory.
1. Open **UE4.sln** in Visual Studio and build it.
1. In your project, right click on **.uproject** file and `Generate Project Files`.
1. Open your project's **.sln** in Visual Studio and build it.

Be sure to join the community on our <a href="https://forums.improbable.io" data-track-link="Join Forums Clicked|product=Docs" target="_blank">forums</a> or on <a href="https://discord.gg/vAT7RSU" data-track-link="Join Discord Clicked|product=Docs|platform=Win|label=Win" target="_blank">Discord</a>. We announce GDK versions there.


<br/>------<br/>
_2019-11-14 Page updated with limited editorial review_
