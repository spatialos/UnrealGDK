<%(Callout type="warn" message="This [pre-alpha](https://docs.improbable.io/reference/latest/shared/release-policy#maturity-stages) release of the SpatialOS Unreal GDK is for evaluation and feedback purposes only, with limited documentation - see the guidance on [Recommended use]({{urlRoot}}/index#recommended-use)")%>

# Set up and get started with the SpatialOS Unreal GDK

This guide explains how to set up the [SpatialOS Unreal GDK](https://github.com/spatialos/UnrealGDK) and our [Unreal Engine fork](https://github.com/improbableio/UnrealEngine/tree/4.19-SpatialOSUnrealGDK) for use with the [Unreal GDK Starter Project](https://github.com/spatialos/UnrealGDKStarterProject). If you want to use your own native Unreal project, follow the below steps up to and including [Cloning](#cloning), and then follow the [guide to porting your project from Unreal]({{urlRoot}}/content/porting-unreal-project-to-gdk.md).

## Prerequisites

<%(TOC)%>

### Hardware
* Refer to the [UE4 recommendations](https://docs.unrealengine.com/en-US/GettingStarted/RecommendedSpecifications) (Unreal documentation)
* Recommended storage: 15GB+ available space

### Network settings
* Refer to the [SpatialOS network settings](https://docs.improbable.io/reference/latest/shared/get-started/requirements#network-settings) (SpatialOS documentation)

### Software
To build the SpatialOS Unreal GDK module you need the following installed:

* Windows 10, with Command Prompt or PowerShell as your terminal
* [Git for Windows](https://gitforwindows.org)
* [SpatialOS version 13.2](https://console.improbable.io/installer/download/stable/latest/win).
This installs:<br>
    - the [`spatial` CLI](https://docs.improbable.io/reference/latest/shared/spatial-cli-introduction)<br>
    - the SpatialOS [Launcher](https://docs.improbable.io/reference/latest/shared/operate/launcher)<br>
    - the 32-bit and 64-bit Visual C++ Redistributables<br>
* The [Windows SDK 8.1](https://developer.microsoft.com/en-us/windows/downloads/sdk-archive)
* Visual Studio [2015](https://visualstudio.microsoft.com/vs/older-downloads/) or [2017](https://visualstudio.microsoft.com/downloads/) (we recommend 2017). During the installation, make sure you select the following items in the Workloads tab:<br>
    - Universal Windows Platform development<br>
    - Desktop development with C++<br>
    - Game development with C++

### Unreal Engine EULA
As part of your Unreal GDK setup, you need to clone our SpatialOS fork of the Unreal Engine source code on GitHub. In order to get access to our fork, you need to link your GitHub account to a verified Epic Games account, and agree to the Unreal Engine End User License Agreement ([EULA](https://www.unrealengine.com/en-US/eula)). You will not be able to use the Unreal GDK without doing this first. To do this, see the [Unreal documentation](https://www.unrealengine.com/en-US/ue4-on-github).

## Getting and building our SpatialOS Unreal GDK fork of Unreal Engine

To use the Unreal GDK, you need to build our SpatialOS fork of Unreal Engine 4 from source.

### Getting the Unreal Engine fork source code and Unreal Linux cross-platform support
1. In a terminal window, clone the [Unreal Engine fork](https://github.com/improbableio/UnrealEngine/tree/4.19-SpatialOSUnrealGDK) repository. (You may get a 404 from this link. See  the instructions above, under _Unreal Engine EULA_, on how to get access.) <br/>
Check out the fork by running either:
    * (HTTPS) `git clone https://github.com/improbableio/UnrealEngine.git -b 4.19-SpatialOSUnrealGDK`
    * (SSH) `git clone git@github.com:improbableio/UnrealEngine.git -b 4.19-SpatialOSUnrealGDK`
1. To build Unreal server-workers for SpatialOS deployments, you need to build targeting Linux. This requires cross-compilation of your SpatialOS project and the Unreal Engine fork.

    In Unreal's [Compiling for Linux](https://wiki.unrealengine.com/Compiling_For_Linux) documentation, in the **getting the toolchain** section, click v11 **clang 5.0.0-based** to download the archive **v11_clang-5.0.0-centos7.zip** containing the Linux cross-compilation toolchain, then unzip.

### Adding environment variables

You need to add two environment variables: one to set the path to the Unreal Engine fork directory, and another one to set the path to the Linux cross-platform support.

1. Go to **Control Panel > System and Security > System > Advanced system settings > Advanced > Environment variables**.
1. Create a system variable named **UNREAL_HOME**.
1. Set the variable value to be the path to the directory you cloned the Unreal Engine fork into.
1. Make sure that the new environment variable is registered by restarting your terminal and running `echo %UNREAL_HOME%` (Command Prompt) or `echo $Env:UNREAL_HOME` (PowerShell). If the environment variable is registered correctly, this returns the path to the directory you cloned the Unreal Engine fork into. If it doesn’t, check that you’ve set the environment variable correctly.
1. Create a system variable named **LINUX_MULTIARCH_ROOT**.
1. Set the variable value to be the path to the directory of your unzipped Linux cross compilation toolchain.
1. Make sure that the new environment variable is registered by restarting your terminal and running `echo %LINUX_MULTIARCH_ROOT%` (Command Prompt) or `echo $Env:LINUX_MULTIARCH_ROOT` (PowerShell).
If the environment variable is registered correctly, this returns the path you unzipped `v11_clang-5.0.0-centos7.zip` into. If it doesn’t, check that you’ve set the environment variable correctly.

### Building Unreal Engine

> Building the Unreal Engine fork from source could take up to a few hours.

1. Open **File Explorer** and navigate to the directory you cloned the Unreal Engine fork into.
1. Double-click **`Setup.bat`**.
</br>This installs prerequisites for building Unreal Engine 4, and may take a while.
    >  While running the Setup file, you should see `Checking dependencies (excluding Mac, Android)...`. If it also says `excluding Linux`, make sure that you set the environment variable `LINUX_MULTIARCH_ROOT` correctly, and run the Setup file again.
1. In the same directory, double-click **`GenerateProjectFiles.bat`**.
</br>This sets up the project files required to build Unreal Engine 4.

    > Note: If you encounter an `error MSB4036: The "GetReferenceNearestTargetFrameworkTask" task was not found` when building with Visual Studio 2017, check that you have NuGet Package Manager installed via the Visual Studio installer.
1. In the same directory, open **UE4.sln** in Visual Studio.
1. In Visual Studio, on the toolbar, go to **Build > Configuration Manager** and set your active solution configuration to **Development Editor** and your active solution platform to **Win64**.
1. In the Solution Explorer window, right-click on the **UE4** project and select **Build** (you may be prompted to install some dependencies first).</br>
    This builds Unreal Engine, which can take up to a couple of hours.
1. Once the build succeeds, in the Solution Explorer, find **Programs > AutomationTool**. Right-click this project and select Build.
</br>You have now built Unreal Engine 4 for cross-compilation for Linux.
    > Once you've built Unreal Engine, *don't move it into another directory*: that will break the integration.

## Setting up the Unreal GDK module and Starter Project

Follow the steps below to:

* Clone the Starter Project repository.
* Clone the Unreal GDK into the `Plugins` folder of your game.
* Build the Unreal GDK module dependencies which the Starter Project needs so it can work with the GDK.

### Cloning

1. Clone the [Unreal GDK Starter Project](https://github.com/spatialos/UnrealGDKStarterProject/) repository by running either:
    * (HTTPS) `git clone https://github.com/spatialos/UnrealGDKStarterProject.git`
    * (SSH) `git clone git@github.com:spatialos/UnrealGDKStarterProject.git`
1. Navigate into `<StarterProjectRepoRoot>/Game/Plugins`. You may need to create the Plugins folder if you don't have any plugins installed yet.
1.  In a Git Bash terminal window, clone the [Unreal GDK](https://github.com/spatialos/UnrealGDK) repository by running either:
    * (HTTPS) `git clone https://github.com/spatialos/UnrealGDK.git`
    * (SSH) `git clone git@github.com:spatialos/UnrealGDK.git`

**Note** You need to ensure that the root folder of the Unreal GDK repo is called UnrealGDK (`<StarterProjectRepoRoot>/Game/Plugins/UnrealGDK/...`)

### Building

> If you want to port an existing Unreal project to the Unreal GDK, follow [this guide]({{urlRoot}}/content/porting-unreal-project-to-gdk) instead of continuing to follow these steps.

Build the Unreal GDK module dependencies which the Starter Project needs to work with the GDK and add the Unreal GDK to the Starter Project.

1. Open **File Explorer**, navigate to the root directory of the Unreal GDK repository, and double-click **`Setup.bat`**. You may be prompted to sign into your SpatialOS account if you have not already.
1. Set the Starter Project to work with the Unreal Engine fork you cloned and installed earlier. To do this:
    1. In File Explorer, navigate to the root directory of the Unreal GDK Starter Project repository, and then to the **Game** directory within it.
    1. Right-click **StarterProject.uproject** and select **Switch Unreal Engine version**.
    1. Select the path to the Unreal Engine fork you cloned earlier.
1. In the same directory, open **StarterProject.sln** with Visual Studio.
1. In the Solution Explorer window, right-click on **StarterProject** and select **Build**.
1. Open **StarterProject.uproject** in the Unreal Editor and click `Generate Schema` to generate schema.

### Running the Starter Project locally

1. In the Unreal Editor, on the SpatialOS Unreal GDK toolbar, click **Launch**. Wait until you see the output `SpatialOS ready. Access the inspector at http://localhost:21000/inspector`.
1. On the Unreal Editor toolbar, open the **Play** drop-down menu.
1. Under **Multiplayer Options**, enter the number of players as **2** and check the box next to Run **Dedicated Server**. Then, under Modes, select **New Editor Window (PIE)**.
1. On the toolbar, click **Play** to run the game.
1. When you're done, click **Stop** to stop the client worker, and click the **Stop** button in the SpatialOS secion of the toolbar to stop your local SpatialOS deployment.

### Running the Starter Project in the cloud

To run a cloud deployment, you need to prepare your server-worker and client-worker assemblies, and upload them to the cloud.

> Building the assemblies can take a while - we recommend installing IncrediBuild, FastBuild, or another build distributor.

1. Change the name of the project
    1. In File Explorer, navigate to the root directory of the Unreal GDK Starter Project repository, then to **`\spatial`**, and open the `spatialos.json` file in an editor of your choice.
    1. Change the `name` field to the name of your project. You can find this in the [Console](https://console.improbable.io). It’ll be something like `beta_someword_anotherword_000`.
1. In a terminal window, navigate to the root directory of the Unreal GDK Starter Project repository.
1. Build a server-worker assembly: `Game\Plugins\UnrealGDK\SpatialGDK\Build\Scripts\BuildWorker.bat StarterProjectServer Linux Development StarterProject.uproject`
1. Build a client-worker assembly: `Game\Plugins\UnrealGDK\SpatialGDK\Build\Scripts\BuildWorker.bat StarterProject Win64 Development StarterProject.uproject`
1. Navigate to `StarterProject/spatial`.
1. Upload the assemblies to the cloud, specifying an assembly name (this covers both assemblies): `spatial cloud upload <assembly_name>`
1. Launch a deployment, specifying a deployment name: `spatial cloud launch <assembly_name> one_worker_test.json <deployment_name> --snapshot=snapshots\default.snapshot`
1. Follow the steps [here](https://docs.improbable.io/reference/latest/shared/get-started/tour#start-a-game-client) (SpatialOS documentation) to launch the game.

## Next steps

### Setting up Actor replication

Unreal provides a system called [Actor replication](https://docs.unrealengine.com/en-us/Gameplay/Networking/Actors) to make it easy to make a networked game. The SpatialOS Unreal GDK allows you to continue using the native Unreal workflow without changes to your game code. However, you need to do an additional step in order for Actor replication to work with the SpatialOS Unreal GDK.

To set up Actor replication:

1. Set up your Actor for replication (including property replication and RPCs) using the [native Unreal workflow](https://docs.unrealengine.com/en-us/Gameplay/Networking/Actors) (Unreal documentation).
1. Tag the Actors and Subobjects you want to replicate with the `SpatialType` class tag. Then generate `schema` for your project via the `Schema Generator`. This allows the SpatialOS Unreal GDK to serialize Unreal's replication data to SpatialOS.
