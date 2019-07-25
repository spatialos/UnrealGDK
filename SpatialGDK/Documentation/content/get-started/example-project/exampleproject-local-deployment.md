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
1. In the Editor, on the [GDK Toolbar]({{urlRoot}}/content/toolbars), open the **Schema** drop-down menu and select **Schema (Full Scan)**. <br/><br/>
   ![Schema]({{assetRoot}}assets/screen-grabs/toolbar/schema-button-full-scan.png)
   </br>
2. Select **Snapshot** to generate a snapshot.<br/><br/>
   ![Toolbar]({{assetRoot}}assets/screen-grabs/toolbar/snapshot-button.png)<br/>

<%(#Expandable title="What is Schema?")%>

Schema is a set of definitions which represent your game’s objects in SpatialOS as entities. Schema is defined in .schema files and written in schemalang by the GDK.</br>
Select **Schema** from the GDK toolbar and the GDK generates schema files and their contents for you, so you do not have to write or edit schema files manually.

You can find out more about schema in the [GDK schema documentation]({{urlRoot}}/content/how-to-use-schema)

<%(/Expandable)%>

<%(#Expandable title="What is an entity?")%>
An entity is the SpatialOS equivalent of an Unreal Actor. It’s made up of a set of SpatialOS components. Each component stores data about the entity. (Note that SpatialOS components are not the same thing as Unreal Actor Components.)
<%(/Expandable)%>

<%(#Expandable title="What is  a snapshot?")%>

A snapshot is a representation of the state of a SpatialOS world at a given point in time. A snapshot stores the current state of each entity’s component data. You start each deployment with a snapshot; if it’s a re-deployment of an existing game, you can use the snapshot you originally started your deployment with, or use a snapshot that contains the exact state of a deployment before you stopped it.

You can find out more about snapshots in the [GDK snapshot documentation]({{urlRoot}}/content/how-to-use-snapshots).

<%(/Expandable)%>
   
### Step 2: Launch a local deployment and play

This section shows you how to  start one SpatialOS server-worker instance and two SpatialOS client-worker instances locally, in your Unreal Editor. The server-worker instance acts as an Unreal server and the two client-worker instances acts as two Unreal game clients (as would be used by two game players). 

1. On the Editor toolbar, open the **Play** drop-down menu.
2. Under **Multiplayer Options**, set the number of players to **2** and ensure that checkboxes next to **Run Dedicated Server** and **Spatial Networking** are checked. Checking **Spatial Networking** makes deployments run using SpatialOS networking as opposed to native Unreal networking. If a checkbox is unchecked, select the checkbox to enable it.<br/></br>
   ![Multiplayer Options]({{assetRoot}}assets/set-up-template/template-multiplayer-options.png))<br/></br>
3. Under **Modes**, select **New Editor Window (PIE)** to run the game. This starts one SpatialOS server-worker instance and two SpatialOS client-worker instances locally, in your Unreal Editor.
   The server-worker instance is acting as an Unreal server and the two client-worker instances are acting as two Unreal game clients (as would be used by two game players).
   You can switch between the two Editor windows to see and interact with each game client. 
4. If the game does not run automatically after selecting **New Editor Window (PIE)**, on the Editor toolbar, select **Play** to start a local deployment and play the game.

![]({{assetRoot}}assets/example-project/first-client-launch.png)<br/>

### Step 3: Inspecting and stopping play

When your game is running, select **Inspector** to open the [Inspector](https://docs.improbable.io/unreal/alpha//content/glossary#inspector) in your default web browser. The Inspector is a web-based tool that you use to explore the internal state of a SpatialOS world. It gives you a real-time view of what’s happening in a local or cloud deployment. <br/><br/>
![]({{assetRoot}}assets/screen-grabs/toolbar/inspector-button.png)<br/>

This will open the Inspector in your browser:

![]({{assetRoot}}assets/set-up-template/template-two-client-inspector.png)<br/>

When you’re done, select Unreal Engine's native **Stop** button to stop the client. 

![]({{assetRoot}}assets/toolbar/stop-button-native.png)<br/>

This stops the running client and server workers but keeps your deployment running. 

### Step 4: Iterating on your game

As you iterate on your game (i.e modifying classes or blueprints), **making changes to replicated components will require incremental schema regeneration**. To do this, select the **Schema** button in the GDK toolbar.

![Stop]({{assetRoot}}assets/screen-grabs/toolbar/schema-button.png)<br/>

Once you've regenerated schema, the GDK will restart the running deployment with the new schema.

If you haven't modified anything related to replication, you don't need to regenerate schema and the previously running deployment will continue to be used.

To test your changes, hit play again which will start your client and server workers.

<%(#Expandable title="Workflow reference diagram")%>

 <%(Lightbox image="https://docs.google.com/drawings/d/e/2PACX-1vQCTOucXKMkDJ3-Vpg17_tpUS7IxOXD6Mps-FzWe2tQl3vw5alQPngCnw339cFy3u2NvrcBxhYASKsS/pub?w=710&h=1033")%>

For more details, see the [Local deployment workflow page]({{urlRoot}}/content/local-deployment-workflow).

<%(/Expandable)%>

### Step 4: Stop your deployment

To fully stop your SpatialOS deployment, select **Stop** in the GDK toolbar.<br/></br>
![Stop]({{assetRoot}}assets/screen-grabs/toolbar/stop-button.png)<br/>
</br>
</br>
**> Next:** [3: Launch a cloud deployment]({{urlRoot}}/content/get-started/example-project/exampleproject-cloud-deployment) 

<br/>
<br/>------------<br/>
_2019-07-22 Page updated with limited editorial review._
