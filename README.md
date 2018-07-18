# SpatialOS Unreal GDK module for developing SpatialOS games

* Repository: [github.com/improbable/unreal-gdk](https://github.com/improbable/unreal-gdk)<br/>
(TODO: Fix repo link for external users - [JIRA TICKET](https://improbableio.atlassian.net/browse/UNR-304))

* By: [improbable.io](https://improbable.io/)

* License: Use of the contents of this repository is subject to the [Improbable license](LICENSE.md).

The **SpatialOS Unreal GDK** is an Unreal Engine 4 (UE4) module made by Improbable. You can use the Unreal GDK to integrate persistent multiplayer worlds into your Unreal game using the SpatialOS platform.

There is an accompanying sample game at [github.com/improbable/unreal-gdk-sample-game](https://github.com/improbable/unreal-gdk-sample-game) which you can use to explore SpatialOS Unreal GDK development. <br/> 
(TODO: Fix repo link for external users - [JIRA TICKET](https://improbableio.atlassian.net/browse/UNR-304))

## Recommended use <br>
To write

## Documentation <br/>
For guidance on SpatialOS concepts see the documentation on the [SpatialOS website](https://docs.improbable.io/reference/13.0/shared/concepts/spatialos). <br/> 
For guidance on using the SpatialOS Unreal GDK, see the [documentation in this GitHub repository](docs/readme.md).

## Installation and setup <br/> 
For prerequisites, installation and setup, see the [Installation and setup](docs/setup-and-installing.md) documentation.

## Give us feedback <br/>
We have released the Unreal GDK this early in development because we want your feedback. Please come and talk to us about the software and the documentation via:

**Discord**<br>
Find us in the [**#unreal** channel](https://discordapp.com/channels/311273633307951114/339471548647866368). You may need to grab Discord [here](https://discordapp.com/).

**The SpatialOS forums**<br>
Visit the **feedback** section in our [forums](https://forums.improbable.io/) and use the **unreal-gdk** tag. [This link](https://forums.improbable.io/new-topic?category=Feedback&tags=unreal-gdk) takes you there and pre-fills the category and tag.

**GitHub issues**<br>
Create an issue in [this repository](https://github.com/spatialos/unreal-gdk/issues).

## Contributions <br/>
**Public contributors**<br>
We are not currently accepting public contributions - see our [contributions](https://github.com/improbabl/unreal-gdk/.github/CONTRIBUTING.md) policy. However, we are accepting issues and we do want your feedback.

Improbable developers
See the [Contributions guide]().

## Support <br/>
TBD [JIRA ticket DEV-2087](https://improbableio.atlassian.net/browse/DEV-2087)

## Known issues <br/>
 See the [Known issues](docs/known-issues.md) documentation.

## Unreal Engine changes
We have to make a small number of changes to UE4 source code: these are mostly limited in scope and only consist of class access, polymorphism, and dll-export-related changes. We will attempt to consolidate and remove (or submit as PR to Epic) as many of these changes as possible. You can see the changes in our [UnrealEngine repo, `UnrealEngine419_SpatialGDK` branch](https://github.com/improbable/UnrealEngine/tree/UnrealEngine419_SpatialGDK). <br/>  (TODO Remove internal download for external users [JIRA TICKET](https://improbableio.atlassian.net/browse/UNR-304))

(c) 2018 Improbable