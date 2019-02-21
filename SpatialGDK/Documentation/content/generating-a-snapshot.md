# Snapshots

A snapshot is a representation of the state of a [SpatialOS world]({{urlRoot}}/content/glossary#spatialos-world) at a given point in time. It stores each [persistent]({{urlRoot}}/content/glossary##persistence) [SpatialOS entity]({{urlRoot}}/content/glossary##spatialos-entity) and the values of their [SpatialOS components]({{urlRoot}}/content/glossary#spatialos-component)' [properties (SpatialOS documentation)](https://docs.improbable.io/reference/latest/shared/glossary#property).

You can find out more about snapshots in the [SpatialOS snapshot documentation](https://docs.improbable.io/reference/latest/shared/operate/snapshot) but note that this documentation concentrates on working with snapshots using the [SpatialOS SDK]({{urlRoot}}/content/glossary#spatialos-sdk).

## When to generate a snapshot

You must generate a snapshot as the starting point for your [SpatialOS world]({{urlRoot}}/content/glossary#spatialos-world) when you create a new Unreal GDK project.

To generate a snapshot, on the [SpatialOS GDK toolbar]({{urlRoot}}/content/toolbars.md) in the Unreal Editor, select **Snapshot** .

![Snapshot]({{assetRoot}}assets/screen-grabs/snapshot.png)

_Image: The GDK Toolbar._

This creates a snapshot called `default.snapshot` which you can find in `spatial\snapshots`.

If you want your snapshots to be exported to a different path you can specify the output path and file name of the snapshot using the [GDK toolbar settings]({{urlRoot}}/content/toolbars.md).

## What’s listed in snapshots

The GDK snapshots contain two kinds of [SpatialOS entities]({{urlRoot}}/content/glossary#spatialos-entity): 
Critical entities, and placeholder entities.

### Critical entities

Critical entities are listed in snapshots by default.

Critical entities are functionality critical for the GDK; do not delete them. SpatialOS needs them for launching a deployment. You save these into your initial snapshot when you generate it. 

The critical entities are:

* `SpatialSpawner` - an entity with the `PlayerSpawner` component which contains the `spawn_player` command. [Client-workers]({{urlRoot}}/content/glossary#workers) connecting to a [deployment]({{urlRoot}}/content/glossary#deployment) use this entity to spawn their player.
* `GlobalStateManager` - an entity with the `GlobalStateManager` component which has a map of [singleton]({{urlRoot}}/content/singleton-actors.md) classes to entity IDs (see [Global State manager]({{urlRoot}}/content/glossary#global-state-manager) glossary entry). The GDK uses this entity to orchestrate the replication of [Singleton Actors]({{urlRoot}}/content/singleton-actors.md).

### Placeholder entities
Placeholder entities are listed in snapshots by default. You can opt to exclude them when generating a snapshot via the [SpatialOS GDK toolbar]({{urlRoot}}/content/toolbars.md).

These entities exists only to set up server-worker boundaries in a way that is easy to test with multiple server-workers. These entities do not spawn as Actors when [checked out]({{urlRoot}}/content/glossary#check-out) by a worker and serve no purpose within the GDK. In most cases you can safely ignore them.





