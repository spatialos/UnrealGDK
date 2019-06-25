# Port your project to SpatialOS

## 3. Launch a local deployment

## Step 1:  Generate schema and a snapshot

You need to generate [schema]({{urlRoot}}/content/spatialos-concepts/schema-and-snapshots#schema) and generate a [snapshot]({{urlRoot}}/content/how-to-use-snapshots) before you start your deployment. To do this:

1. In the Unreal Editor, on the GDK toolbar, open the **Schema** drop-down menu and select **Schema (Full Scan)**. <br/>
       ![Toolbar](C:/GitHub/Improbable/UnrealGDK/SpatialGDK/Documentation/content/tutorials/%7B%7BassetRoot%7D%7Dassets/screen-grabs/toolbar/schema-button-full-scan.png)<br/>
     _Image: On the GDK toolbar in the Unreal Editor, select **Schema (Full Scan)**_<br/>
2. Select **Snapshot** to generate a snapshot.<br/>
   ![Toolbar](C:/GitHub/Improbable/UnrealGDK/SpatialGDK/Documentation/content/tutorials/%7B%7BassetRoot%7D%7Dassets/screen-grabs/toolbar/snapshot-button.png)<br/>
   _Image: On the GDK toolbar in the Unreal Editor, select **Snapshot**_<br/>

## 7. Launch your game

Now you can launch a local deployment of your game. 
To do this: 

1. Switch your game project to use SpatialOS networking. 

- In the Unreal Editor, from the toolbar, open the **Play** drop-down menu and check two checkboxes:
  - Check the box for **Run Dedicated Server**
  - Check the box for **Spatial Networking**

Before you launch a local deployment, you have to:

* generate schema (which creates SpatialOS entities)
* generate a snapshot

<%(#Expandable title="What is Schema?")%>

Schema is a set of definitions which represent your game’s objects in SpatialOS as SpatialOS entities. Schema is defined in .schema files and written in schemalang. When you use the GDK, the schema files and their contents are generated automatically so you do not have to write or edit schema files manually.

You can find out more about schema in the [GDK schema documentation]({{urlRoot}}/content/how-to-use-schema)

<%(/Expandable)%>

<%(#Expandable title="What is a SpatialOS entity?")%>
A SpatialOS entity (usually just called an “entity”) is the SpatialOS equivalent of an Unreal Actor. It’s made up of a set of SpatialOS components. Each component stores data about the entity. (Note that SpatialOS components are not the same thing as Unreal Components.)
<%(/Expandable)%>

<%(#Expandable title="What is  a snapshot?")%>

A snapshot is a representation of the state of a SpatialOS world at a given point in time. A snapshot stores the current state of each entity’s component data. You start each deployment with a snapshot; if it’s a re-deployment of an existing game, you can use the snapshot you originally started your deployment with, or use a snapshot that contains the exact state of a deployment before you stopped it.

You can find out more about snapshots in the [GDK snapshot documentation]({{urlRoot}}/content/how-to-use-snapshots).

<%(/Expandable)%>

To launch a local deployment:

1. In the Editor, on the [GDK Toolbar]({{urlRoot}}/content/toolbars), open the **Schema** drop-down menu and select **Schema (Full Scan)**. <br/>
   ![Schema]({{assetRoot}}assets/screen-grabs/toolbar/schema-button-full-scan.png)<br/>
   _Image: On the GDK toolbar in the Editor, select **Schema (Full Scan)**_
   </br>
1. Select [**Snapshot**]({{UrlRoot}}/content/spatialos-concepts/generating-a-snapshot) to generate a snapshot.<br/>
   ![Snapshot]({{assetRoot}}assets/screen-grabs/toolbar/snapshot-button.png)<br/>
   _Image: On the GDK toolbar in the Editor select **Snapshot**_</br>
1. Select **Start**. This opens a terminal window and starts a local SpatialOS deployment. Your game is ready to play when you see the output `SpatialOS ready. Access the inspector at http://localhost:21000/inspector` in your terminal window.<br/>
   ![Start]({{assetRoot}}assets/screen-grabs/toolbar/start-button.png)<br/>
   _Image: On the GDK toolbar in the Editor select **Start**_</br>
1. On the Editor toolbar, open the **Play** drop-down menu.
1. Under **Multiplayer Options**, set the number of players to **2** and ensure that the checkbox next to **Run Dedicated Server** is checked. (If it is unchecked, select the checkbox to enable it.)<br/>
   ![Multiplayer Options]({{assetRoot}}assets/set-up-template/template-multiplayer-options.png)<br/>
   _Image: The Unreal Engine **Play** drop-down menu, with **Multiplayer Options** and **New Editor Window (PIE)** highlighted_</br>
1. Under **Modes**, select **New Editor Window (PIE)** to run the game. This starts one SpatialOS server-worker instance and two SpatialOS client-worker instances locally, in your Unreal Editor.
   The server-worker instance is acting as an Unreal server and the two client-worker instances are acting as two Unreal game clients (as would be used by two game players).
   You can switch between the two Editor windows to see and interact with each game client. 
1. If the game does not run automatically after selecting **New Editor Window (PIE)**, on the Editor toolbar, select **Play** to run the game.
1. When your game is running, select *Inspector* to open the Inspector in your default web browser. The Inspector lets you <inspect things>
1. When you’re done, select **Stop** in the GDK toolbar to stop your local SpatialOS deployment.<br/>
   ![Stop]({{assetRoot}}assets/screen-grabs/toolbar/stop-button.png)<br/>
   _Image: On the GDK toolbar in the Editor select **Stop**_
</br>

**To run a local deployment with managed workers, or a cloud deployment, take a look at the [Starter Template guide](guide link)**

You have now ported your Unreal game to GDK. 

If you have encountered any problems please check out our [troubleshooting]({{urlRoot}}/content/troubleshooting) and [known-issues]({{urlRoot}}/known-issues).

</br>