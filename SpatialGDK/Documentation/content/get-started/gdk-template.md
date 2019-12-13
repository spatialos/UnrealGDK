
# Get started
## 3 - Set up a project: The Starter Template

If you are ready to start developing your own game with the GDK, follow the steps below to create a new project.

**Note:**</br>

* Before setting up the SpatialOS GDK Starter Template, you _**must**_ follow:

    *  [Get started 1 - Dependencies]({{urlRoot}}/content/get-started/dependencies)
    *  [Get started 2 - Set up the fork and plugin]({{urlRoot}}/content/get-started/build-unreal-fork)

* We recommend setting up the [Example Project]({{urlRoot}}/content/get-started/example-project/exampleproject-intro) before creating a new project. This gives you an overview of the GDK and using SpatialOS.

**Terms used on this page**</br>

* `<GameRoot>` - The directory that contains your project’s .uproject file and Source folder.
* `<ProjectRoot>` - The directory that contains your `<GameRoot>` directory.
* `<YourProject>` - The name of your project and .uproject file (for example, `\<GameRoot>\YourProject.uproject`).

</br>
### 1: Create a new project using the Starter Template

After [building the Unreal Engine fork]({{urlRoot}}/content/get-started/build-unreal-fork), in **File Explorer**, navigate to `UnrealEngine\Engine\Binaries\Win64`and double-click `UE4Editor.exe` to open the Unreal Editor.

