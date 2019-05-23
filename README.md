# The SpatialOS Game Development Kit for Unreal (alpha)
The SpatialOS Game Development Kit (GDK) for Unreal is an Unreal Engine [plugin](https://docs.unrealengine.com/en-us/Programming/Plugins) which gives you the features of SpatialOS, within the familiar workflows and APIs of Unreal. 

* SpatialOS provides:

  - **Global hosting**: Scalable dedicated hosting for your game in every major gaming region.
  - **Easy playtesting**: Deploy and test your game from the start of development, and distribute it to your team and players quickly and easily.
  - **Profiling and debugging tools**: Logs and metrics out of the box to help you quickly understand any bugs and performance issues.
  - **Multiserver networking**: Multiple dedicated servers across one seamless game world, enabling greater numbers of Actors, players and gameplay systems. **Note:** this feature is currently in preview as we work to improve its stability - we recommend you try it out using the [Multiserver Shooter Tutorial](https://docs.improbable.io/unreal/alpha/content/tutorials/multiserver-shooter/tutorial-multiserver-intro) but avoid developing your game with it.
  
If you’re an Unreal game developer and you’re ready to try out the GDK, follow the [Get started guide](https://docs.improbable.io/unreal/alpha/content/get-started/introduction). 

> The SpatialOS GDK for Unreal is in alpha. It is ready to use for development of single-server games, but not recommended for public releases. We are committed to rapid development of the GDK to provide a performant release - for information on this, see our [development roadmap](https://github.com/spatialos/UnrealGDK/projects/1) and [Unreal features support](https://docs.improbable.io/unreal/alpha/unreal-features-support) pages for the status and updates, and contact us via our forums, or on Discord.

----
* [Get started](https://docs.improbable.io/unreal/latest/content/get-started/introduction) (on the SpatialOS documentation website)
* [Documentation](https://docs.improbable.io/unreal/latest) (on the SpatialOS documentation website)
* [Development roadmap](https://github.com/spatialos/UnrealGDK/projects/1) (Github project board)
* Community: [Discord](https://discordapp.com/channels/311273633307951114/339471548647866368) - [Forums](https://forums.improbable.io/) -  [Mailing list](http://go.pardot.com/l/169082/2018-06-15/27ld2t)

----
## Where to get the GDK and related projects
The GDK and its related projects are available on GitHub.
* [GDK: github.com/spatialos/UnrealGDK](https://github.com/spatialos/UnrealGDK)
* [The SpatialOS Unreal Engine fork](https://github.com/improbableio/UnrealEngine)
* [The Example Project](https://github.com/spatialos/UnrealGDKExampleProject) 
* [Third Person Shooter project](https://github.com/spatialos/UnrealGDKThirdPersonShooter)
* [The Test Suite](https://github.com/spatialos/UnrealGDKTestSuite)

## Unreal Engine changes
In order to transform Unreal from a single server engine to a distributed model, we have made a number of small changes to the UE4 code. We will attempt to consolidate and remove (or submit as PR to Epic) as many of these changes as possible. You can see the changes in our forked [Unreal Engine repo](https://github.com/improbableio/UnrealEngine).

> In order to get access to this fork, you need to link your GitHub account to a verified Epic Games account, and to have agreed to Epic's license. You will not be able to use the GDK for Unreal without doing this first. To do this, see the [Unreal documentation](https://www.unrealengine.com/en-US/ue4-on-github).

## Recommended use
The GDK is in [alpha](https://docs.improbable.io/reference/latest/shared/release-policy#maturity-stages) so we can react to feedback and iterate on development quickly. To facilitate this, during our alpha stage we don't have a formal deprecation cycle for APIs and workflows. This means that everything and anything can change. In addition, documentation is limited and some aspects of the GDK are not optimized.

We recommend using the GDK in projects in the early production or prototype stage. This ensures that your project's requirements are in line with the GDK's timeline.

See the full [list of supported features](https://docs.improbable.io/unreal/latest/unreal-features-support) on the SpatialOS documentation website.

## Contributions
We are not currently accepting public contributions - see our [contributions](https://docs.improbable.io/unreal/latest/contributing) policy. However, we are accepting issues and we do want your feedback.

## Run into problems?
* [Troubleshooting](https://docs.improbable.io/unreal/latest/content/troubleshooting)
* [Known issues](https://github.com/spatialos/UnrealGDK/projects/2)

## Give us feedback
We have released the GDK for Unreal this early in development because we want your feedback. Please come and talk to us about the software and the documentation via: [Discord](https://discordapp.com/channels/311273633307951114/339471548647866368) - [Forums](https://forums.improbable.io/) - [GitHub issues in this repository](https://github.com/spatialos/UnrealGDK/issues).

------

* Your access to and use of the Unreal Engine is governed by the [Unreal Engine End User License Agreement](https://www.unrealengine.com/en-US/previous-versions/udk-licensing-resources?sessionInvalidated=true). Please ensure that you have agreed to those terms before you access or use the Unreal Engine.
* Version: alpha (stability and performance improvements pending)

(c) 2019 Improbable