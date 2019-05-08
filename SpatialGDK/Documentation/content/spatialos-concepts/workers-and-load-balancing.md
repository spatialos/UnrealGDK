<%(TOC)%>
# Concepts: workers and load balancing

> **Tip:** Before you read this page, you should read [What is SpatialOS?]({{urlRoot}}/content/spatialos-concepts/what-is-spatialos) and [World, entities, components]({{urlRoot}}/content/spatialos-concepts/world-entities-components).

## Worker instances and worker types

In the same way that Unreal uses a single server and multiple clients, SpatialOS has _server-worker instances_ and _client-worker instances_:

* SpatialOS uses one or more _server-worker instances_ to compute your game world.
* Each player uses a _client-worker instance_ to connect to and interact with your game world.

You can think of a _worker type_ as a template for these worker instances. Worker types are what you write the code for; instances of these types manage and interact with the world. Each worker type has a worker configuration file (<worker_type>.worker.json) where you define how SpatialOS should build, launch, and interact with instances of this worker type.

For example, you could create a server-worker type called `UnrealWorker`, and your deployment could have many instances of this worker type simulating the game world.

### Zoning

As outlined above, SpatialOS can use the combined computation of multiple server-worker instances to compute the game world. We call this _zoning_ - splitting up the world into zones, known as “areas of authority”. Each area is simulated by one server-worker instance. This means that the server-worker instances don’t know anything about each other - they might not even be on the same machine in the cloud. 

So when you’re writing the code to define server-worker types, you set them up so that their worker instances only know about a specific part (or parts) of the world.

![Multiple server-worker instances on separate machines]({{assetRoot}}assets/screen-grabs/workers-machines.png)

_Image: Multiple server-worker instances spread across different machines_

### Load balancing

One of the decisions you need to make as a developer is, “How many server-worker instances does my world need?” To decide this, you need to work out how much computation your world needs, and how many server-worker instances you need to do that work. For a very small world, one instance might be enough; in the GDK, the default out-of-the-box setting is that you have only one instance. 

However, if you’re planning to use zoning (multiple server-worker instances), we recommend trying to scale your game early on in development. You should test early on with at least two server-worker instances running your world using SpatialOS networking. It can be hard to reason about how to architect your game properly to deal with re-assignments of authority from one server-worker instance to another. And some problems won’t be obvious until you have multiple server-worker instances simulating your game world.

> **Tip:** You can switch between Unreal networking and SpatialOS networking from the Unreal toolbar to speed up development iteration.

Using **SpatialOS networking** helps you isolate issues in your game that are related to having multiple server-worker instances.

However, using **Unreal networking** lets you iterate on development faster, because it means you can skip various steps (such as generating [schema]({{urlRoot}}/content/how-to-use-schema)) when you want to test your changes.

## Deployments

SpatialOS hosts your games for you. We call an instance of a running game a _deployment_.

As outlined above, you decide how many server-worker instances your world needs, and how to organize them across the world. In a deployment, SpatialOS starts those server-worker instances for you, running them on machines in the cloud. It orchestrates this for you so that you don’t need to interact with the machines directly.

![A SpatialOS deployment]({{assetRoot}}assets/screen-grabs/deployment.png)

_Image: A SpatialOS deployment, with connected worker instances_

## Client-worker instances

Because each client-worker instance is tied to a player, and runs on the player's local machine, SpatialOS doesn't manage a client-worker instance's workload in the same way as it manages a server-worker instance's workload. This means that during game development, you set up client-worker types and server-worker types differently. The main difference is around how you synchronize data to and from the game world.

Like server-worker instances, client-worker instances can only see a part of the world. However, client-worker instances can see across server-worker instance boundaries.

![Client-worker instances]({{assetRoot}}assets/screen-grabs/client-workers.png)

_Image: A client-worker instance can "see" nearby entities, regardless of the boundaries between server-worker instances_

<!---
We need to create the following how-to docs:
Creating worker types https://improbableio.atlassian.net/browse/DOC-1064
Setting up load balancing https://improbableio.atlassian.net/browse/DOC-1065
-->