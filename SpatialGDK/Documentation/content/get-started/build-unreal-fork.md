<%(TOC)%>
# Get started 
## 2 - Get and build the SpatialOS Unreal Engine Fork

To use the SpatialOS GDK for Unreal, you need to get the SpatialOS-compatible version of Unreal Engine - this is the SpatialOS Unreal Engine Fork. You get it as source code from GitHub and then build it.

### Step 1: Unreal Engine EULA

To get access to the SpatialOS fork, you need to link your GitHub account to a verified Epic Games account, agree to the Unreal Engine End User License Agreement (EULA) and accept the invite to join the [EpicGames organisation on GitHub](https://github.com/EpicGames). You cannot use the GDK without doing this first. </br>
To do this, see the [Unreal Engine documentation](https://www.unrealengine.com/en-US/ue4-on-github).

### Step 2: Get the Unreal Engine Fork source code and Unreal Linux cross-platform support

1. **Unreal Engine Fork**</br> 
Open a terminal and run either of these commands to clone the [Unreal Engine Fork](https://github.com/improbableio/UnrealEngine) repository.

    > **TIP:** Clone the Unreal Engine fork into your root directory to avoid file path length errors. For example: `C:\GitHub\UnrealEngine`. 

    |     |     |
    | --- | --- |
    | HTTPS | `git clone https://github.com/improbableio/UnrealEngine.git` |
    | SSH |`git clone git@github.com:improbableio/UnrealEngine.git`

2. **Unreal Linux cross-platform support**</br>
To build the server software for SpatialOS deployments correctly, you need to build the Unreal Engine Fork targeting Linux. This requires Linux cross-compilation of your SpatialOS project and the Unreal Engine Fork. To do this, you need to download and unzip the Linux cross-compilation toolchain.</br></br>
For guidance on this, see the _Getting the toolchain_ section of Unreal's [Compiling for Linux](https://wiki.unrealengine.com/Compiling_For_Linux) documentation. As you follow the guidance there, select **v11 clang 5.0.0-based** to download the `v11_clang-5.0.0-centos7.zip` archive, then unzip this file into a suitable directory.

### Step 3: Add environment variables

To build the SpatialOS-compatible version of Unreal Engine, you need to add two [envionment variables](https://en.wikipedia.org/wiki/Environment_variable). Both are system variables; one to set the path to the Unreal Engine Fork directory (`UNREAL_HOME`), and the other to set the path to the Linux cross-compilation toolchain so you have Unreal Linux cross-platform support (`LINUX_MULTIARCH_ROOT`).

1. Open File Explorer and navigate to **Control Panel** > **System and Security** > **System** > **Advanced system settings** > **Advanced** > **Environment variables** to display the Environment Variables dialog box.
1. In the dialog box, select **New...** to create a new system variable named `UNREAL_HOME`.<br/>
Set the variable value as the path to the directory you cloned the Unreal Engine Fork into.
1. Test the variable is set correctly: close and restart your terminal window and run `echo %UNREAL_HOME%` (Command Prompt) or `echo $Env:UNREAL_HOME` (PowerShell). </br> 
If you have registered the system variable correctly, this returns the path to the directory you cloned the Unreal Engine fork into. If it doesn’t, go back to the Environment Variables dialog box via File Explorer and check that you’ve set the environment variable correctly.
1. Back in the Environment Variables dialog box, create another system variable named `LINUX_MULTIARCH_ROOT`. </br>
Set the variable value as the path to the directory of the Linux cross-compilation toolchain you downloaded and unzipped earlier.
1. Test the variable is set correctly: close and restart your terminal window and run `echo %LINUX_MULTIARCH_ROOT%` (Command Prompt) or `echo $Env:LINUX_MULTIARCH_ROOT` (PowerShell). </br>
If you have registered the environment variable correctly, this returns the path you unzipped `v11_clang-5.0.0-centos7.zip` into. If it doesn’t, go back to the Environment Variables dialog box via File Explorer and check that you’ve set the environment variable correctly.

### Step 4: Build Unreal Engine

1. In File Explorer navigate to the directory you cloned the Unreal Engine fork into.

1. Double-click **Setup.bat**.
This installs prerequisites for building Unreal Engine 4.<br>
This process can take a long time to complete.

    > While running the Setup file, you should see `Checking dependencies (excluding Mac, Android)...`. If it also says `excluding Linux`, make sure that you set the environment variable `LINUX_MULTIARCH_ROOT` correctly, and run the Setup file again.

1. In the same directory, double-click **GenerateProjectFiles.bat**. This file automatically sets up the project files you require to build Unreal Engine 4.<br/>

    > If you encounter the message, `error MSB4036: The "GetReferenceNearestTargetFrameworkTask" task was not found` when building with Visual Studio 2017, check that you have the NuGet Package Manager installed via the Visual Studio installer.    

1. In the same directory, open **UE4.sln** in Visual Studio.
1. In Visual Studio, on the toolbar, navigate to **Build** > **Configuration Manager**; set your active solution configuration to **Development Editor** and your active solution platform to **Win64**.
1. In the Solution Explorer window, right-click on the **UE4** project and select **Build** (you may be prompted to install some dependencies first). <br>

Visual Studio then builds Unreal Engine, which can take up to a couple of hours.

You have now built Unreal Engine 4 with cross-compilation for Linux.

> **Note:** Once you've built Unreal Engine, *don't move it into another directory*. That will break the integration.

</br>
</br>
**> Next:** 3 - Set up a project...</br>

Choose either:

* [Set up the Example Project]({{urlRoot}}/content/get-started/example-project/exampleproject-intro) </br>
The Example Project is a session-based FPS game. It gives an overview of the GDK and using SpatialOS, including deploying your game to SpatialOS in the cloud and on your development machine -  useful for testing during development.
* [Set up the Starter Template]({{urlRoot}}/content/get-started/gdk-template) </br>
Use as a base for creating your own project running on SpatialOS.

<br/>
<br/>

------</br>
_2019-05-30 Page updated with editorial review_
