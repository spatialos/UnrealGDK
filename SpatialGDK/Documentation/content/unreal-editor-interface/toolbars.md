<%(TOC)%>
# Toolbars

There are two toolbars you can use in the Unreal Editor: the main Unreal toolbar, and the SpatialOS GDK toolbar. Once enabled, the GDK toolbar sits alongside the main Unreal toolbar:

<%(Lightbox title="Toolbars" image="{{assetRoot}}assets/screen-grabs/toolbar/unreal-and-gdk-toolbar.png")%>

## Definitions:
`<ProjectRoot>` - The root folder of your Unreal project.  
`<GameRoot>` - The folder containing your project’s `.uproject` and source folder (for example, `<ProjectRoot>\ShooterGame\`).

## Unreal toolbar

Alongside the standard functionality, we’ve added some extra capabilities to the Unreal toolbar under the Play drop-down menu to help with debugging your game:

 ![Toolbar]({{assetRoot}}assets/screen-grabs/toolbar/multi-player-options.png)

1. The Unreal<->SpatialOS networking switch
1. The “Number of Servers” multiplayer option
1. The "SpatialOS Settings" menu item which opens the [SpatialOS settings]({{urlRoot}}/content/unreal-editor-interface/editor-settings)

### Switching between native Unreal networking and SpatialOS networking

To switch from using Unreal networking to using SpatialOS networking, open the **Play** drop-down menu and check two checkboxes:

* **Run Dedicated Server**
* **Spatial Networking** 

These settings are valid for Editor and command-line builds. They’re stored in your project's Unreal config file, `<GameRoot>\Config\DefaultGame.ini`, under `\Script\EngineSettings.GeneralProjectSettings`.

You can switch back by unchecking the boxes.

> **Warning:** As the GDK is in alpha, switching back to Unreal default networking mode can be a useful way to debug and so speed up your development iteration. However, you lose access to the multiserver features of the GDK in Unreal default networking mode which may lead to erratic behavior.

### Auto-generated launch config for PIE server-worker types

You can launch multiple servers at the same time from within the Unreal Editor in [PIE (Unreal documentation)](https://docs.unrealengine.com/en-us/Engine/UI/LevelEditor/InEditorTesting#playineditor) configuration. To configure the number of servers launched, open the **Play** drop-down menu and use the slider `Number of Servers` within the `Multiplayer Options` section.

If you want to connect multiple server-worker instances to SpatialOS, you need to tell SpatialOS how many instances to connect. You do this in the load balancing section of the launch configuration file (<ProjectPath>\spatial\default_launch.json`). However, by default, when you launch SpatialOS through the editor, this launch configuration file is auto-generated for you based on the settings specified in the [SpatialOS Editor Settings]({{urlRoot}}/content/unreal-editor-interface/editor-settings).

 This uses the [`rectangle_grid`](https://docs.improbable.io/reference/latest/shared/worker-configuration/load-balancer-config-2#rectangular-grid-rectangle-grid) strategy with 1 column and 1 row. To connect 2 servers, change this to 1 column and 2 rows (or vice-versa). Read more about the different kinds of load balancing strategies [here](https://docs.improbable.io/reference/latest/shared/worker-configuration/load-balancing).

## SpatialOS GDK for Unreal toolbar

The GDK toolbar provides several functions required for building and launching your client and server-workers from inside the Unreal Editor.

> Note: follow the [Get Started guide]({{urlRoot}}/content/get-started/introduction) to install the GDK toolbar.

#### Buttons

The GDK toolbar has five features mapped to individual buttons and is displayed in the main editor toolbar to the right of the `Launch` button:

 ![Toolbar]({{assetRoot}}assets/screen-grabs/toolbar/gdk-toolbar.png)

You can also access these from the **Window** menu:

 ![Toolbar]({{assetRoot}}assets/screen-grabs/toolbar/window-access.png)

| Button | Description |
| --- | --- |
| Snapshot | Generates a [SpatialOS snapshot]({{urlRoot}}/content/glossary#snapshot). |
| Schema | Creates [schema]({{urlRoot}}/content/glossary#schema) for your Unreal project. |
| Start | Runs [`spatial worker build build-config`](https://docs.improbable.io/reference/latest/shared/spatial-cli/spatial-worker-build-build-config) to build worker configs and runs `spatial local launch` with the launch configuration specified in the settings (see the [SpatialOS Editor Settings]({{urlRoot}}/content/unreal-editor-interface/editor-settings) documentation). |
| Stop | Stops `spatial local launch`. |
| Inspector | Opens the [Inspector]({{urlRoot}}/content/glossary#inspector) in a browser. |
| Deploy | Opens the Cloud Deployment dialog box. |

#### Settings

Visit the [SpatialOS Editor Settings panel page]({{urlRoot}}/content/unreal-editor-interface/editor-settings) to learn more about the GDK toolbar's settings.

<br/>

<br/>------------<br/>
_2019-07-31 Page updated with limited editorial review_
<br/>_2019-06-27 Page updated with limited editorial review_
<br/>	<br/>
