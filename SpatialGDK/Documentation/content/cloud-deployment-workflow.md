# Cloud deployment workflow

The following flowchart provides a reference of the cloud deployment workflow on the GDK.
 
If you haven't already, please follow the [GDK Starter Template guide]({{urlRoot}}/content/get-started/starter-template/get-started-template-intro) which provides a detailed explanation of the different steps. 

<!-- This is a live embed of a google drawing -->

<img src="https://docs.google.com/drawings/d/e/2PACX-1vQVcAihbYTNe7TjNsIvkfqIR34Vgw5RESKxboxbvgY5VcgxiI-SZT_M2kuGE8RYMU6sAYWqdkoCjMWt/pub?w=505&h=775">

You may find the following command-line snippets useful as reference:

### Build server-worker assembly

```
Game\Plugins\UnrealGDK\SpatialGDK\Build\Scripts\BuildWorker.bat <YourProject>Server Linux Development <YourProject>.uproject
```

Replacing `<YourProject>` with the name of your Unreal project. 

For more information on the available options when using `BuildWorker.bat`, please see the [Helper scripts reference]({{urlRoot}}/content/helper-scripts).

### Build client-worker assembly

```
Game\Plugins\UnrealGDK\SpatialGDK\Build\Scripts\BuildWorker.bat <YourProject> Win64 Development <YourProject>.uproject
```

Replacing `<YourProject>` with the name of your Unreal project.

For more information on the available options when using `BuildWorker.bat`, please see the [Helper scripts reference]({{urlRoot}}/content/helper-scripts).

### Upload assembly

```
spatial cloud upload <myassembly>
```

Replacing `<myassembly>` with the name you choose to give your assembly.

> This command must be run from the `spatial` directory of your project.

### Launch cloud deployment

You can launch a cloud deployment using the Unreal Editor or the SpatialOS CLI. Launching via the CLI is useful if you want to launch cloud deployments as part of continuous integration.

#### Using the Unreal Editor 

1. On the GDK toolbar, click **Deploy**. <br>![GDK toolbar "Deploy" button]({{assetRoot}}assets/screen-grabs/toolbar/gdk-toolbar-deploy.png)<br/>_Image: The Deploy button in the GDK toolbar_<br/><br/>
    This opens the cloud deployment dialog box.
    <%(Lightbox title ="Cloud Deployment" image="{{assetRoot}}assets/screen-grabs/cloud-deploy.png")%>
    <br/>_Image: The Cloud Deployment settings dialog box_<br/>
1. Enter your project name. This will be something like `beta_someword_anotherword_000`, and you can find it in the Console.
1. In the **Assembly Name** field, enter the name you gave your assembly.
1. In the **Deployment Name** field, enter a name for your deployment. This labels the deployment in the [Console]({{urlRoot}}/content/glossary#console).
1. Leave the Snapshot File field as it is. In the **Launch Config File** field, enter the path to the launch configuration file for this deployment (including the file name).
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

#### Using the SpatialOS CLI

To launch a cloud deployment via the CLI, in a terminal window, navigate to `<ProjectRoot>\spatial\` and run:

```
spatial cloud launch --snapshot=snapshots/default.snapshot <myassembly> <launch_config>.json <deployment_name>
```

Where:

* `<myassembly>` identifies the worker assemblies to use (as chosen in the `spatial cloud upload` command).
* `<launch_config>.json` declares the world and load balancing configuration.
* `<deployment_name>` labels the deployment for SpatialOS to reference in the [Console]({{urlRoot}}/content/glossary#console).

<br>
<br/>------<br/>
_2019-07-31 Page updated with limited editorial review_
<br>_2019-07-21 Page updated with limited editorial review_
