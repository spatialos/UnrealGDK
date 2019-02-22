# Next steps: Set up the SpatialOS GDK Starter Template

Before setting up the SpatialOS GDK Starter Template, you need to have followed:

* [Getting started: 1 - Dependencies]({{urlRoot}}/content/get-started/dependencies)
* [Getting started: 2 - Get and build the SpatialOS Unreal Engine Fork]({{urlRoot}}/content/get-started/tutorial).

If you haven't already, follow our [Multiserver Shooter tutorial]({{urlRoot}}/content/get-started/dependencies) to try out the SpatialOS for Unreal GDK development experience by adding a new feature to a project and testing it across two server-workers.

If you are ready to start developing your own game with the GDK, follow the steps below. 

### Terms used on this page

* `<GameRoot>` - The directory that contains your project’s .uproject file and Source folder.
* `<ProjectRoot>` - The directory that contains your `<GameRoot>` directory.
* `<YourProject>` - The name of your project and .uproject file (for example, `\<GameRoot>\YourProject.uproject`).

### Create a new project using the Starter Template

After [building the Unreal Engine fork]({{urlRoot}}/content/get-started/build-unreal-fork), in **File Explorer**, navigate to `UnrealEngine\Engine\Binaries\Win64`and double-click UE4Editor.exe to open the Unreal Editor. 

