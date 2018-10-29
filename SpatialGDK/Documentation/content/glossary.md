# Glossary
This glossary covers:
* Terms used in this documentation
* Terms relevant to the GDK for Unreal
* SpatialOS terms used in the GDK for Unreal documentation

**SpatialOS documentation**<br/>
Many SpatialOS term definitions link to further information in the [SpatialOS documentation](https://docs.improbable.io/reference/latest/index). This documentation covers the SpatialOS SDK - a software development kit for SpatialOS which underpins the GDK for Unreal, as well some GDK-relevant tools; the [Console](#console), [schema](#schema), [snapshots](#snapshots), and [configuration files](#configuration files) in particular.

Note that this SpatialOS documentation assumes you are developing a SpatialOS game using the SpatialOS SDKs ([The Worker SDK and Platform SDK](#https://docs.improbable.io/reference/latest/shared/get-started/working-with-spatialos)), so it may reference content relevant to that workflow only. While the SpatialOS SDK underpins the GDK for Unreal, the two workflows are not always the same.

## GDK for Unreal documentation terms
* `<GameRoot>` - The folder containing your game project's `.uproject` and source folder.  
* `<ProjectRoot>` - The folder containing your `<GameRoot>`.  
* `<YourProject>` - Name of your game project's `.uproject` (for example, `\<GameRoot>\StarterProject.uproject`).

## GDK for Unreal terms

### Actor handover
Handover is a new `UPROPERTY` tag. It allows games built in Unreal which uses single-server architecture to take advantage of SpatialOS’ distributed, persistent server architecture. See [Actor and entity property handover between server-workers]({{urlRoot}}/content/handover-between-server-workers.md).

### Cross-server RPCs
This handles the scenario where a [server-worker](#workers) needs to execute an operation on an Actor that another server-worker has [authority](#authority) over. When a cross-server RPC is invoked by a non-authoritative server-worker, the execution is routed through SpatialOS to the authoritative server-worker - this authoritative server-worker executes the RPC. (See the documentation on [Cross-server RPCs]({{urlRoot}}/content/cross-server-rpcs)).

### Global State Manager
The Global State Manager (GSM):
*  Makes sure that [Singleton Actors](#singleton-actors) are replicated properly, by only allowing the [server-worker](#workers) with [authority](#authority) over the GSM to execute the initial replication of these Actors. See documentation on [Singleton Actors]({{urlRoot}}/content/singleton-actors.md).
*  Maintains the configuration of a [deployment’s](#deployment) currently-loaded [game world](#game-world). (Note that this is the Unreal game world not the [SpatialOS world](#spatialos-world).)<br/> 
The GSM lists both the URL of the [Map (or Level - see Unreal documentation)](http://api.unrealengine.com/INT/Shared/Glossary/index.html#l) that the [server-workers](#servers) have loaded and the `AcceptingPlayers` flag. (This flag controls whether or not client-servers can spawn anything in the game world.)

>Related:
>
> [Server travel]({{urlRoot}}/content/map-travel.md) 

### GSM
Short for [Global State Manager](#global-state-manager).

### Schema Generator
An Unreal Editor toolbar command which takes a set of Unreal classes and generates SpatialOS [schema](#schema) that enables automatic communication between Unreal and SpatialOS. 

>Related:
>[The toolbar]({{urlRoot}}/content/toolbar)

### Singleton Actor
A server-side authoritative Actor that is restricted to one instantiation on SpatialOS. See documentation on [Singleton Actors]({{urlRoot}}/content/singleton-actors.md).

###Spatial Type
Unreal classes have unique properties when running in SpatialOS. Spatial Type (`SpatialType`) is a SpatialOS-specific [class specifier (Unreal documentation)](https://docs.unrealengine.com/en-US/Programming/UnrealArchitecture/Reference/Classes/Specifiers) which reflects this information. There are different categories of Spatial Type, depending on the Actor’s function in your game.
See the documentation on [Spatial Type](#{{urlRoot}}/content/spatial-type). 

###SpatialType
See [Spatial Type](#spatial-type).

## SpatialOS terms
Below is a subset of SpatialOS terms most relevant to the GDK for Unreal. See the [SpatialOS documentation glossary](https://docs.improbable.io/reference/latest/shared/glossary) for a full list of terms specific to SpatialOS.

Note that this SpatialOS documentation glossary assumes you are developing a SpatialOS game using the SpatialOS SDKs ([The Worker SDK and Platform SDK](#https://docs.improbable.io/reference/latest/shared/get-started/working-with-spatialos)),  so it may reference content relevant to that workflow only. While the SpatialOS SDK underpins the GDK for Unreal, the two workflows are not always the same.)

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
<br/>
<br/>
Write access:<br/>
Many [workers](#workers) can connect to a [SpatialOS world](#spatialos-world). For each [component](#spatialos-component) on a [SpatialOS entity](#spatialos-entity), there can be no more than one worker with write access to it. This worker is the only one able to modify the component’s state and handle commands for it.
Workers with write access are said to “have authority” and be “authoritative”; workers without write access are said to be “non-authoritative”.
Which [types of workers](#worker-types) can have write access is governed by each entity’s [access control list (ACL)](#access-control-list-acl). This list specifies which worker actually has write access over an entity. The list is managed by SpatialOS, and can change regularly due to [load balancing (SpatialOS concept documentation)](https://docs.improbable.io/reference/latest/shared/glossary#load-balancing).
<br/>
<br/>
Read access:<br/>
Read access allows [workers](#workers) to know the state of a [SpatialOS component](#spatialos-component)  The [access control list (ACL)](#access-control-list-acl) also controls which workers have read-access to a [SpatialOS entity](#spatialos-entity). Read access does not allow a worker to change a component. Read access is at the entity level; if a worker can read from an entity, it is allowed to read from all components on that entity. 

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
### Configuration files
The configuration files contain information on how elements of your project must work. There are four configuration files:
* The [launch configuration file - `*.json`](#launch-configuration-file) contains the information that the “launch a deployment” commands use to use to  run a [deployment](#deployment).
* The [worker configuration file - `*.worker.json`](#worker-configuration-file) tells SpatialOS how to build, launch, and interact with [workers](#workers).
* The [project definition file - `spatialos.json`](#project-definition-file) 
* The [worker packages file - `spatialos_worker_packages.json`(SpatialOS documentation)](https://docs.improbable.io/reference/latest/shared/reference/file-formats/spatial-worker-packages)

### Command-line tool (CLI)
See [Spatial command-line tool (CLI)](#spatial-command-line-tool-cli).

### Deployment
When you want to try out your game, you need to deploy it. This means launching SpatialOS itself. SpatialOS sets up the [world](#spatialos-world) based on a [snapshot](#snapshot), then starts up the [server-workers](#server-workers) needed to run the world. 
There are two types of deployment: local and cloud.
Local deployments allow you to start the SpatialOS [Runtime](#spatialos-runtime) locally to test changes quickly. Find out more about local deployments in the [SpatialOS documentation](https://docs.improbable.io/reference/latest/shared/deploy/deploy-local.
As their name suggests, cloud deployments run in the cloud on [nodes](#node). They allow you to share your game with other people and run your game at a scale not possible on one local machine. Once a cloud deployment is running, you can connect [game clients](#game-client) to it using the [Launcher](#launcher).
### Entity
See [SpatialOS entity](#spatialos-entity).
### Game client
Not to be confused with [client-worker](#workers).
A game client is a binary file;  it’s an executable computer program that runs on a player’s computer. A player uses a game client to play your game.  A client-worker is an object instantiated by this executable program when it runs.
### Game world
>Not to be confused with [SpatialOS world](#spatialos-world).
Everything in your Unreal game that a player can see or interact with. 
### Inspector
The Inspector is a web-based tool that you use to explore the internal state of a [SpatialOS world](#spatialos-world). It gives you a real-time view of what’s happening in a [local or cloud deployment](#deployment). Among other things, it displays:
which [workers](#workers) are connected to the deployment.
how much [load (SpatialOS documentation)](https://docs.improbable.io/reference/latest/shared/glossary#load-balancing) the workers are under.
which [SpatialOS entitie](#spatialos-entities) are in the SpatialOS world.
