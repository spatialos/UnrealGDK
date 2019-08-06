# Tutorials and guides
## Multiserver zoning shooter tutorial
<%(Callout type="warn" message="This tutorial is a preview demonstrating early multiserver zoning functionality in the GDK. Please note we do not recommend development of multiserver projects using server zoning at this time. </br>
For more information on multiserver zoning, follow our [development roadmap](https://github.com/spatialos/UnrealGDK/projects/1) and [Unreal features support]({{urlRoot}}/unreal-features-support) pages.")%>	


> **This tutorial uses the Example Project from the GDK's [setup guide]({{urlRoot}}/content/get-started/example-project/exampleproject-intro).
> You'll want to begin this tutorial by switching over to the `multiserver-tutorial-start` branch to follow along with the code samples. The `release` branch of this project contains a completed version of this tutorial.**</br>

<br/>
<br/>

**What the tutorial covers**<br/>



Following this tutorial, you implement cross-server remote procedure calls (RPCs) for simple first-person shooter functionality in the GDK's Example Project. The end result is a multiplayer, cloud-hosted Unreal game running across multiple [server-workers]({{urlRoot}}/content/glossary#worker) that players can seamlessly move between and shoot across. It demonstrates multiserver [zoning]({{urlRoot}}/content/glossary#worker) and looks something like the animation below.</br></br>

![]({{assetRoot}}assets/tutorial/cross-server-shooting.gif)
_Image: Example of cross-server shooting using multiserver zoning functionality._ </br></br>

The tutorial demonstrates that the workflows and iteration speed you’re used to as an Unreal developer are almost entirely unaltered by the GDK: it’s just like regular Unreal.
</br>
</br>
</br>
**Notes:**

* Before starting this tutorial you need to complete the [Example Project set up guide]({{urlRoot}}/content/get-started/example-project/exampleproject-intro). 
</br>
</br>

### **> Next:** [1: Replicate health changes]({{urlRoot}}/content/tutorials/multiserver-shooter/tutorial-multiserver-healthchanges)

<br/>
<br/>


<br/>------<br/>
_2019-08-02 Page updated with limited editorial review: updated project name and terms._</br>
_2019-04-30 Page updated with limited editorial review._
