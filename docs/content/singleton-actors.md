> This [pre-alpha](https://docs.improbable.io/reference/13.1/shared/release-policy#maturity-stages) release of the SpatialOS Unreal GDK is for evaluation and feedback purposes only, with limited documentation - see the guidance on [Recommended use](/README.md#recommended-use).

# Singleton Actors

Singleton Actors allow a single source of truth for both operations and data across a multi-server simulation. They are server-side authoritative [Actors](https://docs.unrealengine.com/en-us/Programming/UnrealArchitecture/Actors) that are restricted to one instantiation on SpatialOS. 

There are two kinds of Singleton Actors:

* **Public Singleton Actors** - Singleton Actors which are replicated to client-workers. `GameState` is a Public Singleton Actor.
* **Private Singletons** - Singleton Actors which are not accessible to client-workers. `GameMode` is a Private Singleton Actor.

You can define any class as a Singleton Actor. At the moment the Unreal GDK only supports Public Singleton Actors.

Each server-worker should instantiate their own local version of each Singleton Actor. For `GameMode` and `GameState`, Unreal Engine does this automatically.

Due to Unreal server-workers spawning their own instances of each Singleton Actor, proper replication and authority management of Singleton Actors becomes a bit tricky. To solve this issue, we have introduced the concept of a Global State Manager (GSM) to enable proper replication of Singleton Actors. The GSM solves the problem of replicating Singleton Actors by only allowing the server-worker with [authority](https://docs.improbable.io/reference/13.1/shared/glossary#read-and-write-access-authority) over the GSM to execute the initial replication of these Actors. All other server-workers will then link their local Singleton Actors to their respective SpatialOS entity.

## Setting up Singleton Actors

To properly set up Singleton Actors for your project, you need to:
1. Register Singleton Actors with the `DefaultEditorSpatialGDK.ini` file
1. Add the generated components to the UnrealWorker worker configuration file

### Registering Singleton Actors

You need to specify Singleton Actors in `DefaultEditorSpatialGDK.ini` before creating the snapshot and running SpatialOS.

Using the [Unreal GDK Starter Project](https://github.com/spatialos/UnrealGDKStarterProject) as an example, to do this:
1. Locate the `DefaultEditorSpatialGDK.ini` file in the Starter Project repository which you cloned during installation and setup - it's located at `Game/Config/DefaultEditorSpatialGDK.ini`.
2. Open the file in your editor and add your Singleton Actors as shown in the file snippet below.

### Example file snippet

In the snippet below, your game is `ExampleGame` and the Singleton Actor class you want to add is `ExampleGameGameState`

```
[SnapshotGenerator.SingletonActorClasses]
ExampleGameGameState=

[InteropCodeGen.ClassesToGenerate]
;ClassName=include_path.h
;Leave empty if no includes required.

;ExampleGameGameState
ExampleGameGameState=ExampleGameGameState.h
ExampleGameGameState=IncludePath/MyClassDependency.h
```

### Example file description

For an explanation of the `InteropCodeGen.ClassesToGenerate` section, see the [Interop Code Generator](./interop.md) documentation.

`SnapshotGenerator.SingletonActorClasses`

This section is for specifying the Singleton Actor classes the GSM will manage. In this case we specified one class, `ExampleGameGameState` which is the `GameState` class for `ExampleGame`.

For each Singleton Actor class, you also need to generate type bindings, so you have to add the class to the `InteropCodeGen.ClassesToGenerate` section.

### Streaming queries

To make sure all server-workers check out Singleton Actor entities, you need to configure the worker to have [streaming queries](https://docs.improbable.io/reference/13.1/shared/worker-configuration/bridge-config#streaming-queries) for each Singleton Actor’s components.

In our example with `ExampleGameGameState`, the Interop Code Generator creates a schema component called `ExampleGameGameStateMultiClientRepData`. You need to add this as a streaming query to the worker configuration file (spatial/workers/unreal/spatialos.UnrealWorker.worker.json).

In the `bridge` field of the worker configuration file, there should be a section that looks like this:

```
"streaming_query": [
      {
        "global_component_streaming_query": {
          "component_name": "improbable.unreal.GlobalStateManager"
        }
      },
      {
        "global_component_streaming_query": {
          "component_name": "improbable.unreal.generated.examplegamegamestate.ExampleGameGameStateMultiClientRepData"
        }
      }
    ],
```

This creates two streaming queries, one for the `GlobalStateManager` and one for the `ExampleGameGameState` component. For each Singleton Actor you register, you need to add another streaming query for that Singleton Actor’s `MultiClientRepData` component. We understand this workflow is a little clumsy and will be improved in the future.

And that's it! You have successfully specified a Singleton Actor. Make sure you generate a new snapshot and [type bindings](./interop.md) using the [SpatialOS Unreal GDK toolbar](./toolbar.md).

