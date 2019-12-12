<%(TOC)%>
# Tutorials and guides	
## Port your project to SpatialOS

[block:callout]
{
  "type": "warn",
  "body": "Please be aware of the [GDK's support level of different Unreal features]({{urlRoot}}/unreal-features-support). If you need to port your game, please contact us via our [forums](https://forums.improbable.io/), or [Discord](https://discord.gg/vAT7RSU) so we can best support you."
}
[/block]

This guide shows you how to port your own Unreal project to the GDK; you will modify your project to make it compatible with the GDK and SpatialOS, and you will launch a local deployment to test your project.

**Get to know the GDK before porting your game**</br>
We recommend following steps 1 to 3 of the [Get started]({{urlRoot}}/content/get-started/introduction) guide and setting up the [Example Project]({{urlRoot}}/content/get-started/example-project/exampleproject-intro) before porting your project. This gives you an overview of the GDK and using SpatialOS.
<br/>

## **Terms used in this guide**

- `<GameRoot>` - The directory containing your project's `.uproject` file and `Source` directory.  
- `<ProjectRoot>` - The directory containing your `<GameRoot>`.  
- `<YourProject>` - The name of your project's `.uproject` file (for example, `\<GameRoot>\TP_SpatialGDK.uproject`).

## Before you start

Before porting your project you _**must**_ follow:

1. [Get started 1 - Dependencies]({{urlRoot}}/content/get-started/dependencies)
1. [Get started 2 - Set up the fork and plugin]({{urlRoot}}/content/get-started/build-unreal-fork)
1. [Get started 3 - Set up a project: The Starter Template]({{urlRoot}}/content/get-started/starter-template/get-started-template-intro) </br>
   (Note that you must follow the Starter Template instructions and not the Example Project instructions.)
   </br>

#### **> Next:** [1. Set up your project]({{urlRoot}}/content/tutorials/porting-guide/tutorial-portingguide-setup)

<br/>

<br/>------------<br/>
*2019-09-17 Page updated without editorial review - removal of ShooterGame reference.*<br/>
*2019-07-16 Page updated with editorial review.*<br/>
