<%(TOC)%>
# SpatialOS concepts: introduction

## Working with SpatialOS and Unreal

To work with SpatialOS and Unreal, you install the GDK with Unreal Engine. Once your project is production ready, you upload and run it in the cloud on SpatialOS servers as a cloud deployment. While you work on developing your project, you can test it both as a cloud and deployment and locally on your computer. In this local deployment, you use your computer as if it's a server in the cloud.

While the GDK is a complete development environment, you can use additional SpatialOS tools to test your game and manage it once it's running. You can also access SpatialOS tools to extend the functionality of the GDK.

SpatialOS with Unreal is seamless. The GDK:

* represents your game’s objects in SpatialOS servers as entities made up of components.
* uses workers as the engines to compute your game - these workers are the servers for your game.
* makes sure workers can send and receive updates about entity components to and from each other - workers’ authority is handled by the GDK and SpatialOS.

For more information, see [How the GDK fits into your game stack]({{urlRoot}}/content/technical-overview/how-the-gdk-fits-in)
.
## Using the SpatialOS concepts documentation

Each page within the "SpatialOS concepts" section introduces new concepts that build on the concepts introduced in the previous page(s). For this reason, we recommend you read the pages in order.