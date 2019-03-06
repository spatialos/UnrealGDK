<%(TOC)%>
# Toolbars

There are two toolbars you can use in the Unreal Editor: the main Unreal toolbar, and the SpatialOS GDK toolbar. Once enabled, the GDK toolbar sits alongside the main Unreal toolbar:

 ![Toolbar]({{assetRoot}}assets/screen-grabs/toolbar/toolbars.png)

## Definitions:
`<ProjectRoot>` - The root folder of your Unreal project.  
`<GameRoot>` - The folder containing your project’s `.uproject` and source folder (for example, `<ProjectRoot>\ShooterGame\`).

## Unreal toolbar

Alongside the standard functionality, we’ve added some extra capabilities to the Unreal toolbar under the Play drop-down menu to help with debugging your game:

 ![Toolbar]({{assetRoot}}assets/screen-grabs/toolbar/multi-player-options.png)

1. The Unreal<->SpatialOS networking switch
1. The “Number of Servers” multiplayer option

### Switching between native Unreal networking and SpatialOS networking

To switch from using Unreal networking to using SpatialOS networking, open the **Play** drop-down menu and check two checkboxes:

* **Run Dedicated Server**
* **Spatial Networking** 

These settings are valid for Editor and command-line builds. They’re stored in your project's Unreal config file, `<GameRoot>\Config\DefaultGame.ini`, under `\Script\EngineSettings.GeneralProjectSettings`.

You can switch back by unchecking the boxes.

> **Warning:** As the GDK is in alpha, switching back to Unreal default networking mode can be a useful way to debug and so speed up your development iteration. However, you lose access to the multiserver features of the GDK in Unreal default networking mode which may lead to erratic behavior.

### Launching multiple PIE server-workers

> **Warning:** This option is still experimental and is currently unstable.

You can launch multiple servers at the same time from within the Unreal Editor in [PIE (Unreal documentation)](https://docs.unrealengine.com/en-us/Engine/UI/LevelEditor/InEditorTesting#playineditor) configuration. To configure the number of servers launched, open the **Play** drop-down menu and use the slider `Number of Servers` within the `Multiplayer Options` section.

To connect multiple servers-workers to SpatialOS, you need to tell SpatialOS how many server-workers you will be connecting. In `<ProjectPath>\spatial\default_launch.json` there is a load balancing section which dictates how many workers will be connected to SpatialOS. 

If you haven't modified your load balancing previously, your load balancing strategy should look like:

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
            "manual_worker_connection_only": true
        }
      }
  ]
}
```

This uses the [`rectangle_grid`](https://docs.improbable.io/reference/latest/shared/worker-configuration/load-balancer-config-2#rectangular-grid-rectangle-grid) strategy with 1 column and 1 row. To connect 2 servers, change this to 1 column and 2 rows (or vice-versa). Read more about the different kinds of load balancing strategies [here](https://docs.improbable.io/reference/latest/shared/worker-configuration/load-balancer-config-2#load-balancing-with-the-new-runtime).

## SpatialOS GDK for Unreal toolbar

The GDK toolbar provides several functions required for building and launching your client and server-workers from inside the Unreal Editor.

### Add the GDK toolbar to your Unreal project

> Note: If you based your project off the [StarterProject](https://github.com/spatialos/UnrealGDKStarterProject), the toolbar is already enabled.

To enable the GDK toolbar, navigate to **Edit** > **Plugins** inside the Unreal Editor and scroll down to the bottom. Select the **SpatialOS** section and enable the toolbar:

![Toolbar]({{assetRoot}}assets/screen-grabs/toolbar/enable-toolbar.png)

#### Buttons

The GDK toolbar has five features mapped to individual buttons and is displayed in the main editor toolbar to the right of the `Launch` button:

 ![Toolbar]({{assetRoot}}assets/screen-grabs/toolbar/toolbar-buttons.png)

You can also access these from the **Window** menu:

 ![Toolbar]({{assetRoot}}assets/screen-grabs/toolbar/window-access.png)

| Button | Description |
| --- | --- |
| Snapshot | Generates a [SpatialOS snapshot]({{urlRoot}}/content/glossary#snapshot). | 
| Schema | Creates [schema]({{urlRoot}}/content/glossary#schema) for your Unreal project. |
| Start | Runs [`spatial worker build build-config`](https://docs.improbable.io/reference/latest/shared/spatial-cli/spatial-worker-build-build-config) to build worker configs and runs `spatial local launch` with the launch configuration specified in the settings (see [below](#settings)). |
| Stop | Stops `spatial local launch`. |
| Inspector | Opens the [Inspector]({{urlRoot}}/content/glossary#inspector) in a browser. |

#### Settings

The toolbar settings are in **Edit** > **Project Settings** > **SpatialOS GDK for Unreal** > **Settings**.

 ![Toolbar]({{assetRoot}}assets/screen-grabs/toolbar/toolbar-settings.png)

##### General

| Setting | Description |
| --- | --- |
| SpatialOS directory | If you're using a non-standard structure, you'll need to set this yourself. This is empty by default. If you leave it empty, it defaults to `<GameRoot>/../spatial`. |

##### Play In Editor Settings

| Setting | Description |
| --- | --- |
| Delete dynamically spawned entities | If checked, the GDK deletes any dynamically spawned entities from your SpatialOS deployment when you end the PIE session. |

##### Launch

| Setting | Description |
| --- | --- |
| Generate default launch config | If checked, the GDK creates a [launch configuration file]({{urlRoot}}/content/glossary#launch-configuration-file) by default when you launch a local deployment through the toolbar. |
| Launch configuration | The [launch configuration file]({{urlRoot}}/content/glossary#launch-configuration-file) to use when running `spatial local launch` using the **Start** button. |
| Stop on exit |  If enabled, shuts down running deployments when you close the Unreal Editor. |

##### Snapshots

| Setting | Description |
| --- | --- |
| Snapshot path | Use this to specify the filepath of your [snapshot]({{urlRoot}}/content/glossary#snapshot). If you leave this empty, it defaults to `<GameRoot>/../spatial/snapshots`. |
| Snapshot file name |  Name of your snapshot file. |
| Generate placeholder entities in snapshot | If checked, the GDK adds [placeholder entities]({{urlRoot}}/content/generating-a-snapshot#placeholder-entities) to the snapshot when it is generated |

##### Schema Generation

| Setting | Description |
| --- | --- |
| Output path for the generated schemas | Use this to specify the path of the generated [schema]({{urlRoot}}/content/glossary#schema) files.  If you leave this empty, it defaults to `<GameRoot>/../spatial/schema/improbable/unreal/generated/`. |
