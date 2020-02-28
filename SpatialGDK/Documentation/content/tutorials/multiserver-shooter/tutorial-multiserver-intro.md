<%(TOC)%>
# Tutorials and guides
## Multiserver zoning
<%(Callout type="warn" message="This tutorial is a preview demonstrating early multiserver zoning functionality in the GDK. Please note we do not recommend development of multiserver projects using server [zoning]({{urlRoot}}/content/glossary#worker) at this time. </br></br>
For more information on multiserver zoning, see our [development roadmap](https://github.com/spatialos/UnrealGDK/projects/1) and [Unreal features support]({{urlRoot}}/unreal-features-support) pages.")%>	


> This tutorial uses the Example Project from the GDK's [setup guide]({{urlRoot}}/content/get-started/example-project/exampleproject-intro).</br>

<br/>

### What the tutorial covers



Following this tutorial, you implement cross-server remote procedure calls (RPCs) for simple first-person shooter functionality in the GDK's Example Project. The end result is a multiplayer, cloud-hosted Unreal game running across multiple [server-workers]({{urlRoot}}/content/glossary#server-workers) that players can seamlessly move between and shoot across. It demonstrates multiserver [zoning]({{urlRoot}}/content/glossary##zoning) and looks something like the animation below.</br></br>

![]({{assetRoot}}assets/tutorial/cross-server-shooting.gif)
_Image: Example of cross-server shooting using multiserver zoning functionality._ </br></br>

The tutorial demonstrates that the workflows and iteration speed you’re used to as an Unreal developer are almost entirely unaltered by the GDK: it’s just like regular Unreal.

### Change your branch of the Example Project 

Before starting this tutorial you need to complete the [Example Project setup guide]({{urlRoot}}/content/get-started/example-project/exampleproject-intro). Following this, you clone or download and set up all the elements of the GDK and its dependencies, including the Example Project.

To follow the multiserver zoning tutorial, you need to change your Example Project branch from `release` (which is the default) to `multiserver-tutorial-start`. To do this:

1. If you have Unreal Editor open, close it.
2. In GitHub, change your Example Project branch to `multiserver-tutorial-start`.</br>
To do this, open a terminal window, navigate to directory containing your cloned Example Project (this will be    `...\UnrealEngine\samples\UnrealGDKExampleProject`) and enter `git checkout -b multiserver-tutorial-start`.
3. Open the project in Visual Studio.</br>
To do this, in File Explorer, in the directory `...\UnrealEngine\samples\UnrealGDKExampleProject\Game`, double-click on `GDKShooter.sln`.
4. In Visual Studio, select **Local Windows Debugger** in the toolbar to open your project in the Unreal Editor.

You can now set up replication of health changes. 

</br>
</br>

#### **> Next:** [1: Set up replication]({{urlRoot}}/content/tutorials/multiserver-shooter/tutorial-multiserver-healthchanges)


<br/>
<br/>


<br/>------<br/>
_2019-08-03 Page updated with limited editorial review: added change branch._</br>
_2019-08-02 Page updated with limited editorial review: updated project name and terms._</br>
_2019-04-30 Page updated with limited editorial review._
