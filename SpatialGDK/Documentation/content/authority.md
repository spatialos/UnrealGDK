<%(TOC)%>
# Authority
To work with authority in the GDK, it’s useful to refresh on authority in Unreal’s native networking. See:

* [Unreal’s Network Role documentation](https://wiki.unrealengine.com/Replication#A_Guide_To_Network_Roles)
* [Unreal's Owning Connection documentation](https://docs.unrealengine.com/en-us/Gameplay/Networking/Actors/OwningConnections)

## Unreal networking authority

In native-Unreal networking, the single server has absolute authority over all replicated Actors, while clients have only proxies of Actors. Unreal models this using the `Role` and `RemoteRole` fields within an Actor.

There are 3 main types of networking Roles in Unreal:

* `ROLE_Authority` - The authoritative version of the Actor.
* `ROLE_SimulatedProxy` - Locally simulates the state of the Actor from the server.
* `ROLE_AutonomousProxy` - Locally simulates the state of the Actor but with the ability to execute RPCs on the Actor. This is usually reserved for Actors that are "owned" by a client

For more information about network roles, see [Unreal’s Network Role documentation](https://wiki.unrealengine.com/Replication#A_Guide_To_Network_Roles).

For the majority of use cases in native Unreal, these fields have the values:

Actor on **server**:

* `Role = ROLE_Authority`
* `RemoteRole = ROLE_SimulatedProxy`

Actor on **client**:

* `Role = ROLE_SimulatedProxy`
* `RemoteRole = ROLE_Authority`

Actors that have an [owning connection (Unreal documentation)](https://docs.unrealengine.com/en-us/Gameplay/Networking/Actors/OwningConnections) are slightly different. An Actor can have an owning connection if:

* It is a `PlayerController` with an associated `NetConnection`.
* It is a `Pawn` which is possessed by a `PlayerController` that has an associated `NetConnection`.
* Its `Owner` is set to an Actor that has an owning connection.

For example, if a client has a `PlayerController` which possesses a `Character` which is holding a gun (whose `Owner` is the `Character`), all three (the gun, `Character` and `PlayerController`) have an owning connection.

In any of these cases, Unreal authority looks like:

Actor on **server** with an **owning connection**:

* `Role = ROLE_Authority`
* `RemoteRole = ROLE_AutonomousProxy`

Actor on **owning client**:

* `Role = ROLE_AutonomousProxy`
* `RemoteRole = ROLE_Authority`

Actor on **non-owning client**:

* `Role = ROLE_SimulatedProxy`
* `RemoteRole = ROLE_Authority`

## GDK authority

As the GDK works with multiple [server-workers]({{urlRoot}}/content/glossary#workers), rather than a single server, authority needs to be dictated by SpatialOS so that authority is shared between server-workers. This means server-workers have authority over some Actors but don’t have authority over other Actors, depending on how SpatialOS assigns authority. **This is a key difference between the GDK and native Unreal networking!**

> We use the term “authoritative” when a server-worker has authority over an Actor and “non-authoritative” when it doesn’t.

In the GDK, a server-worker is authoritative over an Actor if it has authority over the [schema]({{urlRoot}}/content/glossary#schema) [component]({{urlRoot}}/content/glossary#spatialos-component) `Position`. 

So, in the SpatialOS GDK multiserver scenario, authority looks like this:

Actor on **authoritative server-worker**:

* `Role = ROLE_Authority`
* `RemoteRole = ROLE_SimulatedProxy`

Actor on **non-authoritative server-worker**:

* `Role = ROLE_SimulatedProxy`
* `RemoteRole = ROLE_Authority`

Actor on **[client-worker]({{urlRoot}}/content/glossary#workers)**:

* `Role = ROLE_SimulatedProxy`
* `RemoteRole = ROLE_Authority`

The GDK models owning connections using a schema component which represents the `ClientRPC`s. The worker authoritative over the `ClientRPC` schema [component]({{urlRoot}}/content/glossary#spatialos-component) is the client which owns the Actor.

In the same example as above, with the `PlayerController`, `Character` and gun, the client-worker would be authoritative over the `ClientRPC` schema component on the `PlayerController`, `Character` and gun [entities]({{urlRoot}}/content/glossary#spatialos-entity). Authority over this schema component dictates the assignment of `ROLE_AutonomousProxy`.

Actor on **authoritative server-worker** with an **owning connection**:

* `Role = ROLE_Authority`
* `RemoteRole = ROLE_AutonomousProxy`

Actor on **non-authoritative server-worker** with an **owning connection**:

* `Role = ROLE_SimulatedProxy`
* `RemoteRole = ROLE_Authority`

Actor on **owning client-worker**:

* `Role = ROLE_AutonomousProxy`
* `RemoteRole = ROLE_Authority`

Actor on **non-owning client-worker**:

* `Role = ROLE_SimulatedProxy`
* `RemoteRole = ROLE_Authority`

A `PlayerController` possessing different `Pawn`s would change their role as expected; the newly-possessed Pawn will become an autonomous proxy on the client-worker while the older `Pawn` will become a simulated proxy.

## Authority Callbacks

Due to authority being dynamic in the GDK, we've added events that can trigger behavior when authority changes. These events will only ever trigger on server-workers.

`Role` and `RemoteRole` will be properly set to their correct values within these events.

There are two kinds of authority events:

### OnAuthorityGained

Triggered when authority is gained over an Actor.

To use, override `void OnAuthorityGained()` in your Actor or use the blueprint event. 

    void AMyActor::OnAuthorityGained()
    {
        Super::OnAuthorityGained(); // Mandatory

        // Custom behavior when authority is gained.
        // ...
    }

![OnAuthorityGained]({{assetRoot}}assets/screen-grabs/on-authority-gained.jpg)

### OnAuthorityLost

Triggered when authority is lost over an Actor.

To use, override `void OnAuthorityLost()` in your Actor or use the blueprint event. 

    void AMyActor::OnAuthorityLost()
    {
        Super::OnAuthorityLost(); // Mandatory

        // Custom behavior when authority is lost.
        // ...
    }

![OnAuthorityLost]({{assetRoot}}assets/screen-grabs/on-authority-lost.jpg)

These events have the same calling order as `BeginPlay()` or `Tick()`.

Behavior that is triggered when authority is gained over an Actor Component or Subobject should be fired through the owning Actor.
