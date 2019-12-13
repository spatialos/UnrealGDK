

## \[Experimental\] Database sync worker

## 2. Communicating with the Database Sync Worker

To store and retrieve data from the database, you will need to communicate with the Database Sync Worker through [SpatialOS commands and events](https://docs.improbable.io/reference/latest/shared/design/object-interaction).

Specifically, you will need to deal with a SpatialOS entity that contains the `DatabaseSyncService` component.

To send SpatialOS Commands you will need to know the EntityID of the target Entity, so you will need to get and store it. The best place to do so is when your UnrealWorker checks out the Entity and receives its Components. You will receive callbacks when that happens and can store the ID at that point.

To do so, in `GDKShooterSpatialGameInstance`, when your game connects to SpatialOS, create a `ExternalSchemaInterface` to deal with SpatialOS messages that come from outside of the GDK.

Create a reference to the `ExternalSchemaInterface` and expose it, since you will be using it from other places. Also, declare the `Init` function that you will use later to register to listen to SpatialOS commands and events.

This means modifying `GDKShooterSpatialGameInstance.h` as such:

[block:code]
{
  "codes": [
  {
      "code": "#include "GDKShooter/ExternalSchemaCodegen/ExternalSchemaInterface.h"\n#include "GDKShooterSpatialGameInstance.generated.h"\n\n…\nGENERATED_BODY()\npublic:\n\nvoid Init();\n\nExternalSchemaInterface* GetExternalSchemaInterface()\n{\nreturn ExternalSchema;\n}\n\nprotected:\n\nExternalSchemaInterface* ExternalSchema;\n};",
      "language": "text"
    }
  ]
}
[/block]

Then, in `GDKShooterSpatialGameInstance.cpp`, add a callback for the `OnConnected` event, like this:

[block:code]
{
  "codes": [
  {
      "code": "#include "GDKShooterSpatialGameInstance.h"\n#include "Editor.h"\n#include "EngineClasses/SpatialNetDriver.h"\n#include "ExternalSchemaCodegen/improbable/database_sync/DatabaseSyncService.h"\n#include "GameFramework/GameStateBase.h"\n#include "Game/Components/DeathmatchScoreComponent.h"\n#include "Interop/Connection/SpatialWorkerConnection.h"\n\nvoid UGDKShooterSpatialGameInstance::Init()\n{\nOnConnected.AddLambda([this]() {\n// On the client the world may not be completely set up, if so we can use the PendingNetGame\nUSpatialNetDriver* NetDriver = Cast<USpatialNetDriver>(GetWorld()->GetNetDriver());\nif (NetDriver == nullptr)\n{\nNetDriver = Cast<USpatialNetDriver>(GetWorldContext()->PendingNetGame->GetNetDriver());\n}\nExternalSchema = new ExternalSchemaInterface(NetDriver->Connection, NetDriver->Dispatcher);\n});\n}",
      "language": "text"
    }
  ]
}
[/block]

### 1. OnAddComponent

You want to listen for when the UnrealWorker receives the `DatabaseSyncService` component, so you can store the EntityID and be able to send commands later.

Again, you will be using this elsewhere, so be sure to expose it by adding this in `GDKShooterSpatialGameInstance.h`:

[block:code]
{
  "codes": [
  {
      "code": "...\nWorker_EntityId GetHierarchyServiceId()\n{\nreturn HierarchyServiceId;\n}\n\nprotected:\n\nExternalSchemaInterface* ExternalSchema;\n\nWorker_EntityId HierarchyServiceId = 0;\n\n};",
      "language": "text"
    }
  ]
}
[/block]

Then, on `GDKShooterSpatialGameInstance.cpp`, add a callback for the `OnAddComponent` event for when the `DatabaseSyncService` is checked out by the UnrealWorker, like this:

[block:code]
{
  "codes": [
  {
      "code": "...\nExternalSchema = new ExternalSchemaInterface(NetDriver->Connection, NetDriver->Dispatcher);\n\n// Listen to the callback for HierarchyService component to be added to an entity to get the EntityId of the service to send commands to it.\nExternalSchema->OnAddComponent([this](const ::improbable::database_sync::DatabaseSyncService::AddComponentOp& Op)\n{\nHierarchyServiceId = Op.EntityId;\n});\n});\n}",
      "language": "text"
    }
  ]
}
[/block]

### 2. Database permissions

Due to the hierarchical structure of the data stored in the database through the Database Sync Worker, you need to register your UnrealWorker as the “owner” of `profiles.UnrealWorker` so you can store the information further down that path (for example, `profiles.UnrealWorker.players.<player_id>.score.TotalKills`).

To do so, you need to send the Command `AssociatePathWithClient` to the Database Sync Worker.

You will need the `WorkerId` of the Worker that is going to be accessing that data, which you can get from the Connection in `GDKShooterSpatialGameInstance.cpp` as such:

[block:code]
{
  "codes": [
  {
      "code": "...\nHierarchyServiceId = Op.EntityId;\n//It can do this by sending the associate_path_with_client command to DBSync in order to allow it.For example, associate_path_with_client(profiles.player1->workerId:Client - {0e61a845 - e978 - 4e5f - b314 - cc6bf1929171}).\nUSpatialNetDriver* NetDriver = Cast<USpatialNetDriver>(GetWorld()->GetNetDriver());\nif (NetDriver == nullptr)\n{\nNetDriver = Cast<USpatialNetDriver>(GetWorldContext()->PendingNetGame->GetNetDriver());\n}\nFString workerId = NetDriver->Connection->GetWorkerId();\n\n::improbable::database_sync::DatabaseSyncService::Commands::AssociatePathWithClient::Request Request(TEXT("profiles.UnrealWorker"), workerId);\n\nExternalSchema->SendCommandRequest(HierarchyServiceId, Request);\n});",
      "language": "text"
    }
  ]
}
[/block]

### 3. Command responses

You will be sending commands to the `DatabaseSyncService`, so you need to listen to the responses. In this example, you will be only interested in a small part of the API (`GetItem`, `CreateItem`, `Increment`), but in your own project you will need to add as many callbacks as commands responses you are interested in.

Because in this example everything that you will be writing/reading from the database is score from the deathmatch game, you will be adding the logic to deal with the responses in that component later on. For now, just add these callbacks in `GDKShooterSpatialGameInstance.cpp` that will call the functions on the component.

[block:code]
{
  "codes": [
  {
      "code": "...\nExternalSchema->SendCommandRequest(HierarchyServiceId, *Request);\n});\n\n// Listen to callbacks for using the DB, like searching for items, creating them and increasing the value\nExternalSchema->OnCommandResponse([this](const ::improbable::database_sync::DatabaseSyncService::Commands::GetItem::ResponseOp& Op)\n{\nUDeathmatchScoreComponent* Deathmatch = Cast<UDeathmatchScoreComponent>(GetWorld()->GetGameState()->GetComponentByClass(UDeathmatchScoreComponent::StaticClass()));\nDeathmatch->GetItemResponse(Op);\n});\n\nExternalSchema->OnCommandResponse([this](const ::improbable::database_sync::DatabaseSyncService::Commands::Create::ResponseOp& Op)\n{\nUDeathmatchScoreComponent* Deathmatch = Cast<UDeathmatchScoreComponent>(GetWorld()->GetGameState()->GetComponentByClass(UDeathmatchScoreComponent::StaticClass()));\nDeathmatch->CreateItemResponse(Op);\n});\n\nExternalSchema->OnCommandResponse([this] (const ::improbable::database_sync::DatabaseSyncService::Commands::Increment::ResponseOp& Op)\n{\nUDeathmatchScoreComponent* Deathmatch = Cast<UDeathmatchScoreComponent>(GetWorld()->GetGameState()->GetComponentByClass(UDeathmatchScoreComponent::StaticClass()));\nDeathmatch->IncrementResponse(Op);\n});\n});",
      "language": "text"
    }
  ]
}
[/block]

### 4. OnComponentUpdate

A key feature of the Database Sync Worker is that it allows you to externally modify the database and changed values will be replicated to your game, so you will also need to listen to updates from the `DatabaseSyncService` component by adding this to `GDKShooterSpatialGameInstance.cpp`:

[block:code]
{
  "codes": [
  {
      "code": "...\nDeathmatch->IncrementResponse(Op);\n});\n\n// Listen to updates of items stored in the DB that have been changed outside of the game\nExternalSchema->OnComponentUpdate([this](const ::improbable::database_sync::DatabaseSyncService::ComponentUpdateOp& Op)\n{\nUDeathmatchScoreComponent* Deathmatch = Cast<UDeathmatchScoreComponent>(GetWorld()->GetGameState()->GetComponentByClass(UDeathmatchScoreComponent::StaticClass()));\nDeathmatch->ItemUpdateEvent(Op);\n});\n\n});",
      "language": "text"
    }
  ]
}
[/block]

So far, you have brought the Database Sync Worker into your project, generated code to communicate with it from the Example Project using schema, and you have registered to listen to the worker responses. Now is time to actually use the worker to sync with the database.

</br>
#### **> Next:** [3: Syncing with the database]({{urlRoot}}/content/tutorials/dbsync/tutorial-dbsync-syncing)
</br>

<br/>------<br/>
_2019-07-31 Page added with limited editorial review_
[//]: # (TODO: https://improbableio.atlassian.net/browse/DOC-1248)
