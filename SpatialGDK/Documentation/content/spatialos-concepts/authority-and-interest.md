<%(TOC)%>

# Concepts: authority and interest

> **Tip:** Before you read this page, you should read [What is SpatialOS?]({{urlRoot}}/content/spatialos-concepts/what-is-spatialos), [World, entities, components]({{urlRoot}}/content/spatialos-concepts/world-entities-components), and [Workers and load balancing]({{urlRoot}}/content/spatialos-concepts/workers-and-load-balancing). 

In Unreal’s single-server architecture, authority over an Actor stays with the single server; an Actor’s properties never leave the server’s memory. When you use multiple server-worker instances in your SpatialOS game world (known as [zoning]({{urlRoot}}/content/spatialos-concepts/workers-and-load-balancing#zoning)),  each worker instance has access only to a part of the game world. This access is governed both by what the worker instance has _authority_ over and what it has _interest_ in.

## Authority
Every SpatialOS component on every entity in the world needs a worker instance to simulate it. But you don’t want more than one worker instance at a time to be able to write to a component. So SpatialOS uses the concept of authority, which we call “write access authority” for clarity: for any component on any entity, there is never more than one server-worker instance which has write access authority over it (that is, is able to write to it).

Write access authority is a responsibility: the responsibility to carry out the computation required for a component. This is the case for both client-worker and server-worker instances, however, it’s only server-worker instances that share the game world between them, with multiple areas of authority. When an entity moves from one server-worker instance’s area of authority to another, write access authority over the entity’s components is [handed over]({{urlRoot}}/content/actor-handover).

<%(#Expandable title="How are those areas of authority defined?")%>You define areas of authority when you decide on the load balancing strategy for the server-worker type that has write access permission to a component type. The “load” in load balancing is the computation associated with the components which instances of the server-worker type have write access permission to.<%(/Expandable)%>

![Areas of authority]({{assetRoot}}assets/screen-grabs/authority-areas.png)

_Image: Areas of authority for three server-worker instances: each instance has write access authority over certain components in their area of authority. Which components they have write access authority over depends on their worker type’s write access permissions_

### Authority in the GDK

Having multiple server-worker instances computing your game world means that authority needs to pass from one server-worker instance to another as an Actor moves around. Passing authority, known as [Actor handover]({{urlRoot}}/content/actor-handover), allows the second server-worker instance to continue where the first one left off. 

Because each server-worker instance has its own area of authority means that one server-worker instance can’t make updates to every entity in the world; it can make updates only to the entities in its area of authority. This means we need a way to allow a server-worker instance that does _not_ have authority over an entity component to tell the server-worker instance that _does_ have authority over that entity component to make an update to it. We use [cross-server RPCs]({{urlRoot}}/content/cross-server-rpcs) to do this.

For information on how to set up authority in the GDK, see [Authority]({{urlRoot}}/content/authority).

## Interest
To carry out the computation associated with a component, a server-worker instance needs more than just write access authority over that component. It also needs to know the status of other components on other entities which it doesn’t have write access authority over.

For example, with your NPC moving around the world, it might need to behave differently depending on what’s nearby. A rabbit might run towards a nearby lettuce, or away from a nearby fox. Even if the lettuce or the fox are in a different area of authority to the rabbit, the rabbit still needs to behave correctly.

To deal with this, a server-worker instance has interest. That is, it wants to read the state of entity components, even if it doesn’t have write access authority over them.
For example, a server-worker instance might have interest in every object within a 100m radius of the components it has write access authority over. 

Note that interest doesn’t only apply to server-workers: a client-worker instance might have interest in objects nearby, but also really big objects far away. It has interest in distant mountains, because it needs to render them so the player can see them as they play the game.

![Areas of interest]({{assetRoot}}assets/screen-grabs/interest-areas.gif)

_Animation: Our three server-worker instances have interest in entity components which are outside each of their areas of authority_

## Authority and interest - what’s the difference?

Authority in SpatialOS means that a worker instance does the computation relevant to a component. Authority also means the worker instance sends updates about that component to the SpatialOS entity database, so other worker instances know about the changes to the component. You can think of a worker instance which has authority over a component as having write access to the component. This is called “write access authority”.

Interest in SpatialOS means a worker instance wants to receive updates about components from the SpatialOS entity database. Whether a worker instance does receive updates depends on certain criteria, but when it does, you can think of the worker instance as having read access to the component. This is called “active read access”.

<!--
TODO
WIP QBI doc: https://improbableio.atlassian.net/browse/UNR-1210
-->
</br>------</br>
_2019-05-21 Page added with editorial review_