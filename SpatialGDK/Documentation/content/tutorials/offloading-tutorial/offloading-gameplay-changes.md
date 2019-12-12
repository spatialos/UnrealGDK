

# Multiserver offloading

## 2. Gameplay Changes

When using offloading, there are a few changes to gameplay logic that you need to make to support using multiple servers.

### RPCs

Sending an RPC to an Actor that is owned by a different server require using [Cross-server RPCs]({{urlRoot}}/content/technical-overview/gdk-concepts#cross-server-rpcs). This can be done in code using a custom function paramter.
[block:code]
{
  "codes": [
  {
      "code": "UFUNCTION(CrossServer, Reliable) \n void TakeDamageCrossServer(float Damage, const struct FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser);",
      "language": "text"
    }
  ]
}
[/block]
Or in Blueprints using the custom replication setting


<%(Lightbox image="{{assetRoot}}assets/offloading-project/blueprint-cross-server-rpc.png")%>

The Example Project contains a custom Pawn subclass `ACrossServerPawn` that overrides the standard TakeDamage behaviour with a cross-server version. This allows any server worker to use the standard TakeDamage method and have it routed to the correct server worker.

### BeginPlay

When using Offloading, the server worker that spawns an Actor might not end up having authority over it. To be able to run initialization logic on the correct worker, The GDK adds an OnAuthorityGained Event. When using offloading, this event gets called once on the owning worker for the Actor, allowing you to do any initialization needed. For backwards compatibility, When SpatialNetworking is disabled, OnAuthorityGained will get called immediately after BeginPlay on the Server.

In the Example Project, the Crashbots disable the Standard 'Auto Posses AI' behaviour to avoid spawning AIControllers on the wrong servers. Instead, OnAuthorityGained is used to spawn the default controller.

<%(Lightbox image="{{assetRoot}}assets/offloading-project/spawn-default-controller.png")%>

### Utility Methods

For more complex behaviours using ActorGroups, you can use the utility methods in `SpatialStatics`. Using ownership of actor groups to control game logic allows you to change your server configuration without having to make code changes.

When Offloading is disabled all Actor Classes belong to a 'Default' actor group, which is owned by the 'Default Worker Type' defined in the Offloading Settings.

<%(Lightbox image="{{assetRoot}}assets/offloading-project/offloading-statics.png")%>

### Conclusion

By using offloading, you can scale your CPU heavy tasks by splitting them to run on instances of different server-workers. You don’t need to worry about the challenges involved with migrating entities across worker boundaries.

Meanwhile, offloading introduces a set of constraints that you need to take care of.  When you set up your feature using offloading properly, you can have significant scaling opportunities.

<br/>------------<br/>
_2019-08-29 Page updated without editorial review_
_2019-07-30 Page added without editorial review_
<br/>
<br/>
[//]: # (TODO: https://improbableio.atlassian.net/browse/DOC-1142)
------------
