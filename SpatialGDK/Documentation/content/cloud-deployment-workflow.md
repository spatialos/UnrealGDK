# Cloud deployment workflow

The following flowchart provides a reference of the cloud deployment workflow on the GDK.
 
If you haven't already, please follow the [GDK Starter Template guide]({{urlRoot}}/content/get-started/starter-template/get-started-template-intro) which provides a detailed explanation of the different steps. 

<!-- This is a live embed of a google drawing -->

 <%(Lightbox image="https://docs.google.com/drawings/d/e/2PACX-1vQVcAihbYTNe7TjNsIvkfqIR34Vgw5RESKxboxbvgY5VcgxiI-SZT_M2kuGE8RYMU6sAYWqdkoCjMWt/pub?w=758&h=1162")%>

You may find the following command-line snippets useful as reference:

### Build server-worker assembly

The filepath you use depends on whether you have the `UnrealGDK` plugin set up as an *engine* plugin or as a *project* plugin. If you followed the default setup instructions which use the `InstallGDK.bat` script, you have it set up as an *engine* plugin.

Engine plugin filepath (default):</br>
[block:code]
{
  "codes": [
  {
      "code": "UnrealEngine\Engine\Plugins\UnrealGDK\SpatialGDK\Build\Scripts\BuildWorker.bat <YourProject>Server Linux Development <YourProject>.uproject",
      "language": "text"
    }
  ]
}
[/block]

Project plugin filepath:</br>
[block:code]
{
  "codes": [
  {
      "code": "<YourProject>\Game\Plugins\UnrealGDK\SpatialGDK\Build\Scripts\BuildWorker.bat <YourProject>Server Linux Development <YourProject>.uproject",
      "language": "text"
    }
  ]
}
[/block]

Replacing `<YourProject>` with the name of your Unreal project. 

For more information on the available options when using `BuildWorker.bat`, please see the [Helper scripts reference]({{urlRoot}}/content/apis-and-helper-scripts/helper-scripts).

### Build client-worker assembly

The filepath you use depends on whether you have the `UnrealGDK` plugin set up as an *engine* plugin or as a *project* plugin. If you followed the default setup instructions which use the `InstallGDK.bat` script, you have it set up as an *engine* plugin.

Engine plugin filepath (default):</br>
[block:code]
{
  "codes": [
  {
      "code": "UnrealEngine\Engine\Plugins\UnrealGDK\SpatialGDK\Build\Scripts\BuildWorker.bat <YourProject> Win64 Development <YourProject>.uproject",
      "language": "text"
    }
  ]
}
[/block]

Project plugin filepath:</br>
[block:code]
{
  "codes": [
  {
      "code": "<YourProject>\Game\Plugins\UnrealGDK\SpatialGDK\Build\Scripts\BuildWorker.bat <YourProject> Win64 Development <YourProject>.uproject",
      "language": "text"
    }
  ]
}
[/block]

Replacing `<YourProject>` with the name of your Unreal project.

For more information on the available options when using `BuildWorker.bat`, please see the [Helper scripts reference]({{urlRoot}}/content/apis-and-helper-scripts/helper-scripts).

### Upload assembly

[block:code]
{
  "codes": [
  {
      "code": "spatial cloud upload <myassembly>",
      "language": "text"
    }
  ]
}
[/block]

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
1. (Optional) Create an additional deployment with [simulated players]({{urlRoot}}/content/simulated-players) that connect to your main game deployment. Simulated players are game clients running in the cloud, mimicking real players of your game from a connection flow and server-worker load perspective. This means they’re useful for scale testing. </br></br>
    Build out the simulated player clients (which will run on Linux in the cloud) using the following command:</br></br>
    Engine plugin filepath (default):</br>
    [block:code]
{
  "codes": [
  {
      "code": "    UnrealEngine\Engine\Plugins\UnrealGDK\SpatialGDK\Build\Scripts\BuildWorker.bat <YourProject>SimulatedPlayer Linux Development <YourProject>.uproject \n",
      "language": "text"
    }
  ]
}
[/block]
    Project plugin filepath:</br>
    [block:code]
{
  "codes": [
  {
      "code": "    <YourProject>\Game\Plugins\UnrealGDK\SpatialGDK\Build\Scripts\BuildWorker.bat <YourProject>SimulatedPlayer Linux Development <YourProject>.uproject\n",
      "language": "text"
    }
  ]
}
[/block]
    <%(#Expandable title="Note: disabling game client plugins which don't run on Linux")%>
    Simulated players run on Linux in the cloud. If your game clients use any plugins which don't run on Linux clients, you'll need to exclude them from building. This can be done in your game's Build.cs file, by wrapping any plugins that shouldn't be used on linux clients in a check like
    `if (Target.Platform != UnrealTargetPlatform.Linux)`
  <%(/Expandable)%>
    Then, back in the Editor Deploy window:</br></br>
	1. Check the box next to **Add simulated players**.
	1. In the **Deployment Name** field, enter enter a name for your simulated player  deployment. This labels the deployment in the [Console]({{urlRoot}}/content/glossary#console).
	1. In the **Number of Simulated Players** field, choose the number of simulated players you want to start. 
	1. (Optional) If needed, change the **Region**.</br></br>
1. Click **Launch Deployment**.

Your deployment(s) won’t launch instantly. A console window is displayed where you can see their progress.

When your deployment(s) have launched, you can open the [Console](https://console.improbable.io/) to view them.

#### Using the SpatialOS CLI

To launch a cloud deployment via the CLI, in a terminal window, navigate to `<ProjectRoot>\spatial\` and run:

[block:code]
{
  "codes": [
  {
      "code": "spatial cloud launch --snapshot=snapshots/default.snapshot <myassembly> <launch_config>.json <deployment_name>",
      "language": "text"
    }
  ]
}
[/block]

Where:

* `<myassembly>` identifies the worker assemblies to use (as chosen in the `spatial cloud upload` command).
* `<launch_config>.json` declares the world and load balancing configuration.
* `<deployment_name>` labels the deployment for SpatialOS to reference in the [Console]({{urlRoot}}/content/glossary#console).

<br>
<br/>------<br/>
_2019-11-14 Page updated without editorial review: added callout for plugins which won't run on Linux._<br/>
_2019-10-31 Page updated without editorial review: add missing build step for simulated player clients._<br/>
_2019-07-31 Page updated with limited editorial review_<br/>
_2019-07-21 Page updated with limited editorial review_
