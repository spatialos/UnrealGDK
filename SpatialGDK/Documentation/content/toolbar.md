# SpatialOS GDK for Unreal toolbar

The GDK toolbar provides several functions required for building and launching your client and server-workers from inside the Unreal Editor.

## Terms used in this document
`<ProjectRoot>` - The root folder of your Unreal project.  
`<GameRoot>` - The folder containing your game's `.uproject` and source folder (for example, `<ProjectRoot>/ShooterGame/`).  

## Add the GDK toolbar to your Unreal project

To enable the GDK toolbar, navigate to **Edit** > **Plugins** inside the Unreal Editor and scroll down to the bottom. Select the **SpatialOS Unreal GDK** section and enable the toolbar:

![Toolbar]({{assetRoot}}assets/screen-grabs/toolbar/enable-toolbar.png)

Note: The toolbar has already been setup if you based your project off the [StarterProject](https://github.com/spatialos/UnrealGDKStarterProject).

## Buttons

The GDK toolbar has five features mapped to individual buttons and is displayed in the main editor toolbar to the right of the `Launch` button:

 ![Toolbar]({{assetRoot}}assets/screen-grabs/toolbar/toolbar-buttons.png)

You can also access these from the **Window** menu:

 ![Toolbar]({{assetRoot}}assets/screen-grabs/toolbar/window-access.png)


* **Snapshot**</br>
Generates a [SpatialOS snapshot](https://docs.improbable.io/reference/latest/shared/glossary#snapshot) (SpatialOS documentation).

* **Schema**</br>
Creates `schema` for your Unreal project.

* **Launch**</br>
Runs [spatial worker build build-config](https://docs.improbable.io/reference/latest/shared/spatial-cli/spatial-worker-build-build-config) to build worker configs and runs `spatial local launch` with the launch configuration specified in the settings (see [below](#settings)).

* **Stop**</br>
Stops `spatial local launch`.

* **Inspector**</br>
Opens the [Inspector](https://docs.improbable.io/reference/latest/shared/glossary#inspector) (SpatialOS documentation) in a browser.

## Settings

The toolbar settings are in **Edit** > **Project Settings** > **SpatialOS Unreal GDK** > **Toolbar**.

 ![Toolbar]({{assetRoot}}assets/screen-grabs/toolbar/toolbar-settings.png)

* **Configuration**

    * **Project root folder**</br>
    If you're using a non-standard structure, you'll need to set this yourself. This is empty by default. If you leave it empty, it defaults to `<GameRoot>/../spatial`.

    * **Launch configuration**</br>
    The [launch configuration file](https://docs.improbable.io/reference/latest/shared/reference/file-formats/launch-config) (SpatialOS documentation) to use when running `spatial local launch` using the `Launch` button.

    * **Stop on exit**</br>
    If enabled, shuts down running deployments when you close the Unreal Editor.

    * **Snapshot path**</br>
    Use this to specify the filepath of your snapshot. If you leave this empty, it defaults to `<GameRoot>/../spatial/snapshots`.

    * **Snapshot file name**</br>
    Name of your snapshot file.

* **Schema Generation**

    * **Generate schema for all supported classes**</br>
    **Experimental** Use this to generate schema for all UObjects that have replicated or handover properties. As the GDK does not currently support multiple replicated Actor components of the same type on an Actor, classes extended from `USceneComponent` doesn't generate schemas.

    * **Output path for the generated schemas**</br>
    Use this to specify the path of the generated schema files.  If you leave this empty, it defaults to `<GameRoot>/../spatial/schema/improbable/unreal/generated/`.
