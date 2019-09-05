<%(TOC)%>

# Tutorials and guides

## Multiserver offloading

> This tutorial uses the Example Project from the GDK's [setup guide]({{urlRoot}}/content/get-started/example-project/exampleproject-intro).</br>

<%(Video file="{{assetRoot}}assets/offloading-project/crashbotgymwithoffloading.mp4")%>

The Example Project contains a map called `Crashbot_Gym`. In the map, a line of AI characters (CrashBots) run forward towards a line of AI turrets.

The turrets use the Perception System to detect AI and player characters and shoot at them. When an AI character dies, it spawns a new character to replace it.

Currently, everything in the world is running on a single server called `UnrealWorker`.

In this tutorial, you will learn the steps needed to offload the AI characters onto a separate server-worker.

</br>
#### **> Next:** [1: Setup]({{urlRoot}}/content/tutorials/offloading-tutorial/offloading-setup)
</br>

<br/>------------<br/>
_2019-08-29 Page updated without editorial review: clarified overview of tutorial_<br/>
_2019-07-30 Page added without editorial review_
<br/>
<br/>
[//]: # (TODO: https://improbableio.atlassian.net/browse/DOC-1142)
------------
