<%(TOC)%>
# The Starter Template

## 3.  Launch a cloud deployment

To launch a cloud deployment, you need to prepare your server-worker and client-worker [assemblies](https://docs.improbable.io/reference/latest/shared/glossary), and upload them to the cloud.

#### Step 1: Set up your SpatialOS project name 

When you signed up for SpatialOS, your account was automatically associated with an organisation and a project, both of which have the same generated name.

1. Find this name by going to the [Console](https://console.improbable.io). 
   The name should look something like `beta_randomword_anotherword_randomnumber`. In the example below, it’s `beta_yankee_hawaii_621`. <br/>![Toolbar]({{assetRoot}}assets/set-up-template/template-project-page.png)<br/>
   _Image: The SpatialOS Console with a project name highlighted._
2. In File Explorer, navigate to the `<YourProject>/spatial` directory and open the `spatialos.json` file in a text editor of your choice.
3. Replace the `name` field with the project name shown in the Console. This tells SpatialOS which SpatialOS project you intend to upload to.

<%(#Expandable title="What is the Console?")%>

The Console is a web-based tool for managing cloud deployments. It gives you access to information about your games’ SpatialOS project names, the SpatialOS assemblies you have uploaded, the internal state of any games you have running (via the Inspector), as well as logs and metrics. 

You can find out more about the Console in the [Glossary]({{urlRoot}}/content/glossary#console).

<%(/Expandable)%>

#### Step 2: Build your workers

**Note:** You must close the Unreal Editor before building your workers. If the Editor is open when you try to build your workers the command will fail.

There are two ways to build your worker assemblies (known as “building workers”):

- Build your workers automatically using the `BuildProject.bat` script. </br>
  This script automatically builds both the server-workers and client-workers required to run your game in the cloud. It then compresses your workers and saves them as .zip files to the `<ProjectRoot>\spatial\build\assembly\worker` directory. Use this script if you want to build server-workers and client-workers at the same time. <br/><br/>
- Build your workers manually using the command line. </br>
  Use the command line when you want to build your server-workers and client-workers separately. 

<%(#Expandable title="Build your workers using `BuildProject.bat`")%>
To build your workers using the BuildProject.bat script: 
In File Explorer, navigate to the `<ProjectRoot>` directory.
Double click BuildProject.bat. This opens a command line window and automatically creates your client and server workers. 
<%(/Expandable)%>

<%(#Expandable title="Build your workers  manually using the command line")%>
In a command line window, navigate to the `<ProjectRoot>` directory.
Build a server-worker assembly by running the following command: 

```
Game\Plugins\UnrealGDK\SpatialGDK\Build\Scripts\BuildWorker.bat YourProjectServer Linux Development YourProject.uproject
```

Build a client-worker assembly by running the following command: 

```
Game\Plugins\UnrealGDK\SpatialGDK\Build\Scripts\BuildWorker.bat <YourProject> Win64 Development <YourProject>.uproject
```

<%(/Expandable)%>

**Troubleshooting**
<%(#Expandable title="BuildProject.bat can’t find the path specified")%>
If you receive the error `The system cannot find the path specified. Builds failed.`, open `ProjectPaths.bat` in a text editor and ensure that `PROJECT_PATH` and `GAME_NAME` are correct. `PROJECT_PATH` needs to be the name of your Unreal project folder (usually Game). `GAME_NAME` needs to be the same name as your Unreal Project `.uproject` file.  
<%(/Expandable)%>

#### Step 3: Upload your workers

Before launching a cloud deployment, you must upload your sever-worker and client-worker assemblies to the cloud. To do this: 

1. In a command line window, navigate to your `<ProjectRoot>\spatial\` directory 
2. Run the following command:  `spatial cloud upload <assembly_name>`

You must replace `<assembly_name>` with a name for your assembly (for example: `gdktemplateassembly`). 

A valid upload command looks like this:

```
spatial cloud upload myassembly
```

#### Step 4: Launch your cloud deployment

The next step is to launch a cloud deployment using the worker assemblies that you just uploaded. 

1. Select the **Deploy** button<br/><br/>
![]({{assetRoot}}assets/toolbar/deploy.png)<br/>_Image: The Deploy button in the GDK toolbar_<br/>
1. Fill out all fields in the **Deploy window**
   * Note: the **assembly name** must be the same name given to the assembly in the previous step
   * Note: the Snapshot File and Launch Config File fields are automatically populated and don't need to be changed

![]({{assetRoot}}assets/toolbar/deploy-settings.png)
<br/>_Image: The Deploy settings_<br/>

<%(#Expandable title="Alternative workflow: launching with the SpatialOS command-line interface")%>

The SpatialOS command-line tool (CLI) provides a set of commands that you use to interact with a SpatialOS project. Among other functions, you use it to deploy your game. You installed the CLI in step 1, when you set up your dependencies and installed SpatialOS. Find out more in the [glossary]({{urlRoot}}/content/glossary#spatialos-command-line-tool-cli).

In a  command line window, navigate to `<ProjectRoot>\spatial\` and run the following command

```
spatial cloud launch --snapshot=snapshots\default.snapshot <assembly_name> one_worker_test.json <deployment_name>
```

Where:

- `default.snapshot` is the snapshot file we have provided for this Example project.
- `<assembly_name>` is the name you gave the assembly in the previous step. 
- `one_worker_test.json` is the launch configuration file we provided with the GDK Template
- `<deployment_name>` is a name of your choice - you create this name when you run this command. 

A valid launch command looks like this: 

```
spatial cloud launch --snapshot=snapshots/default.snapshot myassembly one_worker_test.json mydeployment
```

<%(/Expandable)%>

**> Next:** [4: Play the game]({{urlRoot}}/content/get-started/starter-template/get-started-template-play) 

<br/>

<br/>------------<br/>
*2019-07-22 Page updated with limited editorial review.*<br/>

[//]: # (TODO: https://improbableio.atlassian.net/browse/DOC-1241)