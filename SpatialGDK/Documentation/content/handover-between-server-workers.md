# Actor handover between server-workers
<%(TOC)%>

Actor handover (`handover`) is a new `UPROPERTY` tag. It allows games built in Unreal (which uses single-server architecture) to take advantage of SpatialOS’ distributed, persistent server architecture. 

In Unreal’s native single-server architecture, your game server holds the canonical state of the whole game world. As there is a single game server, there are Actor properties that the server doesn’t need to share with any other server or clients. These properties only need to exist in the game server’s local process space.

In SpatialOS games, the work of the server is spread across several servers (known as “server-workers” in SpatialOS). (Note that in SpatialOS, game clients are “client-workers” - there’s more information on [workers](https://docs.improbable.io/reference/latest/shared/concepts/workers) in the SpatialOS documentation.)

As Unreal expects there to be only one server, rather than several servers, the SpatialOS GDK for Unreal has a custom solution to take advantage of the SpatialOS distributed server architecture. This involves a handover of responsibility for an Actor and its properties between server-workers. (Actors approximate to “entities” in SpatialOS, so we refer to them as “entities” when we are talking about what happens to them in SpatialOS - handily, “properties” in an entity’s components in SpatialOS map to replicated Actor properties. You can find out more about [entities, components and properties](https://docs.improbable.io/reference/latest/shared/concepts/entities) in the SpatialOS documentation.)

Server-workers have [authority]({{urlRoot}}/content/glossary#authority) over entities, meaning that they are responsible for properties of an entity. Only one server-worker has authority over the properties of an entity at a time. In order to [load balance](https://docs.improbable.io/reference/latest/shared/glossary#load-balancing) (SpatialOS documentation) between server-workers, each server-worker has only a certain area of authority, so each server-worker has a boundary.
This means that, at the boundary between server-worker 1 and server-worker 2,  server-worker 1 needs to transfer authority of entity properties to server-worker 2 so that server-worker 2 can seamlessly continue to simulate the entity exactly where server-worker 1 stopped. (See SpatialOS documentation on [`AuthorityChange`](https://docs.improbable.io/reference/latest/shared/design/operations#authoritychange).)

Note that server-worker authority over properties is different to server-worker interest in properties. See SpatialOS documentation on [worker interest](https://docs.improbable.io/reference/latest/shared/glossary#interest).

## How to facilitate Actor handover

To facilitate an Actor’s property handover between server-workers, follow the instructions below:

1.  If your property is defined in a native C++ class, mark the property field with a `Handover` tag in the `UPROPERTY` macro, as shown in the example below.<br/><br/>

    ```
    UPROPERTY(Handover)
    float MyServerSideVariable;
    ```

1. Alternatively, if your property is defined in a Blueprint class, in the Blueprint Editor, set the **Variable**'s  **Replication** setting to `Handover` . <br/><br/>
![Blueprint Editor]({{assetRoot}}assets/screen-grabs/handover-blueprint.png)

1. Tag the Actor with the `SpatialType` specifier. (See documentation on [SpatialType]({{urlRoot}}/content/spatial-type) for guidance.)

1. Generate the [schema]({{urlRoot}}/content/glossary#schema-generation) for your Actor’s class. (In the Unreal Editor, from the [GDK toolbar]({{urlRoot}}/content/toolbars), select the **Schema** icon.)

The GDK now ensures that server-workers transfer these tagged Actor’s properties between them.

## Native-Unreal class properties handover
To ensure native-Unreal classes work with the GDK for Unreal, we are making handover-related changes on a class-by-class basis as we identify appropriate properties for `Handover` tags.

**Classes with properties tagged with `Handover` status (as of 2018-10-26)**

* `UCharacterMovementComponent`
* `APlayerController`
* `MovementComponent`

We will continue to extend our support to more built-in Actor and component types.

## The difference between `Replicated` and `Handover` tags
It’s important to understand that the native-Unreal tag `Replicated` and GDK for Unreal `Handover` tag have different uses:

* `Replicated` tags identify Actor properties that any client-worker or server-worker needs to have interest in.
* `Handover` tags identify Actor properties that only server-workers need to have interest in and allow server-workers to transfer authority between them.

Note that while you could replace all `Handover` tags with `Replicated` tags and your simulation would function correctly, its network performance could suffer. This is because there are a lot of workers with interest in `Replicated`-tagged properties. `Handover`-tagged properties have limited worker interest; they only need to be serialized on demand to server-workers taking over authority.
