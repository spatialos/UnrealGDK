> This [pre-alpha](https://docs.improbable.io/reference/13.1/shared/release-policy#maturity-stages) release of the SpatialOS Unreal GDK is for evaluation and feedback purposes only, with limited documentation - see the guidance on [Recommended use](../../README.md#recommended-use).

# SpatialOS Unreal GDK toolbar

The SpatialOS Unreal GDK toolbar is an easy way to run and alter [`spatial` command-line tool](https://docs.improbable.io/reference/13.1/shared/glossary#the-spatial-command-line-tool) (SpatialOS documentation) commands from inside the Unreal Editor.

## Add the SpatialOS Unreal GDK toolbar to your Unreal project

To enable the SpatialOS Unreal GDK toolbar:

1. Add a dependency to the plugin in your Unreal project file.

	Either:
	* Open your Unreal project's `.uproject` file in a text editor and add the SpatialOS Unreal GDK toolbar in the `Plugins` section:
	    ```
	    "Plugins": [
	        {
	            "Name": "SpatialGDKEditorToolbar",
	            "Enabled": true
	        }
	    ],
	    ```

   Or:
	* Inside the Unreal Editor, navigate to **Edit** > **Plugins** and scroll down
	to the bottom. Select the **SpatialOS Unreal GDK** section and enable the toolbar:

		![Toolbar](../assets/screen_grabs/toolbar/enable_toolbar.png)

## Buttons

The SpatialOS Unreal GDK toolbar has five features mapped to individual buttons, and is displayed in the main editor toolbar to the right of the `Launch` button:

 ![Toolbar](../assets/screen_grabs/toolbar/toolbar_buttons.png)

You can also access these from the **Window** menu:

 ![Toolbar](../assets/screen_grabs/toolbar/window_access.png)


* Snapshot</br>
Generates a [SpatialOS snapshot](https://docs.improbable.io/reference/13.1/shared/glossary#snapshot) (SpatialOS documentation).

* Codegen</br>
Creates [SpatialOS Unreal GDK interop code](./interop.md).

* Launch</br>
Runs `spatial local launch` with the launch configuration specified in the settings (see [below](#settings)).

* Stop</br>
Stops `spatial local launch`.

* Inspector</br>
Opens the [Inspector](https://docs.improbable.io/reference/13.1/shared/glossary#inspector) (SpatialOS documentation) in a browser.

## Settings

The toolbar settings are in **Edit** > **Project Settings** > **SpatialOS Unreal GDK** > **Toolbar**.

 ![Toolbar](../assets/screen_grabs/toolbar/toolbar_settings.png)


* **Project Root Folder**</br>
By default, this points to the root folder of your SpatialOS Unreal GDK project. If you're using a non-standard structure, you'll need to set this yourself.

* **Launch Configuration**</br>
The [launch configuration file](https://docs.improbable.io/reference/13.1/shared/reference/file-formats/launch-config) (SpatialOS documentation) to use when running `spatial local launch` using the `Launch` button.

* **Stop on Exit**</br>
If enabled, shuts down running deployments when you close the Unreal Editor.

* **Snapshot path**</br>
Path to your SpatialOS Unreal GDK snapshot.

* **Snapshot file name**</br>
Name of your SpatialOS Unreal GDK snapshot file.

* **Singleton classes**</br>
Use this to specify the classes which the [ICG](./interop.md) should generate as singleton classes.

* **Classes to generate typebindings for**</br>
Use this to specify the classes which the [ICG](./interop.md) should generate [type bindings](./glossary.md#type-bindings) for.
