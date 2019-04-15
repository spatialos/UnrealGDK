# Cloud development workflow

The following flowchart provides a reference of the cloud development workflow on the GDK.
 
If you haven't already, please follow the [GDK Starter Template guide]({{urlRoot}}/content/get-started/gdk-template) which provides a detailed explanation of the different steps. 

<!-- This is a live embed of a google drawing -->

<img src="https://docs.google.com/drawings/d/e/2PACX-1vQRmK7TxLji8pT7erPl54hqMMMDsdosZY1OZ2wuPYLQ23dWIrx86qCHggEeq-XasTCsqRe40fCKQvKN/pub?w=758&amp;h=1162">

You may find the following command-line snippets useful as reference:

### Build server-worker assembly

```
Game\Plugins\UnrealGDK\SpatialGDK\Build\Scripts\BuildWorker.bat <YourProject>Server Linux Development <YourProject>.uproject
```

Replacing **`<YourProject>`** with the name of your Unreal project.

### Build client-worker assembly

```
Game\Plugins\UnrealGDK\SpatialGDK\Build\Scripts\BuildWorker.bat <YourProject> Win64 Development <YourProject>.uproject
```

Replacing `<YourProject>` with the name of your Unreal project.

### Upload assembly

```
spatial cloud upload myassembly
```

Replacing `myassembly` with the name you choose to give your assembly.

### Launch Cloud Deployment

```
spatial cloud launch --snapshot=snapshots/default.snapshot <assembly_name> launch_config.json <deployment_name>
```

Providing:

* `<assembly_name>`, which identifies the worker assemblies to use (as chosen in the `spatial cloud upload` command).
* `launch_config.json`, which declares the world and load balancing configuration.
* `<deployment name>`, labels the deployment for SpatialOS to reference in the [Console]({{urlroot}}/content/glossary#console).

----

_2019-04-15 Page added with editorial review_