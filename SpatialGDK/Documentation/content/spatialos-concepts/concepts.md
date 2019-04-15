<%(TOC)%>

# SpatialOS concepts

Copy about what SpatialOS is any why you want to know about these concepts

## Entities and components

### Entities
All of the objects inside a SpatialOS world are entities: they’re the basic building block of the SpatialOS world. Examples include players, NPCs, and objects in the world like trees: anything that you want to be synchronized across multiple processes in your game, such as between server-workers and client-workers, or between the multiple server-workers that simulate your world.

SpatialOS entities are made up of an entity ID and a collection of components which store the data associated with that entity.

In the GDK for Unreal, Actors are equivilant to SpatialOS entities. 

###  Components

A SpatialOS entity is defined by a set of components. Common components in a game might be Health, Position, or PlayerControls. They’re structs which store data that you want to persist in the SpatialOS world.

Components can contain:
* properties - these describe persistent values that change over time (for example, a property for a Health component could be “the current health value for this entity”).
* events -  these are transient broadcasts which all worker instances (server-worker and client-worker) with read access to a given entity component recieve a one-time notification for.
* commands - these are like remote procedure calls (RPCs) between worker instances. An instance of any worker type (server-worker or client-worker) can send a command to the worker instance with write-access authority over a component. 

A SpatialOS entity can have as many components as you like, but it must have at least Position (where it is in the SpatialOS world) and EntityAcl (an access control list); most entities also have a Metadata component. 

## Workers

The SpatialOS Runtime  manages the SpatialOS world: it keeps track of all the SpatialOS entities and their SpatialOS components. But on its own, it doesn’t make any changes to the world.

Workers are programs that connect to a SpatialOS world. They perform the computation associated with a world: they can read what’s happening, watch for changes, and make changes of their own.

There are two kinds of workers: server-workers and client-workers. 

## Authority and interest

### Authority
With SpatialOS, you can have multiple worker instances simulating your game world. But you don’t want more than one at a time to be able to write to a particular component. So SpatialOS uses the concept of write access authority: for any component on any entity, there is never more than one worker instance which has write access authority over it (that is, is able to write to it).

Write access authority is a responsibility: the responsibility to carry out the computation required for a component. This is the case for both client-worker and server-worker instances. If you're using zoning, you'll have multiple server-worker instances simulating the game world between them. When an entity moves from one server-worker instance’s area of authority to another, write access authority over the entity’s components is handed over.

You define areas of authority when you decide on the load balancing strategy for the server-worker type which has write access permission to a component type. The “load” in load balancing is the computation associated with the components which instances of the server-worker type have write access permission to.

There is a similar concept in both peer-to-peer games, which migrate "authority" from host to host, and in the client-server game model, where the server has authority for the whole world.

### Interest
When we talk about interest in SpatialOS, we mean a worker instance wants to receive updates about components from the SpatialOS entity database. Whether a worker instance does receive updates depends on certain criteria, but when it does, we can think of the worker instance as having read access to the component. We call this active read access.

#### Chunk-based interest
This is the older way to define interest; it gives you imprecise, simple control.

A worker instance has default areas of interest based on the components it has write access authority over. You can extend these areas of interest by defining a radius distance around the components it has write access authority over.

#### Query-based interest (QBI)
This is the newer way to define interest, which gives you precise, granular control.

Using QBI, in each [entity template](link) you set up, you add a [component](link) called `improbable.interest`. Here you list the types of component that you want a particular worker type to have interest in. The list is based on conditions and constraints to make queries. The combination of these queries from different entities forms a query map which defines a worker instance’s interest.

## Schema

The schema is where you define all the [SpatialOS components](#components) in your SpatialOS world.

You define your schema in `.schema` files that are written in [schemalang](https://docs.improbable.io/reference/latest/shared/glossary#schemalang). Schema files are stored in the `schema` folder in the root directory of your SpatialOS project.

SpatialOS uses the schema to generate code. You can use this generated code in your [workers](#workers) to interact with [SpatialOS entities](#entities) in the SpatialOS world.

## Snapshots

A snapshot is a representation of a SpatialOS world at a given point in time. A snapshot stores each entity's component data. You start each deployment with a snapshot; if it's a re-deployment of an existing game, you can use the snapshot you originally started your deployment with, or use a snapshot that contains the exact state of a deployment before you stopped it 

To learn more about how to use snapshots in the GDK for Unreal, <GO HERE>

## Load balancing

## Deployments

## Local deployment

## Cloud deployment