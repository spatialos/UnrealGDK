<%(TOC)%>

# Database sync worker 

## 3. Syncing with the database

In `DeathmatchScoreComponent.h`, declare all the functions that you are calling from the `GDKShooterSpatialGameInstance` in the callbacks:

```
...
#include "ExternalSchemaCodegen/improbable/database_sync/DatabaseSyncService.h"
#include "GDKShooterSpatialGameInstance.h"
#include "DeathmatchScoreComponent.generated.h"
...
	UPROPERTY(BlueprintAssignable)
		FScoreChangeEvent ScoreEvent;

	void ItemUpdateEvent(const ::improbable::database_sync::DatabaseSyncService::ComponentUpdateOp& Op);

	void GetItemResponse(const ::improbable::database_sync::DatabaseSyncService::Commands::GetItem::ResponseOp& Op);
	void CreateItemResponse(const ::improbable::database_sync::DatabaseSyncService::Commands::Create::ResponseOp& Op);
	void IncrementResponse(const ::improbable::database_sync::DatabaseSyncService::Commands::Increment::ResponseOp& Op);
...
```

And the internal ones that will encapsulate the code to do the requests:

```
...
    UPROPERTY()
		TMap<FString, int32> PlayerScoreMap;

	void RequestGetItem(const FString &Path);
	void RequestCreateItem(const FString &Name, int64 Count, const FString &Path);
	void RequestIncrement(const FString &Path, int64 Count);
...
```
Because you will be sending Commands that can fail or timeout, and you might need to do retries, you need to store a list of “in-flight commands".

In this example, you will be sending 3 types of requests to the Database Sync Worker, so declare the maps to store them:

```
...
    void RequestIncrement(const FString &Path, int64 Count);

    TMap<Worker_RequestId, ::improbable::database_sync::DatabaseSyncService::Commands::GetItem::Request> GetItemRequests;
    TMap<Worker_RequestId, ::improbable::database_sync::DatabaseSyncService::Commands::Create::Request> CreateItemRequests;
    TMap<Worker_RequestId, ::improbable::database_sync::DatabaseSyncService::Commands::Increment::Request> IncrementRequests;
...
```

Each request has a unique Id (per worker) which you can use as the key for the maps.

Finally, because the Database Sync Worker stores the data in a hierarchical way, you will be using some helper functions to deal with the “paths”. Also, store a reference to the `GameInstance` that you will use to send the commands:

```
...
	TMap<Worker_RequestId, ::improbable::database_sync::DatabaseSyncService::Commands::Increment::Request> IncrementRequests;

	void UpdateScoreFromPath(const FString &Path, int64 NewCount);
	void RequestCreateItemFromPath(const FString &Path);

	UGDKShooterSpatialGameInstance* GameInstance = nullptr;

};
```
### 1. Requesting information from the database

With everything declared, it is time to start getting values from the database.

First, you need to send a `GetItem` command to the `DatabaseSyncService` specifying the path of the info you want to retrieve.

To do so, implement the `RequestGetItem` function in `DeathmatchScoreComponent.cpp`. Also, declare some constants with the strings used in the database structure. For this example, we have defined a hierarchy as such: `profiles.UnrealWorker.players.<PlayerName>.score.(AllTimeKills or AllTimeDeaths)`.

You can create this structure in the way that fits your game the best, having inventories for players and NPCs. For more information about this, check out the Database Sync Worker's reference documentation [on Github](https://github.com/spatialos/database-sync-worker).

```
...
#include "ExternalSchemaCodegen/improbable/database_sync/CommandErrors.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "SpatialNetDriver.h"

// Path format to store the score is in the format "profiles.UnrealWorker.players.<PlayerName>.score.(AllTimeKills or AllTimeDeaths)"
namespace DBPaths
{
	static const FString kPlayersRoot = TEXT("profiles.UnrealWorker.players.");
	static const FString kScoreFolder = TEXT("score");
	static const FString kAllTimeKills = TEXT("AllTimeKills");
	static const FString kAllTimeDeaths = TEXT("AllTimeDeaths");
}

...

void UDeathmatchScoreComponent::RequestGetItem(const FString &Path)
{
	FString workerId = Cast<USpatialNetDriver>(GetWorld()->GetNetDriver())->Connection->GetWorkerId();

	::improbable::database_sync::DatabaseSyncService::Commands::GetItem::Request Request(Path, workerId);

	Worker_RequestId requestId = GameInstance->GetExternalSchemaInterface()->SendCommandRequest(GameInstance->GetHierarchyServiceId(), Request);

	GetItemRequests.Add(requestId, Request);

}
```

