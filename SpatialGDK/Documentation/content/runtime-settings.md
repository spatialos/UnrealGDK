<%(TOC)%>
# SpatialOS Runtime Settings

Before you run or package the game for testing outside the Unreal Editor, you can use SpatialOS runtime settings as advanced configurations to affect your game project at runtime and gameplay outside the Unreal Editor as well.

The following use cases show when you need to use SpatialOs runtime settings to configure the properties:

- When you decide the number of Actors that the server spawns at the beginning, if the number that you want is greater than the default initial value of the pool, which is 3,000, you should increase the value.
- When you decide the number of Actors that the server should be able to spawn in a short interval, if the number that you want is greater than the default threshold number, which is 1,000, you should increase the value.

> **Note**: When you change one of the properties from their default values, the changes values are saved in the `DefaultSpatialGDKSettings.ini` file, and you can find the file from the `<GameRoot>\Config\` directory.

To learn more about all the properties available in the **SpatialOS Editor Settings** panel, check the following table:

<table>
<tbody>
<tr>
<td><strong>Sections</strong></td>
<td><strong>Properties</strong></td>
<td><strong>Description</strong></td>
</tr>
<tr>
<td rowspan="3">Entity pool</td>
<td>Initial entity IDs reserved</td>
<td>
<p>The number of entity IDs reserved when the entity pool is first started. The default number is 3,000.</p>
<p><strong>Note</strong>: Ensure that the number of entity IDs reserved is greater than the expected number of Actors that your server spawns at the beginning of gameplay. You can start with native Unreal networking and print the number of Actors spawned, and then use this number for reference.</p>
</td>
</tr>
<tr>
<td>Pool refresh threshold</td>
<td>The minimum number of entity IDs in the pool before new entity IDs are reserved. The default number is 1,000.</td>
</tr>
<tr>
<td>Refresh count</td>
<td>The number of entity IDs reserved when pool refresh threshold is reached. The default number is 2,000.</td>
</tr>
<tr>
<td rowspan="2">Heartbeat</td>
<td>Heartbeat interval</td>
<td>
<p>The amount of time, in seconds, between heartbeat events sent from a client to notify the server(s) that it's connected. By default, the interval is 2 seconds.</p>
</td>
</tr>
<tr>
<td>Heartbeat timeout</td>
<td>
<p>The maximum amount of time, in seconds, that the server(s) wait for a client to send heartbeat events. If the timeout expires, the client has disconnected. By default, the timeout is 10 seconds.</p>
</td>
</tr>
<tr>
<td rowspan="3">Replication</td>
<td>Maximum Actors replicated per tick</td>
<td>
<p>The maximum number of Actors that are replicated per tick.</p>
<p><span style="font-weight: 400;">To avoid client and server slowdown, set the value to around <code>100</code>. </span><span style="font-weight: 400;">If you set the value to <code>0</code>, SpatialOS replicates every Actor per tick to form a large world, which affects both clients and servers. </span>
<p><span style="font-weight: 400;">Use <code>stat Spatial</code> in the console in editor builds to find the number of calls to <code>ReplicateActor</code>, and then use this number for reference.</span></p>
</td>
</tr>
<tr>
<td>Maximum entities created per tick</td>
<td>
<p><span style="font-weight: 400;">The maximum number of entities that are created per tick. Entity creation is handled separately from actor replication to ensure entity creation requests are always handled when under load.</span></p>
  <p><span style="font-weight: 400;"><strong>Note</strong>: If you set the value to <code>0</code>, there is no limit to the number of entities created per tick. However, too many entities created at a time might affect SpatialOS performance.</p>
</td>
</tr>
<tr>
<td>SpatialOS network update rate</td>
<td><span style="font-weight: 400;">The rate, times per second, which server updates are sent to SpatialOS and processed from SpatialOS at.</td>
</tr>
<tr>
<td>Query-based interest</td>
<td>Enable query-based interest</td>
<td>
<p>Query-based interest is required to support&nbsp;<a href="https://docs.unrealengine.com/en-us/Engine/LevelStreaming">Level Streaming</a> and the <code>AlwaysInterested UPROPERTY</code> property when you use SpatialOS networking. By default, the check box is selected.</span></p>
<p><strong>Note</strong>: Using query-based interest for large-scale game projects might affect your SpatialOS performance.</p>
</td>
</tr>
<tr>
<td rowspan="2">SpatialOS position updates&nbsp;</td>
<td>Position update frequency</td>
<td>
<p>The frequency, times per second, to update an Actor's SpatialOS position.</p>
<p><strong>Note</strong>: Consider a low rate for position update because updating position is costly.</p>
</td>
</tr>
<tr>
<td>Position distance threshold</td>
<td>The minimum distance, in centimeters, that an Actor must move before its SpatialOS position is updated.</td>
</tr>
<tr>
<td rowspan="3">Metrics</td>
<td>Enable metrics</td>
<td>
<p>Choose whether to report m<span style="font-weight: 400;">etrics about client and server performance to SpatialOS to monitor the deployment health. By default, the check box is selected.</span></p>
<p><strong>Note</strong>: Currently, the only metric reported is the average frames per second (FPS).</p>
</td>
</tr>
<tr>
<td>Metrics report interval</td>
<td><span style="font-weight: 400;">The amount of time, in seconds, between metrics reported to SpatialOS.</span></td>
</tr>
<tr>
<td>Use frame time as load</td>
  <td><span style="font-weight: 400;">Choose whether to use <code>AverageFrameTime</code> instead of <code>AverageFrameTime / TargetFrameTime</code> as the <code>load</code> value in the Inspector to represent frame time of a worker instance. By default, the check box is not selected.</p>
<p>For example, if you have a target frame rate of 30 FPS, and your game is running at 30 FPS, which is the average FPS.</p>
<ul>
  <li>If the check box is not selected, the <code>load</code> value in the Inspector is <code>AverageFrameTime / TargetFrameTime</code>, which is 1.0.</li>
  <li>If the check box is selected, the <code>load</code> value in the Inspector is <code>AverageFrameTime</code>, which is 0.033.</li>
</ul>
</td>
</tr>
</tbody>
</table>
