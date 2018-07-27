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
* Windows 10, with Command Prompt or PowerShell as your terminal
* [Git for Windows](https://gitforwindows.org)
* [SpatialOS version 13](https://docs.improbable.io/reference/13.0/shared/get-started/setup/win)
* The [Windows SDK 8.1](https://developer.microsoft.com/en-us/windows/downloads/sdk-archive)
* Visual Studio [2015](https://visualstudio.microsoft.com/vs/older-downloads/) or [2017](https://visualstudio.microsoft.com/downloads/) (we recommend 2017)

### Other
You need to have an Epic Games account and be able to access the Unreal Engine source code on GitHub. To set this up, see the [Unreal documentation](https://www.unrealengine.com/en-US/ue4-on-github).

INTRO? SUMMARY

## Getting and building the SpatialOS Unreal GDK fork of Unreal Engine

To use the Unreal GDK, you need to build Unreal Engine 4 from source.

### Getting the Unreal Engine fork source code and Unreal Linux cross-platform support
1. In a terminal window, clone the [Unreal Engine fork](https://github.com/improbable/UnrealEngine/tree/UnrealEngine419_SpatialGDK) repository and check out the fork by running either:
    * (HTTPS) `git clone https://github.com/improbable/UnrealEngine.git -b UnrealEngine419_SpatialGDK`
    * (SSH) `git clone git@github.com:improbable/UnrealEngine.git -b UnrealEngine419_SpatialGDK`
1. To build Unreal server workers for SpatialOS deployments, you need to build targeting Linux. This requires cross-compilation of your SpatialOS project and Unreal Engine fork.

    From Unreal's [Compiling for Linux](https://wiki.unrealengine.com/Compiling_For_Linux) setup guide, download and unzip `v11 clang 5.0.0-based - for UE4 4.19`.

### Adding environment variables

You need to add two environment variables: one to set the path to the Unreal Engine fork directory, and another one to set the path to the Linux cross-platform support.

1. Go to **Control Panel > System and Security > System > Advanced system settings > Advanced > Environment variables**.
1. Create a system variable named **UNREAL_HOME**. 
1. Set the variable value to be the path to the directory you cloned the Unreal Engine fork into.
1. Make sure that the new environment variable is registered by restarting your terminal and running `echo %UNREAL_HOME%` (Command Prompt) or `echo $Env:UNREAL_HOME` (PowerShell). If the environment variable is registered correctly, this returns the path to the directory you cloned the Unreal Engine fork into. If it doesn’t, check that you’ve set the environment variable correctly.
1. Create a system variable named **LINUX_MULTIARCH_ROOT**.
1. Set the variable value to be the path to the directory you unzipped `v11 clang 5.0.0-based - for UE4 4.19` into.
1. Make sure that the new environment variable is registered by restarting your terminal and running `echo %LINUX_MULTIARCH_ROOT%` (Command Prompt) or `echo $Env:LINUX_MULTIARCH_ROOT` (PowerShell). 
If the environment variable is registered correctly, this returns the path you unzipped `v11 clang 5.0.0-based - for UE4 4.19` into. If it doesn’t, check that you’ve set the environment variable correctly.



### Building Unreal Engine

> Building the Unreal Engine fork from source could take up to a few hours.

1. Open **File Explorer** and navigate to the directory you cloned the Unreal Engine fork into.
1. Double-click **Setup.bat**.
<br>This installs prerequisites for building Unreal Engine 4, and may take a while.
    >  While running the Setup file, you should see `Checking dependencies (excluding Mac, Android)...`. If it also says `excluding Linux`, make sure that you set the environment variable `LINUX_MULTIARCH_ROOT` correctly, and run the Setup file again.
1. In the same directory, double-click **GenerateProjectFiles.bat**.
<br>This sets up the project files required to build Unreal Engine 4.
        
    > Note: If you encounter an `error MSB4036: The "GetReferenceNearestTargetFrameworkTask" task was not found` when building with Visual Studio 2017, check that you have NuGet Package Manager installed via the Visual Studio installer.
1. In the same directory, open **UE4.sln** in Visual Studio.
1. In Visual Studio, on the toolbar, go to **Build > Configuration Manager** and set your active solution configuration to **Development Editor** and your active solution platform to **Win64**.
1. In the Solution Explorer window, right-click on the **UE4** project and select **Build** (you may be prompted to install some dependencies first).<br>
    This builds Unreal Engine, which can take up to a couple of hours.
1. Once the build succeeds, in the Solution Explorer, find **Programs > AutomationTool**. Right-click this project and select Build. 
<br>You have now built Unreal Engine 4 for cross-compilation for Linux.
    > Once you've built Unreal Engine, *don't move it into another directory*: that will break the integration.

## Setting up the Unreal GDK module and Starter Project

Follow the steps below to:
* Clone the Unreal GDK and Starter Project repositories.
* Build the Unreal GDK module dependencies which the Starter Project needs so it can work with the GDK, and add the Unreal GDK to the Starter Project.

> You need to clone and set up the Starter Project even if you don’t plan to use it. Otherwise, you won’t be able to use the Unreal GDK.

### Cloning

1. In a Git Bash terminal window, clone the [Unreal GDK](https://github.com/improbable/unreal-gdk) repository by running either:
    * (HTTPS) `git clone https://github.com/improbable/unreal-gdk.git`
    * (SSH) `git clone git@github.com:improbable/unreal-gdk.git`

1. Clone the [Unreal GDK Starter Project](https://github.com/improbable/unreal-gdk-sample-game/) repository by running either:
    * (HTTPS) `git clone https://github.com/improbable/unreal-gdk-sample-game.git`
    * (SSH) `git clone git@github.com:improbable/unreal-gdk-sample-game.git`

### Building

> If you want to port an existing Unreal project to the Unreal GDK, follow [this guide](#porting-a-native-unreal-project) instead of continuing to follow these steps. 

Build the Unreal GDK module dependencies which the Starter Project needs to work with the GDK and add the Unreal GDK to the Starter Project.

1. Open **File Explorer**, navigate to the root directory of the Unreal GDK repository, and double-click **BuildGDK.bat**. This requires authorization with your SpatialOS account.
1. Navigate to the root directory of the Unreal GDK Starter Project repository.
1. Create symlinks between the Starter Project and the Unreal GDK:
    1. Open another instance of **File Explorer** and navigate to the root directory of the Starter Project. Within this directory, there’s a batch script named **GenerateGDKSymlinks.bat**.
    1. Drag the Unreal GDK folder onto the batch script **GenerateGDKSymlinks.bat**.<br>This brings up a terminal window, and the output should be something like `Successfully created symlinks to “C:\Users\name\Documents\unreal-gdk”`.<br>For more information on helper scripts, see [Helper scripts](https://github.com/improbable/UnrealGDKStarterProject#helper-scripts) in the Starter Project readme.
1. Within the root directory of the Starter Project repository, go to **Game > Scripts** and double-click **Codegen.bat**. <br>This initializes the project. It should succeed quickly and silently.
1. Set the Starter Project to work with the Unreal Engine fork you cloned and installed earlier. To do this:
    1. In File Explorer, navigate to the root directory of the Unreal GDK Starter Project repository, and then to the **Game** directory within it. 
    1. Right-click **StarterProject.uproject** and select **Switch Unreal Engine version**.
    1. Select the path to the Unreal Engine fork you cloned earlier.
1. Open **StarterProject.sln** in Visual Studio and make sure it’s set as your StartUp Project. 
1. Build the project.
1. Open **StarterProject.uproject** in the Unreal Editor and click [**Codegen**](#interop.md) to generate [type bindings](#glossary). 
1. Close the Unreal Editor and build the project again in Visual Studio.

### Running the Starter Project locally

1. In the Unreal Editor, on the SpatialOS Unreal GDK toolbar, click **Launch**. Wait until you see the output `SpatialOS ready. Access the inspector at http://localhost:21000/inspector`.
1. On the Unreal Editor toolbar, open the **Play** drop-down menu.
1. Under **Multiplayer Options**, enter the number of players as **2** and check the box next to Run **Dedicated Server**. Then, under Modes, select **New Editor Window (PIE)**.
1. On the toolbar, click **Play** to run the game.

### Running the Starter Project in the cloud

TODO add steps

# Next steps

See the [documentation readme]() for guidance on what to set up next.
