
> This page assumes that you’re familiar with Unreal Engine, but not with SpatialOS.

# How the SpatialOS GDK for Unreal fits into your game stack
The GDK provides a networking integration with SpatialOS, which enables Unreal Engine 4 (UE4) clients and servers to communicate with the SpatialOS Runtime to synchronize state.

Using the GDK, you upload your built-out UE4 server binaries to SpatialOS, which runs them in a game instance. You can also upload clients to SpatialOS and distribute them to players using the [SpatialOS Launcher]({{urlRoot}}/content/glossary#launcher) for early playtesting.

In addition, you can integrate systems from outside the game instance, such as inventory, authentication,  and matchmaking, using SpatialOS’s tools and services. See the [Player identity APIs and other platform services](https://docs.improbable.io/reference/latest/platform-sdk/introduction).

The diagram below shows how SpatialOS and the GDK fit into a typical multiplayer game stack:

<%(Lightbox image="{{assetRoot}}assets/tech-overview/architecture-of-spatialos-gdk-for-unreal-game.jpg")%><br>
_The GDK provides a networking integration with SpatialOS, which enables Unreal Engine 4 clients and servers to communicate with the SpatialOS Runtime to synchronize state._

## How it works
In Unreal, game clients communicate with the game server using Unreal’s networking code. The SpatialOS GDK for Unreal contains a plugin for Unreal Engine that replaces this networking code. You can switch between Unreal networking and SpatialOS networking from the toolbar in the Unreal Editor. 

When we forked Unreal Engine, we extended Unreal’s `UIpNetDriver` (which orchestrates replication) to create a `USpatialNetDriver`. This handles the connection between the GDK and SpatialOS, and translates Unreal’s native replication updates and RPCs into instructions that SpatialOS can follow. We do this by using the `UnrealHeaderTool` to generate reflection data that we then turn into the SpatialOS data format called schema.

<%(Lightbox image="{{assetRoot}}assets/tech-overview/networking-switch.png")%><br>
_Use the Unreal Editor toolbar networking switch to swap out native Unreal networking and swap in SpatialOS networking._

The SpatialOS model differs significantly from Unreal Engine when it comes to replicating an Actor. We don't replicate Actors to each player individually, as Unreal would. Instead, we update the game instance running in the cloud, and SpatialOS handles distributing the data to connected clients, so it is not sent multiple times to each interested client.

<br/>------------<br/>
_2019-12-05 Page amended with editorial review_   
_2019-04-25 Page added with editorial review_
