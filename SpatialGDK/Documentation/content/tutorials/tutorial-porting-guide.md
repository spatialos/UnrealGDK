<%(TOC)%>
# Tutorials and guides	
## Port your project to SpatialOS

<%(Callout type="warn" message="Please be aware of the [GDK's support level of different Unreal features]({{urlRoot}}/unreal-features-support). If you need to port your game, please contact us via our [forums](https://forums.improbable.io/), or [Discord](https://discord.gg/vAT7RSU) so we can best support you.")%>

This guide shows you how to port your own Unreal project to the GDK; you will modify your project to make it compatible with the GDK and SpatialOS, and you will launch a local deployment to test your project.

**Get to know the GDK before porting your game**</br>
We recommend following steps 1 to 3 of the [Get started]({{urlRoot}}/content/get-started/introduction) guide and setting up the [Example Project]({{urlRoot}}/content/get-started/example-project/exampleproject-intro) before porting your project. This gives you an overview of the GDK and using SpatialOS.
<br/>

**Tip: Reference project**
<br/>
 As you port your own Unreal project to SpatialOS, you could use our pre-ported [Unreal Shooter Game](https://docs.unrealengine.com/en-us/Resources/SampleGames/ShooterGame) as a reference. You should already have this project as it is included in the `Samples` directory of [the SpatialOS Unreal Engine fork](https://github.com/improbableio/UnrealEngine) which you downloaded as part of the _Get started_ steps. </br>
(If you want to see the game running, there's a [video on youtube](https://www.youtube.com/watch?v=xojgH7hJgQs&feature=youtu.be) to check out.)

## **Terms used in this guide**

- `<GameRoot>` - The directory containing your project's `.uproject` file and `Source` directory.  
- `<ProjectRoot>` - The directory containing your `<GameRoot>`.  
- `<YourProject>` - The name of your project's `.uproject` file (for example, `\<GameRoot>\TP_SpatialGDK.uproject`).

## Before you start

Before porting your project you _**must**_ follow:

1. [Get started 1 - Dependencies]({{urlRoot}}/content/get-started/dependencies)
2. [Get started 2 - Get and build the GDK’s Unreal Engine Fork]({{urlRoot}}/content/get-started/build-unreal-fork)
3. [Get started 3 - Set up a project: The Starter Template]({{urlRoot}}/content/get-started/gdk-template) </br>
   (Note that you must follow the Starter Template instructions and not the Example Project instructions.)
   </br>

**> Next:** [1. Set up your project]({{urlRoot}}/content/tutorials/porting-guide/tutorial-portingguide-setup)

<br/>

<br/>------------<br/>2019-07-16 Page updated with editorial review.<br/>
