# Get started: 2 - Get and build the GDK’s Unreal Engine Fork

As the GDK extends Unreal Engine's networking capabilities at its core, to use it you first need to build our SpatialOS fork of Unreal Engine 4 from source.

### Unreal Engine EULA

In order to get access to our fork, you need to link your GitHub account to a verified Epic Games account, agree to the Unreal Engine End User License Agreement ([EULA](https://www.unrealengine.com/en-US/eula)) and accept the invite to join the [EpicGames organisation on Github](https://github.com/EpicGames). You will not be able to use the GDK without doing this first. To do this, see the [Unreal documentation](https://www.unrealengine.com/en-US/ue4-on-github).

### Getting the Unreal Engine fork source code and Unreal Linux cross-platform support
1. In a terminal window, clone the [Unreal Engine fork](https://github.com/improbableio/UnrealEngine/tree/4.20-SpatialOSUnrealGDK) repository. (You may get a 404 from this link. See  the instructions above, under _Unreal Engine EULA_, on how to get access.) <br/>
Check out the fork by running either:
    * (HTTPS) `git clone https://github.com/improbableio/UnrealEngine.git -b 4.20-SpatialOSUnrealGDK`
    * (SSH) `git clone git@github.com:improbableio/UnrealEngine.git -b 4.20-SpatialOSUnrealGDK`
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
1. In Visual Studio, on the toolbar, go to **Build** > **Configuration Manager** and set your active solution configuration to **Development Editor** and your active solution platform to **Win64**.
1. In the Solution Explorer window, right-click on the **UE4** project and select **Build** (you may be prompted to install some dependencies first).</br>
    This builds Unreal Engine, which can take up to a couple of hours.
</br>You have now built Unreal Engine 4 for cross-compilation for Linux.
    > Once you've built Unreal Engine, *don't move it into another directory*: that will break the integration.

#### Next: [Follow the Multiserver Shooter tutorial]({{urlRoot}}/content/get-started/tutorial)  
