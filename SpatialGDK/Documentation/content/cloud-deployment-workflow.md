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

You can launch a cloud deployment from the Unreal Editor or via the CLI. Launching via the CLI is useful if you want to launch cloud deployments as part of continuous integration.

#### From the Unreal Editor 

TODO COPY IN ALL STEPS

#### Using the SpatialOS CLI

To launch a cloud deployment via the CLI, in a terminal window, navigate to `<ProjectRoot>\spatial\` and run:

```
spatial cloud launch --snapshot=snapshots/default.snapshot <myassembly> <launch_config>.json <deployment_name>
```

Where:

* `<myassembly>` identifies the worker assemblies to use (as chosen in the `spatial cloud upload` command).
* `<launch_config>.json` declares the world and load balancing configuration.
* `<deployment_name>` labels the deployment for SpatialOS to reference in the [Console]({{urlRoot}}/content/glossary#console).


<br/>------<br/>
_2019-07-21 Page updated with limited editorial review_
