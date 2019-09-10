<%(TOC max="2")%>

# Manual-install: UE fork and plugin

When you follow the [Get started]({{urlRoot}}/content/get-started/introduction.md) guide's steps on [2 - Set up the fork and plugin]({{urlRoot}}/content/get-started/build-unreal-fork.md), we recommend you [auto-install]({{urlRoot}}/content/get-started/build-unreal-fork#step-5-clone-and-install-the-plugin) the fork and plugin. </br>

If you prefer to manually install the the UE fork and plugin, follow these instructions to:

*  set up and build the SpatialOS Unreal Engine fork after cloning the [Unreal Engine fork](https://github.com/improbableio/UnrealEngine) repository.
*  clone and install the SpatialOS GDK for Unreal plugin.

> **Tip:** We recommend you use the auto-install option as this makes setting up the Example Project and following tutorials based on the Example Project quicker. If you manually install, you will need to take extra steps to follow tutorials.

## Before starting
You must have followed the [Get started]({{urlRoot}}/content/get-started/introduction.md) guide:

* [1 - Get the dependencies]({{urlRoot}}/content/get-started/dependencies)
* [2 - Set up the fork and plugin]({{urlRoot}}/content/get-started/build-unreal-fork.md) up to [step 5]({{urlRoot}}/content/get-started/build-unreal-fork#step-5-clone-and-install-the-plugin)

## Step 1: Add a LINUX_MULTIARCH_ROOT environment variable

To build the To build the SpatialOS-compatible version of Unreal Engine, you need to add an environment variable to set the path to the Linux cross-compilation toolchain so you have Unreal Linux cross-platform support (`LINUX_MULTIARCH_ROOT`).

1. Open File Explorer and navigate to **Control Panel** > **System and Security** > **System** > **Advanced system settings** > **Advanced** > **Environment variables** to display the Environment Variables dialog box.
1. In the dialog box, select **New...** to create a new system variable named `LINUX_MULTIARCH_ROOT`. </br>
Set the variable value as the path to the directory of the Linux cross-compilation toolchain you downloaded and unzipped earlier.
1. Test the variable is set correctly: close and restart your terminal window and run `echo %LINUX_MULTIARCH_ROOT%` (Command Prompt) or `echo $Env:LINUX_MULTIARCH_ROOT` (PowerShell). </br>
If you have registered the environment variable correctly, this returns the path you unzipped `v11_clang-5.0.0-centos7.zip` into. If it doesn’t, go back to the Environment Variables dialog box via File Explorer and check that you’ve set the environment variable correctly.

## Step 2: Build Unreal Engine

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

## Step 3: Clone and install the SpatialOS GDK for Unreal plugin

You need to add the plugin to your project's plugins folder in order to use SpatialOS.

To do this: 

1. In File Explorer, navigate to the `<YourProject>\Game` directory and create a `Plugins` folder in this directory.
1. In a terminal window, navigate to the `<YourProject>\Game\Plugins` directory and clone the [GDK for Unreal](https://github.com/spatialos/UnrealGDK) repository by running either:

|  |  |
| ----- | ---- |
| HTTPS | `git clone https://github.com/spatialos/UnrealGDK.git` |
| SSH | `git clone git@github.com:spatialos/UnrealGDK.git`|

1. In File Explorer, navigate to the root directory of the GDK for Unreal repository (`<YourProject>\Game\Plugins\UnrealGDK\...`), and double-click `Setup.bat`. If you haven’t already signed into your SpatialOS account, the SpatialOS developer website may prompt you to sign in.

When the build is complete, you can continue to _3 - Set up a project_.


</br>
#### **> Next:** 3 - Set up project

Choose either:

* [Set up the Example Project]({{urlRoot}}/content/get-started/example-project/exampleproject-intro) </br>
The Example Project is a session-based FPS game. It gives an overview of the GDK and using SpatialOS, including deploying your game to SpatialOS locally and in the cloud.
* [Set up the Starter Template]({{urlRoot}}/content/get-started/starter-template/get-started-template-intro) </br>
Use as a base for creating your own project running on SpatialOS.

</br>------</br>
_2019-08-12 Page updated with editorial review: added to page orientation._

