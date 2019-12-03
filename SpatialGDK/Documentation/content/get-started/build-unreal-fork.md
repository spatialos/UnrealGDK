<%(TOC max="3")%>

# Get started 
## 2 - Set up the fork and plugin

To use the SpatialOS GDK for Unreal, you first need to download and build the SpatialOS fork of Unreal Engine (UE). You will download and install the GDK for Unreal plugin.

**Note:** If you cloned an earlier version of the fork and plugin, the setup steps below may cause errors. See the [Keep your GDK up to date]({{urlRoot}}/content/upgrading) for guidance on installing the latest versions.

### Step 1: Join the Epic Games organization on GitHub

To get access to the SpatialOS Unreal Engine fork, you need to belong to the Epic Games organization on GitHub. </br>
If you haven't already joined, you need to:

* connect your GitHub account to a verified Epic Games account, 
* agree to the Unreal Engine End User License Agreement (EULA) and 
* accept the invitation to join the Epic Games organization on GitHub.

To do this, see [Unreal's connect accounts documentation](https://www.unrealengine.com/en-US/ue4-on-github). </br>
It only takes a few minutes to set up and includes setting up a GitHub account if you don't already have one.
</br>
</br>
<img src="{{assetRoot}}assets/github404.png" style=" float: right; margin: 10px; display: block; width: 30%; padding: 20px 20x"/>
**Note:** You **must** follow this step to use the GDK. If you have not joined the Epic Games organization on GitHub, the [SpatialOS Unreal Engine fork repository](https://github.com/improbableio/UnrealEngine) returns a GitHub 404 error and you can't download it.

### Step 2: Clone the fork repository

You can clone the fork repository using the command line, GitHub Desktop, or any third-party GUI for Git. This guide shows the process for the command line and GitHub Desktop.

**Note:** Third-party GUIs might ignore the default branch: ensure that you clone the latest stable version of the repository by selecting the branch marked **default** in the **Branch** dropdown of the [SpatialOS Unreal Engine fork repository](https://github.com/improbableio/UnrealEngine).  

<img src="{{assetRoot}}assets/screen-grabs/unreal-fork-repo-branch.png" style="width:250px;">

_Image: The default branch you need to clone._

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

> **Tip:** Clone the Unreal Engine fork into your root directory to avoid file path length errors. For example: `C:\Dev\UnrealEngine`.

### Step 3: Add a new SSH key to your GitHub account

If you have not already configured your GitHub account to use an SSH key, you must do this in order to automatically download the GDK repositories as part of the next setup step.

To do this:

1. Before you generate an SSH key, you can check to see if you have any existing SSH keys by following GitHub's tutorial [Checking for existing SSH keys](https://help.github.com/en/articles/checking-for-existing-ssh-keys).
1. If you don't have an existing key, then generate a new SSH key by following GitHub's tutorial [Adding a new SSH key to your GitHub account](https://help.github.com/en/articles/adding-a-new-ssh-key-to-your-github-account).

### Step 4: Prepare the fork

To prepare the fork: 

1. Run Unreal's `setup.bat` script. </br>
This is part of [Unreal's UE set-up](https://docs.unrealengine.com/en-US/GettingStarted/DownloadingUnrealEngine/index.html); it downloads binary content for UE, as well as installing prerequisites and setting up Unreal file associations. </br>
To do this: </br>
In File Explorer, navigate to the root directory of your clone of the SpatialOS Unreal Engine fork and double-click **Setup.bat**.
2. Run Unreal's `GenerateProjectFiles.bat` script - also part of Unreal's UE set-up. 
</br> To do this: </br>
In the same directory, double-click **GenerateProjectFiles.bat**.

### Step 5: Clone and install the plugin
You need to clone the SpatialOS GDK plugin and install it in the UE fork and Example Project directory. You can follow either auto-install or manual-install, we recommend the auto-install. 

This method will reduce the number of manual steps you have to do to get set up, and will install up the GDK as an Engine plugin rather than a Project plugin. This means you won't have to clone the GDK for each new project you set up.

* **Auto-install** (Recommended) </br>
Still in File Explorer, in the root directory of your clone of the SpatialOS Unreal Engine fork, double-click **InstallGDK.bat**. </br>
This process opens a command line window and runs some scripts - it can take a long time to complete. The command line window closes when the process has finished.

<%(#Expandable title="What does `InstallGDK.bat` do?")%>
The script automatically opens a command line window and performs the following:

* Clones the UnrealGDK into your UE fork's `Plugins` directory 
* Clones the Example Project into your Engine's `Samples` directory.
* Sets up the GDK for use with the Example Project by running `Setup.bat` 
* Generates Visual Studio solution files for the `UnrealGDKExampleProject`.<br/>
<%(/Expandable)%>

* **Manual-install**</br>
See the guide on how to [Manually build the SpatialOS Unreal Engine fork]({{urlRoot}}/content/get-started/manual-engine-build) guide. You do not need to follow _Step 6: Build the fork in Visual Studio_, below.</br></br>

### Step 6: Build the fork in Visual Studio
**Note:** You do not need to follow this step if you followed the manual-install instructions.

Set up Visual Studio to build Unreal Engine; the build can take up to two hours.

1. Still in the root directory of your clone of the SpatialOS Unreal Engine fork, double-click on **UE4.sln** to open it in Visual Studio.
2. In Visual Studio's toolbar, navigate to **Build** > **Configuration Manager**. Here, set your active solution configuration to **Development Editor** and your active solution platform to **Win64**.
3. In Visual Studio's Solution Explorer window, right-click on the **UE4** project and select **Set as StartUp Project**.
4. Still in the Solution Explorer window, right-click on the **UE4** project and select **Build**. (Visual Studio might prompt you to install some dependencies first). <br>

When the build is complete, you can continue to _3 - Set up a project_.


</br>
##### **> Next:** 3 - Set up project

Choose either:

* [Set up the Example Project]({{urlRoot}}/content/get-started/example-project/exampleproject-intro) </br>
The Example Project is a session-based FPS game. It gives an overview of the GDK and using SpatialOS, including deploying your game to SpatialOS locally and in the cloud.
* [Set up the Starter Template]({{urlRoot}}/content/get-started/starter-template/get-started-template-intro) </br>
Use as a base for creating your own project running on SpatialOS.

<br/>

</br>------</br>
_2019-11-29 Page updated with editorial review: added note on repository branch selection._</br>
_2019-11-28 Page updated with editorial review: improve discussion of the plugin and example branches._</br>
_2019-09-27 Page updated without editorial review: clearer explanation of the auto-install flow._</br>
_2019-08-12 Page updated with editorial review: terminology and page formatting._</br>
_2019-08-08 Page updated with editorial review: added clarification on SSH key and Linux dependencies._</br>
_2019-05-30 Page updated with editorial review._
