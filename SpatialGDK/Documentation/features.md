<%(TOC)%>
# Feature list

The SpatialOS GDK for Unreal works by approximating SpatialOS entities to Unreal Actors. Any properties and commands within an entity’s component work in the way properties and RPCs work when applied to Actors. (You can find out more about SpatialOS [entities, components and properties](https://docs.improbable.io/reference/latest/shared/concepts/entities) in the SpatialOS documentation.)

The GDK alpha release (2018-10-31) has the following features:

## Replicated properties, conditions and notifications
The SpatialOS GDK for Unreal supports:

* All types that native Unreal replicates.
* All replication conditions (such as `COND_InitialOnly` and `COND_OwnerOnly`, for example) but note that these are not optimized.
* Replication in all Blueprints.
* Replication notifications, including notifications that pass the old value as a parameter.

## RPCs

The SpatialOS GDK for Unreal supports the following RPCs:

* Reliable Client/Server
* Unreliable Client/Server
* Unreliable NetMulticast

In addition, there is also support for a new RPC type called [Cross-server]({{urlRoot}}/features#cross-server-rpcs).

The SpatialOS GDK for Unreal does **not** support the following RPC:

* Reliable NetMulticast
    > If you use a reliable NetMulticast RPC, the GDK downgrades it to unreliable.

The SpatialOS GDK for Unreal also supports:

* Replicated Blueprint events
* RPC validation

## Static subobject replication

The GDK currently supports replication of static subobjects owned by an actor. 

Properties and RPCs within subobjects have the same support as an Actor’s properties and RPCs.

Support for dynamic subobjects is in development.

## Property handover

Actor property handover is a new feature we’re introducing with the GDK. It replicates server-side properties between servers so Unreal games can take advantage of the SpatialOS cloud server architecture. For more information, see [the Actor property handover]({{urlRoot}}/content/handover-between-server-workers) documentation.

## Server only Actors

Actors are able to be specified as "Server only", meaning replication of these Actors will only happen between server-workers. For more information, see [the Spatial Type]({{urlRoot}}/content/spatial-type#spatialtype-descriptors) documentation

## Singleton Actors

`Singleton Actors` is a new term that describes Actors which contain global state and logic. Only one of these Actors exists in a game world. The SpatialOS GDK for Unreal supports both client accessible and server only singletons. For more information see the documentation on [Singleton Actors]({{urlRoot}}/content/singleton-actors).

## Cross-server RPCs

As SpatialOS has multiple servers, only one server has the [authority]({{urlRoot}}/content/glossary#authority) to manipulate an Actor at any one time. Cross-server RPCs enable a server without authority over an Actor to tell the server which has authority over that Actor to manipulate it. For more information see the documentation on [Cross-server RPCs]({{urlRoot}}/content/cross-server-rpcs).

## Networking switch

The GDK offers you a convenient way to switch between native Unreal networking and SpatialOS networking via a checkbox in the Unreal Editor. This is useful to test valid functionality and performance on a single server. See the documentation on the networking switch on the [Troubleshooting]({{urlRoot}}/content/troubleshooting) page. To switch networking mode to Unreal, from the Editor toolbar menu, unmark the `Spatial networking` checkbox under `Play` dropdown in the Unreal Editor toolbar.
