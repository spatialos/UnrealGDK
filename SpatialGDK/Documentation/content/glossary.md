<%(TOC)%>

# Glossary
This glossary covers:

* Terms used in this documentation
* Terms relevant to the GDK for Unreal
* SpatialOS terms used in the GDK for Unreal documentation

**SpatialOS documentation**<br/>
Many SpatialOS term definitions link to further information in the [SpatialOS documentation](https://docs.improbable.io/reference/latest/index). This documentation covers the SpatialOS Worker SDK and Platform SDK as well some GDK-relevant tools; the [Console](#console), [schema](#schema), [snapshots](#snapshot), and [configuration files](#configuration-files) in particular.

Note that this SpatialOS documentation assumes you are developing a SpatialOS game using the [Worker SDK and Platform SDK](https://docs.improbable.io/reference/latest/shared/get-started/working-with-spatialos), so it may reference content relevant to that workflow only. While some of the concepts underpin the GDK for Unreal, the two workflows are not always the same.

## GDK for Unreal documentation terms
* `<GameRoot>` - The folder containing your project's `.uproject` and source folder.
* `<ProjectRoot>` - The folder containing your `<GameRoot>`.
* `<YourProject>` - Name of your project's `.uproject` (for example, `\<GameRoot>\TP_SpatialGDK.uproject`).

## GDK for Unreal terms

### Actor groups
Actor groups facilitate multiserver functionality through [offloading](#offloading). You set them up to configure which Actor types instances of a [server-worker type](#worker-types-and-worker-instances) have [authority](#authority) over. In the Unreal Editor, you can create Actor groups, assign Actor classes to a group, and then assign each group to a server-worker type via the SpatialOS Runtime Settings panel.

> **Find out more:**
> 
> * [Actor groups]({{urlRoot}}/content/workers/offloading-concept#actor-groups)
> * [The SpatialOS Runtime Settings panel]({{urlRoot}}/content/unreal-editor-interface/runtime-settings)
> * [Offloading overview]({{urlRoot}}/content/workers/offloading-concept)

### Actor handover
Actor handover (`handover`) is a GDK-specific `UPROPERTY` tag. It allows games built in Unreal (which uses single-server architecture) to take advantage of SpatialOS’ distributed, persistent server architecture. See [Actor property handover between server-workers]({{urlRoot}}/content/actor-handover.md).

### Authority

When a [server-worker instance](#server-workers) has authority over an Actor, the server-worker instance can make changes to the Actor by sending updates to the [SpatialOS Runtime](#spatialos-runtime). A server-worker instance has authority over an Actor if it has authority over the equivalent [entity](#entity)’s [SpatialOS component](#spatialos-component) `Position`. 

Client-worker instances never have authority over Actors. However, like in native Unreal, they can have [ownership](#ownership). 

In a game with just one server-worker instance (which is the default for the GDK), that server-worker instance has authority over all the Actors in the game. In a game with more than one server-worker instance, different server-worker instances have authority over different Actors:

* with [offloading]({{urlRoot}}/content/workers/offloading-concept), the server-worker instance that has authority over an Actor always stays the same.
* with [zoning](#zoning) (currently in pre-alpha), the server-worker instance that has authority over an Actor can change as the Actor moves around, but only one instance can have authority over an Actor at any one time.

> **Find out more:**
> 
> [How to set up authority]({{urlRoot}}/content/authority)

### Dynamic Typebindings
To enable the network stacks of Unreal and SpatialOS to interoperate, we've implemented [Dynamic Typebindings]({{urlRoot}}/content/dynamic-typebindings.md). `Dynamic Typebindings` operate at runtime so your that your iteration speed is not affected despite your network code running on a completely different represenetations than Unreal's.

### Cross-server RPCs
These handle the scenario where a [server-worker](#worker) needs to execute an operation on an Actor that another server-worker has [authority](#authority) over. When a cross-server RPC is invoked by a server-worker that doesn't have authority, the execution is routed through SpatialOS to the server-worker that has authority - this server-worker executes the RPC. (See the documentation on [Cross-server RPCs]({{urlRoot}}/content/cross-server-rpcs)).

### Global State Manager
The Global State Manager (GSM):

*  Makes sure that [Singleton Actors](#singleton-actor) are replicated properly, by only allowing the [server-worker](#worker) with [authority](#authority) over the GSM to execute the initial replication of these Actors. See documentation on [Singleton Actors]({{urlRoot}}/content/singleton-actors.md).
*  Maintains the configuration of a [deployment’s](#deployment) currently-loaded [game world](#game-world). (Note that this is the Unreal game world not the [SpatialOS world](#spatialos-world).)<br/>  

The GSM lists both the URL of the [Map (or Level - see Unreal documentation)](http://api.unrealengine.com/INT/Shared/Glossary/index.html#l) that the [server-workers](#worker) have loaded and the `AcceptingPlayers` flag. (This flag controls whether or not client-servers can spawn anything in the game world.)

> **Find out more**
>
> [Server travel]({{urlRoot}}/content/map-travel.md) 

### GSM
Short for [Global State Manager](#global-state-manager).

### Interest

Interest means that a [client-worker instance](#worker-types-and-worker-instances) receives updates about an Actor that it doesn’t [own](#ownership). This helps the worker instance correctly manipulate the Actors that it _does_ own, and render the [SpatialOS world](#spatialos-world).

You can define interest in three ways, which you can use alongside each other:

* `NetCullDistanceSquared`
* `ActorInterestComponent`
* `AlwaysInterested`

<!-- TODO update this when we have interest for server-worker instances as well -->

> **Find out more**
> 
> [Game client interest management]({{urlRoot}}/content/game-client-interest-management)

### Offloading

Offloading is one of the multiserver options for working with SpatialOS.  The functionality of the main out-of-the-box Unreal [server-worker type](#server-workers) is split between two server-worker types. For example, you could create an AI server-worker type and offload the AI computation from the main Unreal server-worker type onto it.

Advanced AI and large-scale background physics computation are good candidates for offloading as they are computationally expensive but latency-tolerant. This would leave your game's out-of-the-box main server-worker instance to run other game systems at a larger scale.

> **Find out more**
> 
> [Offloading overview]({{urlRoot}}/content/workers/offloading-concept)

### SchemaDatabase

The SchemaDatabase is a `uasset` file (named `SchemaDatabase.uasset`) that contains information about UObjects and associated [schema]({{urlRoot}}/content/spatialos-concepts/schema-and-snapshots#schema) in your project. Information is automatically added to the `SchemaDatabase` by the GDK whenever you generate schema. It is a generated file which you cannot manually edit. 

### Schema generation

A SpatialOS GDK for Unreal toolbar command (within the Unreal Editor) which takes a set of Unreal classes and generates SpatialOS [schema](#schema) that enables automatic communication between Unreal and SpatialOS. 

> **Find out more**
> 
> [SpatialOS GDK for Unreal toolbar]({{urlRoot}}/content/unreal-editor-interface/toolbars#spatialos-gdk-for-unreal-toolbar)

### Singleton Actor

You can use a Singleton Actor to define a single source of truth for operations or data, or both, across a game world that uses multiple [server-worker instances](#server-workers). Within a game world, SpatialOS makes sure that there is only ever one instance of an entity that represents a Singleton Actor, no matter how many server-worker instances you have. 

> **Find out more**
> 
> [Singleton Actors]({{urlRoot}}/content/singleton-actors)

### Spatial Type
Spatial Type (`SpatialType`) is a SpatialOS-specific [class specifier (Unreal documentation)](https://docs.unrealengine.com/en-US/Programming/UnrealArchitecture/Reference/Classes/Specifiers) which is used to expose network-relevant class information to SpatialOS. There are different categories of Spatial Type, depending on the Actor’s function in your game.

See the documentation on [Spatial Type]({{urlRoot}}/content/spatial-type). 

### SpatialType
See [Spatial Type](#spatial-type).

## SpatialOS terms
Below is a subset of SpatialOS terms most relevant to the GDK for Unreal. See the [SpatialOS documentation glossary](https://docs.improbable.io/reference/latest/shared/glossary) for a full list of terms specific to SpatialOS.

Note that this SpatialOS documentation glossary assumes you are developing a SpatialOS game using the [Worker SDK and Platform SDK](https://docs.improbable.io/reference/latest/shared/get-started/working-with-spatialos),  so it may reference content relevant to that workflow only. While some of the concepts underpin the GDK for Unreal, the two workflows are not always the same.

### Assembly
An assembly is what’s created when you build your project. It contains all the files that your game uses.
This includes executable files for the [client-workers](#worker) and [server-workers](#worker), and the assets these workers use (for example, textures used by a client to visualize the game).
When you run a [cloud deployment](#deployment), you have to specify an assembly to use.

### Chunk
A [world](#spatialos-world) is split up into chunks: the grid squares of the world. A chunk is the smallest area of space the world can be subdivided into. Every [entity](#entity) is in exactly one chunk.
You set the size of chunks for your world in [launch configuration files](https://docs.improbable.io/reference/latest/shared/reference/file-formats/launch-config).

### Component
See [SpatialOS component](#spatialos-component).

### Command-line tool (CLI)
See [Spatial command-line tool (CLI)](#spatialos-command-line-tool-cli).

### Configuration files
The configuration files contain information on how elements of your project must work. There are four configuration files:

* The [launch configuration file - `*.json`](#launch-configuration-file) contains the information that the “launch a deployment” commands use to use to  run a [deployment](#deployment).
* The [worker configuration file - `*.worker.json`](#worker-configuration-file) tells SpatialOS how to build, launch, and interact with [workers](#worker).
* The [project definition file - `spatialos.json`](#project-definition-file) 
* The [worker packages file - `spatialos_worker_packages.json`](https://docs.improbable.io/reference/latest/shared/reference/file-formats/spatial-worker-packages)

### Console

The [Console](https://console.improbable.io/) is the main landing page for managing [cloud deployments](https://docs.improbable.io/reference/latest/shared/glossary#cloud-deployment). It shows you:

* Your [project name](#project-name)
* Your past and present [cloud deployments](https://docs.improbable.io/reference/latest/shared/glossary#cloud-deployment)
* All of the [SpatialOS assemblies](#assembly) you’ve uploaded
* Links to the [Inspector](#inspector), [Launcher](#launcher), and the logs and metrics page for your deployments.

> **Find out more**
>
> * [Logs](https://docs.improbable.io/reference/latest/shared/operate/logs#cloud-deployments)
> * [Metrics](https://docs.improbable.io/reference/latest/shared/operate/metrics)

### Deployment
When you want to try out your game, you need to upload both its built-out server-side and client-side code to SpatialOS. We call this “launching a deployment”. 

When you launch a deployment to SpatialOS, it sets up an instance of the [SpatialOS Runtime](#spatialos-runtime) to run your game with the code you upload. The Runtime sets up the game’s [SpatialOS world](#spatialos-world) based on a [snapshot](#snapshot], then starts up the [server-worker instance(s)](#server-workers] needed to run the world. You can then share game clients via the [Launcher](#launcher).

There are two types of deployment: local and cloud.

* Cloud deployments run in the cloud on Improbable-hosted servers, known as nodes[nodes](#node).
* You can use a local deployment to test your server-side and client-side code on your development machine. When you launch a local deployment, you start a local version of the SpatialOS Runtime. (Note that the Launcher does not work with local deployments.)

> **Find out more:**
> 
> * [Cloud deployment workflow]({{urlRoot}}/content/cloud-deployment-workflow)
> * [Local deployment workflow]({{urlRoot}}/content/local-deployment-workflow)
> * [Example project: launch a cloud deployment]({{urlRoot}}/content/get-started/example-project/exampleproject-cloud-deployment)
> * [Example project: launch a local deployment]({{urlRoot}}/content/get-started/example-project/exampleproject-local-deployment)
> * [The Launcher](https://docs.improbable.io/reference/latest/shared/operate/launcher)

### Dynamically-spawned entity

[Entities](#entity) can be either dynamically-spawned or non-dynamic:

* Dynamically-spawned entities correspond to Actors that your game creates during runtime. For example, your game might spawn pick-ups in a level; these are dynamically-spawned entities.
* Non-dynamic entities correspond to Actors that are already in your game’s [snapshot](#snapshot) or in your game’s Unreal level. 

### Entity

Entities equate to replicated Actors in Unreal. They are the network-enabled objects that you place or that are spawned into your [SpatialOS world](#spatialos-world). 
Entities are made up of a collection of [SpatialOS components](#spatialos-component).

All of the data that you want the [SpatialOS Runtime](#spatialos-runtime) to store in the [SpatialOS Runtime](#spatialos-runtime), and [worker instances](#worker-types-and-worker-instances) to make updates to and receive updates about, is associated with entities, specifically one type of their constituent components.

### Game world

> Not to be confused with [SpatialOS world](#spatialos-world).

Everything in your Unreal game that a player can see or interact with. 

### Inspector
The Inspector is a web-based tool that you use to explore the internal state of a [SpatialOS world](#spatialos-world). It gives you a real-time view of what’s happening in a [local or cloud deployment](#deployment). Among other things, it displays:

* which [workers](#worker) are connected to the deployment.
* how much [load](https://docs.improbable.io/reference/latest/shared/glossary#load-balancing) the workers are under.
* which [entities](#entity) are in the SpatialOS world.
* what their [SpatialOS components](#spatialos-component)’ [properties]({{urlRoot}}/content/spatialos-concepts/world-entities-components#entities-and-components) are.
* which workers have authority over each SpatialOS component.

> **Find out more**
>
> [The Inspector](https://docs.improbable.io/reference/latest/shared/operate/inspector)

### Launch

> Not to be confused with [the Launcher](#launcher).

In SpatialOS, “launch” means start a game [deployment](#deployment). See also [launch configuration file](#launch-configuration-file).

### Launch configuration
The launch configuration is how you set up the start of your game’s [deployment](#deployment) (its launch). It is represented in the launch configuration file.
See [workers](#worker) and [launch configuration file](#launch-configuration-file).

### Launch configuration file
The [launch configuration file](#launch-configuration-file) is a `.json` file containing the information that the “launch a deployment” commands use to start a [deployment](#deployment).

> **Find out more**
>
> [Launch configuration file](https://docs.improbable.io/reference/latest/shared/reference/file-formats/launch-config)

### Launcher
The Launcher is a tool that can download and start clients that connect to [cloud deployments](#deployment). It's available as an application for Windows and macOS. From the [Console](#console), you can use the Launcher to connect a game client to your own cloud deployment or generate a share link so anyone with the link can download a game client and join your game.
The Launcher downloads the client executable from the [SpatialOS assembly](#assembly) you uploaded.

> **Find out more**
>
> [The Launcher](https://docs.improbable.io/reference/latest/shared/operate/launcher)

### Locator

There are two types of connection between a worker instance and the [SpatialOS Runtime](#spatialos-runtime): Locator and [Receptionist](#receptionist).

You use the Locator to connect a [client-worker instance](#worker-types-and-worker-instances) to a cloud [deployment](#deployment). An authenticated Locator service lists cloud deployments and connects the client-worker instance to the right one.

### Network operations

Also known as "ops".

Network operations are network messages sent between a worker instance and the SpatialOS Runtime. They carry information about updates to worker instances, entities, entity components, commands, and more.

For more information, see the SpatialOS documentation on [operations](https://docs.improbable.io/reference/latest/shared/design/operations).

### Node

> Not to be confused with [worker](#worker).

A node refers to a single machine used by a [cloud deployment](#deployment). Its name indicates the role it plays in your deployment. You can see these on the advanced tab of your deployment details in the [Console](#console).

### Ownership

The GDK mimics Unreal’s [owning connections](https://docs.unrealengine.com/en-US/Gameplay/Networking/Actors/OwningConnections/index.html) so that client-worker instances can manipulate certain Actors.

### Ops

See [Network operations](#network-operations).

### Persistence
Most [entities](#entity) in your [game world](#game-world) need to keep existing if you stop a game [deployment](#deployment) and start a new one. However,  some entities don’t need to keep existing from one deployment to another; you may want per-deployment player abilities and a per-deployment score, for example.

To facilitate this continuity in an entity's state between deployments, there is a `persistence` component in the standard [schema](#schema) library. It’s optional, but all entities that you want to persist in the world must have this component. Persistence means that entities are saved into [snapshots](#snapshot).

> **Find out more**
>
> [The persistence component in the standard schema library](https://docs.improbable.io/reference/latest/shared/schema/standard-schema-library#persistence-optional)

### Placeholder entities

Placeholder entities are useful if you have multiple server-worker instances, for visualizing your [worker instance’s](#worker-types-and-worker-instances) areas of [authority](#authority). The GDK can auto-populate your [local deployment](#deployment) with placeholder entities via the Editor Settings panel which you can access from the [Unreal toolbar]({{urlRoot}}/content/unreal-editor-interface/toolbars). From the **Play** menu, select **SpatialOS settings** and scroll down to **SpatialOS GDK for Unreal**.

<!--
TODO link to Editor Settings doc when it’s ready, and then remove the instructions from this glossary entry https://improbableio.atlassian.net/browse/UNR-1207
-->

### Project name
Your project name is a unique identifier for your game project as a deployment. It’s generated for you when you sign up for SpatialOS. It’s usually something like `beta_someword_anotherword_000`.
You must specify this name when you run a [cloud deployment](#deployment). 
Note that your project name is (usually) not the same as the name of the directory your project is in.

### Project definition file
This is a `spatialos.json` file which lives in your project's spatial directory. 
It lists the SpatialOS [project name](#project-name) assigned to you by Improbable when you sign up as well as the version of [SpatialOS SDK](#spatialos-sdks) your project uses.

> **Find out more**
>
> [Project defnition file - `spatialos.json`](https://docs.improbable.io/reference/latest/shared/reference/file-formats/spatialos-json)

### Queries
Queries allow [workers](#worker) to get information about the [world](#spatialos-world) outside the region they’re [interested in](#interest). For more information, see [queries](https://docs.improbable.io/reference/latest/shared/glossary#queries).

Entity queries are useful if you need to get information about an entity at a particular time.

### Receptionist

There are two types of connection between a worker instance and the [SpatialOS Runtime](#spatialos-runtime): Receptionist and [Locator](#locator).

You use the Receptionist connection for:

* connecting a server-worker instance to a local or cloud deployment
* connecting a client-worker instance to a local deployment, or to a cloud deployment via `spatial cloud connect external <deployment_name>` (see [`spatial cloud connect external`](https://docs.improbable.io/reference/latest/shared/spatial-cli/spatial-cloud-connect-external)

> **Find out more**
> 
> * [Command line connection arguments]({{urlRoot}}/content/command-line-arguments#connection-arguments)
> * [Map travel]({{urlRoot}}/content/map-travel)

### Schema

Schema is a set of data definitions which represent your game's objects and the commands and events to interface with them in SpatialOS. Schema is defined in `.schema` files and written in the SpatialOS language “schemalang”. 

You do not have to write or edit schema files manually: the GDK generates schema files and their contents based on the Actors your have set up for schema generation, using the native Unreal tag `Replicated`. (However, you do have to manually start the schema generation via the GDK toolbar.)

SpatialOS uses schema to generate APIs specific to the SpatialOS components on entities in your project. You can then use these APIs in your game's [worker types](#spatialos-component) so their instances can interact with [SpatialOS components](#spatialos-component).

> **Find out more**
> 
> * [How to use schema]({{urlRoot}}/content/how-to-use-schema)
> * Setting up Actors for schema generation: [Spatial Type]({{urlRoot}}/content/spatial-type)

### Simulated player

A simulated player is a client-worker instance that is controlled by simulated player logic as opposed to real player input. You can use simulated players to scale-test your game. They launch inside a standalone cloud deployment and connect to a target cloud deployment.

> **Find out more**
> 
> [Simulated players]({{urlRoot}}/content/simulated-players)

### Snapshot
A snapshot is a representation of the state of a [SpatialOS world](#spatialos-world) at a given point in time. It stores each [persistent](#persistence) [entity](#entity) and the values of their [SpatialOS components](#spatialos-component)' [properties]({{urlRoot}}/content/spatialos-concepts/world-entities-components#entities-and-components).

You use a snapshot as the starting point (using an an “initial snapshot”) for your [SpatialOS world](#spatialos-world) when you [deploy your game](#deployment).

> **Find out more**
> 
> * [SpatialOS concepts: snapshots]({{urlRoot}}/content/spatialos-concepts/schema-and-snapshots#snapshots)
> * [Reference: snapshots]({{urlRoot}}/content/how-to-use-snapshots)

### SpatialOS command-line tool (CLI)
The SpatialOS command-line tool (also known as the “CLI”) provides a set of commands that you use to interact with a [SpatialOS project](https://docs.improbable.io/reference/latest/shared/reference/project-structure#structure-of-a-spatialos-project). Among other things, you use it to [deploy](#deployment) [locally]({{urlRoot}}/content/get-started/example-project/exampleproject-local-deployment) or [in the cloud]({{urlRoot}}/content/get-started/example-project/exampleproject-cloud-deployment).

> **Find out more**
> 
> * [An introduction to the SpatialOS command-line tool](https://docs.improbable.io/reference/latest/shared/spatial-cli-introduction). Note that the GDK does not support any `spatial worker` commands.
> * [SpatialOS CLI reference](https://docs.improbable.io/reference/latest/shared/spatialos-cli-introduction)

### SpatialOS component

> Not the same as [Unreal Actor Components](https://docs.unrealengine.com/en-us/Programming/UnrealArchitecture/Actors/Components) (Unreal documentation).

Each [entity](#entity) is made up of SpatialOS components. Common SpatialOS components in a game might be `Health`, `Position`, or `PlayerControls`. In the GDK, you don’t need to manually create entities or components; [schema generation](#schema-generation) does this automatically, based on the Unreal Actors that you set up.

There are three types of data that a SpatialOS component can contain:

* [Properties](#spatialos-component-property), which describe persistent values that change over time (for example, a property for a `Health` component could be “the current health value for this entity”.) SpatialOS components’ properties approximate to replicated properties in Unreal.
* [Events](https://docs.improbable.io/reference/latest/shared/glossary#event), which are transient occurrences that can happen to an entity (for example, `StartedWalking`).
* [Commands](https://docs.improbable.io/reference/latest/shared/glossary#command) that a worker instance can call to ask the SpatialOS component to do something, optionally returning a value (for example, `Teleport`).

Each component can have many properties, events and commands. and an entity can have many components, but it must have at least a property component called `Position`.

All of the data that you want the [SpatialOS Runtime](#spatialos-runtime) to store, and you want [worker instances](#worker-types-and-worker-instances) to make updates to and receive updates about, is associated with properties in entities’ components.

> **Find out more**
> 
> Setting up Actors for schema generation: [Spatial Type](https://docs.improbable.io/unreal/alpha/content/spatial-type)
> <br><br>
> If you plan to extend the functionality of the GDK using the [SpatialOS Worker SDK](#spatialos-sdks) - for example, if you want to create [non-Unreal server-worker types]({{urlRoot}}/content/workers/non-unreal-server-worker-types) - you’ll need to know about the [standard schema library](https://docs.improbable.io/reference/latest/shared/schema/standard-schema-library).

### SpatialOS component property

[SpatialOS components](#spatialos-component) can contain properties, which describe persistent values that change over time.

The worker instance with [authority](#authority) over a particular Actor can send property updates. These are received by worker instances that have [interest](#interest) in the Actor.

For Actors that a worker instance has interest in, that worker instance can:

* read the current value of its components’ properties
* watch for changes to the value of these properties

The value of a component’s property forms part of the data about an entity. This data is stored in the [SpatialOS Runtime](#spatialos-runtime).

### SpatialOS Runtime

>Not to be confused with the [SpatialOS world](#spatialos-world).

Also sometimes just called “SpatialOS”. 

A SpatialOS Runtime instance manages the [SpatialOS world](#spatialos-world) of each [deployment](#deployment) by storing all [entities](#entity) and the current state of their [SpatialOS components](#spatialos-component). [Workers](#worker) interact with the SpatialOS Runtime to read and modify the components of an entity as well as send messages between each other.

### SpatialOS SDKs

The SpatialOS SDKs are the Worker SDK and the Platform SDK, which are low-level SpatialOS integrations available as APIs. 

You can use the Worker SDK to extend the functionality of the GDK, or to create low-level [worker types](#worker-types-and-worker-instances) for game logic that doesn’t require Unreal.

You can use the Platform SDK to build tools, workflows and services that integrate with the SpatialOS platform.

> **Find out more**
> 
> * [Worker SDK: SpatialOS SDKs and data: overview](https://docs.improbable.io/reference/latest/shared/sdks-and-data-overview)
> * [Non-Unreal server-worker types]({{urlRoot}}/content/workers/non-unreal-server-worker-types)

### SpatialOS world

> This is not the same as the Unreal [game world](#game-world).

Also known as “the world”.

The world is a central concept in SpatialOS. All the world’s data is associated with [entities](#entity) - specifically, their [SpatialOS components](#spatialos-component). SpatialOS manages the world, keeping track of all the entities and what state they’re in and storing this data in the [SpatialOS Runtime](#spatialos-runtime).

[Worker instances](#worker-types-and-worker-instances) make changes to the world by sending updates to the [SpatialOS Runtime](#spatialos-runtime). Each worker instance has a view onto the world (the part of the world that it has [interest](#interest) in) and receives updates from the Runtime when anything changes in that view.

### Worker

Workers are server-side and client-side software. They perform the computation associated with a [SpatialOS world](#spatialos-world): they are responsible for iterating on the game simulation and updating the definitive state of the game.

#### Worker types and worker instances

When you develop your game, you set up _worker types_. These are like molds for the _worker instances_ which actually do the computation.

Worker instances are the programs that compute and connect a SpatialOS world. In general, you use one or more server-worker instances to compute the world, and each player’s client software uses a client-worker instance to interact with the world. 

When you create a worker type using the [Worker SDK](#spatialos-sdks), each worker type has a [worker configuration file](https://docs.improbable.io/reference/latest/shared/project-layout/introduction) (<worker_type>.worker.json) where you define how SpatialOS should build, launch, and interact with instances of this worker type. With the GDK for Unreal, the GDK creates worker types for you, so, to get going, you don’t have to manually set them up. You can, however, add additional non-Unreal server-worker types using the Worker SDK.

> **Find out more**
> 
> [Non-Unreal server-worker types]({{urlRoot}}/content/workers/non-unreal-server-worker-types)

<!-- TODO How do you set up worker types in Unreal https://improbableio.atlassian.net/browse/DOC-1064 -->
<!-- TODO Offloading info added here:  https://improbableio.atlassian.net/browse/DOC-1064 -->

#### Server-workers

A server-worker [instance](#worker-types-and-worker-instances) equates to a server in native Unreal networking but, unlike Unreal networking, in SpatialOS you can have more than one server-worker instance (see [Zoning](#zoning)). A server-worker instance’s lifecycle is managed by SpatialOS. When you run a SpatialOS [deployment](#deployment) of your game, the [SpatialOS Runtime](#spatialos-runtime) starts and stops server-worker instances.

##### Single server vs multiserver
The GDK sets up one server-worker type for you by default: an Unreal server-worker, computing Unreal functionality. Out of the box, you can have a single instance of this type. However, there are also two multiserver options for working with SpatialOS: [offloading](#offloading) and [zoning](#zoning) (note that zoning is currently in pre-alpha).

You can also add additional non-Unreal server-worker types. For example, you could set up an additional server-worker type to implement player logic and another to implement AI logic. 

**Note:** In a local deployment, the server-worker instances run on your development computer. In a cloud deployment, the server-worker instances run in the cloud. For more information, see [Deployment](#deployment).

> **Find out more**
> 
> * [Non-Unreal server-worker types]({{urlRoot}}/content/workers/non-unreal-server-worker-types)
> * [Offloading]({{urlRoot}}/content/workers/offloading-concept/)
> * [Multiserver zoning tutorial]({{urlRoot}}/content/tutorials/multiserver-shooter/tutorial-multiserver-intro)

<!-- TODO How do you set up worker types in Unreal https://improbableio.atlassian.net/browse/DOC-1064 -->
<!-- TODO Offloading info added here:  https://improbableio.atlassian.net/browse/DOC-1064 -->

#### Client-workers

Each player’s game client - that is, the client software - uses a [client-worker instance](#worker-types-and-worker-instances) to interact with the SpatialOS world. A client-worker instance allows the game client to visualize what’s happening in the SpatialOS world as well as allowing game player’s input to that world. 

The lifecycle of a client-worker instance is managed by the game client. The GDK sets up a client-worker type for you by default.

### Worker configuration
The worker configuration is how you set up your workers. It is represented in the worker configuration file.
See [workers](#worker) and [worker configuration file](#worker-configuration-file).

### Worker configuration file

> First, see [workers](#worker).

Each worker needs a worker configuration file. This file tells SpatialOS how to build, launch, and interact with the workers.
The file’s name must be `spatialos.<worker_type>.worker.json`: for example, `spatialos.MyWorkerType.worker.json`.
Once you’ve chosen a label for the worker type (for example, myWorkerType), you use this exact label consistently throughout your project to identify this worker type.

> **Find out more**
>
> [Worker configuration file `worker.json`](https://docs.improbable.io/reference/latest/shared/worker-configuration/worker-configuration)

### Zoning

Zoning is one of the multiserver options for working with SpatialOS (the other option is [offloading](#offloading)). It involves splitting up the world into areas of [authority](#authority), with a different [server-worker instance](#server-workers) responsible for each. A server-worker instance can make updates only to Actors that are in its area of authority.

> **Note:** Support for zoning is currently in pre-alpha. We invite you to try out the [multiserver zoning tutorial]({{urlRoot}}/content/tutorials/multiserver-shooter/tutorial-multiserver-intro) and learn about how it works, but we don’t recommend you start developing features that use zoning yet.
<br><br>
> **Find out more**
>
> [multiserver zoning tutorial]({{urlRoot}}/content/tutorials/multiserver-shooter/tutorial-multiserver-intro)

<br/>
<br/>------<br/>
_2019-08-08 Page updated with editorial review: updated Actor groups, offloading, zoning, workers, authority_
<br/>_2019-07-30 Page updated without editorial review: added Actor groups, offloading_
<br/>_2019-06-15 Added layers, non-Unreal layers, network operations (ops)_
<br/>_2019-03-15 Page updated with editorial review_
