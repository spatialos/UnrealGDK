<%(TOC)%>
# Get started 
## 2 - Get and build the SpatialOS Unreal Engine Fork

To use the SpatialOS GDK for Unreal, you need to get the SpatialOS-compatible version of Unreal Engine - this is the SpatialOS Unreal Engine Fork. You get it as source code from GitHub and then build it.

### Step 1: Unreal Engine EULA

To get access to the SpatialOS fork, you need to link your GitHub account to a verified Epic Games account, agree to the Unreal Engine End User License Agreement (EULA) and accept the invite to join the [EpicGames organisation on GitHub](https://github.com/EpicGames). To do this, see the [Unreal Engine documentation](https://www.unrealengine.com/en-US/ue4-on-github).</br>

<%(Callout type="warn" message="This step is required to use the GDK: without joining the EpicGames organisation on Github, the [Unreal Engine Fork link](https://github.com/improbableio/UnrealEngine) will return a 404 and you will not be able to download it.")%>

### Step 2: Get the Unreal Engine Fork source code

Open a terminal and run either of these commands to clone the [Unreal Engine Fork](https://github.com/improbableio/UnrealEngine) repository.

> **TIP:** Clone the Unreal Engine Fork into your root directory to avoid file path length errors. For example: `C:\GitHub\UnrealEngine`. 

|     |     |
| --- | --- |
| HTTPS | `git clone https://github.com/improbableio/UnrealEngine.git` |
| SSH |`git clone git@github.com:improbableio/UnrealEngine.git`|

### Step 3: Add a new SSH key to your GitHub account

You must add an SSH key to your GitHub account to automatically download the GDK repositories as part of this setup step. 

To do this, follow the GitHub tutorial on [Adding a new SSH key to your GitHub account (GitHub Documentation)](https://help.github.com/en/articles/adding-a-new-ssh-key-to-your-github-account)

### Step 4: Build the Unreal Engine Fork

To build the Unreal Engine Fork: 

1. In File Explorer, navigate to the directory you cloned the Unreal Engine Fork repository into. 
2. Double-click Setup.bat. This automatically opens a command line window and installs prerequisites for building Unreal Engine 4. The command line window closes when the process has finished.
1. Double-click **InstallGDK.bat**
This  automatically opens a command line window and performs the following:
	* Sets LINUX_MULTIARCH_ROOT as an environment variable
	* Clones the UnrealGDK into your Engine's `Plugin` directory
	* Clones the UnrealGDKExampleProject into your Engine's `Samples directory
	* Runs the Unreal GDK Setup.bat script to install the GDK into the cloned UnrealGDKExampleProject directory
	* Generates Visual Studio solution files for the UnrealGDKExampleProject

This process can take a long time to complete. The command line window closes when the process has finished. 
When the command line window closes, you can continue to **3: Set up a project...**

> **Note:** Once you've built Unreal Engine, *don't move it into another directory*. That will break the integration.

If you do not want to build the Engine Fork automatically, follow the instructions for [manually building the Engine Fork]({{urlRoot}}/content/manual-engine-build).

</br>

**> Next:** 3 - Set up project

Choose either:

* [Set up the Example Project]({{urlRoot}}/content/get-started/example-project/exampleproject-intro) </br>
The Example Project is a session-based FPS game. It gives an overview of the GDK and using SpatialOS, including deploying your game to SpatialOS in the cloud and on your development machine -  useful for testing during development.
* [Set up the Starter Template]({{urlRoot}}/content/get-started/starter-template/get-started-template-intro) </br>
Use as a base for creating your own project running on SpatialOS.

<br/>
<br/>

------</br>
_2019-05-30 Page updated with editorial review_
