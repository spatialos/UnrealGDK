# SpatialOS Unreal GDK module for developing SpatialOS games

* Repository: [github.com/improbable/unreal-gdk](https://github.com/improbable/unreal-gdk)<br/>
(TODO: Fix repo link for external users - [JIRA TICKET](https://improbableio.atlassian.net/browse/UNR-304))

* By: [improbable.io](https://improbable.io/)

* License: Use of the contents of this repository is subject to the [Improbable license](LICENSE.md).

The **SpatialOS Unreal GDK** is an Unreal Engine 4 (UE4) module made by Improbable. You can use the Unreal GDK to integrate persistent multiplayer worlds into your Unreal game using the SpatialOS platform. 

There is an accompanying sample game at [github.com/improbable/unreal-gdk-sample-game](https://github.com/improbable/unreal-gdk-sample-game) which you can use to explore SpatialOS Unreal GDK development. <br/> 
(TODO: Fix repo link for external users - [JIRA TICKET](https://improbableio.atlassian.net/browse/UNR-304))

**Documentation** <br/>
For guidance on SpatialOS concepts see the documentation on the [SpatialOS website](https://docs.improbable.io/reference/13.0/shared/concepts/spatialos). <br/> 
For guidance on using the SpatialOS Unity GDK, see the [documentation in this GitHub repository](docs/start_here_table_of_contents.md).

**Prerequisites** <br/> 
See the [Getting started](docs/getting_started.md#prerequisites) documentation.

**Installation and setup** <br/> 
See the [Getting started](docs/getting_started.md#table-of-contents) documentation.

**Contributions** <br/>
TBD [JIRA ticket DEV-2087](https://improbableio.atlassian.net/browse/DEV-2087)

**Support** <br/>
TBD [JIRA ticket DEV-2087](https://improbableio.atlassian.net/browse/DEV-2087)

**Known issues:** <br/>
 See the [Known issues](docs/known_issues.md) documentation.

## Unreal Engine changes
We have to make a small number of changes to UE4 source code: these are mostly limited in scope and only consist of class access, polymorphism, and dll-export-related changes. We will attempt to consolidate and remove (or submit as PR to Epic) as many of these changes as possible. You can see the changes in our [UnrealEngine repo, `UnrealEngine419_SpatialGDK` branch](https://github.com/improbable/UnrealEngine/tree/UnrealEngine419_SpatialGDK). <br/>  (TODO Remove internal download for external users [JIRA TICKET](https://improbableio.atlassian.net/browse/UNR-304))

(c) 2018 Improbable

