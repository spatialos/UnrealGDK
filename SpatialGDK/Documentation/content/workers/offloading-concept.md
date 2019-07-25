<%(TOC)%>

# Overview

Offloading is the new architecture that the SpatialOS GDK for Unreal provides to allocate the authority of specific Actor groups from the main Unreal server-worker instance to a different server-worker instance. By using offloading, you can save the resources of the main Unreal server-worker instance when you want to build richer game features.

In Unreal’s native single-server architecture, the server runs many of the major game systems such as physics simulation, AI decision-making, and navigation. It also processes and validates input from game clients. Using the GDK allows you to execute latency-tolerant systems on a separate server-worker instance.

You can think of _offloading_ as splitting up computation-heavy systems to run on separate server-worker instances. As a result, some CPU-intensive game systems that cannot easily use multithreading with the Unreal’s native architecture are no longer limited by the processing power of a single server. In addition, not all input from game clients needs to be processed by the same server-worker instance.

> **Note**: Offloading increases CPU resources at the cost of bandwidth. If you want more interaction between the offloaded server-worker instance and the main server-worker instance, the cost of bandwidth and the CPU consumption on communication between them increase. Therefore, when you design your game feature using offloading, consider having Actors on each server that don't need to frequently communicate across server boundaries.

## Actor groups

To facilitate offloading, we've created the concept of Actor groups to help you configure which Actor types a given server-worker type will have authority over. In the Unreal Editor, you can create Actor groups, assign Actor classes to a group, and then assign each group to a server-worker type.

Before you start to use offloading in your game, ensure that you’re familiar with the [best practices for using offloading]({{urlRoot}}/content/workers/offloading-concept#best-practices) and [offloading workflow]({{urlRoot}}/content/workers/set-up-offloading).

To get started with configuring Actor groups, see [Offloading example project]({{urlRoot}}/content/tutorials/offloading-tutorial/offloading-intro).

## Best practices

### Adapt game logic for offloading

Before you offload Actors, consider the following scenarios that you need to update the game code to work correctly in:

- `IsServer()`
    
    Because the `IsServer()` function returns true for both the main Unreal server and all offloaded servers, to ensure that the logic is called only on a server-worker that has authority over an Actor, take one of the following options:
    - Use the new APIs defined in [Actor group ownership helpers]({{urlRoot}}/content/apis-and-helper-scripts/actor-group-ownership-helpers).
    - Use `HasAuthority` where `IsServer()` was used.

- `<NM_Client` or `== NM_DedicatedServer` check
    Same as above

- Server RPCs follow similar logic to their usage in a single-server model.
  - **When sent from clients**: if a client net-owns an actor and invokes a server RPC, it sends that RPC to the server that has authority, which can be either the main Unreal server-worker or offloaded server-worker.
  - **When sent from an offloaded server**: Server RPCs invoked by an offloaded worker run only on that offloaded server worker. However, if you want Server RPCs to be run on another server worker you should use [Cross-server RPCs]({{urlRoot}}/content/technical-overview/gdk-concepts#cross-server-rpcs).

- Calls to `SpawnActor` on a server that doesn't have authority over the spawned Actor

    You might want to spawn an Actor from a server where a different server has authority over the spawned Actor. For example, an offloaded AI might drop items that the default server worker instance should have authority over. 

    However, this might cause issues when the initialization logic in the calls such as `BeginPlay` and `PostInitializeComponent` is executed on the wrong server-worker instance. You can usually work around these issues using callbacks / RepNotify-s on the main Unreal server-worker instance to defer the execution of such logic until when the correct server-worker instance has authority over the offloaded Actor.

<br/>------------<br/>
_2019-07-26 Page added with limited editorial review_
<br/>
<br/>
[//]: # (TODO: https://improbableio.atlassian.net/browse/DOC-1142)
------------