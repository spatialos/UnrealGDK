# Dynamic Typebindings
To allow Unreal to replicate through the SpatialOS network stack and combine multiple dedicated server instances across one seamless game world, you need to adapt Unreal networking functionality to work with the [SpatialOS Worker API (SpatialOS documentation)](https://docs.improbable.io/reference/latest/capi/introduction). You can do this seamlessly using `Dynamic Typebindings`. `Dynamic Typebindings` operate at runtime so that your iteration speed is not affected despite your network code running on a completely different representation than Unreal’s.

At the heart of `Dynamic Typebindings` are the dynamically generated [SpatialOS schema](https://docs.improbable.io/reference/latest/shared/schema/introduction), which are the schema representation of any Unreal object and its replicated data and RPCs. `Dynamic Typebindings` also include the binding code that is invoked when converting network relevant data between native Unreal and SpatialOS.

# Schema
The schema used in `Dynamic Typebindings` are generated via the Unreal Editor, for all classes tagged with the [SpatialType]({{urlRoot}}/content/spatial-type) specifier. For each Unreal object that has schema generated for it, there are a number of possible schema components generated, each serving a different function.

For Unreal Actors and subobjects:
* Replicated property component (eg. MyActor) - Contains all the replicated properties (including inherited) present on the object, except those tagged with the `COND_OwnerOnly` or `COND_AutonomousOnly` replication condition.
* Owner only component (eg. MyActorOwnerOnly) - Contains all the `COND_OwnerOnly` or `COND_AutonomousOnly` replicated properties excluded from the Core component.
* Handover component (eg. MyActorHandover) - Contains all handover properties (including inherited) present on the object.

In additional, Unreal Actors also generate (where relevant):
* Client/Server/CrossServer RPC components (eg. MyActorClientRPCS)- Each RPC category has its own SpatialOS component containing all the Actor’s RPCs for that category converted into SpatialOS commands.
* NetMulticast RPC component (eg. MyActorNetMulticastRPCS)- Contains all the multicast RPCs callable on this Actor, converted into SpatialOS [events (SpatialOS documentation)](https://docs.improbable.io/reference/latest/shared/glossary#event).
* Static subobject components - For each static subobject present on this Actor, additional components are generated which wrap the replicated and handover properties defined in the common components mentioned above.

> Note: We don’t currently support dynamic components.

When comparing the two network stacks, it’s useful to keep the following mappings in mind:
* Unreal Actor <-> SpatialOS entity
* Unreal Replicating Property <-> SpatialOS field
* Unreal Client/Server RPC <-> SpatialOS command
* Unreal Multicast RPC <-> SpatialOS event
* Unreal Replication Condition <-> SpatialOS component design


