# How to use snapshots

You must generate a snapshot as the starting point for your [SpatialOS world]({{urlRoot}}/content/glossary#spatialos-world) when you create a new GDK project.

To generate a snapshot, on the [SpatialOS GDK toolbar]({{urlRoot}}/content/unreal-editor-interface/toolbars.md) in the Unreal Editor, select **Snapshot**.

![Snapshot]({{assetRoot}}assets/screen-grabs/snapshot.png)

_Image: The GDK toolbar_

This creates a snapshot called `default.snapshot` which you can find in `spatial\snapshots`.

If you want your snapshots to be exported to a different path you can specify the output path and file name of the snapshot using the [GDK toolbar settings]({{urlRoot}}/content/unreal-editor-interface/toolbars.md).

## Critical entities

Critical entities are listed in snapshots by default.

Critical entities are required for the GDK; do not delete them. SpatialOS needs them for launching a deployment. You save these into your initial snapshot when you generate it.

The critical entities are:

* `SpatialSpawner` - an entity with the `PlayerSpawner` component which contains the `spawn_player` command. [Client-worker instances]({{urlRoot}}/content/glossary#worker) connecting to a [deployment]({{urlRoot}}/content/glossary#deployment) use this entity to spawn their player.
* `GlobalStateManager` - an entity with the `GlobalStateManager` component which has a map of [singleton]({{urlRoot}}/content/singleton-actors.md) classes to entity IDs (see [Global State manager]({{urlRoot}}/content/glossary#global-state-manager) glossary entry). The GDK uses this entity to orchestrate the replication of [Singleton Actors]({{urlRoot}}/content/singleton-actors.md).

## Placeholder entities
Placeholder entities are listed in snapshots by default. You can opt to exclude them when generating a snapshot via the [SpatialOS GDK toolbar]({{urlRoot}}/content/unreal-editor-interface/toolbars.md).

These entities exist only to set up server-worker boundaries in a way that is easy to test with multiple server-workers. These entities do not spawn as Actors and serve no purpose within the GDK. In most cases you can safely ignore them.