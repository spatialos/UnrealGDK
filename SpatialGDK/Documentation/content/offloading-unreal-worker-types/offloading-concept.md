<%(TOC)%>

# Overview

_Offloading_ is the new architecture that the SpatialOS GDK for Unreal provides to allocate the authority of specific [Actor groups](#Actor-groups) from the default server-worker instance to a different dedicated worker instance. By using offloading, you can save the resources of the default server-worker instance when you want to build richer game features.

In Unreal’s native single-server architecture, the server runs many of the major game systems such as physics simulation, AI decision-making, and navigation. It also processes and validates input from game clients. Using the GDK allows you to execute latency-tolerant systems on a separate server-worker instance.

You can think of offloading as splitting up computation-heavy systems to run on separate dedicated server-worker instances. As a result, some CPU-intensive game systems that cannot easily use multithreading with the Unreal’s native architecture are no longer limited by the processing power of a single server. In addition, not all input from game clients needs to be processed by the same server-worker instance.

> **Note**: Offloading increases CPU resources at the cost of bandwidth. If you want more interaction between the offloaded server and the default server-worker, the cost of bandwidth and the CPU consumption on communication between them increase. Therefore, when you design your game feature using offloading, consider having Actors on each server that don't need to frequently communicate across server boundaries.

## Actor groups

We introduce _Actor groups_ to help you set up offloading by configuring what Actor types that a given server-worker type has authority over. In the Unreal Editor, you can create Actor groups, assign Actor classes to a group, and then assign each group to a server-worker type.

Before you start to use offloading in your game, ensure that you’re familiar with the [best practices for using offloading](#best-practice) and [offloading workflow]({{urlRoot}}/content/offloading-unreal-worker-types/setup-offloading).

To get started with configuring Actor groups, see [Offloading example project]({{urlRoot}}/content/tutorials/offloading-tutorial/offloading-intro).

## Best practices

### Adapt game logic for offloading

Before you offload Actors, consider the following scenarios that you need to update the game code to work correctly in:

- `IsServer(...)`
    This function returns true for both the default server-worker and all offloaded servers. To ensure that logic in a code branch is invoked only on a worker that can be authoritative over an Actor, use the new APIs defined in [`USpatialStatics`](link here), which are based on actor group ownership.

- `<NM_Client` or `== NM_DedicatedServer` check
    Same as above

- Server RPCs follow the similar logic to their usage in a single-server model.
  - **When sent from clients**: if a client net-owns an actor and invokes a server RPC, it sends that RPC to the server that has authority, which can be either the default server worker or offloaded server workers.
  - **When sent from an offloaded server**: Server RPCs invoked by an offloaded worker run only on that offloaded server worker. However, if you want Server RPCs to be run on another server worker you should use [Cross-server RPCs]({{urlRoot}}/content/technical-overview/gdk-concepts#cross-server-rpcs).

<br/>------------<br/>
_2019-07-26 Page added with limited editorial review_
<br/>
<br/>
[//]: # (TODO: https://improbableio.atlassian.net/browse/DOC-1142)
------------