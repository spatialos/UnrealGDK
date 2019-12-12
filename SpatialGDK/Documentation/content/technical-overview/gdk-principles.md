

> This page assumes that you’re familiar with Unreal Engine, but not with SpatialOS.

# Principles of the GDK for Unreal

The SpatialOS Game Development Kit (GDK) for Unreal is an Unreal Engine fork and plugin with associated projects. It enables you to use the features of SpatialOS while developing with familiar Unreal Engine workflows and APIs.

* **Unreal-first**

    We want experienced Unreal developers to benefit from the features of Unreal and take advantage of the SpatialOS platform, with a workflow that’s as native to Unreal as possible. 

    To achieve this, we’ve created a version of Unreal Engine which provides SpatialOS networking alongside Unreal’s native networking. We maintain Unreal’s networking API, which means you don’t need to rewrite your game to make it work with the GDK.</br></br>

* **No limits**
    
    An Unreal dedicated server is only as powerful as the single machine running it. The single machine quickly becomes a bottleneck in games with high numbers of Actors or complex game logic. 

    You don’t have to make these technical tradeoffs with the GDK. SpatialOS can spread computation across multiple servers, allowing for far more complex games and much higher player counts.</br></br>

    > This is enabled by [offloading]({{urlRoot}}/content/technical-overview/gdk-concepts#offloading) and will be enabled by [zoning]({{urlRoot}}/content/technical-overview/gdk-concepts#zoning). Zoning is currently in pre-alpha. We invite you to try out the [multiserver zoning tutorial]({{urlRoot}}/content/tutorials/multiserver-shooter/tutorial-multiserver-intro) and learn about how it works, but we don’t recommend you start developing features that use zoning yet.

* **Open development**
    
    The GDK is a community-driven project. We do all our development in the open and under an [MIT license](https://github.com/spatialos/UnrealGDK/blob/release/LICENSE.md).

    We value your contributions (see the [contribution guidelines](https://github.com/spatialos/UnrealGDK/blob/master/CONTRIBUTING.md)) and feature requests. Get in touch on the [forums](https://forums.improbable.io/tags/unreal-gdk) or on [Discord](https://discordapp.com/invite/RFB8S8C).

<br/>------------<br/>
_2019-04-25 Page added with editorial review_
