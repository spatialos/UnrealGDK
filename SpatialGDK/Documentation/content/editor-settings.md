<%(TOC)%>
# GDK Editor Settings

Use Editor Settings as advanced configurations to configure a local deployment, and schema and snapshot generation.

The following use cases show when you need to use Editor Settings:

- When your game world is larger than the default setting of 2km*2km, you can modify the values of simulation dimensions from **Launch** > **Launch configuration file description** > **World**. Alternatively, edit your own launch configuration file, deselect **Generate launch configuration file** in the **Launch** section, and choose your own launch configuration file in the **Upload launch configuration file** field.
- When you are testing your multiserver simulation, you can specify rectangle grid column count and row count to configure the grid size from **Launch** > **Launch configuration file description** > **Workers**. Alternatively, you can specify both counts in your own configuration file, and then choose your own launch configuration file in the **Upload launch configuration file** field.

> **Note**: You can find all the settings that you configure using Editor Settings in the `DefaultSpatialGDKEditorSettings.ini` file from the `<GameRoot>\Config\` directory.

To learn more about all the settings available in the **Editor Settings** panel, check the following table that lists the properties and their description:

<table>
<tbody>
<tr>
<td><strong>Sections</strong></td>
<td><strong>Properties</strong></td>
<td><strong>Description</strong></td>
</tr>
<tr>
<td>General</td>
<td><span style="font-weight: 400;">SpatialOS directory</span></td>
<td><span style="font-weight: 400;">Specify the directory for SpatialOS-related files, for example, `C:/Projects/MyGame/spatial/`.</span></td>
</tr>
<tr>
<td>Play in Editor Settings</td>
<td>Delete dynamically spawned entities</td>
<td>
<p><span style="font-weight: 400;">Decide whether to delete all the dynamically spawned entities when server workers disconnect. By default, the check box is selected.&nbsp;</span></p>
<ul>
<li>If you select it, when you restart <strong>Play In Editor</strong>, the game is started from a clean state.</li>
<li>If you deselect it, when you restart <strong>Play In Editor</strong>, the game reconnects to a live deployment with entities from the previous session present.&nbsp;</li>
</ul>
</td>
</tr>
<tr>
<td rowspan="5">Launch</td>
<td>Generate launch configuration file</td>
<td>
<p><span style="font-weight: 400;">Decide whether to auto-generate a launch configuration file when you launch your project through the toolbar. By default, the check box is selected.</span></p>
<ul>
<li><span style="font-weight: 400;">If you select it, you can specify the properties in the <strong>Launch configuration file description</strong> section.</span></li>
<li>If you deselect it, you must choose your own launch configuration file in the <strong>Upload launch configuration file</strong> field.</li>
</ul>
</td>
</tr>
<tr>
<td>Upload launch configuration file</td>
<td>
<p><span style="font-weight: 400;">Choose the launch configuration file used for `</span><span style="font-weight: 400;">spatial local launch`, for example, `C:/Projects/MyGame/spatial/default_launch.json`.</span><span style="font-weight: 400;">&nbsp;</span></p>
<p><span style="font-weight: 400;"><strong>Note</strong>: This field is valid only when <strong>Generate launch configuration file</strong> is deselected.</span></p>
</td>
</tr>
<tr>
<td>Stop local launch on exit</td>
<td>
<p><span style="font-weight: 400;">Stop `</span><span style="font-weight: 400;">spatial local launch`</span><span style="font-weight: 400;"> when you shut down Unreal Editor. By default, the check box is deselected.</span></p>
</td>
</tr>
<tr>
<td>Command line flags for local launch</td>
<td>
<p><span style="font-weight: 400;">Specify command line flags passed to `</span><span style="font-weight: 400;">spatial local launch`</span><span style="font-weight: 400;">.&nbsp;</span></p>
<p><strong>Tip</strong>: To check available flags, open the CLI and run `spatial local launch --help`. For example, to connect to the local deployment from a different machine on the local network, add the `--runtime_ip` flag.</p>
</td>
</tr>
<tr>
<td>Launch configuration file description</td>
<td>
<p>Specify the properties for the launch configuration file.</p>
<p><strong>Note</strong>: The fields in this section are valid only when you select <strong>Generate launch configuration file</strong>. For information about the definition of each property, see <a href="https://docs.improbable.io/reference/13.7/shared/project-layout/launch-config">Launch configuration file</a>.</p>
</td>
</tr>
<tr>
<td rowspan="3">Snapshots</td>
<td>Snapshot directory</td>
<td>Specify the directory for your SpatialOS snapshot, for example, `C:/Projects/MyGame/spatial/snapshots/`.</td>
</tr>
<tr>
<td>Snapshot file name</td>
<td>Specify the name of your SpatialOS snapshot file, for example, `default.snapshot`.</td>
</tr>
<tr>
<td>Generate placeholder entities in snapshot</td>
<td>
<p><span style="font-weight: 400;">Decide whether to add placeholder entities to the snapshot on generation.&nbsp;By default, the check box is selected.</span></p>
<p><span style="font-weight: 400;">If you select it, you can see these entities in the Inspector, which shows the areas that the server-worker instance has authority over.&nbsp;</span></p>
</td>
</tr>
<tr>
<td>Schema</td>
<td>Generated schema directory</td>
<td>Specify the directory that stores the generated schema files, for example, `C:/Projects/MyGame/spatial/schema/unreal/generated/`.</td>
</tr>
</tbody>
</table>
