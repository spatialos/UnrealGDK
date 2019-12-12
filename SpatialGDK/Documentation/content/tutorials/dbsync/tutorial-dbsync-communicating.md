

## \[Experimental\] Database sync worker

## 2. Communicating with the Database Sync Worker

To store and retrieve data from the database, you will need to communicate with the Database Sync Worker through [SpatialOS commands and events](https://docs.improbable.io/reference/latest/shared/design/object-interaction).

Specifically, you will need to deal with a SpatialOS entity that contains the `DatabaseSyncService` component.

To send SpatialOS Commands you will need to know the EntityID of the target Entity, so you will need to get and store it. The best place to do so is when your UnrealWorker checks out the Entity and receives its Components. You will receive callbacks when that happens and can store the ID at that point.

To do so, in `GDKShooterSpatialGameInstance`, when your game connects to SpatialOS, create a `ExternalSchemaInterface` to deal with SpatialOS messages that come from outside of the GDK.

Create a reference to the `ExternalSchemaInterface` and expose it, since you will be using it from other places. Also, declare the `Init` function that you will use later to register to listen to SpatialOS commands and events.

This means modifying `GDKShooterSpatialGameInstance.h` as such:

```
#include "GDKShooter/ExternalSchemaCodegen/ExternalSchemaInterface.h"
#include "GDKShooterSpatialGameInstance.generated.h"

…
GENERATED_BODY()

public:

	void Init();

	ExternalSchemaInterface* GetExternalSchemaInterface()
	{
		return ExternalSchema;
	}

protected:

	ExternalSchemaInterface* ExternalSchema;
};
```

Then, in `GDKShooterSpatialGameInstance.cpp`, add a callback for the `OnConnected` event, like this:

```
#include "GDKShooterSpatialGameInstance.h"
#include "Editor.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "ExternalSchemaCodegen/improbable/database_sync/DatabaseSyncService.h"
#include "GameFramework/GameStateBase.h"
#include "Game/Components/DeathmatchScoreComponent.h"
#include "Interop/Connection/SpatialWorkerConnection.h"

void UGDKShooterSpatialGameInstance::Init()
{
	OnConnected.AddLambda([this]() {
		// On the client the world may not be completely set up, if so we can use the PendingNetGame
		USpatialNetDriver* NetDriver = Cast<USpatialNetDriver>(GetWorld()->GetNetDriver());
		if (NetDriver == nullptr)
		{
			NetDriver = Cast<USpatialNetDriver>(GetWorldContext()->PendingNetGame->GetNetDriver());
		}

		ExternalSchema = new ExternalSchemaInterface(NetDriver->Connection, NetDriver->Dispatcher);
});
}
```

### 1. OnAddComponent

You want to listen for when the UnrealWorker receives the `DatabaseSyncService` component, so you can store the EntityID and be able to send commands later.

Again, you will be using this elsewhere, so be sure to expose it by adding this in `GDKShooterSpatialGameInstance.h`:

```
...
	Worker_EntityId GetHierarchyServiceId()
	{
		return HierarchyServiceId;
	}

protected:

	ExternalSchemaInterface* ExternalSchema;

	Worker_EntityId HierarchyServiceId = 0;

};
```

Then, on `GDKShooterSpatialGameInstance.cpp`, add a callback for the `OnAddComponent` event for when the `DatabaseSyncService` is checked out by the UnrealWorker, like this:

```
...
		ExternalSchema = new ExternalSchemaInterface(NetDriver->Connection, NetDriver->Dispatcher);

		// Listen to the callback for HierarchyService component to be added to an entity to get the EntityId of the service to send commands to it.
		ExternalSchema->OnAddComponent([this](const ::improbable::database_sync::DatabaseSyncService::AddComponentOp& Op)
		{
			HierarchyServiceId = Op.EntityId;
		});
});
}
```

### 2. Database permissions

Due to the hierarchical structure of the data stored in the database through the Database Sync Worker, you need to register your UnrealWorker as the “owner” of `profiles.UnrealWorker` so you can store the information further down that path (for example, `profiles.UnrealWorker.players.<player_id>.score.TotalKills`).

To do so, you need to send the Command `AssociatePathWithClient` to the Database Sync Worker.

You will need the `WorkerId` of the Worker that is going to be accessing that data, which you can get from the Connection in `GDKShooterSpatialGameInstance.cpp` as such:

```
...
			HierarchyServiceId = Op.EntityId;
			//It can do this by sending the associate_path_with_client command to DBSync in order to allow it.For example, associate_path_with_client(profiles.player1->workerId:Client - {0e61a845 - e978 - 4e5f - b314 - cc6bf1929171}).
			USpatialNetDriver* NetDriver = Cast<USpatialNetDriver>(GetWorld()->GetNetDriver());
			if (NetDriver == nullptr)
			{
				NetDriver = Cast<USpatialNetDriver>(GetWorldContext()->PendingNetGame->GetNetDriver());
			}
			FString workerId = NetDriver->Connection->GetWorkerId();

			::improbable::database_sync::DatabaseSyncService::Commands::AssociatePathWithClient::Request Request(TEXT("profiles.UnrealWorker"), workerId);

			ExternalSchema->SendCommandRequest(HierarchyServiceId, Request);
		});
```

### 3. Command responses

You will be sending commands to the `DatabaseSyncService`, so you need to listen to the responses. In this example, you will be only interested in a small part of the API (`GetItem`, `CreateItem`, `Increment`), but in your own project you will need to add as many callbacks as commands responses you are interested in.

Because in this example everything that you will be writing/reading from the database is score from the deathmatch game, you will be adding the logic to deal with the responses in that component later on. For now, just add these callbacks in `GDKShooterSpatialGameInstance.cpp` that will call the functions on the component.

```
...
ExternalSchema->SendCommandRequest(HierarchyServiceId, *Request);
		});

		// Listen to callbacks for using the DB, like searching for items, creating them and increasing the value
		ExternalSchema->OnCommandResponse([this](const ::improbable::database_sync::DatabaseSyncService::Commands::GetItem::ResponseOp& Op)
		{
			UDeathmatchScoreComponent* Deathmatch = Cast<UDeathmatchScoreComponent>(GetWorld()->GetGameState()->GetComponentByClass(UDeathmatchScoreComponent::StaticClass()));
			Deathmatch->GetItemResponse(Op);
		});

		ExternalSchema->OnCommandResponse([this](const ::improbable::database_sync::DatabaseSyncService::Commands::Create::ResponseOp& Op)
		{
			UDeathmatchScoreComponent* Deathmatch = Cast<UDeathmatchScoreComponent>(GetWorld()->GetGameState()->GetComponentByClass(UDeathmatchScoreComponent::StaticClass()));
			Deathmatch->CreateItemResponse(Op);
		});

		ExternalSchema->OnCommandResponse([this] (const ::improbable::database_sync::DatabaseSyncService::Commands::Increment::ResponseOp& Op)
		{
			UDeathmatchScoreComponent* Deathmatch = Cast<UDeathmatchScoreComponent>(GetWorld()->GetGameState()->GetComponentByClass(UDeathmatchScoreComponent::StaticClass()));
			Deathmatch->IncrementResponse(Op);
		});
	});
```

### 4. OnComponentUpdate

A key feature of the Database Sync Worker is that it allows you to externally modify the database and changed values will be replicated to your game, so you will also need to listen to updates from the `DatabaseSyncService` component by adding this to `GDKShooterSpatialGameInstance.cpp`:

```
...
			Deathmatch->IncrementResponse(Op);
		});

		// Listen to updates of items stored in the DB that have been changed outside of the game
		ExternalSchema->OnComponentUpdate([this](const ::improbable::database_sync::DatabaseSyncService::ComponentUpdateOp& Op)
		{
			UDeathmatchScoreComponent* Deathmatch = Cast<UDeathmatchScoreComponent>(GetWorld()->GetGameState()->GetComponentByClass(UDeathmatchScoreComponent::StaticClass()));
			Deathmatch->ItemUpdateEvent(Op);
		});

	});
```

So far, you have brought the Database Sync Worker into your project, generated code to communicate with it from the Example Project using schema, and you have registered to listen to the worker responses. Now is time to actually use the worker to sync with the database.

</br>
#### **> Next:** [3: Syncing with the database]({{urlRoot}}/content/tutorials/dbsync/tutorial-dbsync-syncing)
</br>

<br/>------<br/>
_2019-07-31 Page added with limited editorial review_
[//]: # (TODO: https://improbableio.atlassian.net/browse/DOC-1248)
