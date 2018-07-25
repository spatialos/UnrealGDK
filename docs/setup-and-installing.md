# <b> NOTE: Please add any comments to [this Google doc](https://docs.google.com/document/d/1dPLJOTLD2sN57zCfLVDfoJ5mHB8KoiHlSsWoBnSGJa8/edit#heading=h.kcr0j92iy3dl).</b>

# Set up and get started with the SpatialOS Unreal GDK

## Contents

* [Prerequisites](#prerequisites)
    * [Hardware](#hardware)
    * [Network settings](#network-settings)
    * [Software](#software)
* [Setting up the SpatialOS fork of Unreal Engine](#setting-up-the-spatialos-fork-of-unreal-engine)
    * [Getting the Unreal Engine fork source code](#getting-the-unreal-engine-fork-source-code)
    * [Building Unreal Engine](#building-unreal-engine)
* [Setting up the Unreal GDK module and Sample Game](#setting-up-the-unreal-gdk-module-and-sample-game)
    * [Cloning](#cloning)
    * [Building](#building)
        * [Building for standard development](#building-for-standard-development)
        * [Building for Unreal GDK modification development](#building-for-unreal-gdk-modification-development)
* [Running the Sample Game](#running-the-sample-game)

## Prerequisites

### Hardware
* Refer to the [UE4 recommendations](https://docs.unrealengine.com/en-US/GettingStarted/RecommendedSpecifications) (Unreal documentation)
* Recommended storage: 15GB+ available space

### Network settings
* Refer to the [SpatialOS network settings](https://docs.improbable.io/reference/latest/shared/get-started/requirements#network-settings) (SpatialOS documentation)

### Software
To build the SpatialOS Unreal GDK module you need the following installed:
* Windows 10
* [Git for Windows](https://gitforwindows.org), with Git Bash as your terminal
* [SpatialOS version 13](https://docs.improbable.io/reference/13.0/shared/get-started/setup/win)
* The [Windows SDK 8.1](https://developer.microsoft.com/en-us/windows/downloads/sdk-archive)
* Visual Studio [2015](https://visualstudio.microsoft.com/vs/older-downloads/) or [2017](https://visualstudio.microsoft.com/downloads/)

## Setting up the SpatialOS fork of Unreal Engine

To use the Unreal GDK, you need to build Unreal Engine 4 from source.

### Getting the Unreal Engine fork source code
1. In a Git Bash terminal window, clone the repository and check out the SpatialOS Unreal Engine from our [Unreal Engine fork](https://github.com/improbable/UnrealEngine/tree/UnrealEngine419_SpatialGDK) by running either:
    * (HTTPS) `git clone https://github.com/improbable/UnrealEngine.git -b UnrealEngine419_SpatialGDK`
    * (SSH) `git clone git@github.com:improbable/UnrealEngine.git -b UnrealEngine419_SpatialGDK`
1. Add a system environment variable (**Control Panel > System and Security > System > Advanced system settings > Advanced > Environment variables**) named UNREAL_HOME. The value should be the path to the directory you cloned into in step 1.
 1. Make sure that the new environment variable is registered by restarting your terminal and running `echo $UNREAL_HOME`. This should output the path to the directory you cloned into in step 1.

### Building Unreal Engine

To build Unreal server workers for SpatialOS deployments, you need to build targeting Linux, which requires cross-compilation of your SpatialOS project and Unreal Engine fork.

> Building the Unreal Engine fork from source could take up to a few hours.

1. From the Compiling for Linux setup guide, download and unzip `v11 clang 5.0.0-based - for UE4 4.19`.
1. Add a system environment variable (**Control Panel > System and Security > System > Advanced system settings > Advanced > Environment variables**) named LINUX_MULTIARCH_ROOT.
<br>The value should be the path to the directory you unzipped into in step 2.
1. Make sure that the new environment variable is registered by restarting your terminal and running `echo $LINUX_MULTIARCH_ROOT`.
<br>This should output the path to the directory you unzipped into in step 2.
1. Open **File Explorer** and navigate to the directory you cloned the SpatialOS fork of Unreal Engine into.
1. Double-click **Setup.bat**.
<br>This installs prerequisites for building Unreal Engine 4.
    >  While running the batch file, you should see `Checking dependencies (excluding Mac, Android)...`. If it also says `excluding Linux`, make sure that you set the environment variable `LINUX_MULTIARCH_ROOT` correctly, and run the batch file again.
1. In the same directory, double-click **GenerateProjectFiles.bat**.
<br>This sets up the project files required to build Unreal Engine 4.
        > Note: If you encounter an `error MSB4036: The "GetReferenceNearestTargetFrameworkTask" task was not found` when building with Visual Studio 2017, check that you have NuGet Package Manager installed via the Visual Studio installer.
1. Open **UE4.sln** in Visual Studio.
1. On the toolbar, go to **Build > Configuration Manager** and set your active solution configuration to **Development Editor** and your active solution platform to **Win64**.
1. In the Solution Explorer window, right-click on the **UE4** project and select **Build** (you may be prompted to install some dependencies first).
1. Once the build succeeds, in the Solution Explorer, find **Programs > AutomationTool**. Right-click this project and select Build.
<br>You have now built Unreal Engine 4 for cross-compilation for Linux.
    > Once you've built Unreal Engine, *don't move it into another directory*: that will break the integration.

## Setting up the Unreal GDK module and Sample Game

Follow the steps below to:
* Clone the Unreal GDK and Sample Game repositories.
* Build the Unreal GDK module dependencies which the Sample Game needs so it can work with the GDK, and add the Unreal GDK to the Sample Game.
* Run the Sample Game.

> You need to clone and set up the Sample Game even if you don’t plan to use it. Otherwise, you won’t be able to use the Unreal GDK.

When you reach the **Building** section, there are two options:
* **Build for standard development**
<br> Follow these steps if you want to develop games with the Unreal GDK but do not want to modify the GDK.
* **Build for Unreal GDK modification development**
<br>Follow these steps if you want to modify the Unreal GDK while developing games with it.

### Cloning

1. In a Git Bash terminal window, clone the [Unreal GDK](https://github.com/improbable/unreal-gdk) repository by running either:
    * (HTTPS) `git clone https://github.com/improbable/unreal-gdk.git`
    * (SSH) `git clone git@github.com:improbable/unreal-gdk.git`

1. Clone the [Unreal GDK Sample Game](https://github.com/improbable/unreal-gdk-sample-game/) repository by running either:
    * (HTTPS) `git clone https://github.com/improbable/unreal-gdk-sample-game.git`
    * (SSH) `git clone git@github.com:improbable/unreal-gdk-sample-game.git`

### Building

Build the Unreal GDK module dependencies which the Sample Game needs to work with the GDK and add the Unreal GDK to the Sample Game.

Choose one of the two build options:

#### Building for standard development

1. In a Git Bash terminal window, navigate to the root directory of the Unreal GDK repository you cloned and run `ci/build.sh`.
1. Within the same repository, run `./setup.sh “<path to the directory of the Sample Game you cloned>”`, remembering to enclose the path in quotation marks.
<br>This copies the Binaries, Script, SpatialGDK and Plugins/SpatialGDK directories from the Unreal GDK repository to the correct locations in your Sample Game repository.
1. Open **File Explorer** and navigate to the root directory of the Unreal GDK Sample Game repository you cloned.
1. Go to **Game > Scripts** and double-click **Codegen.bat**.
<br>This initializes the project. It should succeed quickly and silently.
1. Set the Sample Game to work with the SpatialOS fork of UE4 you cloned and installed earlier. To do this:
    1. In File Explorer, navigate to the root directory of the Unreal GDK Sample Game repository you cloned, and then to the **Game** directory within it.
    1. Right-click **SampleGame.uproject** and select **Switch Unreal Engine version**.
    1. Select the path to the SpatialOS Unreal Engine repository you cloned earlier.
    1. Build the SpatialOS Unreal Engine: in a Git Bash terminal window, navigate to the root directory of the Unreal GDK Sample Game repository you cloned, and run `Game/Scripts/Build.bat SampleGameEditor Win64 Development SampleGame.uproject`.

#### Building for Unreal GDK modification development

1. In a Git Bash terminal window, navigate to the root of the Unreal GDK repository you cloned and run `ci/build.sh`. This requires you to authenticate using your SpatialOS account.
1. Navigate to the root directory of the Unreal GDK Sample Game repository you cloned.
1. Create symlinks between the Sample Game and the Unreal GDK by running `./create_gdk_symlink.bat “<path to Unreal GDK root>”`. Remember to enclose the path in quotation marks.
<br> The output should be something like `Successfully created symlinks to “C:\Users\name\Documents\unreal-gdk”`
1. Set the Sample Game to work with the SpatialOS fork of UE4 you cloned and installed earlier. To do this:
    1. In File Explorer, navigate to the root directory of the Unreal GDK Sample Game repository you cloned, and then to the **Game** directory within it.
    1. Right-click **SampleGame.uproject** and select **Switch Unreal Engine version**.
    1. Select the path to the SpatialOS Unreal Engine repository you cloned earlier.
1. Within the root directory of the Sample Game repository you cloned, go to **Game > Scripts** and double-click **Codegen.bat**.
<br>This initializes the project. It should succeed quickly and silently.
1. Build the SpatialOS Unreal Engine: in a Git Bash terminal window, navigate to the root directory of the Unreal GDK Sample Game repository you cloned, and run `Game/Scripts/Build.bat SampleGameEditor Win64 Development SampleGame.uproject`.

### Running the Sample Game

1. In a Git Bash terminal window, navigate to the **spatial** directory within the root directory of the Unreal GDK Sample Game repository you cloned, and run `spatial local launch`.
<br> You should see the output `SpatialOS ready. Access the inspector at http://localhost:21000/inspector`.
1. In the File Explorer, navigate to the root directory of the Unreal GDK Sample Game repository you cloned, and then to the **Game** directory within it, and double-click on **SampleGame.uproject** to open the Unreal Editor.
1. On the Unreal Editor toolbar, select the **Active Play Mode** drop-down.
1. In the drop-down, under Multiplayer Options, enter the number of players as **2** and check the box next to Run Dedicated Server. Then, under Modes, select **New Editor Window (PIE)**.
1. On the toolbar, click **Play** to run the game.

### Gettings started with the GDK and actor replication

Unreal provides a system called Actor replication to make it easy to make a networked game. The SpatialOS Unreal GDK allows you to continue using the native workflow of Unreal without changes to your game code.

To enable actor replication you need to go through two steps:

1. Setup your actor for replication (including property replication and RPCs) as per native unreal workflow.
1. Generate type bindings for your actor (See Setting up Interop Code Generator, which allows the GDK to serialize Unreals replication data to SpatialOS.
