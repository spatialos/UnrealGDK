<%(TOC)%>
# Singleton Actors

You can use a Singleton Actor to define a single source of truth for operations or data or both across a game world that uses zoning. You can have only one instance of a Singleton Actor per game world.

You create a Singleton Actor by tagging an Actor with the `SpatialType=Singleton` class attribute. For example, if you are implementing a scoreboard, you probably want only one scoreboard in your world, so you can tag the scoreboard Actor as a Singleton Actor.

There are two types of Singleton Actors:

* **Public Singleton Actors** - Singleton Actors that are replicated to [server-workers and client-workers]({{urlRoot}}/content/glossary#workers). [AGameState](https://docs.unrealengine.com/en-US/Gameplay/Framework/GameMode) is a Public Singleton Actor.
* **Private Singleton Actors** - Singleton Actors that are replicated to [server-workers]({{urlRoot}}/content/glossary#workers), but not accessible to [client-workers]({{urlRoot}}/content/glossary#workers). [AGameMode](https://docs.unrealengine.com/en-US/Gameplay/Framework/GameMode) is a Private Singleton Actor.

You can define any class as a Singleton Actor. The following Unreal engine classes are explicitly tagged in source code as Singleton Actors:

* AGameModeBase
* AGameStateBase

> **Note**: Because the `SpatialType` class attribute is inheritable, all the classes that derive from `AGameModeBase` or `AGameStateBase` class are also Singleton Actors, though not explicitly tagged in user code. You can opt out using the [`NotSpatialType`]({{urlRoot}}/content/spatial-type#spatial-type) tag.

## Rules for Singleton Actors

* Never create any Singleton Actors on client-workers explicitly in user code. The SpatialOS Runtime ensures that each client receives the Singleton Actor via [Actor replication flow](https://docs.unrealengine.com/en-us/Gameplay/Networking/Actors/ReplicationFlow).
* Always create all Singleton Actors on server-workers explicitly in user code. The SpatialOS GDK for Unreal allows only one server-worker instance to have authority over a Singleton Actor. For more information, see [Managing Singleton Actors]({{urlRoot}}/content/singleton-actors#managing-singleton-actors).
  * Note: Because `AGameMode` and `AGameState` Singletons Actors are created automatically on server-workers in the [Unreal Engine game flow](https://docs.unrealengine.com/en-US/Gameplay/Framework/GameFlow), you don't need to explicitly create these Singleton Actors.

## Creating Singleton Actors

Before you begin, ensure that you have an Actor ready, which can be either an [Unreal C++ Class](https://docs.unrealengine.com/en-us/Programming/Development/ManagingGameCode/CppClassWizard) or an [Unreal Blueprint Class](https://docs.unrealengine.com/en-US/Engine/Blueprints/UserGuide/Types/ClassBlueprint).

### Unreal C++ Class

1. Open your C++ Class in any text editor.
2. Tag the C++ Class to the type of Singleton Actor that you want:
    * **Public Singleton Actor**: tag it with the `SpatialType=Singleton` class attribute. The following code snippet shows how to tag a C++ Class as a Public Singleton Actor:
      ```
      UCLASS(SpatialType=Singleton)
      class AScoreBoard : public AActor
      {
        GENERATED_BODY()
        ...
      }
      ```
    * **Private Singleton Actor**: tag it with the `SpatialType=(Singleton, ServerOnly)` class attribute. The following code snippet shows how to tag a C++ Class as a Private Singleton Actor:
      ```
      UCLASS(SpatialType=(Singleton, ServerOnly))
      class AScoreBoard : public AActor
      {
        GENERATED_BODY()
        ...
      }
      ```  

### Unreal Blueprint Class

1. Open your Blueprint Class in the [Blueprint Editor](https://docs.unrealengine.com/en-us/Engine/Blueprints/Editor).
2. From the [Toolbar](https://docs.unrealengine.com/en-US/Engine/Blueprints/Editor/UIComponents/Toolbar), click **Class Settings** to open the Blueprint Properties in the **Default** pane.
3. In the **Class Options** section, ensure that the **Spatial Type** check box is selected.
4. In the **Spatial Description** text box, tag the Blueprint Class to the type of Singleton Actor that you want:
    * **Public Singleton Actor**: enter `Singleton`.
    * **Private Singleton Actor**: enter `Singleton ServerOnly`.
  
  Here's a screenshot of what your Blueprint Class looks like if you tag it as a Private Singleton Actor:

![Singleton Blueprint]({{assetRoot}}assets/screen-grabs/blueprint-singleton.png)

## Managing Singleton Actors

Because each server-worker creates its own instances for each Singleton Actor, the [Global State Manager]({{urlRoot}}/content/glossary#global-state-manager) (GSM) is introduced to ensure that single authority is maintained between Singleton Actors across servers, and that data is replicated correctly.

The GSM allows only the server-worker instance with [authority]({{urlRoot}}/content/glossary#authority) over the GSM to replicate the Singleton Actors initially. Then, all the other server-worker instances link their local Singleton Actors to their respective [SpatialOS entity]({{urlRoot}}/content/glossary#spatialos-entity).