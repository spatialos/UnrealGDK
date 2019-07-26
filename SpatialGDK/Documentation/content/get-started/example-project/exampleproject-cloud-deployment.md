<%(TOC)%>
# The Example Project 
## 3: Launch a cloud deployment

Before launching a cloud deployment, as well as generating schema and a snapshot, you also need to:

* Associate your game with its cloud project name.
* Prepare the game’s server-workers and client-workers by building them
* Upload the prepared workers to the cloud.

<%(#Expandable title="What is a worker?")%>

Workers are programs that connect to a SpatialOS world. They perform the computation associated with a world: they can read what’s happening, watch for changes, and make changes of their own. You prepare both your server-workers and client-workers for upload to the cloud by building their assemblies from their `.uproject` files. We call this “building your workers”. 

You can find out more about workers in the [GDK workers documentation]({{urlRoot}}/content/spatialos-concepts/workers-and-zoning)

<%(/Expandable)%>

As you already generated schema and a snapshot for the Example project when you launched a local deployment, and haven’t changed the project since, you don’t need to do this again. If you make any changes to the project, you must generate schema and create a snapshot before launching a local or cloud deployment. 

Everyone who signs up for SpatialOS automatically has free cloud deployment hosting via the free tier, so you can use free tier hosting for this Example project.

<%(#Expandable title="What is the free tier?")%>

The free tier is ideal for starting development, prototyping a game, conducting technical evaluation, or just learning how to use SpatialOS. It gives you the ability to run one cloud deployment at any time using a specified game template. With the free tier you can also use SpatialOS on your local machine, and we offer free support via the forums.

All the hosting options are available as game templates. For information about which game templates you can use on the free tier, and what they provide, see [Pricing details](<https://docs.improbable.io/reference/latest/shared/pricing-and-support/pricing-intro)>."

<%(/Expandable)%>

### Step 1: Associate your game with a cloud project name

When you signed up for SpatialOS, your account was automatically given a SpatialOS cloud organization name and a SpatialOS cloud project name, both of which are the same generated name.

1. Find this name by going to the Console at [https://console.improbable.io](https://console.improbable.io). The name should look something like `beta_randomword_anotherword_randomnumber`. In the example below, it’s `beta_yankee_hawaii_621`. <br/>
   <%(Lightbox image="{{assetRoot}}assets/set-up-template/template-project-page.png")%>
   _Image: The SpatialOS Console with a project name highlighted._</br>

<%(#Expandable title="What is the Console?")%>

The Console is a web-based tool for managing cloud deployments. It gives you access to information about your games’ SpatialOS project names, the SpatialOS assemblies you have uploaded, the internal state of any games you have running (via the Inspector), as well as logs and metrics. 

You can find out more about the Console in the [Glossary]({{urlRoot}}/content/glossary#console).

<%(/Expandable)%>

1. In File Explorer, navigate to the `UnrealGDKExampleProject\spatial` directory and open the `spatialos.json` file in a text editor of your choice.
1. In the file, replace the `name` field with the project name shown in the Console. This associates your SpatialOS cloud project with your Unreal game, telling SpatialOS which cloud project you are uploading your prepared workers to..

### Step 2: Build your workers

<%(Callout type="warn" message="Note: you must close the Unreal Editor before building your workers. If the Editor is open when you try to build your workers the command will fail.")%>

There are two ways to build your worker assemblies (known as “building workers”):  

#### **Option 1**: Build your workers using `BuildProject.bat`

This script automatically builds both the server-workers and client-workers required to run your game in the cloud.  

It then compresses your workers and saves them as .zip files to the `UnrealGDKExampleProject\spatial\build\assembly\worker` directory. Use this script if you want to build server-workers and client-workers at the same time.  

In File Explorer, navigate to the `UnrealGDKExampleProject` directory.
Double click BuildProject.bat. This opens a command line window and automatically creates your client and server workers. 

#### **Option 2**: Build your workers manually using the command line  
Use the command line when you want to build your server-workers and client-workers separately, or, if you want to build different worker configurations, for example: Editor, Test, Shipping or Linux.  
    
In a terminal window, navigate to the UnrealGDKExampleProject directory.

* Build a server-worker assembly by running the following command: <br/>

```
Game\Plugins\UnrealGDK\SpatialGDK\Build\Scripts\BuildWorker.bat GDKShooterServer Linux Development GDKShooter.uproject
```

* Build a client-worker assembly by running the following command: <br/>
 
```
Game\Plugins\UnrealGDK\SpatialGDK\Build\Scripts\BuildWorker.bat GDKShooter Win64 Development GDKShooter.uproject
```

* You can replace `Development` with the [Unreal build configuration](https://docs.unrealengine.com/en-US/Programming/Development/BuildConfigurations/index.html) of your choice (i.e `Editor`, `Test`, `Shipping`). For all options provided by the `BuildWorker.bat`, see the [Helper scripts page]({{urlRoot}}/content/apis-and-helper-scripts/helper-scripts).

**Troubleshooting**
<%(#Expandable title="Reset Spatial networking")%>
You might need to reset [Spatial networking]({{urlRoot}}/content/unreal-editor-interface/toolbars##switching-between-native-unreal-networking-and-spatialos-networking).</br>  
By default, Spatial networking is enabled for Unreal GDK projects, including the Example Project. However, if you have changed the default networking option for this project, you need to reset it to Spatial networking before building workers.</br>
To check which networking your project is using, look in its `DefaultGame.ini` configuration file (located in the `<ProjectRoot>\Game\Config` directory). If there is a `bSpatialNetworking` option in the file, set it to `True` to enable Spatial networking. 
If there is no `SpatialNetworking` option, you do not have to do anything, as the project will default to using Spatial networking.
<%(/Expandable)%>

<%(#Expandable title="BuildProject.bat can’t find the path specified")%>
If you receive the error `The system cannot find the path specified. Builds failed.`, open `ProjectPaths.bat` in a text editor and ensure that `PROJECT_PATH` and `GAME_NAME` are correct. `PROJECT_PATH` needs to be the name of your Unreal project folder (usually Game). `GAME_NAME` needs to be the same name as your Unreal Project `.uproject` file.  
<%(/Expandable)%>


### Step 3: Upload your workers

Before launching a cloud deployment, you must upload your sever-worker and client-worker assemblies to the cloud. To do this: 

1. Open a terminal window and navigate to `\UnrealGDKExampleProject\spatial`.
2. Run the following command: `spatial cloud upload <assembly_name>`.

You must replace `<assembly_name>` with a name for your assembly (for example: `exampleprojectassembly`). 

A valid upload command looks like this:

```
spatial cloud upload exampleprojectassembly
```

<%(#Expandable title="Troubleshooting: No upload progress")%>
This step can take a long time on slower connections < 5mbp/s as the full upload size is around ~800mb
If you start your upload and see no progress or extremely slow progress, don't panic.  
There is a known issue with the uploader where progress does not change during upload of large files, you'll notice a big jump as it completes uploads those files
<%(/Expandable)%>

### Step 4: Launch your cloud deployment

The next step is to launch a cloud deployment using the assembly that you just uploaded. You can do this in the Unreal Editor.

> **Tip:** You can also launch a cloud deployment via the CLI. This is useful if you want to launch cloud deployments as part of continuous integration. For more information, see the generic steps for [launching a cloud deployment]({{urlRoot}}/content/cloud-deployment-workflow#launch-cloud-deployment).

To launch a cloud deployment:

1. On the GDK toolbar, click **Deploy**. <br>![GDK toolbar "Deploy" button]({{assetRoot}}assets/screen-grabs/toolbar/gdk-toolbar-deploy.png)<br/>_Image: The Deploy button in the GDK toolbar_<br/><br/>
    This opens the cloud deployment dialog box.
    <%(Lightbox title ="Cloud Deployment" image="{{assetRoot}}assets/screen-grabs/cloud-deploy.png")%>
    <br/>_Image: The Cloud Deployment settings dialog box_<br/>
1. Enter your project name (see [Set up your SpatialOS project name](#step-1-associate-your-game-with-a-cloud-project-name)). 
1. In the **Assembly Name** field, enter the name you gave your assembly in the [previous step](#step-3-upload-your-workers).
1. In the **Deployment Name** field, enter a name for your deployment. This labels the deployment in the [Console]({{urlRoot}}/content/glossary#console).
1. Leave the Snapshot File field as it is. In the **Launch Config File** field, enter the path to `one_worker_test.json` (including the file name).
1. (Optional) If needed, change the **Region**.
1. (Optional) Create an additional deployment with [simulated players]({{urlRoot}}/content/simulated-players) that connect to your main game deployment. Simulated players are game clients running in the cloud, mimicking real players of your game from a connection flow and server-worker load perspective. This means they’re useful for scale testing. 

    To create an additional deployment with simulated players, in the **Simulated Players** section:
	1. Check the box next to **Add simulated players**.
	1. In the **Deployment Name** field, enter enter a name for your simulated player  deployment. This labels the deployment in the [Console]({{urlRoot}}/content/glossary#console).
	1. In the **Number of Simulated Players** field, choose the number of simulated players you want to start. 
	1. (Optional) If needed, change the **Region**.
1. Click **Launch Deployment**.

Your deployment(s) won’t launch instantly. A console window is displayed where you can see their progress.

When your deployment(s) have launched, you can open the [Console](https://console.improbable.io/) to view them.

</br>
</br>
**> Next:** [4: Play the game]({{urlRoot}}/content/get-started/example-project/exampleproject-play)


<br/>------<br/>
_2019-07-31 Page updated with limited editorial review_
<br>_2019-06-27 Page updated with editorial review_

[//]: # (TODO: https://improbableio.atlassian.net/browse/DOC-1241)