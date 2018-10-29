# Singleton Actors

Singleton Actors allow a single source of truth for both operations and data across a multi-server simulation. They are server-side authoritative [Unreal Actors](https://docs.unrealengine.com/en-us/Programming/UnrealArchitecture/Actors) that are restricted to one instantiation on SpatialOS.

There are two kinds of Singleton Actors:

 * **Public Singleton Actors** - Singleton Actors which are replicated to [server-workers and client-workers]({{urlRoot}/content/glossary#workers). `GameState` is a Public Singleton Actor.
* **Private Singleton Actors** - Singleton Actors which are replicated to [server-workers]({{urlRoot}/content/glossary#workers), but not accessible to [client-workers]({{urlRoot}/content/glossary#workers). `GameMode` is a Private Singleton Actor.

You can define any class as a Singleton Actor.

Each server-worker should instantiate their own local version of each Singleton Actor. For `GameMode` and `GameState`, Unreal Engine does this automatically. Client-workers receive Public Singletons Actors from the server-workers via the normal Actor replication lifecycle.

Due to Unreal server-workers spawning their own instances of each Singleton Actor, proper replication and authority management of Singleton Actors becomes a bit tricky. To solve this issue, we have introduced the concept of a Global State Manager (GSM) to enable proper replication of Singleton Actors. The GSM solves the problem of replicating Singleton Actors by only allowing the server-worker with [authority](https://docs.improbable.io/reference/latest/shared/glossary#read-and-write-access-authority) over the GSM to execute the initial replication of these Actors. All other server-workers will then link their local Singleton Actors to their respective SpatialOS entity. Because of this, you must update your snapshot whenever adding a new Singleton Actor to your project.

## Setting up Singleton Actors

To set up Singleton Actors for your project, you need to:

1. Register Singleton Actors by tagging them with the `SpatialType=Singleton` class attribute. If you wish to make them Private Singletons, tag them with the additional `ServerOnly` class attribute.

The code snippet below shows how to tag a native C++ class with the appropriate Public Singleton identifiers.

```
UCLASS(SpatialType=Singleton)
class TESTSUITE_API AExampleGameGameState : public AGameStateBase
{
  GENERATED_BODY()
  ...
}
```

The code snippet below shows how to tag a native C++ class with the appropriate Private Singleton identifiers.

```
UCLASS(SpatialType=(Singleton, ServerOnly))
class TESTSUITE_API AExampleGameGameMode : public AGameModeBase
{
  GENERATED_BODY()
  ...
}
```

To tag a blueprint class as a Public Singleton, open the class in the blueprint editor and navigate to the `Class Settings`. In the `Advanced` section inside `Class Options`, tick the `Spatial Type` checkbox and add `Singleton` to the `Spatial Description` textbox. To tag a blueprint class as a Private Singleton, follow the same steps, but add `ServerOnly` to the `Spatial Description` textbox.

This is an example of what your blueprint `Class Options` should look like if you've tagged it as a Private Singleton:
![Singleton blueprint]({{assetRoot}}assets/screen-grabs/blueprint_singleton.png)

And that's it! You have successfully specified a Singleton Actor. Make sure you generate schema and create a new snapshot using the [SpatialOS GDK for Unreal toolbar]({{urlRoot}}/content/toolbar).

