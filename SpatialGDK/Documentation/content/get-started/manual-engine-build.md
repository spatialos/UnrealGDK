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

## Step 1: Build Unreal Engine

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

</br>
#### **> Next:** 3 - Set up project

Choose either:

* [Set up the Example Project]({{urlRoot}}/content/get-started/example-project/exampleproject-manual-setup) </br>
The Example Project is a session-based FPS game. It gives an overview of the GDK and using SpatialOS, including deploying your game to SpatialOS locally and in the cloud.
* [Set up the Starter Template]({{urlRoot}}/content/get-started/starter-template/get-started-template-setup-manual) </br>
Use as a base for creating your own project running on SpatialOS.

</br>------</br>
_2019-09-27 Page updated without editorial reivew: remove duplicate clone GDK instructions, link to correct project setup guides._</br>
_2019-08-12 Page updated with editorial review: added to page orientation._