This creates a new request of the specific command you will be sending and then send the command to SpatialOS that will notify the Database Sync Worker, which will read the database and return the value stored. You then have to handle the response defining the `GetItemResponse` you declared before.

If a command response returns correctly, you can remove the request from the map of pending requests and update your local information with the one received.

Otherwise, you will have to see what the error was and deal with it appropriately. In this case, the database may not contain the item you requested and return a “Invalid Request” error, in which case you may want to add it (in the example, this is what needs to be done for when a new player joins and we want to store his new count of “All Time Kills” and “Deaths”).

If the command times out, you may want to send it again (in this example, for simplicity, you will retry for timeouts and simply log errors otherwise).

```
...
void UDeathmatchScoreComponent::GetItemResponse(const ::improbable::database_sync::DatabaseSyncService::Commands::GetItem::ResponseOp& Op)
{
	if (Op.StatusCode == Worker_StatusCode::WORKER_STATUS_CODE_SUCCESS)
	{
		UpdateScoreFromPath(GetItemRequests[Op.RequestId].Data.GetPath(), Op.Data.Data.GetItem().GetCount());

		GetItemRequests.Remove(Op.RequestId);
	}
	else if (Op.StatusCode == Worker_StatusCode::WORKER_STATUS_CODE_APPLICATION_ERROR)
	{
		FString message = Op.Message;
		if (FCString::Atoi(*message) == (int32)::improbable::database_sync::CommandErrors::INVALID_REQUEST)
		{
			RequestCreateItemFromPath(GetItemRequests[Op.RequestId].Data.GetPath());
		}

		GetItemRequests.Remove(Op.RequestId);
	}
	else if(Op.StatusCode == Worker_StatusCode::WORKER_STATUS_CODE_TIMEOUT)
	{
		Worker_RequestId requestId = GameInstance->GetExternalSchemaInterface()->SendCommandRequest(GameInstance->GetHierarchyServiceId(), GetItemRequests[Op.RequestId]);

		GetItemRequests.Add(requestId, GetItemRequests[Op.RequestId]);

		GetItemRequests.Remove(Op.RequestId);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("GetItem Request failed with Error %d : %s"), Op.StatusCode, Op.Message);
	}
}
...
```

### 2. Writing to the database

Because the database starts empty, when a player joins and you try to get their scores, you will find that there is no such info stored. To create it, use the `Create` command in the `RequestCreateItem` function defined previously:

```
...
void UDeathmatchScoreComponent::RequestCreateItem(const FString &Name, int64 Count, const FString &Path)
{
	FString workerId = Cast<USpatialNetDriver>(GetWorld()->GetNetDriver())->Connection->GetWorkerId();

	::improbable::database_sync::DatabaseSyncItem Item (Name, Count, Path);

	::improbable::database_sync::DatabaseSyncService::Commands::Create::Request Request(Item, workerId);

	Worker_RequestId requestId = GameInstance->GetExternalSchemaInterface()->SendCommandRequest(GameInstance->GetHierarchyServiceId(), Request);

	CreateItemRequests.Add(requestId, Request);
}
...
```
This function is very similar to the previous one but creates a different type of request.

In this and the previous step you are using the helper functions to deal with paths, so it would be a good time to define them.

