<%(Callout type="warn" message="This [pre-alpha](https://docs.improbable.io/reference/latest/shared/release-policy#maturity-stages) release of the SpatialOS GDK for Unreal is for evaluation and feedback purposes only, with limited documentation - see the guidance on [Recommended use]({{urlRoot}}/index#recommended-use)")%>

# How to generate a snapshot

If you're not familiar with snapshots in the context of SpatialOS, please look at the [full snapshot documentation](https://docs.improbable.io/reference/latest/shared/operate/snapshots) (SpatialOS documentation).

The SpatialOS GDK for Unreal snapshots contain three kinds of entities: critical entities, stably named replicated actors and placeholders.

### Stably named replicated actors

See Unreal's documentation of [Stably Named Objects](https://docs.unrealengine.com/en-US/Gameplay/Networking/Actors/Properties/ObjectReferences). We write place stably named replicated actors to the snapshot so we can ensure that only one of each entity is spawned when launching multiple `server-workers`.

### Critical entities

Critical entities are entities which are used for functionality critical to the GDK and are never deleted. They are saved into the initial snapshot and must always exist when launching a deployment.

Currently the critical entities are:

* `SpatialSpawner` - an entity with the `PlayerSpawner` component which has a command. Connecting client-workers use this entity to spawn their player.
* `GlobalStateManager` - an entity with the `GlobalStateManager` component which has a map of singleton classes to entity IDs. This entity is used for orchestrating the replication of [Singleton Actors]({{urlRoot}}/content/singleton-actors.md).

### Placeholders

These entities exists only to set up server-worker boundaries in a way that is easy to test in a scenario with two server-workers. They will not spawn as actors when checked out and serve no purpose within the GDK. For most intents and purposes, you can safely ignore them.

## Generating a snapshot

To generate a snapshot, use the **Snapshot** button on the SpatialOS GDK for Unreal toolbar in the Unreal Editor:

 ![Snapshot]({{assetRoot}}assets/screen-grabs/snapshot.png)

 This creates a snapshot called `default.snapshot` in `spatial\snapshots`.

You need to regenerate snapshots when adding/removing `SpatialType` classes or when adding/removing stably named replicated actors in your level.
