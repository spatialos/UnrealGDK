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

There are two types of workers; server-workers and client-workers. 

## Authority and interest

### Authority
Authoirty is write acces

### Interest
Interest is read access

## Schema

The schema is where you define all the [SpatialOS components](#components) in your SpatialOS world.

You define your schema in `.schema` files that are written in [schemalang](https://docs.improbable.io/reference/latest/shared/glossary#schemalang). Schema files are stored in the `schema` folder in the root directory of your SpatialOS project.

SpatialOS uses the schema to generate code. You can use this generated code in your [workers](#workers) to interact with [SpatialOS entities](#entities) in the SpatialOS world.

## Snapshots

A snapshot is a representation of the state of a SpatialOS world at a given point in time. It stores each persistent SpatialOS entity and the values of their [components](#components)' properties.

You use a snapshot as the starting point (using an an “initial snapshot”) for your SpatialOS world when you deploy your game.

## Load balancing

## Layers

In SpatialOS, you can split up [server-worker](#workers) computation into layers, with each layer of server-worker instances handling a specific and unique aspect of your game. By default, the GDK for Unreal uses a single Unreal server-worker layer to handle all server-side computation. However, you can set up additional non-Unreal layers, made up of server-worker instances that do not use Unreal or the GDK.

You can use non-Unreal layers to modularize your game’s functionality so you can re-use the functionality across different games. For example, you could use a non-Unreal layer written in Python that interact with a database or other 3rd party service, such as [Firebase](https://firebase.google.com/) or [PlayFab](https://playfab.com/).

## Deployments

## Local deployment

## Cloud deployment