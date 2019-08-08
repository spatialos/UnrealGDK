<%(TOC)%>
# Get started 
## 2 - Get and build the SpatialOS Unreal Engine Fork

To use the SpatialOS GDK for Unreal, you first need to download and build the SpatialOS fork of Unreal Engine.

### Step 1: Unreal Engine EULA

To get access to the SpatialOS Unreal Engine fork, you need to link your GitHub account to a verified Epic Games account, agree to the Unreal Engine End User License Agreement (EULA) and accept the invitation to join the [EpicGames organisation on GitHub](https://github.com/EpicGames). To do this, see the [Unreal Engine documentation](https://www.unrealengine.com/en-US/ue4-on-github).</br>

<%(Callout type="warn" message="This step is required to use the GDK. Without joining the `EpicGames` organisation on GitHub, the [Unreal Engine Fork link](https://github.com/improbableio/UnrealEngine) will return a 404 error and you will not be able to download it.")%>

### Step 2: Clone the Unreal Engine Fork repository

<%(#Expandable title="Using the command line")%>

1. Open a command line window and navigate to a suitable directory to clone the repository to.
1. Run either of these commands to clone the example project repository:

|  |  |
| ----- | ------------------------------------------------------------ |
| HTTPS | `git clone https://github.com/improbableio/UnrealEngine.git` |
| SSH |`git clone git@github.com:improbableio/UnrealEngine.git`|

<%(/Expandable)%>

<%(#Expandable title="Using Github Desktop")%>

1. In GitHub Desktop, select **File** >  **Clone  Repository**.<br/>
1. In the Clone a repository window, select **URL.**<br/>
1. In the Repository URL field, enter this URL: `https://github.com/improbableio/UnrealEngine.git`<br/>
1. In the **Local Path** field, enter a suitable directory path for this repository, or select **Choose…** to select a directory using File Explorer. <br/>
1. Select **Clone**. <br/>
![img]({{assetRoot}}assets/screen-grabs/github-desktop.png)<br/>
<%(/Expandable)%>

> **TIP:** Clone the Unreal Engine Fork into your root directory to avoid file path length errors. For example: `C:\Dev\UnrealEngine`.

### Step 3: Add a new SSH key to your GitHub account

If you have not already configured your GitHub account to use an SSH key, you must do so in order to automatically download the GDK repositories using **InstallGDK.bat** as part of the next setup step.

To do this:

1. Before you generate an SSH key, you can check to see if you have any existing SSH keys by following the GitHub tutorial [Checking for existing SSH keys](https://help.github.com/en/articles/checking-for-existing-ssh-keys).
1. If you don't have an existing key, then generate a new SSH key by following the GitHub tutorial [Adding a new SSH key to your GitHub account (GitHub Documentation)](https://help.github.com/en/articles/adding-a-new-ssh-key-to-your-github-account).

### Step 4: Build the Unreal Engine Fork

To build the Unreal Engine Fork: 

1. Run **Setup.bat**, found in the root directory of your clone of Unreal Engine.
2. In the same directory, double-click **GenerateProjectFiles.bat**. This file automatically sets up the project files required to build Unreal Engine.<br/>
3. Double-click **InstallGDK.bat**
This automatically opens a command line window and performs the following:
	* Clones the UnrealGDK into your Engine's `Plugins` directory
	* Clones the [UnrealGDKExampleProject](https://github.com/spatialos/UnrealGDKExampleProject) into your Engine's `Samples` directory
	* Runs the Unreal GDK `Setup.bat` script to install the GDK into the cloned `UnrealGDKExampleProject` directory
	* Generates Visual Studio solution files for the `UnrealGDKExampleProject`<br/>
This process can take a long time to complete. The command line window closes when the process has finished.    <br/>
1. In the same directory, open **UE4.sln** in Visual Studio.
2. In Visual Studio, on the toolbar, navigate to **Build** > **Configuration Manager**; set your active solution configuration to **Development Editor** and your active solution platform to **Win64**.
3. In the Solution Explorer window, right-click on the **UE4** project and select **Set as StartUp Project**
4. In the Solution Explorer window, right-click on the **UE4** project and select **Build** (you may be prompted to install some dependencies first). <br>

Visual Studio then builds Unreal Engine, which can take up to a couple of hours.

When the build is complete, you can continue to **3: Set up a project...**

</br>

### **> Next:** 3 - Set up project

Choose either:

* [Set up the Example Project]({{urlRoot}}/content/get-started/example-project/exampleproject-intro) </br>
The Example Project is a session-based FPS game. It gives an overview of the GDK and using SpatialOS, including deploying your game to SpatialOS locally and in the cloud.
* [Set up the Starter Template]({{urlRoot}}/content/get-started/starter-template/get-started-template-intro) </br>
Use as a base for creating your own project running on SpatialOS.

<br/>
<br/>

------</br>
_2019-08-08 Page updated with editorial review: added clarification on SSH key and Linux dependencies._
_2019-05-30 Page updated with editorial review._