```
...
void UDeathmatchScoreComponent::UpdateScoreFromPath(const FString &Path, int64 NewCount)
{
	FString workingPath = *Path;
	if (workingPath.RemoveFromStart(DBPaths::kPlayersRoot))
	{
		FString PlayerName;
		if (workingPath.Split(".", &PlayerName, &workingPath))
		{
			if (workingPath.RemoveFromStart(DBPaths::kScoreFolder + "."))
			{
				if (workingPath.Compare(DBPaths::kAllTimeKills) == 0)
				{
					if (PlayerScoreMap.Contains(PlayerName))
					{
						PlayerScoreArray[PlayerScoreMap[PlayerName]].AllTimeKills = NewCount;
						return;
					}
					else
					{
						UE_LOG(LogTemp, Log, TEXT("Received Update from Player not currently in game, ignoring it."));
					}
				}
				else if (workingPath.Compare(DBPaths::kAllTimeDeaths) == 0)
				{
					if (PlayerScoreMap.Contains(PlayerName))
					{
						PlayerScoreArray[PlayerScoreMap[PlayerName]].AllTimeDeaths = NewCount;
						return;
					}
					else
					{
						UE_LOG(LogTemp, Log, TEXT("Received Update from Player not currently in game, ignoring it."));
					}
				}
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Received update with unexpected path : %s"), *Path);
}

void UDeathmatchScoreComponent::RequestCreateItemFromPath(const FString &Path)
{
	FString workingPath = *Path;
	if (workingPath.RemoveFromStart(DBPaths::kPlayersRoot))
	{
		FString PlayerName;
		if (workingPath.Split(".", &PlayerName, &workingPath))
		{
			if (workingPath.RemoveFromStart(DBPaths::kScoreFolder + "."))
			{
				RequestCreateItem(workingPath, 0, Path);
				return;
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Request to create item from unexpected path : %s"), *Path);
}
...
```

For simplicity, you assume the path is going to be correct and if something happens, you will simply log an error.

As with the `GetItem` request, you need to listen to the answer, being sure to remove it from the list of pending create requests, or retrying if there were errors. (In this tutorial, for simplicity, we always retry, but you should consider a retry limit and appropriate error handling for your project).

```
...
void UDeathmatchScoreComponent::CreateItemResponse(const ::improbable::database_sync::DatabaseSyncService::Commands::Create::ResponseOp& Op)
{
	if (Op.StatusCode == Worker_StatusCode::WORKER_STATUS_CODE_SUCCESS)
	{
		CreateItemRequests.Remove(Op.RequestId);
	}
	else if (Op.StatusCode == Worker_StatusCode::WORKER_STATUS_CODE_TIMEOUT)
	{
		Worker_RequestId requestId = GameInstance->GetExternalSchemaInterface()->SendCommandRequest(GameInstance->GetHierarchyServiceId(), CreateItemRequests[Op.RequestId]);

		CreateItemRequests.Add(requestId, CreateItemRequests[Op.RequestId]);

		CreateItemRequests.Remove(Op.RequestId);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("CreateItem Request failed with Error %d : %s"), Op.StatusCode, Op.Message);
	}
}
...
```

