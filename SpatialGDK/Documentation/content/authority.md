# Authority in the GDK
To work with authority in the GDK, it’s useful to refresh on authority in Unreal’s native networking.

## Unreal networking authority

In native-Unreal networking, the single server has absolute authority over all replicated Actors, while clients have only proxies of Actors. Unreal models this using the `Role` and `RemoteRole` fields within an Actor.

For the majority of use-cases, these fields have the values:

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

For example, if a client has a `PlayerController` which possesses a `Character` which is holding a gun, all three (the gun, `Character` and `PlayerController`) have an owning connection.

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

`ROLE_AutonomousProxy` indicates that this Actor is owned by a client and can receive client RPCs called on the Actor, as well as trigger server RPCs which are handled by the server. For more information on owning connections see the [Unreal documentation](https://docs.unrealengine.com/en-us/Gameplay/Networking/Actors/OwningConnections).

For more information about Actor roles, see [Unreal’s Actor Role documentation](https://wiki.unrealengine.com/Replication#A_Guide_To_Network_Roles).

## GDK authority

As the GDK works with multiple [server-workers]({{urlRoot}}/content/glossary#workers), rather than a single server, authority needs to be dictated by SpatialOS so that it is shared between server-workers. This means server-workers have authority over some Actors but don’t have authority over other Actors, depending on how SpatialOS assigns authority. **This is a key difference between the GDK and traditional Unreal networking!**

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

A `PlayerController` possessing different Pawns would change authority as expected; the client-worker would gain authority over the newly-possessed Pawn while losing authority over the older Pawn.
