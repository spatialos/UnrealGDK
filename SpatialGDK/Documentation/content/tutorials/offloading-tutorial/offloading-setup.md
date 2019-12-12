

# Multiserver offloading

## Setup

### Step 1: Running the Gym

Let's start by opening the Crashbot Gym and verifying it's current behavior.

1. Open the Example Project in the Editor.
1. In the Content Browser, open `Maps/Crashbot_Gym`.
1. Once the map is loaded, Generate Schema & Snapshot.
1. When generation is complete, hit Play.

You should see the Crashbots running towards the turrets and the turrets shooting them.

Watch out, the turrets will also shoot you!

### Step 2: Adding a new Server Worker

The next step is to define a new server-worker and instruct the GDK to run an instance of it.

1. Stop the game.
1. Go to the [SpatialOS Runtime Settings panel]({{urlRoot}}/content/unreal-editor-interface/runtime-settings).
1. Navigate to the Offloading section at the bottom.
1. Check the box to Enable Offloading.
<%(Lightbox image="{{assetRoot}}assets/offloading-project/enable-offloading.png")%> 
1. Go to the [SpatialOS Editor Settings panel]({{urlRoot}}/content/unreal-editor-interface/editor-settings).
1. In the 'Launch' section expand the 'Launch configuration file options' property.
1. Add a new element to the 'Server Workers' Array using the '+'.
1. Expand the new element and set the 'Worker Type Name' to `CrashbotWorker`.
<%(Lightbox image="{{assetRoot}}assets/offloading-project/add-worker.png")%> 
1. Use the toolbar to generate a new snapshot that will support the new server worker.
<%(Lightbox image="{{assetRoot}}assets/offloading-project/generate-snapshot.png")%> 
1. Hit Play, the GDK should restart your local deployment to support the new server worker.
1. Open up the inspector and verify the CrashbotWorker is running.
<%(Lightbox image="{{assetRoot}}assets/offloading-project/inspector-workers.png")%>

### Step 3: Creating an Actor Group

Now that you have a new server worker running, you need to offload Actors to it.
This is done using 'Actor Groups'

An Actor Group is a named set of Actor Classes that is owned by a server worker.
In order to offload the Crashbots, you must define a new Actor Group containing the Crashbots and their Weapons.

1. Stop the game.
1. Go to the [SpatialOS Runtime Settings panel]({{urlRoot}}/content/unreal-editor-interface/runtime-settings).
1. Navigate to the Offloading section at the bottom.
1. Add a new element to the 'Actor Groups' Map.
1. Name the new group `Crashbots` and set its 'Owning Worker Type' to `CrashbotWorker`.
1. Add an element to the 'Actor Classes' Set and select `BP_Crashbot`.
1. Add `BP_Crashbot_Rifle` to 'Actor Classes' as well.
<%(Lightbox image="{{assetRoot}}assets/offloading-project/actor-groups.png")%>
</br>
_Final Actor Group Configuration_

### Step 4: Verifying the Results

You can verify everything is working correctly using the Inspector.

1. Since you've changed Actor Group configurations, you'll need to restart the deployment by pressing Stop, then Play
<%(Lightbox image="{{assetRoot}}assets/offloading-project/restart-deployment.png")%>
1. Once the game is running, you should see the Crashbots running forwards and getting shot as usual.
1. Open the inspector, select the CrashbotWorker and UnrealWorker in the Workers tab.
1. In the 'Show Me' tab, select 'Authority / Interest' and 'Authority area'.
1. You should be able to see that CrashbotWorker has authority over the Crashbots and that UnrealWorker has
authority over the turrets & player.
</br>
<%(Lightbox image="{{assetRoot}}assets/offloading-project/inspector-crashbot.gif")%>


</br>
#### **> Next:** [2: Gameplay changes]({{urlRoot}}/content/tutorials/offloading-tutorial/offloading-gameplay-changes)
</br>

<br/>------------<br/>
_2019-08-29 Page updated without editorial review: added setting up offloading and removed pre-set configurations_<br/>
_2019-07-30 Page added without editorial review_
<br/>
<br/>
[//]: # (TODO: https://improbableio.atlassian.net/browse/DOC-1142)
------------
