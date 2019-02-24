# The SpatialOS Game Development Kit for Unreal (alpha)
The SpatialOS Game Development Kit (GDK) for Unreal is an Unreal Engine 4 (UE4) [plugin (Unreal documentation)](https://docs.unrealengine.com/en-us/Programming/Plugins) which allows you to host your game and combine multiple dedicated server instances across one seamless game world, whilst using the Unreal Engine networking API.

The GDK offers:
* **Multiserver support**: leveraging our cloud platform [SpatialOS (SpatialOS documentation)](https://docs.improbable.io/reference/latest/shared/concepts/spatialos), the GDK allows you to run multiple game servers in a single game instance so your Unreal-developed games can have more players, more Actors, and better gameplay systems than previously possible.

* **An Unreal-native experience:** keeping traditional workflows and networking APIs that Unreal Engine developers are familiar with, the GDK introduces new native-feeling concepts that turn a single-server engine into a distributed one. This enables the GDK to retain the functionality of the networking features which Unreal offers out of the box, including transform synchronization, character movement, and map travel.
* **An easy onboarding experience**: we have made sure it’s easy to get started with the GDK by including a Starter Project which you can use as a tour of SpatialOS and a base for your own game, as well as a guide to porting your current multiplayer Unreal game to run on SpatialOS.

>This is an [alpha (SpatialOS documentation)](https://docs.improbable.io/reference/latest/shared/release-policy#maturity-stages) release of the SpatialOS GDK for Unreal, pending stability and performance improvements. The API may change as we learn from feedback  - see the guidance on [Recommended use](#recommended-use), below.

----
* [Get started](https://docs.improbable.io/unreal/latest/content/get-started/introduction) (on the SpatialOS documentation website)
* [Documentation](https://docs.improbable.io/unreal/latest) (on the SpatialOS documentation website)
* [Development roadmap](https://trello.com/b/7wtbtwmL/spatialos-gdk-for-unreal-roadmap) (Trello board)
* Community: [Discord](https://discordapp.com/channels/311273633307951114/339471548647866368) - [Forums](https://forums.improbable.io/) -  [Mailing list](http://go.pardot.com/l/169082/2018-06-15/27ld2t)
----

## Recommended use
We are releasing the GDK in [alpha](https://docs.improbable.io/reference/latest/shared/release-policy#maturity-stages) so we can react to feedback and iterate on development quickly. To facilitate this, during our alpha stage we don't have a formal deprecation cycle for APIs and workflows. This means that everything and anything can change. In addition, documentation is limited and some aspects of the GDK are not optimized.

Given this, for now we recommend using the GDK in projects in the early production or prototype stage. This ensures that your project's requirements are in line with the GDK's timeline.

Although the GDK is not fully ready in terms of performance, stability and documentation yet, this is a great time to get involved and shape it with us. We are committed to improving the GDK rapidly, aiming for a beta release in Q1 2019.

See the [full feature list](https://docs.improbable.io/unreal/latest/features) on the documentation website.

## Contributions
We are not currently accepting public contributions - see our [contributions](https://docs.improbable.io/unreal/latest/contributing) policy. However, we are accepting issues and we do want your feedback.

## Run into problems?
* [Troubleshooting](https://docs.improbable.io/unreal/latest/content/troubleshooting)
* [Known issues](https://docs.improbable.io/unreal/latest/known-issues)

## Unreal Engine changes
In order to transform Unreal from a single server engine to a distributed model, we had to make a small number of changes to UE4 code. We will attempt to consolidate and remove (or submit as PR to Epic) as many of these changes as possible. You can see the changes in our forked [Unreal Engine repo](https://github.com/improbableio/UnrealEngine).
> You may get a 404 error from this link. To get access, see [these instructions](https://docs.improbable.io/unreal/latest/setup-and-installing#unreal-engine-eula) <br/>

## Give us feedback
We have released the GDK for Unreal this early in development because we want your feedback. Please come and talk to us about the software and the documentation via: [Discord](https://discordapp.com/channels/311273633307951114/339471548647866368) - [Forums](https://forums.improbable.io/) - [GitHub issues in this repository](https://github.com/spatialos/UnrealGDK/issues).

## Where to get the GDK and related projects
The GDK and its related projects are available on GitHub.
* [GDK: github.com/spatialos/UnrealGDK](https://github.com/spatialos/UnrealGDK)
* [The SpatialOS Unreal Engine fork](https://github.com/improbableio/UnrealEngine)
**NOTE:** This link may give you a 404.

In order to get access to this fork, you need to link your GitHub account to a verified Epic Games account, and to have agreed to Epic's license. You will not be able to use the GDK for Unreal without doing this first. To do this, see the [Unreal documentation](https://www.unrealengine.com/en-US/ue4-on-github).
* [Starter Project](https://github.com/spatialos/UnrealGDKStarterProject)
* [Third-Person Shooter game](https://github.com/spatialos/UnrealGDKThirdPersonShooter) (Not actively developed)
* [The Test Suite](https://github.com/spatialos/UnrealGDKTestSuite)</br>


------

* Your access to and use of the Unreal Engine is governed by the [Unreal Engine End User License Agreement](https://www.unrealengine.com/en-US/previous-versions/udk-licensing-resources?sessionInvalidated=true). Please ensure that you have agreed to those terms before you access or use the Unreal Engine.
* Version: alpha (stability and performance improvements pending)
* GDK repository: [github.com/spatialos/UnrealGDK](https://github.com/spatialos/UnrealGDK)

(c) 2018 Improbable
