<%(TOC)%>

# Tutorials and guides

## Offloading Example Project
> **This tutorial uses the Example Project from the GDK's [setup guide]({{urlRoot}}/content/get-started/example-project/exampleproject-intro).**</br>

The UnrealGDK Example project contains a map called `CrashBot_Gym` that demonstrates how to offload gameplay features to separate workers.


The scenario in the `CrashBot_Gym` contains a set of turrets that will attempt to attack any nearby players or AI-controlled bots. The turrets are offloaded to a worker called `AIWorker` and the bots are offloaded to a worker called `CrashBot_Worker`. This section will go through the details of what was required to enable offloading in this scenario.

<br/>------------<br/>
_2019-07-26 Page added as draft_
<br/>
<br/>
[//]: # (TODO: https://improbableio.atlassian.net/browse/DOC-1142)
------------
