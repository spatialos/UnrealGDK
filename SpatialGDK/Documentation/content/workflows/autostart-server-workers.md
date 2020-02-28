# Auto-start server-workers

> Out of the box, the GDK for Unreal uses a single Unreal server-worker type to handle all the game’s server-side computation. This document assumes you are using the default single Unreal server-worker type in your project.

To debug both server-workers and game clients direct from Visual Studio, you first need to follow the guide to [set up Visual Studio]({{urlRoot}}/content/workflows/set-up-vs). To debug game clients, you also need to up the server-worker type in your project to auto-start in a local deployment. This ensures that when you run the game client you are debugging, the local [SpatialOS Runtime]({{urlRoot}}/content/glossary#spatialos-runtime) starts up an instance of the server-worker type to do the server-side computation in a local deployment.

**Notes:**

* You never need to set up game clients to auto-start.
* You only have to set up a server-worker type to auto-start in order to support debugging game clients from Visual Studio.
* The ExampleProject already incorporates this flow. If you are using the ExampleProject as a base, these steps have already been completed.

#### Summary of steps

You only need to set up your server-worker type in your project once. There are four steps to do this:

1. Build the project’s server-worker from the command line using a specific flag
1. Set up the project’s launch configuration
1. Generate [schema]({{urlRoot}}/content/glossary#schema) and generate a [snapshot]({{urlRoot}}/content/glossary#snapshot)
1. Run the launch command (as you would to start any local deployment).

## 1. Build your server worker

<%(Callout type="warn" message="Note: you must close the Unreal Editor before building your server worker. If the Editor is open when you try to build your worker the command will fail.")%>

When you are using a local deployment, you don’t usually need to build your project’s server-worker, but to set up the project so the Editor launches a server-worker instance automatically when you are debugging from Visual Studio, you need to build the server-worker once, using a specific flag.

To do this, in a terminal window, use the following command from the root folder of your project (where `<YourGame>` is the name of your Unreal project:

```
Game\Plugins\UnrealGDK\SpatialGDK\Build\Scripts\BuildWorker.bat <YourGame>Editor Win64 Development <YourGame>.uproject
```

This creates a zip file at: `spatial/build/assembly/worker/UnrealEditor@Windows.zip` which contains the file, `StartEditor.bat`.

When you launch a local deployment the local SpatialOS Runtime uses the configuration at `spatial/workers/unreal/spatialos.UnrealWorker.worker.json` to run `StartEditor.bat` with the necessary arguments to launch a server-worker instance and connect it to the local SpatialOS Runtime.

## 2. Set up launch configuration
You can do this via the Unreal Editor or outside the Editor by manually editing your project’s `.json` launch file.

### Using the default in Unreal Editor
If you are using the default generated launch configuration which comes out of the box with the GDK, follow these steps to enable server-worker types:

1. Open the [SpatialOS Editor Settings panel]({{urlRoot}}/content/unreal-editor-interface/editor-settings)
1. Select **Launch** and then **Launch configuration file options** to open the drop-down menu.
1. Locate **Server Workers** and the find **Worker Type Name** you would like the local SpatialOS Runtime to start. This is likely to be `UnrealWorker`.
1. Locate the **Manual worker connection only** check box and select it, so it’s on.
1. Ensure  **Rectangle grid column count** and **Rectangle grid row count** for that worker type are both set to _at least_ `1`.  This ensures that the SpatialOS Runtime starts at least one worker instance of that worker type.

### Manually outside Unreal Editor

If you are using a launch configuration which you have manually defined in a file, outside of the GDK’s default generated launch configuration, make the following changes:

1. In File Explorer or from a terminal window, locate the `.json` launch file (default filename `default_launch.json`) in the `<YourGame>\spatial` directory.
1. Open the file with an editor of your choice and locate the `"load_balancing"` and `"layer_configurations"` section.
1. Find the listing for the worker type that you would like the local SpatialOS Runtime to start. </br> This is likely to be `“layer”: “UnrealWorker”`.
1. Set `“cols”` and `“rows”` to `1` and `"manual_worker_connection_only”` to `“false”`. </br> this is likely to look like the example below.

**Example launch configuration `.json` file**</br>

```
"load_balancing": {
    "layer_configurations": [
      {
          "layer": "UnrealWorker",
          "rectangle_grid": {
              "cols": 1,
              "rows": 1
          },
          "options": {
            "manual_worker_connection_only": false
        }
      }
    ]
  }


```
## 3. Generate schema and a snapshot

Generate schema and a snapshot as you would before any deployment launch.

## 4. Launch your game in a local deployment

You can now launch game clients or server-worker instances direct from Visual Studio.

However, now you have set your project up so its server-worker type auto-starts an instance on deployment, you can no longer launch a local deployment and a server-worker instance from the Editor by selecting **Play**.

To work around this, you can launch a local deployment from the SpatialOS CLI and the local SpatialOS Runtime will auto-launch a server-worker instance.

### Launch from the CLI

From a terminal window, use the following command from the root folder of your project:  `spatial local launch`.

The command assumes you are using the default file name `default_launch.json` for your launch configuration file. You can add the launch configuration file name as an argument after the command if you are not using the default file; `spatial local launch MyLaunchConfig.json` (where `MyLaunchConfig.json` is the project’s launch configuration file name.)

See the [CLI documentation](https://docs.improbable.io/reference/latest/shared/spatial-cli/spatial-local-launch#spatial-local-launch) for more information.)

### Tips on launching game clients

You could launch a game client from the Editor by unselecting the **Run dedicated server** checkbox in the Editor’s **Play** options, but this starts up a client that only acts as a listen server for other clients which isn’t useful for testing. </br>

You can launch game clients from a console window: From the root directory of your project, run a version of the  `LaunchClient.bat` script provided in the [Example Project]({{urlRoot}}//content/get-started/example-project/exampleproject-setup).





<br/>------------<br/>
_2019-07-19 Page added with editorial review(s)_
