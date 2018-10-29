<%(Callout type="warn" message="This [pre-alpha](https://docs.improbable.io/reference/latest/shared/release-policy#maturity-stages) release of the SpatialOS GDK for Unreal is for evaluation and feedback purposes only, with limited documentation - see the guidance on [Recommended use]({{urlRoot}}/index#recommended-use)")%>

# Singleton Actors

Singleton Actors allow a single source of truth for both operations and data across a multi-server simulation. They are server-side authoritative [Actors](https://docs.unrealengine.com/en-us/Programming/UnrealArchitecture/Actors) that are restricted to one instantiation on SpatialOS.

There are two kinds of Singleton Actors:

* **Public Singleton Actors** - Singleton Actors which are replicated to server-workers and client-workers. `GameState` is a Public Singleton Actor.
* **Private Singletons** - Singleton Actors which are replicated to server-workers, but not accessible to client-workers. `GameMode` is a Private Singleton Actor.

You can define any class as a Singleton Actor. At the moment the GDK for Unreal only supports Public Singleton Actors.

Each server-worker should instantiate their own local version of each Singleton Actor. For `GameMode` and `GameState`, Unreal Engine does this automatically.

Due to Unreal server-workers spawning their own instances of each Singleton Actor, proper replication and authority management of Singleton Actors becomes a bit tricky. To solve this issue, we have introduced the concept of a Global State Manager (GSM) to enable proper replication of Singleton Actors. The GSM solves the problem of replicating Singleton Actors by only allowing the server-worker with [authority](https://docs.improbable.io/reference/latest/shared/glossary#read-and-write-access-authority) over the GSM to execute the initial replication of these Actors. All other server-workers will then link their local Singleton Actors to their respective SpatialOS entity.

## Setting up Singleton Actors

To set up Singleton Actors for your project, you need to:

1. Register Singleton Actors by tagging them with the `SpatialType=Singleton` class attribute.
1. Add the generated components to the UnrealWorker worker configuration file.

## How to tag classes with Singleton Actor identifiers

The code snippet below shows how to tag a class with the appropriate identifiers.

```
UCLASS(SpatialType=Singleton)
class TESTSUITE_API AExampleGameGameState : public AGameStateBase
{
  GENERATED_BODY()
  ...
}
```

### Streaming queries

To make sure all server-workers check out Singleton Actor entities, you need to configure the worker to have [streaming queries](https://docs.improbable.io/reference/latest/shared/worker-configuration/bridge-config#streaming-queries) for each Singleton Actor’s components.

In our example with `ExampleGameGameState`, the Schema Generator creates a schema component called `ExampleGameGameStateMultiClientRepData`. You need to add this as a streaming query to the worker configuration file (spatial/workers/unreal/spatialos.UnrealWorker.worker.json).

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

And that's it! You have successfully specified a Singleton Actor. Make sure you generate a new snapshot and schema using the [SpatialOS GDK for Unreal toolbar]({{urlRoot}}/content/toolbar).

