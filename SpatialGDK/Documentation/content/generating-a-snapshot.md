# Snapshots

<%(TOC)%>

A snapshot is a representation of the state of a [SpatialOS world]({{urlRoot}}/content/glossary#spatialos-world) at a given point in time. It stores each [persistent]({{urlRoot}}/content/glossary##persistence) [SpatialOS entity]({{urlRoot}}/content/glossary##spatialos-entity) and the values of their [SpatialOS components]({{urlRoot}}/content/glossary#spatialos-component)' [properties (SpatialOS documentation)](https://docs.improbable.io/reference/latest/shared/glossary#property).

You need a snapshot as the starting point (using an “initial snapshot”) for your [SpatialOS world]({{urlRoot}}/content/glossary##spatialos-world) when you [deploy your game]({{urlRoot}}/content/glossary##deployment) either locally or to the cloud. In the GDK, you need to generate a snapshot when you make certain changes to your project configuration; see [How to generate a snapshot](#how-to-generate-a-snapshot), below.

You can find out more about snapshots in the [SpatialOS snapshot documentation](https://docs.improbable.io/reference/latest/shared/operate/snapshot) but note that this documentation concentrates on working with snapshots using the [SpatialOS SDK]({{urlRoot}}/content/glossary#spatialos-sdk).


## What’s listed in snapshots

The GDK snapshots contain three kinds of [SpatialOS entities]({{urlRoot}}/content/glossary#spatialos-entity): 
Startup Actors, critical entities, and placeholder entities.

### Startup Actors

Startup Actors are [stably named replicated Actors (Unreal documentation)](https://docs.unrealengine.com/en-US/Gameplay/Networking/Actors/Properties/ObjectReferences) that you have placed in a Level. The startup Actors need to be listed in the snapshot so that SpatialOS spawns an entity only once for each Actor. If the startup Actors are not in the snapshot, you may get multiple entities for each Actor when you launch multiple [server-workers]({{urlRoot}}/content/glossary#workers). 

### Critical entities

Critical entities are listed in snapshots by default.

Critical entities are functionality critical for the GDK; do not delete them. SpatialOS needs them for launching a deployment. You save these into your initial snapshot when you generate it. 

The critical entities are:

* `SpatialSpawner` - an entity with the `PlayerSpawner` component which contains the `spawn_player` command. [Client-workers]({{urlRoot}}/content/glossary#workers) connecting to a [deployment]({{urlRoot}}/content/glossary#deployment) use this entity to spawn their player.
* `GlobalStateManager` - an entity with the `GlobalStateManager` component which has a map of [singleton]({{urlRoot}}/content/singleton-actors.md) classes to entity IDs (see [Global State manager]({{urlRoot}}/content/glossary#global-state-manager) glossary entry). The GDK uses this entity to orchestrate the replication of [Singleton Actors]({{urlRoot}}/content/singleton-actors.md).

### Placeholder entities
Placeholder entities are listed in snapshots by default.

These entities exists only to set up server-worker boundaries in a way that is easy to test with multiple server-workers. These entities do not spawn as Actors when [checked out]({{urlRoot}}/content/glossary#check-out) by a worker and serve no purpose within the GDK. For most intents and purposes, you can safely ignore them.

## How to generate a snapshot

To generate a snapshot, use the **Snapshot** button on the [SpatialOS GDK toolbar]({{urlRoot}}/content/toolbars.md) in the Unreal Editor.

 ![Snapshot]({{assetRoot}}assets/screen-grabs/snapshot.png)

This creates a snapshot called `default.snapshot` which you can find in `spatial\snapshots`.

If you want your snapshots to be exported to a different path you can specify the output path and file name of the snapshot using the [GDK toolbar settings]({{urlRoot}}/content/toolbars.md).

## When to generate a snapshot
You need to regenerate a snapshot when:

* Generating [schema]({{urlRoot}}/content/glossary#schema) for a new class.
* Modifying replicated properties or RPC signatures for any class whose schema was previously generated.
* Placing or removing replicated Actors in the Level.
* Modifying replicated values on placed replicated Actors.
* Adding or removing a [singleton class]({{urlRoot}}/content/singleton-actors).

