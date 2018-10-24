<%(Callout type="warn" message="This [alpha](https://docs.improbable.io/reference/latest/shared/release-policy#maturity-stages) release of the SpatialOS Unreal GDK is for evaluation and feedback purposes only, with limited documentation - see the guidance on [Recommended use]({{urlRoot}}/index#recommended-use)")%>

# SpatialOS Unreal GDK toolbar

The SpatialOS Unreal GDK toolbar provides several functions required for building and launching your client- and server-workers from inside the Unreal Editor.

## Terms used on this doc
`<ProjectRoot>` - The root folder of your Unreal project.  
`<GameRoot>` - The folder containing your game's `.uproject` and source folder (for example, `<ProjectRoot>/ShooterGame/`).  

## Add the SpatialOS Unreal GDK toolbar to your Unreal project

To enable the SpatialOS Unreal GDK toolbar you need to add a dependency to the plugin in your Unreal project file.

Either open your Unreal project's `.uproject` file in a text editor and add the SpatialOS Unreal GDK toolbar in the `Plugins` section:

```
"Plugins": [
    {
        "Name": "SpatialGDKEditorToolbar",
        "Enabled": true
    }
],
```

Or inside the Unreal Editor, navigate to **Edit** > **Plugins** and scroll down to the bottom. Select the **SpatialOS Unreal GDK** section and enable the toolbar:

![Toolbar]({{assetRoot}}assets/screen-grabs/toolbar/enable-toolbar.png)

## Buttons

The SpatialOS Unreal GDK toolbar has five features mapped to individual buttons and is displayed in the main editor toolbar to the right of the `Launch` button:

 ![Toolbar]({{assetRoot}}assets/screen-grabs/toolbar/toolbar-buttons.png)

You can also access these from the **Window** menu:

 ![Toolbar]({{assetRoot}}assets/screen-grabs/toolbar/window-access.png)


* Snapshot</br>
Generates a [SpatialOS snapshot](https://docs.improbable.io/reference/latest/shared/glossary#snapshot) (SpatialOS documentation).

* Schema </br>
Creates `schema` for your Unreal project.

* Launch</br>
Runs [spatial worker build build-config](https://docs.improbable.io/reference/latest/shared/spatial-cli/spatial-worker-build-build-config) to build worker configs and runs `spatial local launch` with the launch configuration specified in the settings (see [below](#settings)).

* Stop</br>
Stops `spatial local launch`.

* Inspector</br>
Opens the [Inspector](https://docs.improbable.io/reference/latest/shared/glossary#inspector) (SpatialOS documentation) in a browser.

## Settings

The toolbar settings are in **Edit** > **Project Settings** > **SpatialOS Unreal GDK** > **Toolbar**.

 ![Toolbar]({{assetRoot}}assets/screen-grabs/toolbar/toolbar-settings.png)

* **Configuration**

    * **Project Root Folder**</br>
    If you're using a non-standard structure, you'll need to set this yourself. This is empty by default. If you leave it empty, it defaults to `<GameRoot>/../spatial`.

    * **Launch Configuration**</br>
    The [launch configuration file](https://docs.improbable.io/reference/latest/shared/reference/file-formats/launch-config) (SpatialOS documentation) to use when running `spatial local launch` using the `Launch` button.

    * **Stop on Exit**</br>
    If enabled, shuts down running deployments when you close the Unreal Editor.

    * **Snapshot path**</br>
    Use this to specify the filepath of your Unreal GDK snapshot. If you leave this empty, it defaults to `<GameRoot>/../spatial/snapshots`.

    * **Snapshot file name**</br>
    Name of your SpatialOS Unreal GDK snapshot file.

* **Schema Generator**

    * **Generate Schema for all Supported Classes**</br>
    **Experimental** Use this to generate schema for all `AActor` and `UActorComponent` classes (which *don't* extend `USceneComponent`) without explicitly tagging them as `SpatialType` within their UCLASS defintion. 
    `USceneComponent` is unsupported due to there often being multiple of these components on an Actor which isn't currently supported.
    If you have `UActorComponent`s which don't extend `USceneComponent` but exist in multiple on an Actor, *this can potentially crash your game*. Make sure there are never multiple components of the same type on an Actor. If there are make sure you modify your Actor to only have one, or use the explicit `SpatialType` tagging approach and don't tag the duplicate `UActorComponent`.

    * **Output path for the generated schemas**</br>
    Use this to specify the filepath of ICG-generated schema file.  If you leave this empty, it defaults to `<GameRoot>/../spatial/schema/improbable/unreal/generated/`.
