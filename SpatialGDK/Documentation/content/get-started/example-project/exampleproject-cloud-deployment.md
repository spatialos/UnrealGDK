<%(TOC)%>
# Step 3: Launch a cloud deployment

Before launching a cloud deployment, as well as generating schema and a snapshot, you also need to:

* Associate your game with its cloud project name.
* Prepare the game’s server-workers and client-workers by building them
* Upload the prepared workers to the cloud.

<%(#Expandable title="What is a worker?")%>

Workers are programs that connect to a SpatialOS world. They perform the computation associated with a world: they can read what’s happening, watch for changes, and make changes of their own. You prepare both your server-workers and client-workers for upload to the cloud by building their assemblies from their `.uproject` files. We call this “building your workers”. 

You can find out more about workers in the [GDK workers documentation]({{urlRoot}}/content/spatialos-concepts/workers-and-load-balancing)

<%(/Expandable)%>

As you already generated schema and a snapshot for the Example project when you launched a local deployment, and haven’t changed the project since, you don’t need to do this again. If you make any changes to the project, you must generate schema and create a snapshot before launching a local or cloud deployment. 

Everyone who signs up for SpatialOS automatically has free cloud deployment hosting via the free tier, so you can use free tier hosting for this Example project.

<%(#Expandable title="What is the free tier?")%>

The free tier is ideal for starting development, prototyping a game, conducting technical evaluation, or just learning how to use SpatialOS. It gives you the ability to run one cloud deployment at any time using a specified game template. With the free tier you can also use SpatialOS on your local machine, and we offer free support via the forums.

All the hosting options are available as game templates. For information about which game templates you can use on the free tier, and what they provide, see [Pricing details](<https://docs.improbable.io/reference/latest/shared/pricing-and-support/pricing-intro)>."

<%(/Expandable)%>

## Step 1: Associate your game with a cloud project name

When you signed up for SpatialOS, your account was automatically given a SpatialOS cloud organization name and a SpatialOS cloud project name, both of which are the same generated name.

1. Find this name by going to the Console at [https://console.improbable.io](https://console.improbable.io). The name should look something like `beta_randomword_anotherword_randomnumber`. In the example below, it’s `beta_yankee_hawaii_621`. <br/>
   ![Toolbar]({{assetRoot}}assets/set-up-template/template-project-page.png)
   _Image: The SpatialOS Console with a project name highlighted._</br>

<%(#Expandable title="What is the Console?")%>

The Console is a web-based tool for managing cloud deployments. It gives you access to information about your games’ SpatialOS project names, the SpatialOS assemblies you have uploaded, the internal state of any games you have running (via the Inspector), as well as logs and metrics. 

You can find out more about the Console in the [Glossary]({{urlRoot}}/content/glossary#console).

<%(/Expandable)%>

1. In File Explorer, navigate to the `UnrealGDKExampleProject\Game\spatial` directory and open the `spatialos.json` file in a text editor of your choice.
1. In the file, replace the `name` field with the project name shown in the Console. This associates your SpatialOS cloud project with your Unreal game, telling SpatialOS which cloud project you are uploading your prepared workers to..

## Step 2: Build your workers

**Note:** You must close the Unreal Editor before building your workers. If the Editor is open when you try to build your workers the command will fail.

There are two ways to build your worker assemblies (known as “building workers”):

* Build your workers automatically using the `BuildProject.bat` script. </br>
  This script automatically builds both the server-workers and client-workers required to run your game in the cloud. It then compresses your workers and saves them as .zip files to the `UnrealGDKExampleProject\spatial\build\assembly\worker` directory. Use this script if you want to build server-workers and client-workers at the same time. <br/><br/>
* Build your workers manually using the command line. </br>
  Use the command line when you want to build your server-workers and client-workers separately, or, if you want to build different worker configurations, for example: Editor, Test, Shipping or Linux. 
  <%(#Expandable title="Build your workers using `BuildProject.bat`")%>
  To build your workers using the BuildProject.bat script: <br/>
  In File Explorer, navigate to the `UnrealGDKExampleProject` directory.
  Double click BuildProject.bat. This opens a command line window and automatically creates your client and server workers. 
  <%(/Expandable)%>
  
    <%(#Expandable title="Build your workers  manually using the command line")%>
  In a terminal window, navigate to the UnrealGDKExampleProject directory.
  Build a server-worker assembly by running the following command: <br/>
`Game\Plugins\UnrealGDK\SpatialGDK\Build\Scripts\BuildWorker.bat GDKShooterServer Linux Development GDKShooter.uproject`
  Build a client-worker assembly by running the following command: <br/>
`Game\Plugins\UnrealGDK\SpatialGDK\Build\Scripts\BuildWorker.bat GDKShooter Win64 Development GDKShooter.uproject`
  <%(/Expandable)%>

**Troubleshooting**
<%(#Expandable title="BuildProject.bat can’t find the path specified")%>
If you receive the error `The system cannot find the path specified. Builds failed.`, open `ProjectPaths.bat` in a text editor and ensure that `PROJECT_PATH` and `GAME_NAME` are correct. `PROJECT_PATH` needs to be the name of your Unreal project folder (usually Game). `GAME_NAME` needs to be the same name as your Unreal Project `.uproject` file.  
<%(/Expandable)%>


## Step 3: Upload your workers

Before launching a cloud deployment, you must upload your sever-worker and client-worker assemblies to the cloud. To do this: 

1. Open a terminal window and navigate to `\UnrealGDKExampleProject\spatial`.
1. Run the following command: `spatial cloud upload <assembly_name>`.

You must replace `<assembly_name>` with a name for your assembly (for example: `exampleprojectassembly`). 

A valid upload command looks like this:

```
spatial cloud upload exampleprojectassembly
```

## Step 4: Launch your cloud deployment

The next step is to launch a cloud deployment using the worker assemblies that you just uploaded. You can only do this through the SpatialOS command-line interface (also known as the “CLI”).

<%(#Expandable title="What is the CLI?")%>

The SpatilOS command-line tool (CLI) provides a set of commands that you use to interact with a SpatialOS project. Among other functions, you use it to deploy your game. You installed the CLI in step 1, when you set up your dependencies and installed SpatialOS.

Find out more in the [glossary]({{urlRoot}}/content/glossary#spatialos-command-line-tool-cli).
<%(/Expandable)%>

When launching a cloud deployment you must provide four parameters:

* **The snapshot file** -  defines the starting state of the game world
* **The assembly name** - identifies which workers to use for your deployment.
* **A launch configuration file** - defines the SpatialOS game world and load balancing configuration.
* **A name for your deployment** -  labels the deployment in the Console.

<%(#Expandable title="What is a launch configuration file?")%>

Use this file to list the settings of a deployment. These include: how big the SpatialOS game world is, which worker types SpatialOS must use in the deployment, which worker types can create and delete Actors, and your game template. You installed the Launcher in step 1, when you set up your dependencies and installed SpatialOS.

  You can find out more about the launch configuration file in the [glossary]({{urlRoot}}/content/glossary#launch-configuration).
<%(/Expandable)%>


1. In a terminal window, navigate to `UnrealGDKExampleProject\spatial\` and run the following command

```
spatial cloud launch --snapshot=snapshots\default.snapshot <assembly_name> one_worker_test.json <deployment_name>
```
   Where:

   * `default.snapshot` is the snapshot file provided for this Example project.
   *  `assembly_name` is the name you gave the assembly in the previous step. 
   * `one_worker_test.json` is the launch configuration file provided for this Example project.
   *  `deployment_name` is a name of your choice - you create this name when you run this command. 

A valid launch command looks like this: 

```
spatial cloud launch --snapshot=snapshots\default.snapshot exampleprojectassembly one_worker_test.json mydeployment
```

When your deployment has launched, SpatialOS automatically opens the Console in your browser.

#### Next: [Play the game]({{urlRoot}}/content/get-started/example-project/exampleproject-play)
<br/>--------<br/>

_2019-05-21 Page added with full review_