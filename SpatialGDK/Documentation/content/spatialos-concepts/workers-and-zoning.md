

# SpatialOS concepts: workers and zoning

> **Tip:** Before you read this page, you should read [What is SpatialOS?]({{urlRoot}}/content/spatialos-concepts/what-is-spatialos) and [World, entities, components]({{urlRoot}}/content/spatialos-concepts/world-entities-components).

## Worker instances and worker types
In the same way that Unreal uses a single server and multiple clients, SpatialOS has _server-worker instances_ and _client-worker instances_:

* SpatialOS uses one or more _server-worker instances_ to compute your game world.
* Each player uses a _client-worker instance_ to connect to and interact with your game world.

You can think of a _worker type_ as a sort of mold for these worker instances. Worker types are what you write the code for; instances of these types manage and interact with the world. Each worker type has a worker configuration file (<worker_type>.worker.json) where you define how SpatialOS should build, launch, and interact with instances of this worker type.

For example, you could create a server-worker type called `UnrealWorker`, and your deployment could have many instances of this worker type simulating the game world.

## Zoning
As outlined above, SpatialOS can use the combined computation of multiple server-worker instances to compute the game world. We call this _zoning_ - splitting up the world into zones (areas of authority). Each zone is simulated by one server-worker instance. This means that the server-worker instances don’t know anything about each other - they might not even be on the same machine in the cloud.

So when you’re writing the code to define server-worker types, you set them up so that their worker instances only know about a specific part (or parts) of the world.

![Multiple server-worker instances on separate machines]({{assetRoot}}assets/screen-grabs/workers-machines.png)
_Image: Multiple server-worker instances spread across different machines_

> **Tip:** You can switch between Unreal networking and SpatialOS networking from the Unreal toolbar to speed up development iteration.
> 
> * Using **SpatialOS networking** helps you isolate issues in your game that are related to having multiple server-worker instances.
> 
> * However, using **Unreal networking** lets you iterate on development faster, because it means you can skip various steps (such as generating [schema]({{urlRoot}}/content/how-to-use-schema)) when you want to test your changes.

## Deployments
SpatialOS hosts your games for you. We call an instance of a running game a _deployment_.
In a deployment, SpatialOS starts server-worker instance(s) for you, running them on machines in the cloud. You don’t need to interact with the machines directly.

![A SpatialOS deployment]({{assetRoot}}assets/screen-grabs/deployment.png)
_Image: A SpatialOS deployment, with connected worker instances_

## Client-worker instances
Because each client-worker instance is tied to a player, and runs on the player's local machine, SpatialOS doesn't manage a client-worker instance's workload in the same way as it manages a server-worker instance's workload. This means that during game development, you set up client-worker types and server-worker types differently. The main difference is around how you synchronize data to and from the game world.
Like server-worker instances, client-worker instances can only see a part of the world. However, client-worker instances can see across server-worker instance boundaries.

![Client-worker instances]({{assetRoot}}assets/screen-grabs/client-workers.png)
_Image: A client-worker instance can "see" nearby entities, regardless of the boundaries between server-worker instances_

<!--
TODO
We need to create the following how-to docs:
Creating worker types https://improbableio.atlassian.net/browse/DOC-1064
Setting up load balancing https://improbableio.atlassian.net/browse/DOC-1065
-->

<br/>
</br>------</br>
_2019-07-31 Page updated with limited editorial review_
</br>_2019-05-21 Page added with editorial review_
