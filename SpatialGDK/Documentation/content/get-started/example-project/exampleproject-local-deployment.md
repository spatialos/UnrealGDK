<%(TOC)%>
# The Example Project 

## 2.  Launch a local deployment

When you want to try out your game, you need to run a deployment. 

There are two types of deployment: local and cloud.

- A [local deployment]({{urlRoot}}/content/glossary#deployment) launches your game with its own instance of SpatialOS, running on your development machine. 
- [Cloud deployments]({{urlRoot}}/content/glossary#deployment) run in the cloud on [nodes]({{urlRoot}}/content/glossary#node). A node refers to a single machine used by a cloud deployment. When you have deployed your game, you can share it with other people and run your game at a scale not possible on a single machine. Once a cloud deployment is running, you can connect clients to it using the [Launcher]({{urlRoot}}/content/glossary#launcher).

Use local deployments for small-scale tests, to quickly test and iterate on changes to your project. For large-scale tests with several players, use a cloud deployment. 

### Step 1: Generate schema and a snapshot

Before you launch a deployment (local or cloud) you must generate [schema]({{urlRoot}}/content/spatialos-concepts/schema-and-snapshots#schema) and a [snapshot]({{urlRoot}}/content/spatialos-concepts/schema-and-snapshots#snapshots). 
1. In the Editor, on the [GDK Toolbar]({{urlRoot}}/content/toolbars), open the **Schema** drop-down menu and select **Schema (Full Scan)**. <br/>
   ![Schema]({{assetRoot}}assets/screen-grabs/toolbar/schema-button-full-scan.png)<br/>
   _Image: On the GDK toolbar in the Editor, select **Schema (Full Scan)**_
   </br>
2. Select **Snapshot** to generate a snapshot.<br/>
   ![Toolbar]({{assetRoot}}assets/screen-grabs/toolbar/snapshot-button.png)<br/>
   _Image: On the GDK toolbar in the Unreal Editor, select **Snapshot**_<br/>

<%(#Expandable title="What is Schema?")%>

Schema is a set of definitions which represent your game’s objects in SpatialOS as SpatialOS entities. Schema is defined in .schema files and written in schemalang by the GDK.</br>
Select **Schema** from the GDK toolbar and the GDK generates schema files and their contents for you, so you do not have to write or edit schema files manually.

You can find out more about schema in the [GDK schema documentation]({{urlRoot}}/content/how-to-use-schema)

<%(/Expandable)%>

<%(#Expandable title="What is a SpatialOS entity?")%>
A SpatialOS entity (usually just called an “entity”) is the SpatialOS equivalent of  an Unreal Actor. It’s made up of a set of SpatialOS components. Each component stores data about the entity. (Note that SpatialOS components are not the same thing as Unreal Components.)
<%(/Expandable)%>

<%(#Expandable title="What is  a snapshot?")%>

A snapshot is a representation of the state of a SpatialOS world at a given point in time. A snapshot stores the current state of each entity’s component data. You start each deployment with a snapshot; if it’s a re-deployment of an existing game, you can use the snapshot you originally started your deployment with, or use a snapshot that contains the exact state of a deployment before you stopped it.

You can find out more about snapshots in the [GDK snapshot documentation]({{urlRoot}}/content/how-to-use-snapshots).

<%(/Expandable)%>
   
### Step 2: Launch a local deployment and play

This section shows you how to  start one SpatialOS server-worker instance and two SpatialOS client-worker instances locally, in your Unreal Editor. The server-worker instance acts as an Unreal server and the two client-worker instances acts as two Unreal game clients (as would be used by two game players). 

1. On the Editor toolbar, open the **Play** drop-down menu.
2. Under **Multiplayer Options**, set the number of players to **2** and ensure that checkboxes next to **Run Dedicated Server** and **Spatial Networking** are checked. Checking **Spatial Networking** makes deployments run using SpatialOS networking as opposed to native Unreal networking. If a checkbox is unchecked, select the checkbox to enable it.<br/></br>
   ![Multiplayer Options]({{assetRoot}}assets/set-up-template/template-multiplayer-options.png))<br/>
   _Image: The Unreal Engine **Play** drop-down menu, with **Multiplayer Options** and **New Editor Window (PIE)** highlighted_</br></br>
3. Under **Modes**, select **New Editor Window (PIE)** to run the game. This starts one SpatialOS server-worker instance and two SpatialOS client-worker instances locally, in your Unreal Editor.
   The server-worker instance is acting as an Unreal server and the two client-worker instances are acting as two Unreal game clients (as would be used by two game players).
   You can switch between the two Editor windows to see and interact with each game client. 
4. If the game does not run automatically after selecting **New Editor Window (PIE)**, on the Editor toolbar, select **Play** to start a local deployment and play the game.

![]({{assetRoot}}assets/example-project/first-client-launch.png)<br/>_Image: New editor window with the game playing._<br/>

### Step 3: Inspecting and stopping play

When your game is running, select **Inspector** to open [the Inspector](https://docs.improbable.io/reference/latest/shared/operate/inspector#the-inspector) in your default web browser. The Inspector is a web-based tool that you use to explore the internal state of a SpatialOS world. It gives you a real-time view of what’s happening in a local or cloud deployment. 

![]({{assetRoot}}assets/screen-grabs/toolbar/inspector-button.png)<br/>_Image: On GDK toolbar, select Inspector_<br/>

When you’re done, select Unreal Engine's native **Stop** button to stop the client. 

![]({{assetRoot}}assets/screen-grabs/toolbar/stop-button-native.png)<br/>_Image: On the native Unreal Engine toolbar, select Stop_<br/>

This stops the running clients but keeps your deployment running. As you iterate on your game, the GDK can (in most cases) automatically reload your client and server workers, so you can iterate on your game without having to restart your deployment. For more information and a reference diagram, see the [Local deployment workflow page]({{urlRoot}}/content/local-deployment-workflow).

To fully stop your SpatialOS deployment, select **Stop** in the GDK toolbar to stop your SpatialOS deployment.<br/></br>
   ![Stop]({{assetRoot}}assets/screen-grabs/toolbar/stop-button.png)<br/>
   _Image: On the GDK toolbar in the Editor select **Stop**_

</br>
</br>
**> Next:** [3: Launch a cloud deployment]({{urlRoot}}/content/get-started/example-project/exampleproject-cloud-deployment) 

<br/>
<br/>------------<br/>
_2019-07-22 Page updated with limited editorial review._
