<%(TOC)%>

# Offloading example project tutorial

## 1. Setup

### Step 1: Configure Actor groups

The map is set up to have two Actor groups apart from the default Actor group, namely the `AI` and `CrashBot` Actor groups. The `AI` Actor group is owned by the `AIWorker`, and therefore the authority of any Actors added to this group have their authority assigned to `AIWorkers`. Similarly, the Actors listed in the `CrashBot` Actor group have their authority assigned to `CrashBotWorker`.

> **Note**: Any actors not listed in an actor group have their authority assigned to the default server-worker type, which in the case of the example project is called `UnrealWorker`.

![img]({{assetRoot}}assets/offloading-project/actor-groups.png)

### Step 2: Configure the launch configuration

In the launch configuration settings(which can be found under the Play dropdown -> Spatial Settings ... settings tab), the server worker section has been modified to contain three server worker types, UnrealWorker, AIWorker, and CrashBotWorker (Marked up in read).

Apart from the names, all of these workers have the same launch configuration and the Instances to launch in the editor setting (Marked up in blue) have been configured such that only one instance of each worker type is launched when starting a PIE session.

![img]({{assetRoot}}assets/offloading-project/launch-configuration.png)

### Step 3: Configure the worker type

If there are no worker configuration for any of the server-worker types listed in the launch configuration settings, then the Unreal GDK will generate a new worker config for the worker based on a default worker configuration template. In the example project, there are three worker configs already created, one for the `UnrealWorker`, one for the `AIWorker` and one for the `CrashBotWorker`.

### Step 4: Enable offloading

In order for offloading to work, it needs to be enabled using the toggle in the Spatial GDK settings. In the example project, the settings have been configured to have offloading enabled.

![img]({{assetRoot}}assets/offloading-project/enable-offloading.png)

<br/>------------<br/>
_2019-07-26 Page added as draft_
<br/>
<br/>
[//]: # (TODO: https://improbableio.atlassian.net/browse/DOC-1142)
------------