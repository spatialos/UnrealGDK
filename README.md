# The SpatialOS Games Development Kit for Unreal (alpha)
>This is an [alpha (SpatialOS website)](https://docs.improbable.io/reference/latest/shared/release-policy#maturity-stages) release of the SpatialOS Unreal GDK, pending stability and performance improvements. The API may change as we learn from feedback  - see the guidance on [Recommended use](#recommended-use), below.

The SpatialOS Games Development Kit (GDK) for Unreal is an Unreal Engine 4 (UE4) [plugin](https://docs.unrealengine.com/en-us/Programming/Plugins) made by [Improbable](. You can use the GDK to make use of multiple Unreal servers in a single game instance using the SpatialOS platform, unlocking the ability to implement large-scale, complex and persistent worlds. Find out more about the GDK in [our blogpost](https://improbable.io/games/blog/spatialos-unreal-gdk-pre-alpha).

----
* [Installation and setup](https://docs.improbable.io/unreal/latest/setup-and-installing) (on the documentation website)
* [Documentation](https://docs.improbable.io/unreal/alpha/index) (on the documentation website)
* [Development roadmap](https://trello.com/b/7wtbtwmL/spatialos-gdk-for-unreal-roadmap) (Trello board)
* Community: [Discord](https://discordapp.com/channels/311273633307951114/339471548647866368) - [Forums](https://forums.improbable.io/) -  [Mailing list](http://go.pardot.com/l/169082/2018-06-15/27ld2t)
----

## Recommended use
We are releasing the GDK in [alpha](https://docs.improbable.io/reference/latest/shared/release-policy#maturity-stages) so we can react to feedback and iterate on development quickly. To facilitate this, during our alpha stage we don't have a formal deprecation cycle for APIs and workflows. This means that everything and anything can change. In addition, documentation is limited and some aspects of the GDK are not optimized.

Given this, for now we recommend using the GDK in projects in the early production or prototype stage. This ensures that your project's requirements are in line with the GDK's timeline.

Although the GDK is not fully ready in terms of performance and stability yet, this is a great time to get involved and shape it with us. We are committed to improving the GDK rapidly, aiming for a beta release in Q1 2019.

See the [full feature list](https://docs.improbable.io/unreal/latest/features) on the documentation website.

## Contributions
We are not currently accepting public contributions - see our [contributions](https://docs.improbable.io/unreal/latest/contributing) policy. However, we are accepting issues and we do want your feedback.

## Run into problems?
* [Troubleshooting](https://docs.improbable.io/unreal/latest/content/troubleshooting)
* [Known issues](https://docs.improbable.io/unreal/latest/known-issues)

## Unreal Engine changes
In order to transform Unreal from a single server engine to a distributed model, we had to make a small number of changes to UE4 code. We will attempt to consolidate and remove (or submit as PR to Epic) as many of these changes as possible. You can see the changes in our forked [Unreal Engine repo, `4.20-SpatialOSUnrealGDK` branch](https://github.com/improbableio/UnrealEngine/tree/4.20-SpatialOSUnrealGDK).
> You may get a 404 error from this link. To get access, see [these instructions](https://docs.improbable.io/unreal/latest/setup-and-installing#unreal-engine-eula) <br/>

## Give us feedback
We have released the GDK for Unreal this early in development because we want your feedback. Please come and talk to us about the software and the documentation via: [Discord](https://discordapp.com/channels/311273633307951114/339471548647866368) - [Forums](https://forums.improbable.io/) - [GitHub issues in this repository](https://github.com/spatialos/UnrealGDK/issues).

## Where to get the GDK and starter projects
The GDK and its starter projects are available on GitHub.
* [GDK: github.com/spatialos/UnrealGDK](https://github.com/spatialos/UnrealGDK)
* [Starter Project](https://github.com/spatialos/UnrealGDKStarterProject)
* [Third Person Shooter Game](https://github.com/spatialos/UnrealGDKThirdPersonShooter) (Not actively developed)
* [The Test Suite](https://github.com/spatialos/UnrealGDKTestSuite)
* [The SpatialOS Unreal Engine fork](https://github.com/improbableio/UnrealEngine/tree/4.20-SpatialOSUnrealGDK) </br>
**NOTE:** This link may give you a 404. 
In order to get access to this fork, you need to link your GitHub account to a verified Epic Games account, and to have agreed to Epic's license. You will not be able to use the GDK for Unreal without doing this first. To do this, see the [Unreal documentation](https://www.unrealengine.com/en-US/ue4-on-github).

------

* License: use of the contents of this repository is subject to the [Improbable license](https://docs.improbable.io/unreal/latest/license) </br>
(Your access to and use of the Unreal Engine is governed by the [Unreal Engine End User License Agreement](https://www.unrealengine.com/en-US/previous-versions/udk-licensing-resources?sessionInvalidated=true). Please ensure that you have agreed to those terms before you access or use the Unreal Engine.)
* Version: alpha (stability and performance improvements pending)
* GDK repository: [github.com/spatialos/UnrealGDK](https://github.com/spatialos/UnrealGDK)


(c) 2018 Improbable
