<toc>
# SpatialOS Runtime Settings panel

Before you launch your game in a [cloud deployment]({{urlRoot}}/content/glossary#deployment), you need to set up its [SpatialOS Runtime]({{urlRoot}}/content/glossary##spatialos-runtime) settings. You can use the default settings or change them in the Runtime Settings panel.

## Access the SpatialOS Runtime settings

* From the Unreal Editor:
    * From the **Edit** menu:
        0. Select **Project Settings** to display the Project Settings editor.
        0.  Scroll down to **SpatialOS GDK for Unreal** and select **Runtime Settings** to display the SpatialOS Runtime Settings panel.
* From the Unreal Toolbar:
	0. Select **Play** > **SpatialOS Settings** to display the Project Settings editor. 
	0. Scroll down to **SpatialOS GDK for Unreal** and select **Runtime Settings** to display the SpatialOS Runtime Settings panel.
* From the file system: The SpatialOS Runtime settings are in the `DefaultSpatialGDKSettings.ini` file in the `<GameRoot>\Config\` directory, which you can edit with any text editor.


TODO ADD IMAGE

<br>
_Image: The SpatialOS Runtime Settings panel_

## Settings
The following table lists all the settings in the SpatialOS Runtime Settings panel:


<table>
  <tr>
    <th>Sections</th>
    <th>Settings</th>
    <th>Description</th>
  </tr>
  <tr>
    <td rowspan="3"><strong>Entity pool</strong></td>
    <td><strong>Initial entity IDs reserved</strong></td>
    <td>Enity IDs are unique identifiers for the <a href="{{urlRoot}}/content/glossary#entities">entities</a> which represent Actors in your game’s SpatialOS world.<br>Use this setting to specify the number of entity IDs the SpatialOS Runtime reserves when you first your game. <br><br>Default: <code>3,000</code>.<br><br><strong>Note:</strong> Ensure that the number of entity IDs reserved is greater than the number of Actors that you expect the <a href="{{urlRoot}}/content/glossary#workers">server-worker instances</a> to spawn at game deployment. <br><br><strong>Tip:</strong> Use native Unreal networking to establish how many Actors your game spawns.</td>
  </tr>
  <tr>
    <td><strong>Pool refresh threshold</strong></td>
    <td>Specifies when the SpatialOS Runtime should reserve a new batch of entity IDs. The value is the number of un-used entity IDs left in the entity pool which triggers the SpatialOS Runtime to reserve new entity IDs. <br><br>Default: <code>1,000</code></td>
  </tr>
  <tr>
    <td><strong>Refresh count</strong></td>
    <td>Specifies the number of new entity IDs the SpatialOS Runtime reserves when <strong>Pool refresh threshold</strong> triggers a new batch.<br><br>Default: <code>2,000</code></td>
  </tr>
  <tr>
    <td rowspan="2"><strong>Heartbeat</strong></td>
    <td><strong>Heartbeat interval (seconds)</strong></td>
    <td>Specifies the amount of time, in seconds, between heartbeat events sent from a game client to notify the server-worker instances that it's connected.<br><br>Default: <code>2</code> seconds</td>
  </tr>
  <tr>
    <td><strong>Heartbeat timeout (seconds)</strong></td>
    <td>Specifies the maximum amount of time, in seconds, that the server-worker instances wait for a game client to send heartbeat events. (If the timeout expires, the game client has disconnected.)<br><br>Default: <code>10</code> seconds</td>
  </tr>
  <tr>
    <td rowspan="5"><strong>Replication</strong></td>
    <td><strong>Maximum Actors replicated per tick</strong></td>
    <td>Specifies the maximum number of Actors replicated per tick. <br><br>Default: <code>0</code> per tick (no limit)<br><br><strong>Tips:</strong>
    <ul>
        <li>To avoid game client and server-worker instance slowdown, set the value to around <code>100</code>. (If you set the value to <code>0</code>, the SpatialOS Runtime replicates every Actor per tick; this forms a large SpatialOS world affecting the performance of both game clients and server-worker instances.)</li>
        <li>You can use the <code>stat Spatial</code> flag when you run project builds to find the number of calls to <code>ReplicateActor</code>, and then use this number for reference.</li>
    </ul>
    </td>
  </tr>
  <tr>
    <td><strong>Maximum entities created per tick</strong></td>
    <td>Specifies the maximum number of entities created by the SpatialOS Runtime per tick. (The SpatialOS Runtime handles entity creation separately from Actor replication to ensure it can handle entity creation requests under load.)<br><br><strong>Note:</strong> If you set the value to 0, there is no limit to the number of entities created per tick. However, too many entities created at the same time might overload the SpatialOS Runtime, which can negatively affect your game.<br><br>Default: <code>0</code> per tick (no limit)</td>
  </tr>
  <tr>
    <td><strong>SpatialOS network update rate</strong></td>
    <td>Specifies the rate, in number of times per second, at which server-worker instance updates are sent to and received from the SpatialOS Runtime.<br><br>Default: 1000 per second</td>
  </tr>
    <tr>
    <td><strong>Enable handover</strong></td>
    <td>Define TODO</td>
  </tr>
    <tr>
    <td><strong>Maximum net cull distance squared</strong></td>
    <td>Define TODO</td>
  </tr>
  <tr>
    <td rowspan="2"><strong>SpatialOS position updates</strong></td>
    <td><strong>Position update rate</strong></td>
    <td>Specifies the rate, in number of times per second, at which the server-worker instance updates an Actor's position in the SpatialOS game world.<br><br>Default: 1 per second <br><br><strong>Tip:</strong> We recommend keeping a low rate for position update (around the default of 1) because updating position is increases SpatialOS Runtime load which can have a detrimental effect on your game’s gameplay experience.</td>
  </tr>
  <tr>
    <td><strong>Position distance threshold</td>
    <td>Specifies the minimum distance, in centimeters, that an Actor must move before the server-worker instance updates its position in the SpatialOS world.<br><br>Default: 100 per second</td>
  </tr>
  <tr>
    <td rowspan="4"><strong>Metrics</strong></td>
    <td><strong>Enable metrics</strong></td>
    <td>Specifies whether to report metrics about game client and server-worker instance performance to the <a href="{{urlRoot}}/content/glossary#console">SpatialOS Console</a> to monitor the deployment’s health.<br><br>Default: selected<br><br><strong>Note:</strong> Currently, the only metric reported is the average frames per second (FPS).</td>
  </tr>
    <tr>
    <td><strong>Enable metrics display</strong></td>
    <td>Define TODO</td>
  </tr>
  <tr>
    <td><strong>Metric report interval</strong></td>
    <td>The time, in seconds, between reporting metrics to the SpatialOS Console via the SpatialOS Runtime.<br><br>Default: 2 seconds</td>
  </tr>
  <tr>
    <td><strong>Use frame time as load</strong></td>
    <td>By default the SpatialOS Runtime reports server-worker instance’s load in frames per second (FPS). Select this to switch so it reports as seconds per frame - that is, how long a frame takes. <br><br>Default: Not selected<br><br>For example, you have a target frame rate of 30 FPS, and your game is running at 30 FPS, which is the average FPS.<br><br>
    <ul>
        <li>If the check box is not selected, the <strong>load</strong> value in the SpatialOS Console’s Inspector is <code>AverageFrameTime / TargetFrameTime</code>, which is 1.0 FPS.</li>
        <li>If the check box is selected, the <strong>load</strong> value in the SpatialOS Console’s Inspector is <code>AverageFrameTime</code>, which is 0.033.</li>
    </ul>
    </td>
  </tr>
  <tr>
    <td><strong>Schema generation</strong></td>
    <td><strong>Maximum dynamically attached subobjects per class</strong></td>
    <td>Define TODO</td>
  </tr>
  <tr>
    <td><strong>Local connection</strong></td>
    <td><strong>Default Receptionist host</strong></td>
    <td>Define TODO</td>
  </tr>
  <tr>
    <td rowspan="3"><strong>Cloud connection</strong></td>
    <td><strong>Use development authentication flow</strong></td>
    <td>Define TODO</td>
  </tr>
  <tr>
    <td><strong>Development authentication token</strong></td>
    <td>Define TODO</td>
  </tr>
  <tr>
    <td><strong>Development deployment to connect</strong></td>
    <td>Define TODO</td>
  </tr>
    <tr>
    <td rowspan="3"><strong>Offloading</strong></td>
    <td><strong>Default worker type</strong></td>
    <td>Define TODO</td>
  </tr>
  <tr>
    <td><strong>Enable offloading</strong></td>
    <td>Define TODO</td>
  </tr>
  <tr>
    <td><strong>Actor groups</strong></td>
    <td>Define TODO</td>
  </tr>
</table>

## Use cases

In the following use cases, you use the SpatialOS Runtime Settings panel to configure your project:

* You know that the number of Actors that you need the SpatialOS Runtime to reserve entity IDs for at the start of your game deployment is greater than the default 3,000: Increase the value of **Initial entity IDs reserved** to the number of Actors the game needs as it starts up.
* The SpatialOS Runtime is not reserving a large enough batch of new entity IDs quickly enough during your game’s Runtime: Increase the value of the **Pool refresh threshold** so that the SpatialOS Runtime is triggered to reserve new batch of entity IDs sooner.