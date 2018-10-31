# Dynamic Typebindings
To allow Unreal to replicate through the SpatialOS network stack and combine multiple dedicated server instances across one seamless game world, the GDK has to adapt Unreal networking functionality to work with the [SpatialOS Worker API (SpatialOS documentation)](https://docs.improbable.io/reference/latest/capi/introduction). It does  this seamlessly using `Dynamic Typebindings`. `Dynamic Typebindings` operate at [runtime]({{urlRoot}}/content/glossary#spatialos-runtime) so that your development iteration speed is not affected, despite your network code running on a completely different representation than Unreal’s.

At the heart of `Dynamic Typebindings` are the dynamically generated [SpatialOS schema]({{urlRoot}}/content/glossary#schema), which are the schema representation of any Unreal object, its replicated data, and RPCs. `Dynamic Typebindings` also include the binding code that is invoked when converting network-relevant data between native Unreal and SpatialOS.

# Schema
The [schema]({{urlRoot}}/content/glossary#schema) used in `Dynamic Typebindings` are generated via the Unreal Editor, for all classes tagged with the [Spatial Type]({{urlRoot}}/content/spatial-type) specifier. For each Unreal object that the GDK generates schema for, there are a number of possible schema components generated, each serving a different function.

For Unreal Actors and sub-objects:

* Replicated property schema component (eg. _MyActor_): Contains all the replicated properties (including inherited) present on the object, except those tagged with the `COND_OwnerOnly` or `COND_AutonomousOnly` replication condition.
* Owner-only schema component (eg. _MyActorOwnerOnly_): Contains all the `COND_OwnerOnly` or `COND_AutonomousOnly` replicated properties excluded from the replicated property schema component.
* Handover schema component (eg. _MyActorHandover_): Contains all handover properties (including inherited) present on the object.

In additional, Unreal Actors also generate (where relevant):

* Client/Server/CrossServer RPC schema components (eg. _MyActorClientRPCS_)- Each RPC category has its own SpatialOS component containing all the Actor’s RPCs for that category converted into SpatialOS commands.
* NetMulticast RPC schema component (eg. _MyActorNetMulticastRPCS_)- Contains all the multicast RPCs callable on this Actor, converted into SpatialOS [events (SpatialOS documentation)](https://docs.improbable.io/reference/latest/shared/glossary#event).
* Static subobject schema components - For each static subobject present on this Actor, additional components are generated which wrap the replicated and handover properties defined in the common components mentioned above.

> Note: We don’t currently support dynamic components.

When comparing the two network stacks, it’s useful to keep the following mappings in mind:

* Unreal Actor <-> SpatialOS entity
* Unreal Replicating Property <-> SpatialOS field
* Unreal Client/Server RPC <-> SpatialOS command
* Unreal NetMulticast RPC <-> SpatialOS event
* Unreal Replication Condition <-> SpatialOS component design