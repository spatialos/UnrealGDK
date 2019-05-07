<%(TOC)%>
# Concepts: schema and snapshots

## Schema

Schema is a set of definitions which represent your game's Actors and subobjects in SpatialOS. Schema is defined in `.schema` files and written in schemalang.

When you use the GDK, the schema files and their contents are generated automatically so you do not have to write or edit schema files manually.

SpatialOS uses schema to generate APIs specific to the SpatialOS entity components in your project. You can then use these APIs in your game's [worker types]({{urlRoot}}//content/glossary#spatialos-component) so their instances can interact with [SpatialOS entity components]({{urlRoot}}/content/glossary#spatialos-component).

You can find out how to use schema in the [schema reference documentation]({{urlRoot}}/content/how-to-use-schema)

## Snapshots

A snapshot is a representation of the state of a [SpatialOS world]({{urlRoot}}/content/glossary#spatialos-world) at a given point in time. It stores each [persistent]({{urlRoot}}/content/glossary#persistence) [SpatialOS entity]({{urlRoot}}/content/glossary#spatialos-entity) and the values of their [SpatialOS components]({{urlRoot}}/content/glossary#spatialos-component)' [properties](https://docs.improbable.io/reference/latest/shared/glossary#property).

You can find out how to use snapshots in the [snapshot reference documentation]({{urlRoot}}/content/how-to-use-snapshots).

### What’s listed in snapshots

The GDK snapshots contain two kinds of [SpatialOS entities]({{urlRoot}}/content/glossary#spatialos-entity):
critical entities and placeholder entities.

#### Critical entities

Critical entities are listed in snapshots by default.

Critical entities are required for the GDK; do not delete them. SpatialOS needs them for launching a deployment. You save these into your initial snapshot when you generate it.

The critical entities are:

* `SpatialSpawner` - an entity with the `PlayerSpawner` component which contains the `spawn_player` command. [Client-worker instances]({{urlRoot}}/content/glossary#workers) connecting to a [deployment]({{urlRoot}}/content/glossary#deployment) use this entity to spawn their player.
* `GlobalStateManager` - an entity with the `GlobalStateManager` component which has a map of [singleton]({{urlRoot}}/content/singleton-actors.md) classes to entity IDs (see [Global State manager]({{urlRoot}}/content/glossary#global-state-manager) glossary entry). The GDK uses this entity to orchestrate the replication of [Singleton Actors]({{urlRoot}}/content/singleton-actors.md).

#### Placeholder entities
Placeholder entities are listed in snapshots by default. You can opt to exclude them when generating a snapshot via the [SpatialOS GDK toolbar]({{urlRoot}}/content/toolbars.md).

These entities exist only to set up server-worker boundaries in a way that is easy to test with multiple server-workers. These entities do not spawn as Actors when [checked out]({{urlRoot}}/content/glossary#check-out) by a worker and serve no purpose within the GDK. In most cases you can safely ignore them.