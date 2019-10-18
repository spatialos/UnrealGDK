![GDK for Unreal Documentation]({{assetRoot}}assets/spatialos-gdkforunreal-header.png)

<%(Callout  message="The SpatialOS GDK for Unreal is in alpha. It is ready to use for development of games using a single server, or multiserver offloading functionality. We do not recommend development using multiserver zoning functionalty. For more information on multiserver zoning availability, see the [development roadmap](https://github.com/spatialos/UnrealGDK/projects/1) and [Unreal features support]({{urlRoot}}/unreal-features-support) page.")%>

The SpatialOS Game Development Kit (GDK) for Unreal is an Unreal Engine fork and plugin with associated projects; it provides features of SpatialOS, within the familiar workflows and APIs of Unreal. 

You can [get started](#get-started) immediately or [find out more](#find-out-more) before diving in. 

<img src="{{assetRoot}}assets/unrealgdk-headline-image.png" style=" float: right; margin: 0; display: block; width: 60%; padding: 20px 20px"/>

SpatialOS provides:<br/>

* **Global hosting**: Scalable dedicated hosting for your game in every major gaming region.<br/>
* **Easy playtesting**: Deploy and test your game from the start of development, and distribute it to your team and players quickly and easily with a ready-made link. Scale-test your build by connecting simulated players.<br/>
* **Profiling and debugging tools**: Logs and metrics out of the box to help you quickly understand any bugs and performance issues.
* **Single and multiserver networking**: Use one instance of server software or multiple instances of server software to compute your game world. Multiple servers enable a greater numbers of Actors, players and gameplay systems in your game.</br>
Multiserver is available through either:
    * server **offloading** (available in alpha), in which Unreal server functionality is split between multiple servers and those servers compute different functionality across the whole game world, or 
    * server **zoning** (available in pre-alpha only), in which the game world is split into several geographical areas and each area has a dedicated Unreal server computing all the functionality for it.

## Get started

If you’re an Unreal game developer and you’re ready to try out the GDK, follow the [Get started]({{urlRoot}}/content/get-started/introduction) guide, or you can [find out more](#find-out-more) before diving in.

The _Get started_ guide takes you through setting up the GDK and getting the Starter Template project or the Example 
Project running in the cloud, as well as running locally on your computer. The Example Project gives an overview of the GDK and using SpatialOS, and is the basis for tutorials, and you can use the Starter Template as a basis for your own projects.

After you set up the SpatialOS GDK and the Example 
Project, you can learn more about the GDK’s functionality with tutorials and guides:
<img src="{{assetRoot}}assets/example-project/example-project-headline.png" style=" float: right; margin: 0; display: block; width: 60%; padding: 20px 20px"/>

* **Multiserver offloading**: Learn how to offload groups of Actors so Unreal server functionality is split between multiple servers using the Example Project.
* **Multiserver zoning**: Implement shooting across the boundaries of different servers computing one game world using the Example Project. (**Note:** Zoning is in preview.)
* **Database sync worker**: Learn how to integrate server database synchronization into your project using the Example Project.
* **The porting guide**: Port your existing UE project to SpatialOS.

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

#### **> Next:** [Get started]({{urlRoot}}/content/get-started/introduction.md)

</br>------</br>
_2019-10-16 Updated with editorial review: removed mention of the deprecated tutorial -  multiple deployments for session-based games_</br>
_2019-08-09 Updated with editorial review_</br>
_2019-08-08 Updated with editorial review: renamed "multiserver shooter tutorial" to "multiserver zoning tutorial"_ </br>
_2019-07-31 Updated with limited editorial review_
