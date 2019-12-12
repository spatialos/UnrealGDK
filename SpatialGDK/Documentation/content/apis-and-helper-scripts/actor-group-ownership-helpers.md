

# Actor group ownership helpers

Unlike the [authority callbacks]({{urlRoot}}/content/authority#authority-callbacks), which indicate when authority is gained or lost, when you write gameplay code for a feature using offloading, you might need to query whether an actor or a class is part of an actor group. Now you can use the GDK to achieve it through the `USpatialStatics` class. 

The following table lists the APIs available through the `USpatialStatics` class. You can call these functions both in C++ and from Blueprint classes:

## `IsActorGroupOwnerForClass`

Returns `true` if the current worker type owns the Actor group the given Actor Class belongs to. Equivalent to `World->GetNetMode() != NM_Client` when Spatial Networking is disabled.

![img]({{assetRoot}}assets/offloading-apis/is-actor-group-owner-for-class.png)

## `IsActorGroupOwnerForActor`

Returns `true` if the current worker type owns the Actor group that the Actor belongs to. Equivalent to `World->GetNetMode() != NM_Client` when Spatial Networking is disabled.

![img]({{assetRoot}}assets/offloading-apis/is-actor-group-owner-for-actor.png)

## `IsActorGroupOwner`

Returns `true` if the current worker type owns this Actor Group. Equivalent to `World->GetNetMode() != NM_Client` when Spatial Networking is disabled.

![img]({{assetRoot}}assets/offloading-apis/is-actor-group-owner.png)

## `GetActorGroupForClass`

Returns the `ActorGroup` that the Actor Class belongs to.


![img]({{assetRoot}}assets/offloading-apis/get-actor-group-for-class.png)

## `GetActorGroupForActor`

Returns the `ActorGroup` that the Actor belongs to.

![img]({{assetRoot}}assets/offloading-apis/get-actor-group-for-actor.png)

<br/>------------<br/>
_2019-07-30 Page added as limited editorial review_
<br/>
<br/>
[//]: # (TODO: https://improbableio.atlassian.net/browse/DOC-1142)
------------
