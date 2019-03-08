<%(TOC)%>
# Get started: 2 - Get and build the SpatialOS Unreal Engine fork

To use the SpatialOS GDK for Unreal, you first need to build the SpatialOS fork of Unreal Engine.

### Step 1: Unreal Engine EULA

To get access to our fork, you need to link your GitHub account to a verified Epic Games account, agree to the Unreal Engine End User License Agreement ([EULA](https://www.unrealengine.com/en-US/eula)) and accept the invite to join the [EpicGames organisation on Github](https://github.com/EpicGames). You cannot use the GDK without doing this first. To do this, see the [Unreal Engine documentation](https://www.unrealengine.com/en-US/ue4-on-github).

### Step 2: Get the Unreal Engine fork source code and Unreal Linux cross-platform support

1\.  Open a terminal and run either of these commands to clone the [Unreal Engine fork](https://github.com/improbableio/UnrealEngine) repository.

> **TIPS:** <br/> * Clone the Unreal Engine fork into your root directory to avoid file path length errors. For example: C:\GitHub\UnrealEngine <br/> * You may get a 404 from this link. See  the instructions above, under _Unreal Engine EULA_, on how to get access to this repository. 

|     |     |
| --- | --- |
| HTTPS | `git clone https://github.com/improbableio/UnrealEngine.git` |
| SSH |`git clone git@github.com:improbableio/UnrealEngine.git`

2\.  To build Unreal server-workers for SpatialOS deployments you need to build the Unreal Engine fork targeting Linux. This requires cross-compilation of your SpatialOS project and the Unreal Engine fork.

In Unreal's [Compiling for Linux](https://wiki.unrealengine.com/Compiling_For_Linux) documentation, in the **getting the toolchain** section, click **v11 clang 5.0.0-based** to download the **v11_clang-5.0.0-centos7.zip** archive, then unzip this file into a suitable directory.

### Step 3: Add environment variables

You need to add two [environment variables](https://docs.microsoft.com/en-us/windows/desktop/procthread/environment-variables): one to set the path to the Unreal Engine fork directory, and another one to set the path to the Linux cross-platform support directory.

1. Go to **Control Panel > System and Security > System > Advanced system settings > Advanced > Environment variables**.
2. Create a system variable named **UNREAL_HOME**.
3. Set the variable value to the path to the directory you cloned the Unreal Engine fork into.
4. Restart your terminal and run `echo %UNREAL_HOME%` (Command Prompt) or `echo$Env:UNREAL_HOME` (PowerShell). If you have registered the environment variable correctly, this returns the path to the directory you cloned the Unreal Engine fork into. If it doesn’t, check that you’ve set the environment variable correctly.
5. Create a system variable named **LINUX_MULTIARCH_ROOT**.
6. Set the variable value to the path to the directory of your unzipped Linux cross compilation toolchain.
7. Restart your terminal and run `echo %LINUX_MULTIARCH_ROOT%` (Command Prompt) or `echo $Env:LINUX_MULTIARCH_ROOT` (PowerShell). If you have registered the environment variable correctly, this returns the path you unzipped `v11_clang-5.0.0-centos7.zip` into. If it doesn’t, check that you’ve set the environment variable correctly.

### Step 4: Build Unreal Engine

1. Open **File Explorer** and navigate to the directory you cloned the Unreal Engine fork into.

2. Double-click **`Setup.bat`**.

This installs prerequisites for building Unreal Engine 4. This process can take a long time to complete.

> While running the Setup file, you should see `Checking dependencies (excluding Mac, Android)...`. If it also says `excluding Linux`, make sure that you set the environment variable `LINUX_MULTIARCH_ROOT` correctly, and run the Setup file again.

1. In the same directory, double-click **`GenerateProjectFiles.bat`**.

This file automatically sets up the project files you require to build Unreal Engine 4.

> If you encounter an `error MSB4036: The "GetReferenceNearestTargetFrameworkTask" task was not found` when building with Visual Studio 2017, check that you have NuGet Package Manager installed via the Visual Studio installer.

2. In the same directory, open **UE4.sln** in Visual Studio.

3. In Visual Studio, on the toolbar, go to **Build** > **Configuration Manager** and set your active solution configuration to **Development Editor** and your active solution platform to **Win64**.

4. In the Solution Explorer window, right-click on the **UE4** project and select **Build** (you may be prompted to install some dependencies first). <br>

Visual Studio then builds Unreal Engine, which can take up to a couple of hours.

You have now built Unreal Engine 4 with cross-compilation for Linux.

> Once you've built Unreal Engine, *don't move it into another directory*. That will break the integration.

#### Next: [Follow the Multiserver Shooter tutorial]({{urlRoot}}/content/get-started/tutorial)  
