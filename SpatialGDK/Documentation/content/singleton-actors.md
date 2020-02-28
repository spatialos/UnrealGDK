<%(TOC)%>
# Singleton Actors

You can use a Singleton Actor to define a single source of truth for operations or data, or both, across a game world that uses zoning. You can have only one instance of an [entity]({{urlRoot}}/content/glossary#entity) that represents a Singleton Actor per game world.

You define a Singleton Actor by tagging an Actor with the `SpatialType=Singleton` class attribute. For example, if you are implementing a scoreboard, you probably want only one scoreboard in your world, so you can tag the scoreboard Actor as a Singleton Actor.

There are two Singleton Actor types:

* **Public Singleton Actors** - Singleton Actors that are replicated to [server-worker and client-worker instances]({{urlRoot}}/content/glossary#worker). [`AGameState`](https://docs.unrealengine.com/en-US/Gameplay/Framework/GameMode) and [`ALevelScriptActor`](https://api.unrealengine.com/INT/API/Runtime/Engine/Engine/ALevelScriptActor/index.html) are Public Singleton Actors.
* **Private Singleton Actors** - Singleton Actors that are replicated to [server-worker instances]({{urlRoot}}/content/glossary#worker), but not accessible to [client-worker instances]({{urlRoot}}/content/glossary#worker). [`AGameMode`](https://docs.unrealengine.com/en-US/Gameplay/Framework/GameMode) is a Private Singleton Actor.

You can define any class as a Singleton Actor. However, the following Unreal Engine classes are automatically tagged in source code as Singleton Actors, so you don’t need to define them:

* `AGameModeBase`
* `AGameStateBase`
* `ALevelScriptActor`

> **Note**: Because the [`SpatialType`]({{urlRoot}}/content/spatial-type) class attribute is inheritable, all the classes that derive from `AGameModeBase`, `AGameStateBase`, or `ALevelScriptActor` are also Singleton Actors, which means that you don’t need to explicitly tag them in your code. You can opt out of this by tagging these classes with the [`NotSpatialType`]({{urlRoot}}/content/spatial-type#spatial-type) tag.

Before you begin to use Singleton Actors across your game world, review the following high-level procedure. See each section for more details:

1. [Define Singleton Actors](#defining-singleton-actors): Manually tag an Unreal C++ class or an Unreal Blueprint class with the relevant attributes to be a Public Singleton Actor or Private Singleton Actor.
1. [Spawn Singleton Actors](#spawning-singleton-actors): Understand how Singleton Actors are spawned on server-worker instances at runtime.

In addition, learn about how SpatialOS can [manage Singleton Actors](#managing-singleton-actors) to ensure that there is only ever one instance of an entity that represents a Singleton Actor, and to ensure data is replicated properly. 

## Defining Singleton Actors

Before you begin, ensure that you have an Actor, which can be either an [Unreal C++ Class](https://docs.unrealengine.com/en-us/Programming/Development/ManagingGameCode/CppClassWizard) or an [Unreal Blueprint Class](https://docs.unrealengine.com/en-US/Engine/Blueprints/UserGuide/Types/ClassBlueprint).

### Unreal C++ Class

1. Open your C++ Class in any text editor.
2. Tag the C++ Class with the relevant attribute(s), depending on what type of Singleton Actor you want it to be:
   * **Public Singleton Actor**: tag it with the `SpatialType=Singleton` class attribute. The following code snippet shows how to tag a C++ Class as a Public Singleton Actor:
     ```
     UCLASS(SpatialType=Singleton)
     class AScoreBoard : public AActor
     {
       GENERATED_BODY()
       ...
     }
     ```
   * **Private Singleton Actor**: tag it with the `SpatialType=(Singleton, ServerOnly)` class attributes. The following code snippet shows how to tag a C++ Class as a Private Singleton Actor:
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
2. From the [Toolbar](https://docs.unrealengine.com/en-US/Engine/Blueprints/Editor/UIComponents/Toolbar), click **Class Settings** to open Blueprint Properties in the Default pane.
3. In the **Class Options** section, ensure that the **Spatial Type** check box is selected.
4. In the **Spatial Description** text box, tag the Blueprint Class with the relevant attribute(s), depending on what type of Singleton Actor you want it to be:
   * **Public Singleton Actor**: enter `Singleton`.
   * **Private Singleton Actor**: enter `Singleton ServerOnly`.
    
    Here's a screenshot of what your Blueprint Class looks like if you tag it as a Private Singleton Actor:
    ![Singleton Blueprint]({{assetRoot}}assets/screen-grabs/blueprint-singleton.png)

## Spawning Singleton Actors

For all the following classes, which are automatically tagged as Singleton Actors, Singleton Actors are spawned automatically on all server-worker instances as part of the [Unreal Engine game flow](https://docs.unrealengine.com/en-US/Gameplay/Framework/GameFlow):

  - `AGameModeBase`
  - `AGameStateBase`
  - `ALevelScriptActor`
  - any classes that derive from the preceding classes

For all classes that you manually tag as Singleton Actors, make sure that Singleton Actors are spawned on all server-worker instances at runtime.

> **Note**: Never spawn Singleton Actors on client-worker instances.

## Managing Singleton Actors

Because each server-worker instance spawns each Singleton Actor locally, we have introduced the [Global State Manager]({{urlRoot}}/content/glossary#global-state-manager) (GSM) to ensure that there is only ever one instance of an [entity]({{urlRoot}}/content/glossary#entity) that represents a Singleton Actor, and to ensure data is replicated properly. 

The GSM allows only the server-worker instance with [authority]({{urlRoot}}/content/glossary#authority) over the GSM to spawn a representative entity for each Singleton Actor. Every other server-worker instance spawns each Singleton Actor locally and then links it to the one on the server-worker instance that has authority over the GSM. Additionally, each Public Singleton Actor is replicated to every client-worker instance via the normal [Actor replication flow](https://docs.unrealengine.com/en-us/Gameplay/Networking/Actors/ReplicationFlow).



<br/>------<br/>
_2019-06-13 Page updated with editorial review_