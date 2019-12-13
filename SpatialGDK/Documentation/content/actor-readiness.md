

# Actor readiness
Before executing any logic that requires or affects a replicated Actor, you must ensure that the Actor is _ready_. What constitutes an Actor’s “readiness” depends on what you want to do with the Actor. When using the GDK, you must define your own validity checks to signal that an Actor is ready.

## Background
When you use native Unreal networking, you cannot guarantee that a client receives Actors in a specific order (for example, the PlayerController before the Pawn). However, in ideal network conditions, the order in which a client receives Actors is usually the same. 

This means you might develop your game with assumptions about when Actors are ready to interact with on clients without creating validity checks to ensure this. (The order in which clients receive Actors can change in adverse network conditions, such as high latency or packet loss, but during development or local playtests these networking issues often don’t arise.)

In the GDK, [server-worker and client-worker instances]({{urlRoot}}/content/glossary#worker) communicate with each other via SpatialOS. Unlike native Unreal, where Actors arrive on clients in a similar order each time in stable networking conditions, SpatialOS can cause Actors to arrive in a different order, with different latencies each time you run your game. This means you need to define validity checks to determine when an Actor is ready to interact with.

## Defining validity checks
A validity check could be, for example, whether the Actor exists on the client-worker instance, whether a particular reference within the Actor is valid (for example the Pawn having a reference to the PlayerController), or whether a setup function has run.

If you are porting your game from native Unreal to the GDK and you have not created validity checks to ensure that Actors are ready, you might find that certain behaviors don't work with SpatialOS, even though they always worked in native Unreal.

### Examples 
The following examples show you how to define validity checks to signal that an Actor is ready, for various use cases.

#### When can I modify my Actor on the server-worker instance?

Whenever you want to modify an Actor, you _must_ check that the server-worker instance has [authority]({{urlRoot}}/content/glossary#authority) over the Actor. The server-worker instance has authority over an Actor if the Actor's `Role` is set to `ROLE_Authority`.

In the GDK, when an Actor is created on the server-worker instance:

0. The server-worker instance has authority over the Actor for the duration of the Actor's `BeginPlay` call.
0. After this call, the server-worker instance loses authority while the [SpatialOS Runtime]({{urlRoot}}/content/glossary#spatialos-runtime) creates an [entity]({{urlRoot}}/content/glossary#entity) for this Actor.
0. When SpatialOS has created the entity, it returns authority over the Actor to the server-worker instance. At this point, the Actor's `Role` changes back to `ROLE_Authority` and [`OnAuthorityGained` is triggered]({{urlRoot}}/content/authority#onauthoritygained) on the Actor. This is an event you can override. 

For more information on authority, refer to the [authority]({{urlRoot}}/content/authority) documentation.

#### When can I access the Pawn from the PlayerController on the client-worker instance?

The PlayerController’s reference to its Pawn has a [replication notify (Unreal documentation)](https://docs.unrealengine.com/en-US/Resources/ContentExamples/Networking/1_4/index.html) called `OnRep_Pawn`. `OnRep_Pawn` is triggered when the reference to the Pawn is set. You can override this event to know when the Pawn has been successfully replicated and is ready for interaction.

For example:

[block:code]
{
  "codes": [
  {
      "code": "void AMyContoller::OnRep_Pawn()\n{\n\tSuper::OnRep_Pawn();\n\t// Interact with the Pawn\n}",
      "language": "text"
    }
  ]
}
[/block]

#### When can an object that my Actor refers to be used on the client-worker instance?

For any replicated reference to an object, you can use [replication notifies (Unreal documentation)](https://docs.unrealengine.com/en-US/Resources/ContentExamples/Networking/1_4/index.html) to know when the referenced object has been replicated on the client-worker instance.

For example:

[block:code]
{
  "codes": [
  {
      "code": "UPROPERTY(ReplicatedUsing=OnRep_MyActorReference);\nAActor* MyActorReference;\nUFUNCTION()\nvoid OnRep_MyActorReference();",
      "language": "text"
    }
  ]
}
[/block]

#### When has my HUD been created on the client-worker instance?

When you create a PlayerController Actor on a server-worker instance, the server-worker instance calls the [`ClientSetHUD` client RPC (Unreal documentation)](https://api.unrealengine.com/INT/API/Runtime/Engine/GameFramework/APlayerController/ClientSetHUD/index.html) to tell the client-worker instance to create the HUD. In native Unreal, it can look as if this RPC is instant and the HUD is immediately available to use, but SpatialOS can make the RPC latent, meaning it might not execute immediately after the client-worker instance receives the PlayerController. 

The PlayerController stores the HUD in the `MyHUD` variable, which doesn't have a replication notify to hook into. Instead, you can make your HUD trigger an event in the PlayerController, using the reference `PlayerOwner`, to let the PlayerController know when the HUD is ready.

<br/>
<br/>------------<br/>
_2019-07-18 Page added with editorial review._
