<%(TOC)%>

# SpatialOS concepts: authority and interest

> **Tip:** Before you read this page, you should read [What is SpatialOS?]({{urlRoot}}/content/spatialos-concepts/what-is-spatialos), [World, entities, components]({{urlRoot}}/content/spatialos-concepts/world-entities-components), and [Workers and zoning]({{urlRoot}}/content/spatialos-concepts/workers-and-zoning).

In Unreal’s single-server architecture, authority over an Actor stays with the single server; an Actor’s properties never leave the server’s memory. When you use multiple server-worker instances in your SpatialOS game world (known as [zoning]({{urlRoot}}/content/spatialos-concepts/workers-and-zoning#zoning) - a feature that is currently in pre-alpha), each instance has access only to a part of the game world. This access is governed both by what the worker instance has _authority_ over and what it has _interest_ in.

## Authority
Every Actor needs a worker instance to compute it. If your game has only one server-worker instance (which is the default for the GDK), this instance has authority over all the Actors. But if you’re using zoning, you must have only one worker instance at a time that is able to write to an Actor. So SpatialOS makes sure that for any Actor, there is never more than one server-worker instance which has authority over it (that is, is able to send updates about it).

Authority is a responsibility: the responsibility to carry out the computation required for an Actor. This is the case for both client-worker and server-worker instances, however, it’s only server-worker instances that can share the game world between them, with multiple areas of authority (zoning). When an Actor moves from one server-worker instance’s area of authority to another, authority over the Actor is [handed over]({{urlRoot}}/content/actor-handover). This allows the second server-worker instance to continue where the first one left off.

When you’re using zoning, one server-worker instance can’t make updates to every Actor in the game world; it can make updates only to the Actors in its area of authority. This means you need a way to allow a server-worker instance that does _not_ have authority over an Actor to tell the server-worker instance that _does_ have authority over that Actor to make an update to it. You use [cross-server RPCs]({{urlRoot}}/content/cross-server-rpcs) to do this.

For information on how to set up authority in the GDK, see [Authority]({{urlRoot}}/content/authority).

## Interest
To carry out the computation associated with an Actor, a server-worker instance needs more than just authority over the Actor. It also needs to know about other Actors that it doesn’t have authority over, but that are relevant to the Actors that it does have authority over.

For example, with your NPC moving around the world, it might need to behave differently depending on what’s nearby. A rabbit might run towards a nearby lettuce, or away from a nearby fox. Even if the lettuce or the fox are in a different area of authority to the rabbit, the rabbit still needs to behave correctly.

To deal with this, a server-worker instance has interest. That is, it wants to receive updates about Actors, even if it doesn’t have authority over them. For example, a server-worker instance might have interest in every object within a 100m radius of the Actors it has authority over.

Note that interest doesn’t only apply to server-workers: a client-worker instance might have interest in objects nearby, but also really big objects far away. It has interest in distant mountains, because it needs to render them so the player can see them as they play the game.

You don’t need to set up interest for server-worker types; the GDK does this for you. For information on how to set up interest for client-worker types, see [Game client interest management]({{urlRoot}}/content/game-client-interest-management).

## Authority and interest - what’s the difference?

Authority in SpatialOS means that a worker instance does the computation relevant to an Actor. Authority also means the worker instance sends updates about that Actor to the SpatialOS entity database, so that other worker instances know about the changes to the Actor.

Interest in SpatialOS means a worker instance wants to receive updates about Actors from the SpatialOS entity database. 

<!--
TODO
WIP QBI doc: https://improbableio.atlassian.net/browse/UNR-1210
-->
</br>
</br>------</br>
_2019-07-31 Page updated with limited editorial review_
</br>_2019-05-21 Page added with editorial review_