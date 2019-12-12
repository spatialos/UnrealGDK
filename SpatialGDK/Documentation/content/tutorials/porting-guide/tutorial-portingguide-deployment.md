

# Port your project to SpatialOS

## 3. Launch a local deployment

Now you can launch a local deployment of your game. 
To do this: 

## Step 1. Make your project use SpatialOS networking

In the Unreal Editor, from the toolbar, open the **Play** drop-down menu and check two checkboxes:

  * Check the box for **Run Dedicated Server**
  * Check the box for **Spatial Networking**<br/>

![Play options]({{assetRoot}}assets/screen-grabs/toolbar/toolbar-checkboxes.png)<br/>
_Image:The Unreal Engine **Play** drop-down menu, with **Run Dedicated Server** and **Spatial Networking** highlighted_

## Step 2. Launch a local deployment
Before you launch a local deployment, you must:

* generate schema (which creates entities)
* generate a snapshot

<button class="collapsible">What is Schema?</button>
<div>


Schema is a set of definitions which represent your game’s objects in SpatialOS as entities. Schema is defined in .schema files and written in schemalang. When you use the GDK, the schema files and their contents are generated automatically so you do not have to write or edit schema files manually.

You can find out more about schema, including how to generate it from the command line, making schema work with source control, and how to exclude certain directories from schema in the [GDK schema documentation]({{urlRoot}}/content/how-to-use-schema)


</div>

<button class="collapsible">What is an entity?</button>
<div>

An entity is the SpatialOS equivalent of an Unreal Actor. It’s made up of a set of SpatialOS components. Each component stores data about the entity. (Note that SpatialOS components are not the same thing as Unreal Actor Components.)

</div>

<button class="collapsible">What is  a snapshot?</button>
<div>


A snapshot is a representation of the state of a SpatialOS world at a given point in time. A snapshot stores the current state of each entity’s component data. You start each deployment with a snapshot; if it’s a re-deployment of an existing game, you can use the snapshot you originally started your deployment with, or use a snapshot that contains the exact state of a deployment before you stopped it.

You can find out more about snapshots in the [GDK snapshot documentation]({{urlRoot}}/content/how-to-use-snapshots).


</div>

To launch a local deployment:

1. In the Editor, on the [GDK Toolbar]({{urlRoot}}/content/unreal-editor-interface/toolbars), open the **Schema** drop-down menu and select **Schema (Full Scan)**. <br/>
   ![Schema]({{assetRoot}}assets/screen-grabs/toolbar/schema-button-full-scan.png)<br/>
   _Image: On the GDK toolbar in the Editor, select **Schema (Full Scan)**_
   </br>
1. Select [**Snapshot**]({{UrlRoot}}/content/spatialos-concepts/generating-a-snapshot) to generate a snapshot.<br/>
   ![Snapshot]({{assetRoot}}assets/screen-grabs/toolbar/snapshot-button.png)<br/>
   _Image: On the GDK toolbar in the Editor, select **Snapshot**_</br>
1. Select **Start**. This opens a terminal window and starts a local SpatialOS deployment. Your game is ready to play when you see the output `SpatialOS ready` in your terminal window.</br>
   ![Start]({{assetRoot}}assets/screen-grabs/toolbar/start-button.png)<br/>
   _Image: On the GDK toolbar in the Editor, select **Start**_</br>
1. On the Editor toolbar, open the **Play** drop-down menu.
1. Under **Multiplayer Options**, set the number of players to **2** and ensure that the checkbox next to **Run Dedicated Server** is checked. (If it is unchecked, select the checkbox to enable it.)<br/>
   ![Multiplayer Options]({{assetRoot}}assets/set-up-template/spatialos-multiplayer-options.png)<br/>
   _Image: The Unreal Engine **Play** drop-down menu, with **Multiplayer Options** and **New Editor Window (PIE)** highlighted_</br>
1. Under **Modes**, select **New Editor Window (PIE)** to run the game. This starts one SpatialOS server-worker instance and two SpatialOS client-worker instances locally, in your Unreal Editor.
   The server-worker instance is acting as an Unreal server and the two client-worker instances are acting as two Unreal game clients (as would be used by two game players).
   You can switch between the two Editor windows to see and interact with each game client. 
1. If the game does not run automatically after selecting **New Editor Window (PIE)**, on the Editor toolbar, select **Play** to run the game.</br>
  ![Play]({{assetRoot}}assets/screen-grabs/toolbar/play-button.png)</br>
  _Image: On the Unreal Engine toolbar, select **Play**_</br>
1. When your game is running, select **Inspector** to open the [Inspector](https://docs.improbable.io/unreal/alpha//content/glossary#inspector) in your default web browser. The Inspector is a web-based tool that you use to explore the internal state of a SpatialOS world. It gives you a real-time view of what’s happening in a local or cloud deployment. </br>
  ![Inspector]({{assetRoot}}assets/screen-grabs/toolbar/inspector-button.png)</br>
  _Image: On the Unreal Engine toolbar, select **Inspector**_</br>
1. When you’re done, select **Stop** in the GDK toolbar to stop your local SpatialOS deployment.<br/>
   ![Stop]({{assetRoot}}assets/screen-grabs/toolbar/stop-button.png)<br/>
   _Image: On the GDK toolbar in the Editor, select **Stop**_</br>
</br>

**To run a local deployment with managed workers, or a cloud deployment, take a look at the [Starter Template guide]({{urlRoot}}/content/get-started/starter-template/get-started-template-intro)**

You have now ported your Unreal game to GDK. 

If you have encountered any problems please check out our [troubleshooting]({{urlRoot}}/content/troubleshooting) and [known-issues]({{urlRoot}}/known-issues).

#### **> Next:** [Logs and modifications]({{urlRoot}}/content/tutorials/porting-guide/tutorial-portingguide-logs)

<br/>

<br/>------------<br/>_2019-08-02 Page updated with editorial review: updated project.<br/>
2019-07-16 Page updated with editorial review.<br/>
