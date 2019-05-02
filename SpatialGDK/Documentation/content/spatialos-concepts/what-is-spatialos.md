<%(TOC)%>
# Concepts: what is SpatialOS?

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

## How does it fit with Unreal Engine and the GDK?

As part of the GDK, we’ve created a version of Unreal Engine which provides SpatialOS networking alongside Unreal’s native networking. We maintain Unreal’s networking API, which means you don’t need to rewrite your game to make it work with the GDK.

This SpatialOS networking means you can use multiple servers to simulate your game world, because it enables Unreal Engine clients and servers to communicate with the SpatialOS Runtime to synchronize state.

For more information on how SpatialOS works with the GDK and Unreal Engine, see [How the GDK fits into your game stack]({{urlRoot}}/content/technical-overview/how-the-gdk-fits-in.md).