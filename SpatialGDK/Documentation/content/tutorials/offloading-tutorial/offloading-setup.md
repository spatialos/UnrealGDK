<%(TOC)%>

# Offloading example project tutorial

## 1. Setup

### Step 1: Configure Actor groups

In this tutorial, you set up the `CrashBot_Gym` map to have two Actor groups.

1. Open the Example project in the Unreal Editor.
2. Go to the [SpatialOS Runtime Settings panel]({{urlRoot}}/content/unreal-editor-interface/runtime-settings).
3. Apart from the default Actor group, set up the following Actor groups for offloading, as is shown in the following screenshot:

    - `AI` Actor group, which is owned by the by the `AIWorker`. Any Actors added to this group have their authority assigned to `AIWorkers`.
    - `CrashBot` Actor group, which is owned by the `CrashBotWorker`. Any Actors added to this group have their authority assigned to `CrashBotWorker`.

> **Note**: Any actors not listed in An actor group have their authority assigned to the default server-worker type. In the Example Project, the default server-worker is called `UnrealWorker`.

<%(Lightbox image="{{assetRoot}}assets/offloading-project/actor-groups.png")%> 
<br>
_Actor classes added for each Actor group_

### Step 2: Configure launch configuration

1. In the Unreal Editor, go to the [SpatialOS Editor Settings panel]({{urlRoot}}/content/unreal-editor-interface/editor-settings). 
2. In the `Server Workers` section, in addition to the default `UnrealWorker` server-worker type, add the `AIWorker` and the `CrashBotWorker` server-worker types, as is shown in the following screenshot.
3. In the `Instances to launch in editor` setting, enter `1` for all these server-worker types so that only one instance of each server-worker type is launched when you start a PIE session.

<%(Lightbox image="{{assetRoot}}assets/offloading-project/launch-configuration.png")%>
<br>
_Launch configuration for each server-worker type_

### Step 3: Configure the worker type

If there are no server-worker configuration for any of the server-worker types listed in the launch configuration settings, the Unreal GDK generates a new worker config for the worker based on a default server-worker configuration template.

In the Example Project, the configuration files are automatically created for the `UnrealWorker`, the `AIWorker` and the `CrashBotWorker` server-workers.

### Step 4: Enable offloading

1. In the Unreal Editor, go to the [SpatialOS Runtime Settings panel]({{urlRoot}}/content/unreal-editor-interface/runtime-settings).
2. In the `Offloading` section, ensure that the `Enable Offloading` check box is selected.
 
<%(Lightbox image="{{assetRoot}}assets/offloading-project/enable-offloading.png")%>
<br>
_Enable offloading_

<br/>------------<br/>
_2019-07-30 Page added as limited editorial review_
<br/>
<br/>
[//]: # (TODO: https://improbableio.atlassian.net/browse/DOC-1142)
------------