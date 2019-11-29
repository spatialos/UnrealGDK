![GDK for Unreal Documentation]({{assetRoot}}assets/spatialos-gdkforunreal-header.png)

<%(Callout  message="The SpatialOS GDK for Unreal is in alpha. It can be used to develop games using a single server, and some multiserver functionality is also available, as described below.")%>

The SpatialOS Game Development Kit (GDK) for Unreal is an Unreal Engine fork and plugin with associated projects; it provides features of SpatialOS, within the familiar workflows and APIs of Unreal. 

<img src="{{assetRoot}}assets/unrealgdk-headline-image.png" style=" float: right; margin: 0; display: block; width: 60%; padding: 20px 20px"/>

SpatialOS provides:<br/>

* **Global hosting**: Scalable dedicated hosting for your game in every major gaming region.<br/>
* **Easy playtesting**: Deploy and test your game from the start of development, and distribute it to your team and players quickly and easily with a ready-made link. Scale-test your build by connecting simulated players.<br/>
* **Profiling and debugging tools**: Logs and metrics out of the box to help you quickly understand any bugs and performance issues.
* **Single and multiserver networking**: Use one instance of server software or multiple instances of server software to compute your game world. Multiple servers enable a greater number of Actors, players and gameplay systems in your game.</br>



## Multiserver networking

Multiserver functionality is available through either:

* server **offloading** (available in alpha), in which Unreal server functionality is split between multiple servers and those servers compute different functionality across the whole game world, or 
* server **zoning** (available in pre-alpha only), in which the game world is split into several geographical areas and each area has a dedicated Unreal server computing all the functionality for it.

<%(Callout  message="We do not currently recommend development using multiserver zoning functionality. For more information on multiserver zoning availability, see the [development roadmap](https://github.com/spatialos/UnrealGDK/projects/1) and [Unreal features support]({{urlRoot}}/unreal-features-support) page.")%>


## Find out more

The [Get started]({{urlRoot}}/content/get-started/introduction) guide takes you through setting up the GDK and also explains how to set up:

* a Starter Template project that you can use as a basis for your own projects.
* the Example Project running in the cloud, as well as running locally on your computer. The Example Project gives an overview of the GDK and using SpatialOS, and is the basis of our tutorials.

* Want to learn more about how the GDK works and how it fits into your game stack? 
<br/>**Read the [Technical overview]({{urlRoot}}/content/technical-overview/gdk-principles)**. 
<br/>
<br/>
* Unfamiliar with SpatialOS? Need to  find out about the concepts behind it?
<br/> **Read the [SpatialOS concept docs]({{urlRoot}}/content/spatialos-concepts/what-is-spatialos)**.
<br/>
<br/>

* Want to tell us more about your game ideas or ask questions about making games on SpatialOS?<br/>
**Join the community on our <a href="https://forums.improbable.io" data-track-link="Join Forums Clicked|product=Docs" target="_blank">forums</a>, or on <a href="https://discordapp.com/invite/vAT7RSU" data-track-link="Join Discord Clicked|product=Docs|platform=Win|label=Win" target="_blank">Discord</a>.**
<br/>
<br/>

#### **> Next:** [Get started]({{urlRoot}}/content/get-started/introduction.md)

</br>------</br>
_2019-11-28 Page updated with editorial review: removed duplicate content</br>
_2019-10-18 Page updated with editorial review: removed mention of the deprecated tutorial - multiple deployments for session-based games_</br>
_2019-08-09 Page updated with editorial review_</br>
_2019-08-08 Page updated with editorial review: renamed "multiserver shooter tutorial" to "multiserver zoning tutorial"_ </br>
_2019-07-31 Updated with limited editorial review_


