# Cloud development workflow

<!-- This is a live embed of a google drawing -->

<img src="https://docs.google.com/drawings/d/e/2PACX-1vQRmK7TxLji8pT7erPl54hqMMMDsdosZY1OZ2wuPYLQ23dWIrx86qCHggEeq-XasTCsqRe40fCKQvKN/pub?w=758&amp;h=1162">

You may find the following snippets useful as reference:

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
* `<deployment name>`, which labels the deployment in the [Console](https://console.improbable.io).

----

Last updated: April 2019
