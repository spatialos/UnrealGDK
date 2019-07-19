<%(TOC)%>
# SpatialOS Editor Settings panel
In the [Project Settings](https://docs.unrealengine.com/en-us/Engine/UI/ProjectSettings) editor, you can use the SpatialOS Editor Settings panel to change the default settings for local deployments, schema generation, and snapshot generation. For example, you might want to use the panel to extend the size of your [game world]({{urlRoot}}/content/glossary#game-world) or change your [load balancing]({{urlRoot}}/content/glossary#load-balancing) strategy.

> **Note**: The SpatialOS editor settings are in the `DefaultSpatialGDKEditorSettings.ini file` in the `<GameRoot>\Config\` directory, which you can edit with any text editor.

## Access the panel
To access the **SpatialOS Editor Settings** panel in the Unreal Editor, use either of the following methods:

* From the **Edit** menu:
	1. Select **Project Settings** to display the Project Settings editor.
	1.  Scroll down to **SpatialOS GDK for Unreal** and select **Editor Settings** to display the SpatialOS Editor Settings panel.


* From the Unreal Toolbar:
	1. Select **Play** > **SpatialOS Settings** to display the Project Settings editor. 
	1. Scroll down to **SpatialOS GDK for Unreal** and select **Editor Settings** to display the SpatialOS Editor Settings panel.

  <img src="{{assetRoot}}assets/screen-grabs/editor-settings.png"/> **!!!    NEEDS A NEW IMAGE    !!!** 
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
    <td><strong>Show Spatial service button</strong></td>
    <td>TODO ADD WORDING HERE</td>
  </tr>
  <tr>
    <td><strong>Play In Editor settings</strong></td>
    <td><strong>Delete dynamically-spawned entities</strong></td>
    <td>Select the check box to delete all dynamically-spawned entities on <a href="{{urlRoot}}/content/glossary#workers">server-worker instance</a> shut-down. <br><br>The <a href="{{urlRoot}}/content/glossary#spatialos-runtime">SpatialOS Runtime</a> deletes dynamically-spawned entities when a server-worker instance with <a href="{{urlRoot}}/content/glossary#authority">authority</a> over the entity’s the <code>Position</code> of <a href="{{urlRoot}}/content/glossary#component">component</a> shuts down. <br><br>A server-worker instance may shut down during your <a href="{{urlRoot}}/content/glossary#deployment">deployment's</a> runtime or when you manually stop a local deployment from the Unreal Editor.<br><br>Default: selected<br>
    <ul>
      <li>If selected: when you start a new server-worker instance’s session using <strong>Play In Editor</strong>, it has <i>no entities</i> from the previous server-worker instance’s session.</li>
      <li>If NOT selected: when you start new a server-worker instance’s session using <strong>Play In Editor</strong>, it <i>has entities</i> from the previous server-worker instance’s session.</li>
    </ul></td>
  </tr>
  <tr>
    <td rowspan="6"><strong>Local deployment (TODO CHECK THIS WITH VALENTYN!)</strong></td>
    <td><strong>Auto-generate launch configuration file</strong></td>
    <td>Select the check box for the GDK to auto-generate a <a href="{{urlRoot}}/content/glossary#launch-configuration-file">launch configuration file</a> for your game every time you launch a <a href="{{urlRoot}}/content/glossary#deployment">local deployment</a>.<br><br>Default: selected
    <ul>
      <li>If selected: the GDK generates a file for you.</li>
      <li>If NOT selected: you must set up a launch configuration <code>.json</code> file and specify it in the <strong>Launch configuration file name and directory</strong> field.</li>
    </ul></td>
  </tr>
  <tr>
    <td><strong>Launch configuration file name and directory</strong></td>
    <td>If you have created a <a href="{{urlRoot}}/content/glossary#launch-configuration-file">launch configuration <code>.json</code> file</a> for <a href="{{urlRoot}}/content/glossary#deployment">local deployments</a> of your game,clear the <strong>Auto-generate launch configuration file</strong> check box, and then,specify the file name and location here. <br><br>Default: <code>C:/Projects/MyGame/spatial/default_launch.json</code></td>
  </tr>
  <tr>
    <td><strong>Stop local deployment on exit</strong></td>
    <td>Select the check box to stop your game’s local deployment when you shut down Unreal Editor. Default: NOT selected</td>
  </tr>
    <tr>
    <td><strong>Auto-start local deployment</strong></td>
    <td>TODO ADD WORDING HERE</td>
  </tr>
  <tr>
    <td><strong>Command line flags for local launch</strong></td>
    <td>Select <strong>+</strong> and enter command-line flags to alter the deployment’s behavior when you run the <code>spatial local launch</code> command. To remove all the flags, click the trash icon.<br><br><strong>Note</strong>: To view the available flags, open the <a href="{{urlRoot}}/content/glossary#command-line-tool-cli">CLI</a> and run the <code>spatial local launch --help</code> command.<br><br>Default: none listed</td>
  </tr>
  <tr>
    <td><strong>Launch configuration file options</strong></td>
    <td>Note: These settings are only available if you have selected <strong>Auto-generate launch configuration file</strong>.Select this drop-down menu to change the options in any auto-generated launch configuration file. <br><br>For information about the definition of each option, see the Worker SDK’s flexible project layout documentation on the <a href="https://docs.improbable.io/reference/latest/shared/flexible-project-layout/reference/launch-configuration">launch configuration file</a>.</td>
  </tr>
  <tr>
    <td rowspan="2"><strong>Snapshots</strong></td>
    <td><strong>Snapshot file name</strong></td>
    <td>The name of your project’s <a href="{{urlRoot}}/content/glossary#snapshot">snapshot</a> file.<br><br>Default: <code>default.snapshot</code></td>
  </tr>
  <tr>
    <td><strong>Snapshot directory</strong></td>
    <td>The directory containing your project’s <a href="{{urlRoot}}/content/glossary#snapshot">snapshot</a> file. <br><br>Default: <code>C:/Projects/MyGame/spatial/snapshots/</code></td>
  </tr>
  <tr>
  <td rowspan="6"><strong>Cloud deployment</strong></td>
    <td><strong>SpatialOS project</strong></td>
    <td>The name of your project (see the <a href="{{urlRoot}}/content/glossary#console">Console</a> - this should look something like <code>beta_randomword_anotherword_randomnumber</code>). TODO CHECK WITH SOMEONE</td>
  </tr>
  <tr>
    <td><strong>Assembly name</strong></td>
    <td>The name of an assembly you have uploaded. TODO CHECK WITH SOMEONE</td>
  </tr>
  <tr>
    <td><strong>Deployment name</strong></td>
    <td>Specify a name for your deployment. This labels the deployment in the Console. TODO CHECK IF YOU SPECIFY THIS HERE, or if you've already done this elsewhere</td>
  </tr>
  <tr>
    <td><strong>Launch configuration file name and path TODO CHECK WITH VALENTYN</strong></td>
    <td>The path to the launch configuration file you want to use, including the file name.</td>
  </tr>
  <tr>
    <td><strong>Snapshot path</strong></td>
    <td>The path to the snapshot file you want to use, including the file name TODO CHECK WITH SOMEONE</td>
  </tr>
  <tr>
    <td><strong>Region</strong></td>
    <td>The region where the deployment should be hosted TODO CHECK WITH SOMEONE</td>
  </tr>
  <td rowspan="4"><strong>Simulated players TODO CHECK WITH VALENTYN</strong></td>
    <td><strong>Region</strong></td>
    <td>Description</td>
  </tr>
  <tr>
    <td><strong>Include simulated players</strong></td>
    <td>Select the check box if you want to launch a simulated player deployment alongside your cloud deployment. TODO CHECK</td>
  </tr>
  <tr>
    <td><strong>Deployment name TODO TYPO IN IMAGE</strong></td>
    <td>Specify a name for the simulated player deployment. This labels the deployment in the Console. TODO CHECK</td>
  </tr>
  <tr>
    <td><strong>Number of simulated players</strong></td>
    <td>The number of simulated players to start.</td>
  </tr>
</table>

## Use cases

In the following use cases, you use the SpatialOS Editor Settings panel to configure your project:

* To make your game world larger than the default setting of 2km by 2km, you change the values of dimensions for your game world. Alternatively, write this directly in your game project's [launch configuration file]({{urlRoot}}/content/glossary#launch-configuration-file).
* To configure the [rectangular grid](https://docs.improbable.io/reference/latest/shared/worker-configuration/load-balancing#rectangular-grid-rectangle-grid) load balancing strategy when you're testing your multiserver game, you specify a rectangle grid column and a row count. Alternatively, write this directly in your game project’s [launch configuration file]({{urlRoot}}/content/glossary#launch-configuration-file).


<br/>
<br/>------------<br/>
_2019-07-31 Page added with editorial review._
