Singleton Actors

<%(TOC)%>

# Concepts: authority and interest

> **Tip:** Before you read this page, you should read [What is SpatialOS?]({{urlRoot}}/content/spatialos-concepts/what-is-spatialos), [World, entities, components]({{urlRoot}}/content/spatialos-concepts/world-entities-components), and [Workers and load balancing]({{urlRoot}}/content/spatialos-concepts/workers-and-load-balancing). 

When you use [zoning]({{urlRoot}}/content/spatialos-concepts/workers-and-load-balancing#zoning) in your SpatialOS game world, one of the central ideas is that your worker instances have access only to a part of the game world. This access is governed both by what the worker instance has _authority_ over and what it has _interest_ in.

## Authority (known as “write access authority”)
Every SpatialOS component on every entity in the world needs to be simulated. That is, it has computation associated with it, and something needs to carry out that computation.

For example, if there’s an NPC with a `Position` component, you probably want something to move that NPC around in the world.

But you don’t want more than one worker instance at a time to be able to write to a component. So SpatialOS uses the concept of write access authority: for any component on any entity, there is never more than one server-worker instance which has write access authority over it (that is, is able to write to it).

Write access authority is a responsibility: the responsibility to carry out the computation required for a component. This is the case for both client-worker and server-worker instances, however, it’s only server-worker instances that share the game world between them, with multiple areas of authority. When an entity moves from one server-worker instance’s area of authority to another, write access authority over the entity’s components is [handed over]({{urlRoot}}/content/actor-handover).

How are those areas of authority defined? You define them when you decide on the load balancing strategy for the server-worker type that has write access permission to a component type. The “load” in load balancing is the computation associated with the components which instances of the server-worker type have write access permission to.

![Areas of authority]({{assetRoot}}assets/screen-grabs/authority-areas.png)

_Image: Areas of authority for three server-worker instances: each instance has write access authority over certain components in their area of authority. Which components they have write access authority over depends on their worker type’s write access permissions._

Because different server-worker instances have different areas of authority, we created a type of RPC that enables a server-worker instance which does not have authority over an entity to tell the server-worker instance that does have authority over that entity to make an update to it. We call this a [cross-server RPC]({{urlRoot}}/content/cross-server-rpcs). This is necessary if you’re using [zoning]({{urlRoot}}/content/spatialos-concepts/workers-and-load-balancing#zoning) because areas of authority mean that one server-worker instance can’t make updates to every entity in the world; it can make updates only to the entities in its area of authority.

For information on how to set up authority in the GDK, see [Authority]({{urlRoot}}/content/authority).

## Interest
To carry out the computation associated with a component, a server-worker instance needs more than  just write access authority over that component. It also needs to know the status of other components on other entities which it doesn’t have write access authority over.

For example, with your NPC moving around the world, it might need to behave differently depending on what’s nearby. A rabbit might run towards a nearby lettuce, or away from a nearby fox. Even if the lettuce or the fox are in a different area of authority to the rabbit, the rabbit still needs to behave correctly.

To deal with this, a server-worker instance has interest. That is, it wants to read the state of entity components, even if it doesn’t have write access authority over them.
For example, a server-worker instance might have interest in every object within a 100m radius of the components it has write access authority over. 

Note that interest doesn’t only apply to server-workers: a client-worker instance might have interest in objects nearby, but also really big objects far away. It has interest in distant mountains, because it needs to render them so the player can see them as they play the game.

![Areas of interest]({{assetRoot}}assets/screen-grabs/interest-areas.gif)

_Animation: Our three server-worker instances have interest in entity components which are outside each of their areas of authority._

For information on how to set up interest in the GDK, see Query-based interest (TODO add link).

## Authority and interest - what’s the difference?

When we talk about authority in SpatialOS, we mean a worker instance does the computation relevant to a component. Authority also means the worker instance sends updates about that component to the SpatialOS entity database, so other worker instances know about the changes to the component. You can think of a worker instance which has authority over a component as having write access to the component. We call this write access authority.

When we talk about interest in SpatialOS, we mean a worker instance wants to receive updates about components from the SpatialOS entity database. Whether a worker instance does receive updates depends on certain criteria, but when it does, you can think of the worker instance as having read access to the component. We call this active read access.