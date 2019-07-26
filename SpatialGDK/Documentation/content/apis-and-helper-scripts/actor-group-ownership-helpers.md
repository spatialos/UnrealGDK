<%(TOC)%>

# Actor group ownership helpers

Unlike the authority callbacks which tell you when authority is gained or lost, when writing gameplay code for offloaded features, you may need to query for whether an actor or a class is part of an actor group. The GDK provides functionality for doing so through the class USpatialStatics. These functions are also accessible through Blueprint.

The table below details the APIs available through the `USpatialStatics` class:

## `IsActorGroupOwnerForClass`

Returns true if the current Worker Type owns the actor group the given Actor Class belongs to. Equivalent to `World->GetNetMode() != NM_Client` when Spatial Networking is disabled.

![img]({{assetRoot}}assets/offloading-apis/is-actor-group-owner-for-class.png)

## `IsActorGroupOwnerForActor`

Returns true if the current Worker Type owns the actor group this Actor belongs to. Equivalent to `World->GetNetMode() != NM_Client` when Spatial Networking is disabled.

![img]({{assetRoot}}assets/offloading-apis/is-actor-group-owner-for-actor.png)

## `IsActorGroupOwner`

Returns true if the current Worker Type owns this Actor Group. Equivalent to `World->GetNetMode() != NM_Client` when Spatial Networking is disabled.

![img]({{assetRoot}}assets/offloading-apis/is-actor-group-owner.png)

## `GetActorGroupForClass`

Returns the ActorGroup this Actor Class belongs to.

![img]({{assetRoot}}assets/offloading-apis/get-actor-group-for-class.png)

## `GetActorGroupForActor`

Returns the ActorGroup this Actor belongs to.

![img]({{assetRoot}}assets/offloading-apis/get-actor-group-for-actor.png)

<br/>------------<br/>
_2019-07-26 Page added as draft_
<br/>
<br/>
[//]: # (TODO: https://improbableio.atlassian.net/browse/DOC-1142)
------------