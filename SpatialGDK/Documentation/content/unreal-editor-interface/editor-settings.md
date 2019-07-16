<%(TOC)%>
# SpatialOS Editor Settings panel
In the [Project Settings](https://docs.unrealengine.com/en-us/Engine/UI/ProjectSettings) editor, you can use the SpatialOS Editor Settings panel to change the default settings for local deployments, schema generation, and snapshot generation. For example, you might want to use the panel to extend the size of your [game world]({{urlRoot}}/content/glossary#game-world) or change your [load balancing]({{urlRoot}}/content/glossary#load-balancing) strategy.

> **Note** : The SpatialOS editor settings are in the `DefaultSpatialGDKEditorSettings.ini file` in the `<GameRoot>\Config\ directory`, which you can edit with any text editor.

## Access the panel
To access the **SpatialOS Editor Settings** panel in the Unreal Editor, use either of the following methods:

* From the **Edit** menu:
	1. Select **Project Settings** to display the Project Settings editor.
	1.  Scroll down to **SpatialOS GDK for Unreal** and select **Editor Settings** to display the SpatialOS Editor Settings panel.


* From the Unreal Toolbar:
	1. Select **Play** > **SpatialOS Settings** to display the Project Settings editor. 
	1. Scroll down to **SpatialOS GDK for Unreal** and select **Editor Settings** to display the SpatialOS Editor Settings panel.

  <img src="{{assetRoot}}assets/screen-grabs/editor-settings.png"/>
    <br>_Image: the SpatialOS Editor Settings panel_

## Settings
The following table lists all the settings in the SpatialOS Editor Settings panel:

<table>
  <tr>
    <th>Sections</th>
    <th>Settings</th>
    <th>Description</th>
  </tr>
  <tr>
    <td><strong>General</strong></td>
    <td><strong>SpatialOS directory</strong></td>
    <td>The directory for SpatialOS-related files, for example, `C:Projects/MyGame/spatial/.`</td>
  </tr>
  <tr>
    <td><strong>Play In Editor settings</strong></td>
    <td><strong>Delete dynamically-spawned entities</strong></td>
    <td>Select the check box to delete all dynamically-spawned entities on <a href="{{urlRoot}}/content/glossary#workers">server-worker instance</a> shut-down. <br><br>The <a href="{{urlRoot}}/content/glossary#spatialos-runtime">SpatialOS Runtime</a> deletes dynamically-spawned entities when a server-worker instance with <a href="{{urlRoot}}/content/glossary#authority">authority</a> over the entity’s the `Position` of <a href="{{urlRoot}}/content/glossary#component">component</a> shuts down. <br><br>A server-worker instance may shut down during your <a href="{{urlRoot}}/content/glossary#deployment">deployment's</a> runtime or when you manually stop a local deployment from the Unreal Editor.<br><br>Default: selected<br>
    <ul>
      <li>If selected: when you start a new server-worker instance’s session using <strong>Play In Editor</strong>, it has <i>no entities</i> from the previous server-worker instance’s session.</li>
      <li>If NOT selected: when you start new a server-worker instance’s session using <strong>Play In Editor</strong>, it <i>has entities</i> from the previous server-worker instance’s session.</li>
    </ul></td>
  </tr>
  <tr>
    <td rowspan="5"><strong>Launch</strong></td>
    <td><strong>Auto-generate launch configuration file</strong></td>
    <td>Select the check box for the GDK to auto-generate a <a href="{{urlRoot}}/content/glossary#launch-configuration-file">launch configuration file</a> for your game every time you launch a <a href="{{urlRoot}}/content/glossary#deployment">local deployment</a>.<br><br>Default: selected
    <ul>
      <li>If selected: the GDK generates a file for you.</li>
      <li>If NOT selected: you must set up a launch configuration `.json` file and specify it in the <strong>Launch configuration file name and directory</strong> field.</li>
    </ul></td>
  </tr>
  <tr>
    <td><strong>Launch configuration file name and directory</strong></td>
    <td>If you have created a <a href="{{urlRoot}}/content/glossary#launch-configuration-file">launch configuration `.json` file</a> for <a href="{{urlRoot}}/content/glossary#deployment">local deployments</a> of your game,clear the <strong>Auto-generate launch configuration file</strong> check box, and then,specify the file name and location here. <br><br>Default: `C:/Projects/MyGame/spatial/default_launch.json`</td>
  </tr>
  <tr>
    <td><strong>Stop local deployment on exit</strong></td>
    <td>Select the check box to stop your game’s local deployment when you shut down Unreal Editor. Default: NOT selected</td>
  </tr>
  <tr>
    <td><strong>Command line flags for local launch</strong></td>
    <td>Select <strong>+</strong> and enter command-line flags to alter the deployment’s behavior when you run the `spatial local launch` command. To remove all the flags, click the trash icon.<br><br><strong>Note</strong>: To view the available flags, open the <a href="{{urlRoot}}/content/glossary#command-line-tool-cli">CLI</a> and run the `spatial local launch --help` command.<br><br>Default: none listed</td>
  </tr>
  <tr>
    <td><strong>Launch configuration file options</strong></td>
    <td>Note: These settings are only available if you have selected <strong>Auto-generate launch configuration file</strong>.Select this drop-down menu to change the options in any auto-generated launch configuration file. <br><br>For information about the definition of each option, see the Worker SDK’s flexible project layout documentation on the <a href="https://docs.improbable.io/reference/latest/shared/flexible-project-layout/reference/launch-configuration">launch configuration file</a>.</td>
  </tr>
  <tr>
    <td><strong>Snapshots</strong></td>
    <td><strong>Snapshot file name</strong></td>
    <td>The name of your project’s <a href="{{urlRoot}}/content/glossary#snapshot">snapshot</a> file.<br><br>Default: `default.snapshot`</td>
  </tr>
  <tr>
    <td></td>
    <td><strong>Snapshot directory</strong></td>
    <td>The directory containing your project’s <a href="{{urlRoot}}/content/glossary#snapshot">snapshot</a> file. <br><br>Default: `C:/Projects/MyGame/spatial/snapshots/`</td>
  </tr>
  <tr>
    <td></td>
    <td><strong>Auto-generate placeholder entities in snapshot</strong></td>
    <td>Select the check box for the GDK to auto-populate your project’s snapshot file with <a href="{{urlRoot}}/content/glossary#entity">entities</a>. <br><br>When you have deployed your game, you can see the entities in your game world in the <a href="{{urlRoot}}/content/glossary#inspector">Inspector</a>. You can use this to test the worker instances in your game deployment.<br><br>Default: selected</td>
  </tr>
  <tr>
    <td><strong>Schema</strong></td>
    <td><strong>Schema file’s directory</strong></td>
    <td>The location for the `.json` file that stores your project’s generated <a href="{{urlRoot}}/content/glossary#schema">schema</a>. The GDK creates the directory when you generate schema.<br><br>Default: `C:/Projects/MyGame/spatial/schema/unreal/generated/`</td>
  </tr>
</table>