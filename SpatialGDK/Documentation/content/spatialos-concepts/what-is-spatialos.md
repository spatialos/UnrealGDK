
# SpatialOS concepts: what is SpatialOS?

SpatialOS is a platform-as-a-service that runs and manages online games in the cloud.

But while it runs your game and manages the infrastructure for you, SpatialOS also enables
something more than that. It runs games in a way that lets them scale further, be more complex, and have long-living persistence.

## How does it work?

The traditional ways to develop large online games mean that you’re either limited by the capacity of a single game server, or you have to shard your game world. 

![The traditional client-server model]({{assetRoot}}assets/screen-grabs/trad-client-server.png)

_Image: The traditional client-server model_

SpatialOS works differently: it brings together many servers so they’re working as one. But it does this in a way that makes a single game world which looks seamless to players.

![A SpatialOS deployment]({{assetRoot}}assets/screen-grabs/deployment.png)

_Image: A SpatialOS deployment_

## What sorts of games is SpatialOS for?

You can use SpatialOS for any real-time online multiplayer game, not just MMOs. We tend to reference MMOs to describe the scalability of SpatialOS, but you can build any sort of game to make use of SpatialOS networking, including action, RPG, and strategy games.

## How does it fit with Unreal Engine and the GDK?

As part of the GDK, we’ve created a version of Unreal Engine which provides SpatialOS networking alongside Unreal’s native networking. We maintain Unreal’s networking API, which means you don’t need to rewrite your game to make it work with the GDK.

This SpatialOS networking means you can use multiple servers to simulate your game world, because it enables Unreal Engine clients and servers to communicate with the SpatialOS Runtime to synchronize state.

SpatialOS with Unreal is seamless. The GDK:

* represents your game’s Actors on SpatialOS servers as SpatialOS [entities]({{urlRoot}}/content/spatialos-concepts/world-entities-components#entities-and-components).
* sets up your game to run SpatialOS servers, known as [worker instances]({{urlRoot}}/content/spatialos-concepts/workers-and-zoning#worker-instances-and-worker-types). These are the engines to compute your game.
* makes sure worker instances can send and receive updates about entity [components]({{urlRoot}}/content/spatialos-concepts/world-entities-components#entities-and-components) to and from each other. Worker instances’ authority over entities is handled by the GDK and SpatialOS. 

For more information on how SpatialOS works with the GDK and Unreal Engine, see [How the GDK fits into your game stack]({{urlRoot}}/content/technical-overview/how-the-gdk-fits-in.md).

</br>------</br>
_2019-05-21 Page added with editorial review_
