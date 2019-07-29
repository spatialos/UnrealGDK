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

You can replace `Development` with the [Unreal build configuration](https://docs.unrealengine.com/en-US/Programming/Development/BuildConfigurations/index.html) of your choice (i.e `Editor`, `Test`, `Shipping`). For all options provided by the `BuildWorker.bat`, see the [Helper scripts page]({{urlRoot}}/content/apis-and-helper-scripts/helper-scripts).


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

<%(#Expandable title="Troubleshooting: No upload progress")%>
This step can take a long time on slower connections < 5mbp/s as the full upload size is around ~800mb
If you start your upload and see no progress or extremely slow progress, don't panic.  
There is a known issue with the uploader where progress does not change during upload of large files, you'll notice a big jump as it completes uploads those files
<%(/Expandable)%>

#### Step 4: Launch your cloud deployment

The next step is to launch a cloud deployment using the assembly that you just uploaded. You can do this in the Unreal Editor.

> **Tip:** You can also launch a cloud deployment via the CLI. This is useful if you want to launch cloud deployments as part of continuous integration. For more information, see the generic steps for [launching a cloud deployment]({{urlRoot}}/content/cloud-deployment-workflow#launch-cloud-deployment).

To launch a cloud deployment:

1. On the GDK toolbar, click **Deploy**. <br/><br/>![GDK toolbar "Deploy" button]({{assetRoot}}assets/screen-grabs/toolbar/gdk-toolbar-deploy.png)<br/><br/>
    This opens the cloud deployment dialog box:
    <%(Lightbox title ="Cloud Deployment" image="{{assetRoot}}assets/screen-grabs/cloud-deploy.png")%> <br/>
1. Enter your project name (see [Set up your SpatialOS project name](#step-1-set-up-your-spatialos-project-name)). 
1. In the **Assembly Name** field, enter the name you gave your assembly in the [previous step](#step-3-upload-your-workers).
1. In the **Deployment Name** field, enter a name for your deployment. This labels the deployment in the [Console]({{urlRoot}}/content/glossary#console).
1. Add the absolute path to the **Launch Config File** `~yourprojectname/spatial/one_worker_test.json`  
1. Add the absolute path to the **Snapshot File** `~yourprojectname/spatial/snapshots/default.snapshot`
1. (Optional) If needed, change the **Region**.

#### Optional: Launch Simulated Players

[Simulated players]({{urlRoot}}/content/simulated-players) are game clients running in the cloud, mimicking real players of your game from a connection flow and server-worker load perspective. This means they’re useful for scale testing. 
 
To create an additional deployment with simulated players, in the **Simulated Players** section:

1. Check the box next to **Add simulated players**.
1. In the **Deployment Name** field, enter enter a name for your simulated player  deployment. This labels the deployment in the [Console]({{urlRoot}}/content/glossary#console).
1. In the **Number of Simulated Players** field, choose the number of simulated players you want to start. 
1. (Optional) If needed, change the **Region**.

<%(#Expandable title="Developing Simulated Players")%>

A basic implementation of Simulated Players is included in this project, which you can try out by deploying them and find out by exploring the source (look for `SimulatedPlayerCharacter_BP`). For more information on developing Simulated Players for you project, see the [reference page]({{urlRoot}}/content/simulated-players).

<%(/Expandable)%>

Click **Launch Deployment**.

<%(Callout type="tip" message="You can set default values for all the fields in the Deploy window, using the Cloud section of the [SpatialOS Editor Settings panel]({{urlRoot}}/content/unreal-editor-interface/editor-settings) ")%>

Your deployment(s) won’t launch instantly. A console window is displayed where you can see their progress.

When your deployment(s) have launched, you can open the [Console](https://console.improbable.io/) to view them.

<%(#Expandable title="Cloud workflow reference diagram")%>

 <%(Lightbox image="https://docs.google.com/drawings/d/e/2PACX-1vQVcAihbYTNe7TjNsIvkfqIR34Vgw5RESKxboxbvgY5VcgxiI-SZT_M2kuGE8RYMU6sAYWqdkoCjMWt/pub?w=758&h=1162")%>

For more details, see the [Cloud deployment workflow page]({{urlRoot}}/content/cloud-deployment-workflow).

<%(/Expandable)%>

### **> Next:** [4: Play the game]({{urlRoot}}/content/get-started/starter-template/get-started-template-play) 

<br/>

<br/>------------<br/>
*2019-07-22 Page updated with limited editorial review.*<br/>

[//]: # (TODO: https://improbableio.atlassian.net/browse/DOC-1241)
