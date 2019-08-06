![GDK for Unreal Documentation]({{assetRoot}}assets/spatialos-gdkforunreal-header.png)

<%(Callout  message="The SpatialOS GDK for Unreal is in alpha. It is ready to use for development of games using a single Unreal server, or using multiple servers in an offloading architecture. It it not yet recommended for development of multiserver games using the zoning architecture, and is not ready for public releases. For more information, please follow our [development roadmap](https://github.com/spatialos/UnrealGDK/projects/1) and [Unreal features support]({{urlRoot}}/unreal-features-support) pages.")%>

The SpatialOS Game Development Kit (GDK) for Unreal gives you the features of SpatialOS within the familiar workflows and APIs of Unreal. 

<img src="{{assetRoot}}assets/unrealgdk-headline-image.png" style=" float: right; margin: 0; display: block; width: 60%; padding: 20px 20px"/>

SpatialOS provides:<br/>

* **Global hosting**: Scalable dedicated hosting for your game in every major gaming region.<br/>
* **Easy playtesting**: Deploy and test your game from the start of development, and distribute it to your team and players quickly and easily with a ready-made link. Scale-test your build by connecting in Simulated Players.<br/>
* **Profiling and debugging tools**: Logs and metrics out of the box to help you quickly understand any bugs and performance issues.
* **Single and multiserver networking**: Use one or multiple server-worker instances simulating your game world, enabling greater numbers of Actors, players and gameplay systems. This is available today through the offloading architecture, in which you allocate the authority of specific Actor groups from the main Unreal server worker instance to a different worker. This is available as a preview - not recommended for development - through the zoning architecture, in which the world is split into zones of authority for each server-worker. 

## Get started

If you’re an Unreal game developer and you’re ready to try out the GDK, follow the [Get started]({{urlRoot}}/content/get-started/introduction) guide.

It takes you through setting up the GDK and getting the Starter Template project or the Example 
Project running in the cloud, as well as running locally on your computer. This gives an overview of the GDK and using SpatialOS, and you can use the Starter Template as a basis for your own projects.

After you set up the SpatialOS GDK Starter Template or the Example 
Project, you can learn more about the GDK’s functionality with:
<img src="{{assetRoot}}assets/example-project/example-project-headline.png" style=" float: right; margin: 0; display: block; width: 60%; padding: 20px 20px"/>

* **The offloading example project tutorial**: learn how to offload groups of actors to separate Unreal server-workers
* **The tutorial on multiple deployments for session-based games**: upload a session-based FPS example game to the cloud.
* **The multiserver shooter tutorial**: implement shooting across the boundaries of different servers simulating one game world.
* **The porting guide**: porting your existing UE project to SpatialOS.
* **The database sync worker tutorial**: learn how to integrate the Database Sync Worker Example in your GDK project

## Find out more

* Learn more about how the GDK works and how it fits into your game stack. 
<br/>**Read the [Technical overview]({{urlRoot}}/content/technical-overview/gdk-principles)** (10-minute read).
<br/>
<br/>
* If you aren’t already familiar with SpatialOS, you can find out about the concepts which enable it to support game worlds with more persistence, scale, and complexity than previously possible.
<br/> **Read the [SpatialOS concept docs]({{urlRoot}}/content/spatialos-concepts/what-is-spatialos)** (10-minute read).
<br/>
<br/>
* We’d love to hear your game ideas and answer any questions you have about making games on SpatialOS. <br/>
**Join the community on our <a href="https://forums.improbable.io" data-track-link="Join Forums Clicked|product=Docs" target="_blank">forums</a>, or on <a href="https://discordapp.com/invite/vAT7RSU" data-track-link="Join Discord Clicked|product=Docs|platform=Win|label=Win" target="_blank">Discord</a>.**
<br/>
<br/>

### **> Next:** [Get Started]({{urlRoot}}/content/get-started/dependencies.md)

</br>------</br>
_2019-07-31 Page updated with limited editorial review_
