# Tutorials and guides
## Multiserver zoning shooter tutorial

<%(Callout type="warn" message="This tutorial is available as a preview to demonstrate early multiserver [zoning]({{urlRoot}}/content/glossary#zoning) functionality in the GDK. Please note we do not yet recommend development of multiserver games using zoning. For more information, please follow our [development roadmap](https://github.com/spatialos/UnrealGDK/projects/1) and [Unreal features support]({{urlRoot}}/unreal-features-support) pages.")%>	

**What the tutorial covers**<br/>
>**This tutorial uses the Example Project from the GDK’s [setup guide]({{urlRoot}}/content/get-started/example-project/exampleproject-intro)**.

In this tutorial you’ll implement cross-server remote procedure calls (RPCs) for simple first-person shooter functionality in the Example Project. The end result will be a multiplayer, cloud-hosted Unreal game running across multiple [server-workers]({{urlRoot}}/content/glossary#worker) that players can seamlessly move between and shoot across. It demonstrates multiserver [zoning]({{urlRoot}}/content/glossary#worker) and looks something like this:

![]({{assetRoot}}assets/tutorial/cross-server-shooting.gif)

The tutorial demonstrates that the workflows and iteration speed you’re used to as an Unreal developer are almost entirely unaltered by the GDK: it’s just like regular Unreal.
</br>
</br>
</br>
**Notes:**

* Before starting this tutorial you need to complete the [Example Project set up guide]({{urlRoot}}/content/get-started/example-project/exampleproject-intro). 
### **> Next:** 
[1: Replicate health changes]({{urlRoot}}/content/tutorials/multiserver-shooter/tutorial-multiserver-healthchanges)
<br/>
<br/>


<br/>------<br/>
_2019-08-02 Page updated with limited editorial review: updated project name._</br>
_2019-04-30 Page updated with limited editorial review._
