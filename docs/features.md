# Feature list

The SpatialOS Unreal GDK works by approximating SpatialOS entities to Unreal Actors. Any properties and commands within an entity’s component work in the way properties and RPCs work when applied to Actors. (You can find out more about SpatialOS [entities, components and properties](https://docs.improbable.io/reference/13.1/shared/concepts/entities) in the SpatialOS documentation.) 

The SpatialOS Unreal GDK pre-alpha release (2018-07-31) has the following features:

## Replicated properties, conditions and notifications
The SpatialOS Unreal GDK supports:
* All types that native Unreal replicates (see the [documentation listing replicated types](./content/supported-replicated-types.md)).
* All replication conditions (such as `COND_InitialOnly` and `COND_OwnerOnly`, for example) but note that these are not optimized.
* Replication in all Blueprints.
* Replication notifications, including notifications that pass the old value as a parameter.

## RPCs

The SpatialOS Unreal GDK supports the following RPCs:
* Reliable Client/Server
* Unreliable Client/Server
* Unreliable NetMulticast

The SpatialOS Unreal GDK does **not** support the following RPC:
* Reliable NetMulticast
    > If you use a reliable NetMulticast RPC, the GDK downgrades it to unreliable.

The SpatialOS Unreal GDK also supports:
* Replicated Blueprint events
* RPC validation

## Static component replication

The SpatialOS Unreal GDK supports only one instance of each type of static component replication for components owned by an Actor. For example, an Actor can have only one FooComponent and one BarComponent; it can’t have two FooComponents or two BarFooComponents. 


Properties and RPCs within components have the same support as an Actor’s properties and RPCs.

> The SpatialOS Unreal GDK does not yet support dynamic component replication.

## Property handover

Actor property handover is a new feature we’ve added to Unreal. It replicates server-side properties between servers so Unreal games can take advantage of the SpatialOS cloud server architecture. For more information, see [the Actor property handover](./content/handover-between-server-workers.md) documentation.

## Singleton Actors

“Singleton Actors” is a new term that describes Actors which contain global state and logic.  Only one of these Actors exists in a game world. For more information see the documentation on [Singleton Actors](https://github.com/spatialos/UnrealGDK/blob/master/docs/contents/singleton-actors.md).

**Note:** Currently, the SpatialOS Unreal GDK supports proper replication of public singletons, such as GameState, between multiple servers. It does not support private singletons, such as GameMode, but we plan to support this in the future..

## Networking switch

We’ve implemented a way to switch between the native Unreal networking to SpatialOS networking with a command line argument. This is useful to test valid functionality on a single server and performance. See the documentation on the networking switch in the [Troubleshooting](??? Not sure of doc link) documentation.