1. In the [Project Browser](https://docs.unrealengine.com/en-us/Engine/Basics/Projects/Browser) window, select the **New Project** tab and then the **C++ tab**. 
2. In this tab, select **SpatialOS GDK Starter**. 
3. In the **Folder** field, choose a suitable directory for your project.
4. In the **Name** field, enter a project name of your choice.
5. Select **Create Project**.

**Note:** When you create a project, the Unreal Engine automatically creates a directory named after the project name you entered. This page uses `YourProject` as an example project name.

![The Unreal Engine Project Browser]({{assetRoot}}assets/set-up-template/template-project-browser.png)

*Image: The Unreal Engine Project Browser, with the project file path and project name highlighted.*

After you have selected **Create Project**, the Unreal Engine generates the necessary project files and directories, it then closes the Editor and automatically opens Visual Studio. 

After Visual Studio has opened, save your solution then close Visual Studio before proceeding to the Clone the GDK step.

### Clone the GDK

Now you need to clone the SpatialOS GDK for Unreal into your project. To do this: 

1. In **File Explorer**, navigate to the `<GameRoot>` directory and create a `Plugins` folder in this directory.
2. In a Git Bash terminal window, navigate to `<GameRoot>\Plugins` and clone the [GDK for Unreal](https://github.com/spatialos/UnrealGDK) repository by running either:
    * (HTTPS) `git clone https://github.com/spatialos/UnrealGDK.git`
    * (SSH) `git clone git@github.com:spatialos/UnrealGDK.git`

**Note:** You need to ensure that the root directory of the GDK for Unreal repository is called `UnrealGDK` so the file path is: `<GameRoot>\Plugins\UnrealGDK\...`

### Build the dependencies 

To use the Starter Template, you must build the GDK for Unreal module dependencies and then add the GDK to your project. To do this: 

1. Open **File Explorer**, navigate to the root directory of the GDK for Unreal repository (`<GameRoot>\Plugins\UnrealGDK\...`), and double-click **`Setup.bat`**. If you haven't already signed into your SpatialOS account, the SpatialOS developer website may prompt you to sign in. 
1. In **File Explorer**, navigate to your `<GameRoot>` directory, right-click `<YourProject>`.uproject and select Generate Visual Studio Project files.
1. In the same directory, double-click **`<YourProject>`.sln** to open it with Visual Studio.
1. In the Solution Explorer window, right-click on **`<YourProject>`** and select **Build**.
1. When Visual Studio has finished building your project, right-click on **`<YourProject>`** and select **Set as StartUp Project**.
1. Press F5 on your keyboard or select **Local Windows Debugger** in the Visual Studio toolbar to open your project in the Unreal Editor.<br/>
![Visual Studio toolbar]({{assetRoot}}assets/set-up-template/template-vs-toolbar.png)
_Image: The Visual Studio toolbar_

Note: Ensure that your Visual Studio Solution Configuration is set to **Development Editor**.



### Deploy your project 

To test your project, you can launch a [local deployment (SpatialOS documentation)](https://docs.improbable.io/reference/13.5/shared/glossary#local-deployment) or a [cloud deployment (SpatialOS documentation)](https://docs.improbable.io/reference/13.5/shared/glossary#cloud-deployment).  

#### Deploy locally with multiple clients

To launch a local deployment: 

1. In the Unreal Editor, on the GDK toolbar, select [**Schema**](https://docs.improbable.io/reference/latest/shared/glossary) to generate schema.<br/>
![Toolbar]({{assetRoot}}assets/screen-grabs/toolbar/schema-button.png)<br/>
_Image: On the GDK toolbar in the Unreal Editor select **Schema**_<br/>
1. Select [**Snapshot**]({{urlRoot}}/content/generating-a-snapshot) to generate a snapshot.<br/>
![Toolbar]({{assetRoot}}assets/screen-grabs/toolbar/snapshot-button.png)<br/>
_Image: On the GDK toolbar in the Unreal Editor select **Snapshot**_<br/>
1. Select **Start**. This opens a terminal window and starts a local SpatialOS deployment. Wait until you see the output `SpatialOS ready. Access the inspector at http://localhost:21000/inspector` in your terminal window.<br/>
![Toolbar]({{assetRoot}}assets/set-up-template/template-start.png)<br/>
_Image: On the GDK toolbar in the Unreal Editor select **Start**_<br/>
1. On the Unreal Editor toolbar, open the **Play** drop-down menu.
2. Under **Modes**, select **New Editor Window (PIE)**.<br/>
1. Under **Multiplayer Options**, set the number of players to **2** and ensure that the check box next to **Run Dedicated Server** is checked. (If it is unchecked, select the checkbox to enable it.)<br/>
![]({{assetRoot}}assets/set-up-template/template-multiplayer-options.png)<br/>
_Image: The Unreal Engine **Play** drop-down menu, with **Multiplayer Options** and **New Editor Window (PIE)** highlighted_<br/>
1. On the Unreal Engine toolbar, select **Play** to run the game.<br/>
1. When you're done, select **Stop** in the GDK toolbar to stop your local SpatialOS deployment.<br/>![Toolbar]({{assetRoot}}assets/set-up-template/template-stop.png)<br/>
_Image: On the GDK toolbar in the Unreal Editor select **Stop**_<br/>

If you want to run multiple server-workers in the Editor, see the [Toolbar documentation]({{urlRoot}}/content/toolbars#launching-multiple-pie-server-workers) for details on launching multiple PIE server-workers.

#### Deploy in the cloud

To launch a cloud deployment, you need to prepare your server-worker and client-worker [assemblies](https://docs.improbable.io/reference/latest/shared/glossary), and upload them to the cloud.        

> **TIP:** Building the assemblies can take a while - we recommend installing <a href="https://www.incredibuild.com/" data-track-link="Incredibuild|product=Docs|platform=Win|label=Win" target="_blank">IncrediBuild</a> to speed up build times.

##### Step 1: Set up your SpatialOS project name. 
When you signed up for SpatialOS, your account was automatically associated with an organisation and a project, both of which have the same generated name.

1. Find this name by going to the [Console (SpatialOS Documentation)](https://console.improbable.io). 
The name should look something like `beta_randomword_anotherword_randomnumber`. In the example below, it’s `beta_yankee_hawaii_621`. <br/>![Toolbar]({{assetRoot}}assets/set-up-template/template-project-page.png)<br/>_Image: The SpatialOS Console with a project name highlighted._
2. In File Explorer, navigate to the `<YourProject>/spatial` directory and open the `spatialos.json` file in a text editor of your choice.
3. Replace the `name` field with the project name shown in the Console. This tells SpatialOS which SpatialOS project you intend to upload to.

##### Step 2: Build your worker assemblies

An assembly is what’s created when you run `BuildWorker.bat`. Assemblies are `.zip` files that contain all the files that your game uses when running in the cloud.

**Note:** In the following commands, you must replace **`YourProject`** with the name of your project.
 
1. In a terminal window, navigate to your `<ProjectRoot>` directory.
1. Build a server-worker assembly by running the following command: `Game\Plugins\UnrealGDK\SpatialGDK\Build\Scripts\BuildWorker.bat YourProject Linux Development YourProject.uproject`
1. Build a client-worker assembly by running the following command: `Game\Plugins\UnrealGDK\SpatialGDK\Build\Scripts\BuildWorker.bat YourProject Win64 Development YourProject.uproject`

### Upload your game

1. In a terminal window, navigate to your `<ProjectRoot>\spatial\` directory and run the following command: 
`spatial cloud upload <assembly_name>`<br/>Where `<assembly_name>` is a name of your choice (for example `myassembly`).

A valid upload command looks like this:

```
spatial cloud upload myassembly
```

### Launch a cloud deployment

The next step is to launch a cloud deployment using the assembly that you just uploaded. You can only do this through the SpatialOS command-line interface (also known as the [`spatial` CLI]({{urlRoot}}/content/glossary#spatial-command-line-tool-cli).

When launching a cloud deployment you must provide three parameters:

* **the assembly name**, which identifies the worker assemblies to use.
* **a launch configuration**, which declares the world and load balancing configuration.
* **a name for your deployment**, which labels the deployment in the [Console](https://console.improbable.io).

1. In a  terminal window, navigate to `<ProjectRoot>\spatial\` and run: `spatial cloud launch --snapshot=snapshots/default.snapshot <assembly_name> two_worker_test.json <deployment_name>` 
<br/>where `assembly_name` is the name you gave the assembly in the previous step and `deployment_name` is a name of your choice. A valid launch command would look like this:

```
spatial cloud launch --snapshot=snapshots/default.snapshot myassembly two_worker_test.json mydeployment
```

**Note:** This command defaults to deploying to clusters located in the US. If you’re in Europe, add the `--cluster_region=eu` flag for lower latency.

### Play your game

![]({{assetRoot}}assets/tutorial/console.png)
_Image: The SpatialOS Console_

When your deployment has launched, SpatialOS automatically opens the [Console](https://console.improbable.io) in your browser.

In the Console, Select **Launch** on the left of the page, and then select the **Launch** button that appears in the centre of the page to open the SpatialOS Launcher. The Launcher automatically downloads the game client for this deployment and runs it on your local machine.<br/>
![]({{assetRoot}}assets/tutorial/launch.png)<br/>
_Image: The SpatialOS console launch window_

**Note:** You install the SpatialOS Launcher during [Getting started: 1 - Dependencies]({{urlRoot}}/content/get-started/dependencies).

### Congratulations!

You've successfully set up and launched the Starter Template and the GDK! You are now ready to start developing a game with SpatialOS.

If you have an existing Unreal multiplayer project, follow our detailed [porting guide]({{urlRoot}}/content/get-started/porting-unreal-project-to-gdk.md) to get it onto the GDK.