1. In the [Project Browser](https://docs.unrealengine.com/en-us/Engine/Basics/Projects/Browser) window, select the **New Project** tab and then the **C++ tab**. 
2. In this tab, select **SpatialOS GDK Starter**. 
3. In the **Folder** field, choose a suitable directory for your project.
4. In the **Name** field, enter a project name of your choice.
5. Select **Create Project**.

**Note:** When you create a project, the Unreal Engine automatically creates a directory named after the project name you entered. This page uses `<YourProject>` as an example project name.

<%(Lightbox image="{{assetRoot}}assets/set-up-template/template-project-browser.png")%>
*Image: The Unreal Engine Project Browser, with the project file path and project name highlighted.*

After you have selected **Create Project**, the Unreal Engine generates the necessary project files and directories, it then closes the Editor and automatically opens Visual Studio. 

After Visual Studio has opened, save your solution then close Visual Studio before proceeding to the Clone the GDK step.

### 2: Clone the GDK

Now you need to clone the SpatialOS GDK for Unreal into your project. To do this: 

1. In **File Explorer**, navigate to the `<GameRoot>` directory and create a `Plugins` folder in this directory.
2. In a Git Bash terminal window, navigate to `<GameRoot>\Plugins` and clone the [GDK for Unreal](https://github.com/spatialos/UnrealGDK) repository by running either:
    * (HTTPS) `git clone https://github.com/spatialos/UnrealGDK.git`
    * (SSH) `git clone git@github.com:spatialos/UnrealGDK.git`

The GDK's [default branch (GitHub documentation)](https://help.github.com/en/articles/setting-the-default-branch) is `release`. This means that, at any point during the development of your game, you can get the latest release of the GDK by running `git pull` inside the `UnrealGDK` directory. When you pull the latest changes, you must also run `git pull` inside the `UnrealEngine` directory, so that your GDK and your Unreal Engine fork remain in sync.

**Note:** You need to ensure that the root directory of the GDK for Unreal repository is called `UnrealGDK` so the file path is: `<GameRoot>\Plugins\UnrealGDK\...`

### 3: Build the dependencies 

To use the Starter Template, you must build the GDK for Unreal module dependencies and then add the GDK to your project. To do this: 

1. Open **File Explorer**, navigate to the root directory of the GDK for Unreal repository (`<GameRoot>\Plugins\UnrealGDK\...`), and double-click **`Setup.bat`**. If you haven't already signed into your SpatialOS account, the SpatialOS developer website may prompt you to sign in. 
1. In **File Explorer**, navigate to your `<GameRoot>` directory, right-click `<YourProject>`.uproject and select Generate Visual Studio Project files.
1. In the same directory, double-click **`<YourProject>`.sln** to open it with Visual Studio.
1. In the Solution Explorer window, right-click on **`<YourProject>`** and select **Build**.
1. When Visual Studio has finished building your project, right-click on **`<YourProject>`** and select **Set as StartUp Project**.
1. Press F5 on your keyboard or select **Local Windows Debugger** in the Visual Studio toolbar to open your project in the Unreal Editor.<br/>
![Visual Studio toolbar]({{assetRoot}}assets/set-up-template/template-vs-toolbar.png)<br/>
_Image: The Visual Studio toolbar_

Note: Ensure that your Visual Studio Solution Configuration is set to **Development Editor**.

### 4: Deploy your project 

To test your game, you need to launch a [deployment]({{urlRoot}}/content/spatialos-concepts/workers-and-zoning#deployments). This means launching your game with its own instance of the [SpatialOS Runtime]({{urlRoot}}/content/glossary#spatialos-runtime), either locally using a [local deployment](https://docs.improbable.io/reference/latest/shared/glossary#local-deployment), or in the cloud using a [cloud deployment](https://docs.improbable.io/reference/latest/shared/glossary#cloud-deployment).

When you launch a deployment, SpatialOS sets up the world based on a [snapshot]({{urlRoot}}/content/spatialos-concepts/schema-and-snapshots#snapshots), and then starts up the [worker instances]({{urlRoot}}/content/spatialos-concepts/workers-and-zoning#worker-instances-and-worker-types) needed to run the game world.

### 5: Deploy locally with multiple clients

Before you launch a deployment (local or cloud) you must generate [schema]({{urlRoot}}/content/spatialos-concepts/schema-and-snapshots#schema) and a [snapshot]({{urlRoot}}/content/spatialos-concepts/schema-and-snapshots#snapshots). 

1. In the Editor, on the [GDK Toolbar]({{urlRoot}}/content/unreal-editor-interface/toolbars), open the **Schema** drop-down menu and select **Schema (Full Scan)**. <br/>
    ![Schema]({{assetRoot}}assets/screen-grabs/toolbar/schema-button-full-scan.png)<br/>
    _Image: On the GDK toolbar in the Editor, select **Schema (Full Scan)**_
    </br>
1. Select **Snapshot** to generate a snapshot.<br/>
![Toolbar]({{assetRoot}}assets/screen-grabs/toolbar/snapshot-button.png)<br/>
_Image: On the GDK toolbar in the Unreal Editor, select **Snapshot**_<br/>

<button class="collapsible">What is Schema?</button>
<div>


Schema is a set of definitions which represent your game’s objects in SpatialOS as entities. Schema is defined in .schema files and written in schemalang by the GDK.</br>
Select **Schema** from the GDK toolbar and the GDK generates schema files and their contents for you, so you do not have to write or edit schema files manually.

You can find out more about schema, including how to generate it from the command line, making schema work with source control, and how to exclude certain directories from schema in the [GDK schema documentation]({{urlRoot}}/content/how-to-use-schema)


</div>

[block:html]
{
  "html": "<button class="collapsible">What is an entity?</button>
<div>

An entity is the SpatialOS equivalent of  an Unreal Actor. It’s made up of a set of SpatialOS components. Each component stores data about the entity. (Note that SpatialOS components are not the same thing as Unreal Actor Components.)

</div>"
}
[/block]


[block:html]
{
  "html": "<button class="collapsible">What is  a snapshot?</button>
<div>


A snapshot is a representation of the state of a SpatialOS world at a given point in time. A snapshot stores the current state of each entity’s component data. You start each deployment with a snapshot; if it’s a re-deployment of an existing game, you can use the snapshot you originally started your deployment with, or use a snapshot that contains the exact state of a deployment before you stopped it.

You can find out more about snapshots in the [GDK snapshot documentation]({{urlRoot}}/content/how-to-use-snapshots).


</div>"
}
[/block]


To launch a local deployment:

1. Select **Start**. This opens a terminal window and starts a local SpatialOS deployment. Wait until you see the output `SpatialOS ready. Access the inspector at http://localhost:21000/inspector` in your terminal window.<br/>
    ![Toolbar]({{assetRoot}}assets/screen-grabs/toolbar/start-button.png)<br/>
    _Image: On the GDK toolbar in the Unreal Editor select **Start**_<br/>

    > **Want to debug your project?**
    > 
    > There is a [known issue](https://github.com/spatialos/UnrealGDK/issues/940) debugging local deployments using C++ or Blueprint breakpoints. For a workaround, follow [these steps]({{urlRoot}}/content/troubleshooting#q-my-worker-instances-are-being-disconnected-from-the-spatialos-runtime-unexpectedly-while-debugging-locally).

1. On the Unreal Editor toolbar, open the **Play** drop-down menu.

1. Under **Multiplayer Options**, set the number of players to **2** and ensure that the check box next to **Run Dedicated Server** is checked. (If it is unchecked, select the checkbox to enable it.)<br/>
    ![]({{assetRoot}}assets/set-up-template/template-multiplayer-options.png)<br/>
    _Image: The Unreal Engine **Play** drop-down menu, with **Multiplayer Options** and **New Editor Window (PIE)** highlighted_<br/>

1. Under **Modes**, select **New Editor Window (PIE)**.<br/>

1. On the Unreal Engine toolbar, select **Play** to run the game, and you should see two clients start.<br/><br/>
    <%(Lightbox image="{{assetRoot}}assets/set-up-template/template-two-clients.png")%><br/>
    _Image: Two clients running in the Editor, with player Actors replicated by SpatialOS and the GDK_<br/>

1. Open the Inspector using the local URL you were given above: `http://localhost:21000/inspector`.</br>  You should see that a local SpatialOS deployment is running with one server-worker instance and two client-worker instances connected. You can also find and follow around the two player entities.<br/><br/>
     <%(Lightbox image="{{assetRoot}}assets/set-up-template/template-two-client-inspector.png")%><br/>
    _Image: The Inspector showing the state of your local deployment_<br/>

2. When you're done, select **Stop** in the GDK toolbar to stop your local SpatialOS deployment.<br/>![Toolbar]({{assetRoot}}assets/screen-grabs/toolbar/stop-button.png)<br/>
    _Image: On the GDK toolbar in the Unreal Editor select **Stop**_<br/>

  <button class="collapsible">What is the Inspector?</button>
<div>
 The Inspector is a browser-based tool that you use to explore the internal state of a game's SpatialOS world. It gives you a real-time view of what’s happening in a local or cloud deployment. <br/>
  The Inspector we are using here is looking at a local deployment running on your computer and not in the cloud, so we use a local URL for the Inspector as it's also running locally on your computer. When running locally, the Inspector automatically downloads and caches the latest Inspector client from the internet. When you use the Inspector in a cloud deployment, you access the Inspector through the Console via the web at https://console.improbable.io.
  
</div>

> **TIP:** Check out the [local deployment workflow page]({{urlRoot}}/content/local-deployment-workflow) for a reference diagram of this workflow.

### 6: Launch a cloud deployment

To launch a cloud deployment, you need to prepare your server-worker and client-worker [assemblies](https://docs.improbable.io/reference/latest/shared/glossary), and upload them to the cloud.        

> **TIP:** Building the assemblies can take a while - we recommend installing <a href="https://www.incredibuild.com/" data-track-link="Incredibuild|product=Docs|platform=Win|label=Win" target="_blank">IncrediBuild</a> to speed up build times.

#### Step 1: Set up your SpatialOS project name 
When you signed up for SpatialOS, your account was automatically associated with an organisation and a project, both of which have the same generated name.

1. Find this name by going to the [Console](https://console.improbable.io). 
    The name should look something like `beta_randomword_anotherword_randomnumber`. In the example below, it’s `beta_yankee_hawaii_621`. <br/>![Toolbar]({{assetRoot}}assets/set-up-template/template-project-page.png)<br/>
    _Image: The SpatialOS Console with a project name highlighted._
1. In File Explorer, navigate to the `<YourProject>/spatial` directory and open the `spatialos.json` file in a text editor of your choice.
1. Replace the `name` field with the project name shown in the Console. This tells SpatialOS which SpatialOS project you intend to upload to.

[block:html]
{
  "html": "<button class="collapsible">What is the Console?</button>
<div>


The Console is a web-based tool for managing cloud deployments. It gives you access to information about your games’ SpatialOS project names, the SpatialOS assemblies you have uploaded, the internal state of any games you have running (via the Inspector), as well as logs and metrics. 

You can find out more about the Console in the [Glossary]({{urlRoot}}/content/glossary#console).


</div>"
}
[/block]


#### Step 2: Build your workers

**Note:** You must close the Unreal Editor before building your workers. If the Editor is open when you try to build your workers the command will fail.

**Note:** Unreal GDK projects default to using Spatial for networking. However, if the `bSpatialNetworking` option is present in your `DefaultGame.ini` configuration file (located in `<ProjectRoot>\Game\Config` directory), ensure that it is set to `True` (as in, `bSpatialNetworking=True`) to enable networking with Spatial for your cloud deployment.

There are two ways to build your worker assemblies (known as “building workers”):

* Build your workers automatically using the `BuildProject.bat` script. </br>
This script automatically builds both the server-workers and client-workers required to run your game in the cloud. It then compresses your workers and saves them as .zip files to the `<ProjectRoot>\spatial\build\assembly\worker` directory. Use this script if you want to build server-workers and client-workers at the same time. <br/><br/>

* Build your workers manually using the command line. </br>
Use the [SpatialOS CLI]({{urlRoot}}/content/glossary#spatialos-command-line-tool-cli) command line to build server-workers and client-workers separately, or to use command-line arguments to build different worker configurations for different purposes.</br>
For example:
 
 * `Editor` to build server-workers to run on local machine for testing.
 * `Linux` to build server-workers to run on SpatialOS cloud servers.
 <!--TODO: Add link to doc on this when it's done here: https://improbableio.atlassian.net/browse/DOC-361 -->

<button class="collapsible">Build your workers using `BuildProject.bat`</button>
<div>

To build your workers using the BuildProject.bat script: 
In File Explorer, navigate to the `<ProjectRoot>` directory.
Double click BuildProject.bat. This opens a command line window and automatically creates your client and server workers. 

</div>

<button class="collapsible">Build your workers  manually using the command line</button>
<div>

In a terminal window, navigate to the `<ProjectRoot>` directory.
Build a server-worker assembly by running the following command: 

[block:code]
{
  "codes": [
  {
      "code": "Game\\Plugins\\UnrealGDK\\SpatialGDK\\Build\\Scripts\\BuildWorker.bat YourProjectServer Linux Development YourProject.uproject",
      "language": "text"
    }
  ]
}
[/block]

Build a client-worker assembly by running the following command: 

[block:code]
{
  "codes": [
  {
      "code": "Game\\Plugins\\UnrealGDK\\SpatialGDK\\Build\\Scripts\\BuildWorker.bat <YourProject> Win64 Development <YourProject>.uproject",
      "language": "text"
    }
  ]
}
[/block]


</div>

**Troubleshooting**
[block:html]
{
  "html": "<button class="collapsible">BuildProject.bat can’t find the path specified</button>
<div>

If you receive the error `The system cannot find the path specified. Builds failed.`, open `ProjectPaths.bat` in a text editor and ensure that `PROJECT_PATH` and `GAME_NAME` are correct. `PROJECT_PATH` needs to be the name of your Unreal project folder (usually Game). `GAME_NAME` needs to be the same name as your Unreal Project `.uproject` file.  

</div>"
}
[/block]


#### Step 3: Upload your workers

Before launching a cloud deployment, you must upload your sever-worker and client-worker assemblies to the cloud. To do this: 

1. In a terminal window, navigate to your `<ProjectRoot>\spatial\` directory 
2. Run the following command:  `spatial cloud upload <assembly_name>`

You must replace `<assembly_name>` with a name for your assembly (for example: `gdktemplateassembly`). 

A valid upload command looks like this:

[block:code]
{
  "codes": [
  {
      "code": "spatial cloud upload myassembly",
      "language": "text"
    }
  ]
}
[/block]

#### Step 4: Launch your cloud deployment
The next step is to launch a cloud deployment using the worker assemblies that you just uploaded. You can only do this through the SpatialOS command-line interface (also known as the “CLI”).

[block:html]
{
  "html": "<button class="collapsible">What is the CLI?</button>
<div>


The SpatilOS command-line tool (CLI) provides a set of commands that you use to interact with a SpatialOS project. Among other functions, you use it to deploy your game. You installed the CLI in step 1, when you set up your dependencies and installed SpatialOS.

Find out more in the [glossary]({{urlRoot}}/content/glossary#spatialos-command-line-tool-cli).

</div>"
}
[/block]


When launching a cloud deployment you must provide four parameters:

* **The snapshot file** -  defines the starting state of the game world
* **The assembly name** - identifies which workers to use for your deployment.
* **A launch configuration file** - defines the SpatialOS game world and load balancing configuration.
* **A name for your deployment** -  labels the deployment in the Console.

[block:html]
{
  "html": "<button class="collapsible">What is a launch configuration file?</button>
<div>


Use this file to list the settings of a deployment. These include: how big the SpatialOS game world is, which worker types SpatialOS must use in the deployment, which worker types can create and delete Actors, and your game template. You installed the Launcher in step 1, when you set up your dependencies and installed SpatialOS.

You can find out more about the launch configuration file in the [glossary]({{urlRoot}}/content/glossary#launch-configuration).

</div>"
}
[/block]


1. In a  terminal window, navigate to `<ProjectRoot>\spatial\` and run the following command

[block:code]
{
  "codes": [
  {
      "code": "spatial cloud launch --snapshot=snapshots\\default.snapshot <assembly_name> one_worker_test.json <deployment_name>",
      "language": "text"
    }
  ]
}
[/block]

Where:

* `default.snapshot` is the snapshot file we have provided for this Example project.
*  `assembly_name` is the name you gave the assembly in the previous step. 
* `one_worker_test.json` is the launch configuration file we provided with the GDK Template
*  `deployment_name` is a name of your choice - you create this name when you run this command. 

A valid launch command looks like this: 

[block:code]
{
  "codes": [
  {
      "code": "spatial cloud launch --snapshot=snapshots/default.snapshot myassembly one_worker_test.json mydeployment",
      "language": "text"
    }
  ]
}
[/block]

### 7. Play your game

![]({{assetRoot}}assets/tutorial/old-console.png)
_Image: The SpatialOS Console_

When your deployment has launched, SpatialOS automatically opens the [Console](https://console.improbable.io) in your browser.

In the Console, Select **Launch** on the left of the page, and then select the **Launch** button that appears in the centre of the page to open the SpatialOS Launcher. The Launcher automatically downloads the game client for this deployment and runs it on your local machine.

[block:html]
{
  "html": "<button class="collapsible">What is the SpatialOS Launcher?</button>
<div>


The Launcher is a distribution tool which downloads and launches game clients for your deployment. You access the Launcher from the Console; use the Console to create a URL to give end-users access to a game client for your game.

Find out more in the [glossary](({{urlRoot}}/content/glossary#launcher).

</div>"
}
[/block]

<br/>
![]({{assetRoot}}assets/tutorial/launch.png)<br/>
_Image: The SpatialOS console launch window_

**Note:** You install the SpatialOS Launcher during [Getting started: 1 - Dependencies]({{urlRoot}}/content/get-started/dependencies).

> **TIP:** Check out the [cloud deployment workflow page]({{urlRoot}}/content/cloud-deployment-workflow) for a reference diagram of this workflow.

</br>
**Congratulations!**

You've successfully set up and launched the Starter Template and the GDK! You are now ready to start developing a game with SpatialOS.

If you have an existing Unreal multiplayer project, follow the detailed [porting guide]({{urlRoot}}/content/tutorials/tutorial-porting-guide) to get it onto the GDK.

<br/>
<br/>------------<br/>
_2019-07-02 Page updated with limited editorial review: added debug workaround_