In this example, there is never a need to delete information, but there is a command for it. Take a look at the full Database Sync Worker API [here](https://github.com/spatialos/database-sync-worker/blob/master/README.md#interacting-with-the-database) to see what it offers.

### 3. Modifying information in the database

Once you have created the information for the players, you will need to update it accordingly. In this example, that is when a player kills another or dies. Because both of those operations always increase a value, you will only be using the `Increment` command.

```
...
void UDeathmatchScoreComponent::RequestIncrement(const FString &Path, int64 Count)
{

	FString workerId = Cast<USpatialNetDriver>(GetWorld()->GetNetDriver())->Connection->GetWorkerId();

	::improbable::database_sync::DatabaseSyncService::Commands::Increment::Request Request(Path, Count, workerId);

	Worker_RequestId requestId = GameInstance->GetExternalSchemaInterface()->SendCommandRequest(GameInstance->GetHierarchyServiceId(), Request);

	IncrementRequests.Add(requestId, Request);
}
...
```

As previously, you need to be sure that the Command has worked or retry it hasn't:

```
...
void UDeathmatchScoreComponent::IncrementResponse(const ::improbable::database_sync::DatabaseSyncService::Commands::Increment::ResponseOp& Op)
{
	if (Op.StatusCode == Worker_StatusCode::WORKER_STATUS_CODE_SUCCESS)
	{
		IncrementRequests.Remove(Op.RequestId);
	}
	else if (Op.StatusCode == Worker_StatusCode::WORKER_STATUS_CODE_TIMEOUT)
	{
		Worker_RequestId requestId = GameInstance->GetExternalSchemaInterface()->SendCommandRequest(GameInstance->GetHierarchyServiceId(), IncrementRequests[Op.RequestId]);

		IncrementRequests.Add(requestId, IncrementRequests[Op.RequestId]);

		IncrementRequests.Remove(Op.RequestId);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Increment Request failed with Error %d : %s"), Op.StatusCode, Op.Message);
	}
}
...
```

### 4. Receiving database changes

When the database is changed externally, i.e from a manual operation, or an external service allocating players items, your UnrealWorker will receive an event with the path to the items that have been changed. You will then need to update your local copies of it, as you would do if you had requested the information through a `GetItem` command:

```
...
void UDeathmatchScoreComponent::ItemUpdateEvent(const ::improbable::database_sync::DatabaseSyncService::ComponentUpdateOp& Op)
{
	for (int32 i = 0; i < Op.Update.GetPathsUpdatedList().Num(); i++)
	{
		for (int32 j = 0; j < Op.Update.GetPathsUpdatedList()[i].GetPaths().Num(); j++)
		{
			RequestGetItem(Op.Update.GetPathsUpdatedList()[i].GetPaths()[j]);
		}
	}
}
...
```

### 5. Updating the game logic

With all the internal functions created, the only step left is to use them in the game logic. This means requesting the information of a player when they join the game, requesting the creation of it if it doesn’t exist, and updating it every time a player kills another or dies.

```
void UDeathmatchScoreComponent::RecordNewPlayer(APlayerState* PlayerState)
{
	if (GameInstance == nullptr)
	{
		GameInstance = Cast<UGDKShooterSpatialGameInstance>(GetWorld()->GetGameInstance());
	}

	if (!PlayerScoreMap.Contains(PlayerState->GetPlayerName()))
	{
		FPlayerScore NewPlayerScore;
		NewPlayerScore.PlayerId = PlayerState->PlayerId;
		NewPlayerScore.PlayerName = PlayerState->GetPlayerName();
		NewPlayerScore.Kills = 0;
		NewPlayerScore.Deaths = 0;
		NewPlayerScore.AllTimeKills = 0;
		NewPlayerScore.AllTimeDeaths = 0;

		int32 Index = PlayerScoreArray.Add(NewPlayerScore);
		PlayerScoreMap.Emplace(NewPlayerScore.PlayerName, Index);

		// Only use the Database Sync Worker if the entity exists.
		if (GameInstance->GetHierarchyServiceId() != 0)
		{
			RequestGetItem(DBPaths::kPlayersRoot + PlayerState->GetPlayerName() + "." + DBPaths::kScoreFolder + "." + DBPaths::kAllTimeKills); // Get this value from persistent storage
			RequestGetItem(DBPaths::kPlayersRoot + PlayerState->GetPlayerName() + "." + DBPaths::kScoreFolder + "." + DBPaths::kAllTimeDeaths); // Get this value from persistent storage
		}
	}
}

void UDeathmatchScoreComponent::RecordKill(const FString Killer, const FString Victim)
{
	if (Killer != Victim && PlayerScoreMap.Contains(Killer))
	{
		++PlayerScoreArray[PlayerScoreMap[Killer]].Kills;

		++PlayerScoreArray[PlayerScoreMap[Killer]].AllTimeKills;
		// Only use the Database Sync Worker if the entity exists.
		if (GameInstance->GetHierarchyServiceId() != 0)
		{
			RequestIncrement(DBPaths::kPlayersRoot + Killer + "." + DBPaths::kScoreFolder + "." + DBPaths::kAllTimeKills, 1);	// Store this value in persistent storage
		}
	}
	if (PlayerScoreMap.Contains(Victim))
	{
		++PlayerScoreArray[PlayerScoreMap[Victim]].Deaths;

		++PlayerScoreArray[PlayerScoreMap[Victim]].AllTimeDeaths;
		// Only use the Database Sync Worker if the entity exists.
		if (GameInstance->GetHierarchyServiceId() != 0)
		{
			RequestIncrement(DBPaths::kPlayersRoot + Victim + "." + DBPaths::kScoreFolder + "." + DBPaths::kAllTimeDeaths, 1); // Store this value in persistent storage
		}
	}
}

```

With all these changes made, let's run the project and test our changes in game.

</br>
#### **> Next:** [4: Testing your changes]({{urlRoot}}/content/tutorials/dbsync/tutorial-dbsync-testing)
</br>


<br/>------<br/>
_2019-07-31 Page added with limited editorial review_
[//]: # (TODO: https://improbableio.atlassian.net/browse/DOC-1248)


