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
* `<YourProject>` - Name of your project's `.uproject` (for example, `\<GameRoot>\StarterProject.uproject`).

## GDK for Unreal terms

### Actor handover
Actor handover (`handover`) is a new `UPROPERTY` tag. It allows games built in Unreal (which uses single-server architecture) to take advantage of SpatialOS’ distributed, persistent server architecture. See [Actor property handover between server-workers]({{urlRoot}}/content/handover-between-server-workers.md).

### Dynamic Typebindings
To enable the network stacks of Unreal and SpatialOS to interoperate, we've implemented [Dynamic Typebindings]({{urlRoot}}/content/dynamic-typebindings.md). `Dynamic Typebindings` operate at runtime so your that your iteration speed is not affected despite your network code running on a completely different represenetations than Unreal's.

### Cross-server RPCs
These handle the scenario where a [server-worker](#workers) needs to execute an operation on an Actor that another server-worker has [authority](#authority) over. When a cross-server RPC is invoked by a non-authoritative server-worker, the execution is routed through SpatialOS to the authoritative server-worker - this authoritative server-worker executes the RPC. (See the documentation on [Cross-server RPCs]({{urlRoot}}/content/cross-server-rpcs)).

### Global State Manager
The Global State Manager (GSM):

*  Makes sure that [Singleton Actors](#singleton-actor) are replicated properly, by only allowing the [server-worker](#workers) with [authority](#authority) over the GSM to execute the initial replication of these Actors. See documentation on [Singleton Actors]({{urlRoot}}/content/singleton-actors.md).
*  Maintains the configuration of a [deployment’s](#deployment) currently-loaded [game world](#game-world). (Note that this is the Unreal game world not the [SpatialOS world](#spatialos-world).)<br/>  

The GSM lists both the URL of the [Map (or Level - see Unreal documentation)](http://api.unrealengine.com/INT/Shared/Glossary/index.html#l) that the [server-workers](#workers) have loaded and the `AcceptingPlayers` flag. (This flag controls whether or not client-servers can spawn anything in the game world.)

>Related:
>
> [Server travel]({{urlRoot}}/content/map-travel.md) 

### GSM
Short for [Global State Manager](#global-state-manager).

### SchemaDatabase

The SchemaDatabase is a `uasset` file (named `schemadatabase.uasset`) that contains information about UObjects and associated [schema (SpatialOS documentation)](https://docs.improbable.io/reference/13.6/shared/concepts/schema#schema) in your project. Information is automatically added to the SchemaDatabase by the GDK whenever you generate schema. It is an auto-generated file which you cannot manually edit. 

### Schema generation
A SpatialOS GDK for Unreal toolbar command (within the Unreal Editor) which takes a set of Unreal classes and generates SpatialOS [schema](#schema) that enables automatic communication between Unreal and SpatialOS. 

>Related:
>[SpatialOS GDK for Unreal toolbar]({{urlRoot}}/content/toolbars#spatialos-gdk-for-unreal-toolbar)

### Singleton Actor
A server-side authoritative Actor that is restricted to one instantiation on SpatialOS. See documentation on [Singleton Actors]({{urlRoot}}/content/singleton-actors.md).

### Spatial Type
Spatial Type (`SpatialType`) is a SpatialOS-specific [class specifier (Unreal documentation)](https://docs.unrealengine.com/en-US/Programming/UnrealArchitecture/Reference/Classes/Specifiers) which is used to expose network-relevant class information to SpatialOS. There are different categories of Spatial Type, depending on the Actor’s function in your game.

See the documentation on [Spatial Type]({{urlRoot}}/content/spatial-type). 

### SpatialType
See [Spatial Type](#spatial-type).

## SpatialOS terms
Below is a subset of SpatialOS terms most relevant to the GDK for Unreal. See the [SpatialOS documentation glossary](https://docs.improbable.io/reference/latest/shared/glossary) for a full list of terms specific to SpatialOS.

Note that this SpatialOS documentation glossary assumes you are developing a SpatialOS game using the [Worker SDK and Platform SDK](https://docs.improbable.io/reference/latest/shared/get-started/working-with-spatialos),  so it may reference content relevant to that workflow only. While some of the concepts underpin the GDK for Unreal, the two workflows are not always the same.

### Access control list (ACL)
In order to read from a [component](#spatialos-component), or make changes to a component, [workers](#workers) need to have [access](#authority), which they get through an access control list. 
Access control lists are a component in the standard schema library: `EntityAcl`. Every [entity](#spatialos-entity) needs to have one. The ACL determines:

* which types of workers have read access to an entity
* for each component on the entity, which types of workers can have write access

### Assembly
An assembly is what’s created when you build your project. It contains all the files that your game uses.
This includes executable files for the [client-workers](#workers) and [server-workers](#workers), and the assets these workers use (for example, textures used by a client to visualize the game).
When you run a [cloud deployment](#deployment), you have to specify an assembly to use.

### Authority
Also known as “write access”.

* Write access:<br/>
Many [workers](#workers) can connect to a [SpatialOS world](#spatialos-world). For each [component](#spatialos-component) on a [SpatialOS entity](#spatialos-entity), there can be no more than one worker with write access to it. This worker is the only one able to modify the component’s state and handle commands for it.
Workers with write access are said to “have authority” and be “authoritative”; workers without write access are said to be “non-authoritative”.
Which [types of workers](#worker-types) can have write access is governed by each entity’s [access control list (ACL)](#access-control-list-acl). A write ACL is specified per component. The write authority is managed by SpatialOS, and can change regularly due to [load balancing (SpatialOS concept documentation)](https://docs.improbable.io/reference/latest/shared/glossary#load-balancing).
<br/>
* Read access:<br/>
Read access allows [workers](#workers) to know the state of a [SpatialOS component](#spatialos-component). The [access control list (ACL)](#access-control-list-acl) also controls which workers have read-access to a [SpatialOS entity](#spatialos-entity). Read access does not allow a worker to change a component. Read access is at the entity level; if a worker can read from an entity, it is allowed to read from all components on that entity. 

### Check out
Each individual [worker](#workers) checks out only part of the [SpatialOS world](#spatialos-world). This happens on a [chunk](#chunk)-by-chunk basis. A worker “checking out a chunk” means that: 

* the worker has a local representation of every [entity](#spatialos-entity) in that chunk. 
* the SpatialOS Runtime sends updates about those entities to the worker.

A worker checks out all chunks that it is [interested in](#interest).

### Chunk
A [world](#spatialos-world) is split up into chunks: the grid squares of the world. A chunk is the smallest area of space the world can be subdivided into. Every [entity](#spatialos-entity) is in exactly one chunk.
You set the size of chunks for your world in [launch configuration files](https://docs.improbable.io/reference/latest/shared/reference/file-formats/launch-config) (SpatialOS documentation).

### Component
See [SpatialOS component](#spatialos-component).

### Command-line tool (CLI)
See [Spatial command-line tool (CLI)](#spatial-command-line-tool-cli).

### Configuration files
The configuration files contain information on how elements of your project must work. There are four configuration files:

* The [launch configuration file - `*.json`](#launch-configuration-file) contains the information that the “launch a deployment” commands use to use to  run a [deployment](#deployment).
* The [worker configuration file - `*.worker.json`](#worker-configuration-file) tells SpatialOS how to build, launch, and interact with [workers](#workers).
* The [project definition file - `spatialos.json`](#project-definition-file) 
* The [worker packages file - `spatialos_worker_packages.json`(SpatialOS documentation)](https://docs.improbable.io/reference/latest/shared/reference/file-formats/spatial-worker-packages)

### Console

The [Console](https://console.improbable.io/) is the main landing page for managing [cloud deployments (SpatialOS documentation)](https://docs.improbable.io/reference/latest/shared/glossary#cloud-deployment). It shows you:

* Your [project name](#project-name)
* Your past and present [cloud deployments (SpatialOS documentation)](https://docs.improbable.io/reference/latest/shared/glossary#cloud-deployment)
* All of the [SpatialOS assemblies](#assembly) you’ve uploaded
* Links to the [Inspector](#inspector), [Launcher](#launcher), and the logs and metrics page for your deployments.

> Related:
>
> * [Logs (SpatialOS documentation)](https://docs.improbable.io/reference/latest/shared/operate/logs#cloud-deployments)
> * [Metrics (SpatialOS documentation)](https://docs.improbable.io/reference/latest/shared/operate/metrics)

### Deployment
When you want to try out your game, you need to deploy it. This means launching SpatialOS itself. SpatialOS sets up the [world](#spatialos-world) based on a [snapshot](#snapshot), then starts up the [server-workers](#workers) needed to run the world. 

There are two types of deployment: local and cloud.

Local deployments allow you to start the SpatialOS [Runtime](#spatialos-runtime) locally to test changes quickly. Find out more about local deployments in the [SpatialOS documentation](https://docs.improbable.io/reference/latest/shared/deploy/deploy-local).  

As their name suggests, cloud deployments run in the cloud on [nodes](#node). They allow you to share your game with other people and run your game at a scale not possible on one local machine. Once a cloud deployment is running, you can connect clients to it using the [Launcher](#launcher).

### Entity
See [SpatialOS entity](#spatialos-entity).

### Game world

>Not to be confused with [SpatialOS world](#spatialos-world).

Everything in your Unreal game that a player can see or interact with. 

### Inspector
The Inspector is a web-based tool that you use to explore the internal state of a [SpatialOS world](#spatialos-world). It gives you a real-time view of what’s happening in a [local or cloud deployment](#deployment). Among other things, it displays:

* which [workers](#workers) are connected to the deployment.
* how much [load (SpatialOS documentation)](https://docs.improbable.io/reference/latest/shared/glossary#load-balancing) the workers are under.
* which [SpatialOS entities](#spatialos-entity) are in the SpatialOS world.
* what their [SpatialOS components](#spatialos-component)’ [properties (SpatialOS documentation)](https://docs.improbable.io/reference/latest/shared/glossary#property) are.
* which workers are authoritative over each SpatialOS component.

>Related:
>
>[The Inspector (SpatialOS documentation)](https://docs.improbable.io/reference/latest/shared/operate/inspector)

### Interest
There are two types of interest: entity interest and component interest.

#### Entity interest
A [worker](#workers) is interested in all [chunks](#chunk) that contain [entities](#spatialos-entity) it has
[write access](#authority) to a [component](#spatialos-component) on. It's *also* interested in chunks within a configurable
radius of those entities: this makes sure that workers are aware of entities nearby. You can set this radius
in the [worker configuration file](#worker-configuration-file).
If a worker is interested in a chunk, it will [check out](#check-out) all the entities in that chunk.

#### Component interest
Each [worker](#workers), in its [worker configuration file](#worker-configuration-file), specifies which [components](#spatialos-component) it is
interested in. SpatialOS only sends updates about components to a worker which is interested in that component.

> Related:
>
> * [Entity interest settings (SpatialOS documentation)](https://docs.improbable.io/reference/latest/shared/worker-configuration/bridge-config#entity-interest)
> * [Component delivery settings (SpatialOS documentation)](https://docs.improbable.io/reference/latest/shared/worker-configuration/bridge-config#component-delivery)


### Launch

>Not to be confused with [the Launcher](#launcher).

In SpatialOS, “launch” means start a game [deployment](#deployment). See also [launch configuration file](#launch-configuration-file).

### Launch configuration
The launch configuration is how you set up the start of your game’s [deployment](#deployment) (its launch). It is represented in the launch configuration file.
See [workers](#workers) and [launch configuration file](#launch-configuration-file).

### Launch configuration file
The [launch configuration file](#launch-configuration-file) is a `.json` file containing the information that the “launch a deployment” commands use to start a [deployment](#deployment).

>Related:
>
>[Launch configuration file (SpatialOS documenation)](https://docs.improbable.io/reference/latest/shared/reference/file-formats/launch-config)

### Launcher
The Launcher is a tool that can download and start clients that connect to [cloud deployments](#deployment). It's available as an application for Windows and macOS. From the [Console](#console), you can use the Launcher to connect a game client to your own cloud deployment or generate a share link so anyone with the link can download a game client and join your game.
The Launcher downloads the client executable from the [SpatialOS assembly](#assembly) you uploaded.

> Related:
>
> [The Launcher (SpatialOS documentation)](https://docs.improbable.io/reference/latest/shared/operate/launcher)

### Load balancing
One of the features of SpatialOS is load balancing: dynamically adjusting how many [components](#spatialos-component) on [entities](#spatialos-entity) in the [world](#spatialos-world) each [worker](#workers) has [write access](#authority) to, so that workers don’t get overloaded.

Load balancing only applies to [server-workers](#workers).
When an instance of a worker is struggling with a high workload, SpatialOS can start up new instances of the worker, and give them write access to some components on entities.

This means that an [entity](#spatialos-entity) won’t necessarily stay on the same worker instance, even if that entity doesn’t move. SpatialOS may change which components on which entities a worker instance has write access to: so entities move “from” one worker instance to another. Even though the entity may be staying still, the worker instance’s [area of interest](#interest) is moving.

>Related pages:
>
> [Configuring load balancing (SpatialOS documentation)](https://docs.improbable.io/reference/latest/shared/worker-configuration/loadbalancer-config)

### Node

>Not to be confused with [worker](#workers).

A node refers to a single machine used by a [cloud deployment](#deployment). Its name indicates the role it plays in your deployment. You can see these on the advanced tab of your deployment details in the [Console](#console).

### Persistence
Most [entities](#spatialos-entity) in your [game world](#game-world) need to keep existing if you stop a game [deployment](#deployment) and start a new one. However,  some entities don’t need to keep existing from one deployment to another; you may want per-deployment player abilities and a per-deployment score, for example.

To facilitate this continuity in an entity's state between deployments, there is a `persistence` component in the standard [schema](#schema) library. It’s optional, but all entities that you want to persist in the world must have this component. Persistence means that entities are saved into [snapshots](#snapshot).

>Related:
>
>[The persistence component in the standard schema library (SpatialOS documentation](https://docs.improbable.io/reference/latest/shared/schema/standard-schema-library#persistence-optional)

### Project name
Your project name is a unique identifier for your game project as a deployment. It’s generated for you when you sign up for SpatialOS. It’s usually something like `beta_someword_anotherword_000`.
You must specify this name when you run a [cloud deployment](#deployment). 
Note that your project name is (usually) not the same as the name of the directory your project is in.

### Project definition file
This is a `spatialos.json` file which lives in your project's spatial directory. 
It lists the SpatialOS [project name](#project-name) assigned to you by Improbable when you sign up as well as the version of [SpatialOS SDK](#spatialos-sdk) your project uses.

>Related
>
>[Project defnition file - `spatialos.json`](https://docs.improbable.io/reference/latest/shared/reference/file-formats/spatialos-json)

### Queries
Queries allow [workers](#workers) to get information about the [world](#spatialos-world) outside the region they’re [interested in](#interest). For more information, see [queries (SpatialOS documentation)](https://docs.improbable.io/reference/latest/shared/glossary#queries).

> Entity queries are useful if you need to get information about an entity at a particular time.
> If you need regular updates about an entity, use [streaming queries](#streaming-queries) instead.

### Read access
See [authority](#authority).

### Schema
The schema is where you define all the [SpatialOS components](#spatialos-component) in your [SpatialOS world](#spatialos-world).

You define your schema in `.schema` files that are written in [schemalang (SpatialOS documentation)](https://docs.improbable.io/reference/latest/shared/glossary#schemalang). Schema files are stored in the `schema` folder in the root directory of your SpatialOS project.

SpatialOS uses the schema to generate code. You can use this generated code in your [workers](#workers) to interact with [SpatialOS entities](#spatialos-entity) in the SpatialOS world.

> Related:
>
> * [Schema (Unreal GDK documentation)]({{urlRoot}}/content/schema)
> * [Introduction to schema (SpatialOS documentation)](https://docs.improbable.io/reference/latest/shared/schema/introduction)
> * [Schema reference (SpatialOS documentation)](https://docs.improbable.io/reference/latest/shared/schema/reference)

### `spatial` command-line tool (CLI)
The `spatial` command-line tool (also known as the “CLI”) provides a set of commands that you use to interact with a [SpatialOS project (SpatialOS documentation)](https://docs.improbable.io/reference/latest/shared/reference/project-structure#structure-of-a-spatialos-project). Among other things, you use it to [deploy](#deployment) your game (using [`spatial local launch` (SpatialOS documentation)](https://docs.improbable.io/reference/latest/shared/spatial-cli/spatial-local-launch) or [`spatial cloud launch` (SpatialOS documentation)](https://docs.improbable.io/reference/latest/shared/spatial-cli/spatial-cloud-launch)). You can run the CLI commands `spatial build` and `spatial local launch` from the [GDK toolbar]({{urlRoot}}/content/toolbars.md#spatialos-gdk-for-unreal-toolbar) in the Unreal Editor.

> Related:
> 
> * [An introduction to the `spatial` command-line tool (SpatialOS documentation)](https://docs.improbable.io/reference/latest/shared/spatial-cli-introduction). Note that the GDK does not support any `spatial worker` commands.
> * [`spatial` reference (SpatialOS documentation)](https://docs.improbable.io/reference/latest/shared/spatial-cli/spatial)


### SpatialOS component
> Not to be confused with [Unreal Actor Components (Unreal documentation](https://docs.unrealengine.com/en-us/Programming/UnrealArchitecture/Actors/Components)

A [SpatialOS entity](#spatialos-entity) is defined by a set of components. Common components in a game might be things like `Health`, `Position`, or `PlayerControls`. They're the storage mechanism for data about the [world](#spatialos-world) that you want to be shared between [workers](#workers).
Components can contain:

* [properties (SpatialOS documentation)](https://docs.improbable.io/reference/latest/shared/glossary#property), which describe persistent values that change over time (for example, a property for a `Health` component could be “the current health value for this entity”.)
* [events (SpatialOS documentation)](https://docs.improbable.io/reference/latest/shared/glossary#event), which are transient things that can happen to an entity (for example, `StartedWalking`)
* [commands (SpatialOS documentation)](https://docs.improbable.io/reference/latest/shared/glossary#command) that another worker can call to ask the component to do something, optionally returning a value (for example, `Teleport`)

A SpatialOS entity can have as many components as you like, but it must have at least [`Position` (SpatialOS documentation)](https://docs.improbable.io/reference/latest/shared/glossary#position) and [`EntityAcl`](#access-control-list-acl). Most entities will have the [`Metadata` (SpatialOS documentation)](https://docs.improbable.io/reference/latest/shared/glossary#metadata) component.

SpatialOS components are defined as files in your [schema](#schema).
[Entity access control lists](#access-control-list-acl) govern which workers can [read from](#read-access) or [write to](#write-access) each component on an entity.

> Related:
>
> * [Designing components (SpatialOS documentation)](https://docs.improbable.io/reference/latest/shared/design/design-components)
> * [Component best practices (SpatialOS documentation)](https://docs.improbable.io/reference/latest/shared/design/component-best-practices)
> * [Introduction to schema (SpatialOS documentation)](https://docs.improbable.io/reference/latest/shared/schema/introduction)

### SpatialOS entity
All of the objects inside a [SpatialOS world](#spatialos-world) are SpatialOS entities: they’re the basic building blocks of the world. Examples include players, NPCs, and objects in the world like trees. A SpatialOS entity approximates to an Unreal Actor.  

SpatialOS entities are made up of [SpatialOS components](#spatialos-component), which store data associated with that entity.

[Workers](#workers) can only see the entities they're [interested in](#interest).

> Related:
>
> * [SpatialOS concepts: Entities (SpatialOS documentation)](https://docs.improbable.io/reference/latest/shared/concepts/world-entities-components#entities)
> * [Designing SpatialOS entities (SpatialOS documentation)](https://docs.improbable.io/reference/latest/shared/design/design-entities)

### SpatialOS Runtime

>Not to be confused with the [SpatialOS world](#spatialos-world).


Also sometimes just called “SpatialOS”. 

A SpatialOS Runtime instance manages the [SpatialOS world](#spatialos-world) of each [deployment](#deployment) by storing all [SpatialOS entities](#spatialos-entity) and the current state of their [SpatialOS components](#spatialos-component). [Workers](#workers) interact with the SpatialOS Runtime to read and modify the components of an entity as well as send messages between each other.

### SpatialOS SDK
This is a set of low-level tools in several programming languages which you can use to integrate your game project with SpatialOS. It consists of the Worker SDK (or “Worker module”) and Platform SDK (or “Platform module”). 
If you are using the GDK for Unreal, you do not need to use the Worker SDK or Platform SDK, however, you can use the Worker SDK to extend or complement the functionality of the GDK for Unreal. 

> Related:
>
> Worker SDK: [Game development tools overview](https://docs.improbable.io/reference/latest/shared/dev-tools-intro)
> [Platform SDK overview](https://docs.improbable.io/reference/latest/platform-sdk/introduction)

### SpatialOS world

>Not to be confused with the Unreal [game world](#game-world).

Also known as "the world".

The world is a central concept in SpatialOS. It’s the canonical source of truth about your game. All the world's data is stored within [entities](#entity) - specifically, within their [components](#component).
SpatialOS manages the world, keeping track of all the entities and what state they’re in.

Changes to the world are made by [workers](#workers). Each worker has a view onto the world (the part of the world that they're [interested](#interest) in), and SpatialOS sends them updates when anything changes in that view.

It's important to recognise this fundamental separation between the SpatialOS world and the view of that world that a worker [checks out](#check-out). Workers send updates to SpatialOS when they want to change the world: they don't control the canonical state of the world; they must use SpatialOS APIs to change it.

### Snapshot
A snapshot is a representation of the state of a [SpatialOS world](#spatialos-world) at a given point in time. It stores each [persistent](#persistence) [SpatialOS entity](#spatialos-entity) and the values of their [SpatialOS components](#spatialos-component)' [properties (SpatialOS documentation)](https://docs.improbable.io/reference/latest/shared/glossary#property).

You use a snapshot as the starting point (using an an “initial snapshot”) for your [SpatialOS world](#spatialos-world) when you [deploy your game](#deployment).

> Related:
> 
> * [How to generate a snapshot]({{urlRoot}}/content/generating-a-snapshot)
> * [Snapshots (SpatialOS documentation)](https://docs.improbable.io/reference/latest/shared/operate/snapshots)

### Streaming queries
Streaming queries allow workers to get information about the [world](#spatialos-world) outside the region they’re [interested in](#interest), so that they know about entities that they don’t have checked out (for example, entities that are far away, or that don’t have a physical position).

> Streaming queries are useful if you need to get information about an entity periodically - for example, so that a player can see and interact with it.
> 
> If you just need information about an entity at one particular time, use [queries](#queries) instead.

### Workers
The [SpatialOS Runtime](#spatialos-runtime) manages the [SpatialOS world](#spatialos-world): it keeps track of all the [SpatialOS entities](#spatialos-entity) and their
[SpatialOS components](#spatialos-component). But on its own, it doesn’t make any changes to the world.

Workers are programs that connect to a SpatialOS world. They perform the computation associated with a world: they can read what’s happening, watch for changes, and make changes of their own.

There are two types of workers; server-workers and client-workers. 

A server-worker approximates to a server in native Unreal networking but, unlike Unreal networking, in SpatialOS you can have have more than one server-worker.

* **Server-worker**
    A server-worker is a [worker](#workers) whose lifecycle is managed by SpatialOS. When running a [deployment](#deployment), the SpatialOS Runtime starts and stops server-workers based on your chosen [load balancing (SpatialOS documentation)](https://docs.improbable.io/reference/latest/shared/glossary#load-balancing) configuration.

    You usually set up server-workers to implement game logic and physics simulation. You can have one server-worker connected to your [deployment](#deployment), or dozens, depending on the size and complexity of your [SpatialOS world](#spatialos-world).
    If you run a [local deployment](#deployment), the server-workers run on your development computer. If you run a [cloud deployment](#deployment), the server-workers run in the cloud.
* **Client-worker**

    While the lifecycle of a server-worker is managed by the SpatialOS Runtime, the lifecycle of a client-worker is managed by the game client.
    You usually set up client-workers to visualize what’s happening in the [SpatialOS world](#spatialos-world). They also deal with player input.

    > Related:
    >
    > * [External worker (client-worker) launch configuration (SpatialOS documentation)](https://docs.improbable.io/reference/latest/shared/worker-configuration/launch-configuration#external-worker-launch-configuration)

**More about workers**</br>
In order to achieve huge scale, SpatialOS divides up the SpatialOS entities in the world between workers, balancing the work so none of them are overloaded. For each SpatialOS entity in the world, it decides which worker should have [write access](#authority) to each SpatialOS component on the SpatialOS entity. To prevent multiple workers writing to a component at the same time, only one worker at a time can have write access to a SpatialOS component.

As the world changes over time, the position of SpatialOS entities and the amount of updates associated with them changes. Server-workers report back to SpatialOS how much load they're under, and SpatialOS adjusts which workers have write access to components on which SpatialOS entities. SpatialOS then starts up new workers when needed. This is [load balancing (SpatialOS documentation)](https://docs.improbable.io/reference/latest/shared/worker-configuration/loadbalancer-config).
Around the SpatialOS entities which they have write access to, every worker has an area of the world they are [interested in](#interest).

A worker can read the current state of components of the SpatialOS entities within this area, and SpatialOS sends [updates and messages (SpatialOS documentation)](https://docs.improbable.io/reference/latest/shared/glossary#sending-an-update) about these SpatialOS entities to the worker.

If the worker has write access to a SpatialOS component, it can [send updates and messages (SpatialOS documentation)](https://docs.improbable.io/reference/latest/shared/glossary#sending-an-update):
it can update [properties (SpatialOS documentation)](https://docs.improbable.io/reference/latest/shared/glossary#property), send and handle [commands (SpatialOS documentation)](https://docs.improbable.io/reference/latest/shared/glossary#command) and trigger [events (SpatialOS documentation)](https://docs.improbable.io/reference/latest/shared/glossary#event).

> Related:
>
> * [Concepts: Workers and load balancing (SpatialOS documentation)](https://docs.improbable.io/reference/latest/shared/concepts/workers-load-balancing)

### Worker configuration
The worker configuration is how you set up your workers. It is represented in the worker configuration file.
See [workers](#workers) and [worker configuration file](#worker-configuration-file).

### Worker configuration file

> First, see [workers](#workers).

Each worker needs a worker configuration file. This file tells SpatialOS how to build, launch, and interact with the workers.
The file’s name must be `spatialos.<worker_type>.worker.json`: for example, `spatialos.MyWorkerType.worker.json`.
Once you’ve chosen a label for the worker type (for example, myWorkerType), you use this exact label consistently throughout your project to identify this worker type.

> Related:
>
>[Worker configuration file `worker.json` (SpatialOS documenation)](https://docs.improbable.io/reference/latest/shared/worker-configuration/worker-configuration)

### Worker types

> First, see [workers](#workers).

There are two generic types of worker that define how you would want to connect these workers and what kind of capabilities they have:

* [server-worker](#workers)
* [client-worker](#workers)

Within these broad types, you can define your own worker sub-types to create more specialized workers.

### Write access
See [authority](#authority).
