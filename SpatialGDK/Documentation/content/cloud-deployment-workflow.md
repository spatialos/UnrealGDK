# Cloud deployment workflow

The following flowchart provides a reference of the cloud deployment workflow on the GDK.
 
If you haven't already, please follow the [GDK Starter Template guide]({{urlRoot}}/content/get-started/gdk-template) which provides a detailed explanation of the different steps. 

<!-- This is a live embed of a google drawing -->

<img src="https://docs.google.com/drawings/d/e/2PACX-1vQVcAihbYTNe7TjNsIvkfqIR34Vgw5RESKxboxbvgY5VcgxiI-SZT_M2kuGE8RYMU6sAYWqdkoCjMWt/pub?w=505&h=775">

You may find the following command-line snippets useful as reference:

### Build server-worker assembly

```
Game\Plugins\UnrealGDK\SpatialGDK\Build\Scripts\BuildWorker.bat <YourProject>Server Linux Development <YourProject>.uproject
```

Replacing `<YourProject>` with the name of your Unreal project.

### Build client-worker assembly

```
Game\Plugins\UnrealGDK\SpatialGDK\Build\Scripts\BuildWorker.bat <YourProject> Win64 Development <YourProject>.uproject
```

Replacing `<YourProject>` with the name of your Unreal project.

### Upload assembly

```
spatial cloud upload <myassembly>
```

Replacing `<myassembly>` with the name you choose to give your assembly.

> This command must be run from the `spatial` directory of your project.

### Launch cloud deployment

There are two ways to launch a cloud deployment:

#### Using the Editor toolbar 

Click Deploy in the Spatial editor toolbar and fill out the snapshot, assembly, and deployment name.

![Deploy]({{assetRoot}}assets/toolbar/deploy.png)<br/>

#### Using the Spatial CLI 

```
spatial cloud launch --snapshot=snapshots/default.snapshot <myassembly> <launch_config>.json <deployment_name>
```

Replacing:

* `<myassembly>` - identifies the worker assemblies to use (as chosen in the `spatial cloud upload` command).
* `<launch_config>.json` - declares the world and load balancing configuration.
* `<deployment_name>` - labels the deployment for SpatialOS to reference in the [Console]({{urlRoot}}/content/glossary#console).


<br/>------<br/>
_2019-07-21 Page updated with limited editorial review_
