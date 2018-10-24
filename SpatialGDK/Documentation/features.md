<%(Callout type="warn" message="This [pre-alpha](https://docs.improbable.io/reference/latest/shared/release-policy#maturity-stages) release of the SpatialOS Unreal GDK is for evaluation and feedback purposes only, with limited documentation - see the guidance on [Recommended use]({{urlRoot}}/index#recommended-use)")%>

# Feature list

The SpatialOS GDK for Unreal Engine works by approximating SpatialOS entities to Unreal Actors. Any properties and commands within an entity’s component work in the way properties and RPCs work when applied to Actors. (You can find out more about SpatialOS [entities, components and properties](https://docs.improbable.io/reference/latest/shared/concepts/entities) in the SpatialOS documentation.)

The GDK alpha release (2018-10-25) has the following features:

## Replicated properties, conditions and notifications
The SpatialOS Unreal GDK supports:

* All types that native Unreal replicates.
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

## Static subobject replication

The GDK currently supports only one instance of each static subobject type when replicating objects owned by an Actor. This includes Actor's components. For example, multiple Actors can have a `FooComponent` but no Actor can have two `FooComponent`s.

Properties and RPCs within subobjects have the same support as an Actor’s properties and RPCs.

The GDK does not yet support dynamic subobject replication.

## Property handover

Actor property handover is a new feature we’re introducing with the GDK. It replicates server-side properties between servers so Unreal games can take advantage of the SpatialOS cloud server architecture. For more information, see [the Actor property handover]({{urlRoot}}/content/handover-between-server-workers) documentation.

## Singleton Actors

`Singleton Actors` is a new term that describes Actors which contain global state and logic.  Only one of these Actors exists in a game world. The SpatialOS Unreal GDK supports both client accessible and server only singletons (respectively known as public and private singletons). For more information see the documentation on [Singleton Actors]({{urlRoot}}/content/singleton-actors).

## Cross-server RPCs

`Cross-server RPC`s are a new type of RPC that functions between two servers. These enable a server without authority over an Actor to communicate with the server which has authority and is able to manipulate the Actor. For more information, see documentation.

[//]: # (TODO: Add link to the Cross-server RPC doc)

## Networking switch

The GDK offers you a convenient way to switch between native Unreal networking and SpatialOS networking via a checkbox in the Unreal Editor. This is useful to test valid functionality and performance on a single server. See the documentation on the networking switch on the [Troubleshooting]({{urlRoot}}/content/troubleshooting) page. To switch networking mode to Unreal, from the Editor toolbar menu, unmark the `Spatial networking` checkbox under `Play` dropdown in the Unreal Editor toolbar.
