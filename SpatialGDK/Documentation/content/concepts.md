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
Workers are cool fun things. 

## Authority and interest

### Authority
Authoirty is write acces

### Interest
Interest is read access

## Components

## Schema

## Snapsots

## Load balancing

## Layers

In SpatialOS, you can split up [server-worker](https://docs.improbable.io/unreal/alpha/content/glossary#workers) computation into layers, with each layer of server-worker instances handling a specific and unique aspect of your game. By default, the GDK for Unreal uses a single Unreal server-worker layer to handle all server-side computation. However, you can set up additional non-Unreal layers, made up of server-worker instances that do not use Unreal or the GDK.

You can use non-Unreal layers to modularize your game’s functionality so you can re-use the functionality across different games. For example, you could use a non-Unreal layer written in Python that interact with a database or other 3rd party service, such as [Firebase](https://firebase.google.com/) or [PlayFab](https://playfab.com/).

Layers in SpatialOS
A SpatialOS layer has two elements;
a group of SpatialOS [entity components](https://docs.improbable.io/unreal/alpha/content/glossary#spatialos-entity),
server-worker instances of a worker type that have [write access authority](https://docs.improbable.io/unreal/alpha/content/glossary#authority) over the group of components.
(For information on layers in non-GDK SpatialOS development, see the [SpatialOS documentation] (https://docs.improbable.io/reference/latest/shared/worker-configuration/layers#layers).)
