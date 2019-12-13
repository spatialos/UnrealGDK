

## \[Experimental\] Database sync worker

## 3. Syncing with the database

In `DeathmatchScoreComponent.h`, declare all the functions that you are calling from the `GDKShooterSpatialGameInstance` in the callbacks:

[block:code]
{
  "codes": [
  {
      "code": "...\n#include \"ExternalSchemaCodegen/improbable/database_sync/DatabaseSyncService.h\"\n#include \"GDKShooterSpatialGameInstance.h\"\n#include \"DeathmatchScoreComponent.generated.h\"\n...\nUPROPERTY(BlueprintAssignable)\nFScoreChangeEvent ScoreEvent;\n\nvoid ItemUpdateEvent(const ::improbable::database_sync::DatabaseSyncService::ComponentUpdateOp& Op);\n\nvoid GetItemResponse(const ::improbable::database_sync::DatabaseSyncService::Commands::GetItem::ResponseOp& Op);\nvoid CreateItemResponse(const ::improbable::database_sync::DatabaseSyncService::Commands::Create::ResponseOp& Op);\nvoid IncrementResponse(const ::improbable::database_sync::DatabaseSyncService::Commands::Increment::ResponseOp& Op);\n...",
      "language": "text"
    }
  ]
}
[/block]

And the internal ones that will encapsulate the code to do the requests:

[block:code]
{
  "codes": [
  {
      "code": "...\nUPROPERTY()\nTMap<FString, int32> PlayerScoreMap;\n\nvoid RequestGetItem(const FString &Path);\nvoid RequestCreateItem(const FString &Name, int64 Count, const FString &Path);\nvoid RequestIncrement(const FString &Path, int64 Count);\n...",
      "language": "text"
    }
  ]
}
[/block]
Because you will be sending Commands that can fail or timeout, and you might need to do retries, you need to store a list of “in-flight commands".

In this example, you will be sending 3 types of requests to the Database Sync Worker, so declare the maps to store them:

[block:code]
{
  "codes": [
  {
      "code": "...\nvoid RequestIncrement(const FString &Path, int64 Count);\n\nTMap<Worker_RequestId, ::improbable::database_sync::DatabaseSyncService::Commands::GetItem::Request> GetItemRequests;\nTMap<Worker_RequestId, ::improbable::database_sync::DatabaseSyncService::Commands::Create::Request> CreateItemRequests;\nTMap<Worker_RequestId, ::improbable::database_sync::DatabaseSyncService::Commands::Increment::Request> IncrementRequests;\n...",
      "language": "text"
    }
  ]
}
[/block]

Each request has a unique Id (per worker) which you can use as the key for the maps.

Finally, because the Database Sync Worker stores the data in a hierarchical way, you will be using some helper functions to deal with the “paths”. Also, store a reference to the `GameInstance` that you will use to send the commands:

[block:code]
{
  "codes": [
  {
      "code": "...\nTMap<Worker_RequestId, ::improbable::database_sync::DatabaseSyncService::Commands::Increment::Request> IncrementRequests;\n\nvoid UpdateScoreFromPath(const FString &Path, int64 NewCount);\nvoid RequestCreateItemFromPath(const FString &Path);\n\nUGDKShooterSpatialGameInstance* GameInstance = nullptr;\n\n};",
      "language": "text"
    }
  ]
}
[/block]
### 1. Requesting information from the database

With everything declared, it is time to start getting values from the database.

First, you need to send a `GetItem` command to the `DatabaseSyncService` specifying the path of the info you want to retrieve.

To do so, implement the `RequestGetItem` function in `DeathmatchScoreComponent.cpp`. Also, declare some constants with the strings used in the database structure. For this example, we have defined a hierarchy as such: `profiles.UnrealWorker.players.<PlayerName>.score.(AllTimeKills or AllTimeDeaths)`.

You can create this structure in the way that fits your game the best, having inventories for players and NPCs. For more information about this, check out the Database Sync Worker's reference documentation [on Github](https://github.com/spatialos/database-sync-worker).

[block:code]
{
  "codes": [
  {
      "code": "...\n#include \"ExternalSchemaCodegen/improbable/database_sync/CommandErrors.h\"\n#include \"Interop/Connection/SpatialWorkerConnection.h\"\n#include \"SpatialNetDriver.h\"\n\n// Path format to store the score is in the format \"profiles.UnrealWorker.players.<PlayerName>.score.(AllTimeKills or AllTimeDeaths)\"\nnamespace DBPaths\n{\nstatic const FString kPlayersRoot = TEXT(\"profiles.UnrealWorker.players.\");\nstatic const FString kScoreFolder = TEXT(\"score\");\nstatic const FString kAllTimeKills = TEXT(\"AllTimeKills\");\nstatic const FString kAllTimeDeaths = TEXT(\"AllTimeDeaths\");\n}\n\n...\n\nvoid UDeathmatchScoreComponent::RequestGetItem(const FString &Path)\n{\nFString workerId = Cast<USpatialNetDriver>(GetWorld()->GetNetDriver())->Connection->GetWorkerId();\n\n::improbable::database_sync::DatabaseSyncService::Commands::GetItem::Request Request(Path, workerId);\n\nWorker_RequestId requestId = GameInstance->GetExternalSchemaInterface()->SendCommandRequest(GameInstance->GetHierarchyServiceId(), Request);\n\nGetItemRequests.Add(requestId, Request);\n\n}",
      "language": "text"
    }
  ]
}
[/block]

This creates a new request of the specific command you will be sending and then send the command to SpatialOS that will notify the Database Sync Worker, which will read the database and return the value stored. You then have to handle the response defining the `GetItemResponse` you declared before.

If a command response returns correctly, you can remove the request from the map of pending requests and update your local information with the one received.

Otherwise, you will have to see what the error was and deal with it appropriately. In this case, the database may not contain the item you requested and return a “Invalid Request” error, in which case you may want to add it (in the example, this is what needs to be done for when a new player joins and we want to store his new count of “All Time Kills” and “Deaths”).

If the command times out, you may want to send it again (in this example, for simplicity, you will retry for timeouts and simply log errors otherwise).

[block:code]
{
  "codes": [
  {
      "code": "...\nvoid UDeathmatchScoreComponent::GetItemResponse(const ::improbable::database_sync::DatabaseSyncService::Commands::GetItem::ResponseOp& Op)\n{\nif (Op.StatusCode == Worker_StatusCode::WORKER_STATUS_CODE_SUCCESS)\n{\nUpdateScoreFromPath(GetItemRequests\[Op.RequestId\].Data.GetPath(), Op.Data.Data.GetItem().GetCount());\n\nGetItemRequests.Remove(Op.RequestId);\n}\nelse if (Op.StatusCode == Worker_StatusCode::WORKER_STATUS_CODE_APPLICATION_ERROR)\n{\nFString message = Op.Message;\nif (FCString::Atoi(*message) == (int32)::improbable::database_sync::CommandErrors::INVALID_REQUEST)\n{\nRequestCreateItemFromPath(GetItemRequests\[Op.RequestId\].Data.GetPath());\n}\n\nGetItemRequests.Remove(Op.RequestId);\n}\nelse if(Op.StatusCode == Worker_StatusCode::WORKER_STATUS_CODE_TIMEOUT)\n{\nWorker_RequestId requestId = GameInstance->GetExternalSchemaInterface()->SendCommandRequest(GameInstance->GetHierarchyServiceId(), GetItemRequests\[Op.RequestId\]);\n\nGetItemRequests.Add(requestId, GetItemRequests\[Op.RequestId\]);\n\nGetItemRequests.Remove(Op.RequestId);\n}\nelse\n{\nUE_LOG(LogTemp, Error, TEXT(\"GetItem Request failed with Error %d : %s\"), Op.StatusCode, Op.Message);\n}\n}\n...",
      "language": "text"
    }
  ]
}
[/block]

### 2. Writing to the database

Because the database starts empty, when a player joins and you try to get their scores, you will find that there is no such info stored. To create it, use the `Create` command in the `RequestCreateItem` function defined previously:

[block:code]
{
  "codes": [
  {
      "code": "...\nvoid UDeathmatchScoreComponent::RequestCreateItem(const FString &Name, int64 Count, const FString &Path)\n{\nFString workerId = Cast<USpatialNetDriver>(GetWorld()->GetNetDriver())->Connection->GetWorkerId();\n\n::improbable::database_sync::DatabaseSyncItem Item (Name, Count, Path);\n\n::improbable::database_sync::DatabaseSyncService::Commands::Create::Request Request(Item, workerId);\n\nWorker_RequestId requestId = GameInstance->GetExternalSchemaInterface()->SendCommandRequest(GameInstance->GetHierarchyServiceId(), Request);\n\nCreateItemRequests.Add(requestId, Request);\n}\n...",
      "language": "text"
    }
  ]
}
[/block]
This function is very similar to the previous one but creates a different type of request.

In this and the previous step you are using the helper functions to deal with paths, so it would be a good time to define them.

[block:code]
{
  "codes": [
  {
      "code": "...\nvoid UDeathmatchScoreComponent::UpdateScoreFromPath(const FString &Path, int64 NewCount)\n{\nFString workingPath = *Path;\nif (workingPath.RemoveFromStart(DBPaths::kPlayersRoot))\n{\nFString PlayerName;\nif (workingPath.Split(\".\", &PlayerName, &workingPath))\n{\nif (workingPath.RemoveFromStart(DBPaths::kScoreFolder + \".\"))\n{\nif (workingPath.Compare(DBPaths::kAllTimeKills) == 0)\n{\nif (PlayerScoreMap.Contains(PlayerName))\n{\nPlayerScoreArray\[PlayerScoreMap\[PlayerName\]\].AllTimeKills = NewCount;\nreturn;\n}\nelse\n{\nUE_LOG(LogTemp, Log, TEXT(\"Received Update from Player not currently in game, ignoring it.\"));\n}\n}\nelse if (workingPath.Compare(DBPaths::kAllTimeDeaths) == 0)\n{\nif (PlayerScoreMap.Contains(PlayerName))\n{\nPlayerScoreArray\[PlayerScoreMap\[PlayerName\]\].AllTimeDeaths = NewCount;\nreturn;\n}\nelse\n{\nUE_LOG(LogTemp, Log, TEXT(\"Received Update from Player not currently in game, ignoring it.\"));\n}\n}\n}\n}\n}\n\nUE_LOG(LogTemp, Log, TEXT(\"Received update with unexpected path : %s\"), *Path);\n}\n\nvoid UDeathmatchScoreComponent::RequestCreateItemFromPath(const FString &Path)\n{\nFString workingPath = *Path;\nif (workingPath.RemoveFromStart(DBPaths::kPlayersRoot))\n{\nFString PlayerName;\nif (workingPath.Split(\".\", &PlayerName, &workingPath))\n{\nif (workingPath.RemoveFromStart(DBPaths::kScoreFolder + \".\"))\n{\nRequestCreateItem(workingPath, 0, Path);\nreturn;\n}\n}\n}\n\nUE_LOG(LogTemp, Log, TEXT(\"Request to create item from unexpected path : %s\"), *Path);\n}\n...",
      "language": "text"
    }
  ]
}
[/block]

For simplicity, you assume the path is going to be correct and if something happens, you will simply log an error.

As with the `GetItem` request, you need to listen to the answer, being sure to remove it from the list of pending create requests, or retrying if there were errors. (In this tutorial, for simplicity, we always retry, but you should consider a retry limit and appropriate error handling for your project).

[block:code]
{
  "codes": [
  {
      "code": "...\nvoid UDeathmatchScoreComponent::CreateItemResponse(const ::improbable::database_sync::DatabaseSyncService::Commands::Create::ResponseOp& Op)\n{\nif (Op.StatusCode == Worker_StatusCode::WORKER_STATUS_CODE_SUCCESS)\n{\nCreateItemRequests.Remove(Op.RequestId);\n}\nelse if (Op.StatusCode == Worker_StatusCode::WORKER_STATUS_CODE_TIMEOUT)\n{\nWorker_RequestId requestId = GameInstance->GetExternalSchemaInterface()->SendCommandRequest(GameInstance->GetHierarchyServiceId(), CreateItemRequests\[Op.RequestId\]);\n\nCreateItemRequests.Add(requestId, CreateItemRequests\[Op.RequestId\]);\n\nCreateItemRequests.Remove(Op.RequestId);\n}\nelse\n{\nUE_LOG(LogTemp, Error, TEXT(\"CreateItem Request failed with Error %d : %s\"), Op.StatusCode, Op.Message);\n}\n}\n...",
      "language": "text"
    }
  ]
}
[/block]

In this example, there is never a need to delete information, but there is a command for it. Take a look at the full Database Sync Worker API [here](https://github.com/spatialos/database-sync-worker/blob/master/README.md#interacting-with-the-database) to see what it offers.

### 3. Modifying information in the database

Once you have created the information for the players, you will need to update it accordingly. In this example, that is when a player kills another or dies. Because both of those operations always increase a value, you will only be using the `Increment` command.

[block:code]
{
  "codes": [
  {
      "code": "...\nvoid UDeathmatchScoreComponent::RequestIncrement(const FString &Path, int64 Count)\n{\n\nFString workerId = Cast<USpatialNetDriver>(GetWorld()->GetNetDriver())->Connection->GetWorkerId();\n\n::improbable::database_sync::DatabaseSyncService::Commands::Increment::Request Request(Path, Count, workerId);\n\nWorker_RequestId requestId = GameInstance->GetExternalSchemaInterface()->SendCommandRequest(GameInstance->GetHierarchyServiceId(), Request);\n\nIncrementRequests.Add(requestId, Request);\n}\n...",
      "language": "text"
    }
  ]
}
[/block]

As previously, you need to be sure that the Command has worked or retry it hasn't:

[block:code]
{
  "codes": [
  {
      "code": "...\nvoid UDeathmatchScoreComponent::IncrementResponse(const ::improbable::database_sync::DatabaseSyncService::Commands::Increment::ResponseOp& Op)\n{\nif (Op.StatusCode == Worker_StatusCode::WORKER_STATUS_CODE_SUCCESS)\n{\nIncrementRequests.Remove(Op.RequestId);\n}\nelse if (Op.StatusCode == Worker_StatusCode::WORKER_STATUS_CODE_TIMEOUT)\n{\nWorker_RequestId requestId = GameInstance->GetExternalSchemaInterface()->SendCommandRequest(GameInstance->GetHierarchyServiceId(), IncrementRequests\[Op.RequestId\]);\n\nIncrementRequests.Add(requestId, IncrementRequests\[Op.RequestId\]);\n\nIncrementRequests.Remove(Op.RequestId);\n}\nelse\n{\nUE_LOG(LogTemp, Error, TEXT(\"Increment Request failed with Error %d : %s\"), Op.StatusCode, Op.Message);\n}\n}\n...",
      "language": "text"
    }
  ]
}
[/block]

### 4. Receiving database changes

When the database is changed externally, i.e from a manual operation, or an external service allocating players items, your UnrealWorker will receive an event with the path to the items that have been changed. You will then need to update your local copies of it, as you would do if you had requested the information through a `GetItem` command:

[block:code]
{
  "codes": [
  {
      "code": "...\nvoid UDeathmatchScoreComponent::ItemUpdateEvent(const ::improbable::database_sync::DatabaseSyncService::ComponentUpdateOp& Op)\n{\nfor (int32 i = 0; i < Op.Update.GetPathsUpdatedList().Num(); i++)\n{\nfor (int32 j = 0; j < Op.Update.GetPathsUpdatedList()\[i\].GetPaths().Num(); j++)\n{\nRequestGetItem(Op.Update.GetPathsUpdatedList()\[i\].GetPaths()\[j\]);\n}\n}\n}\n...",
      "language": "text"
    }
  ]
}
[/block]

### 5. Updating the game logic

With all the internal functions created, the only step left is to use them in the game logic. This means requesting the information of a player when they join the game, requesting the creation of it if it doesn’t exist, and updating it every time a player kills another or dies.

[block:code]
{
  "codes": [
  {
      "code": "void UDeathmatchScoreComponent::RecordNewPlayer(APlayerState* PlayerState)\n{\nif (GameInstance == nullptr)\n{\nGameInstance = Cast<UGDKShooterSpatialGameInstance>(GetWorld()->GetGameInstance());\n}\n\nif (!PlayerScoreMap.Contains(PlayerState->GetPlayerName()))\n{\nFPlayerScore NewPlayerScore;\nNewPlayerScore.PlayerId = PlayerState->PlayerId;\nNewPlayerScore.PlayerName = PlayerState->GetPlayerName();\nNewPlayerScore.Kills = 0;\nNewPlayerScore.Deaths = 0;\nNewPlayerScore.AllTimeKills = 0;\nNewPlayerScore.AllTimeDeaths = 0;\n\nint32 Index = PlayerScoreArray.Add(NewPlayerScore);\nPlayerScoreMap.Emplace(NewPlayerScore.PlayerName, Index);\n\n// Only use the Database Sync Worker if the entity exists.\nif (GameInstance->GetHierarchyServiceId() != 0)\n{\nRequestGetItem(DBPaths::kPlayersRoot + PlayerState->GetPlayerName() + \".\" + DBPaths::kScoreFolder + \".\" + DBPaths::kAllTimeKills); // Get this value from persistent storage\nRequestGetItem(DBPaths::kPlayersRoot + PlayerState->GetPlayerName() + \".\" + DBPaths::kScoreFolder + \".\" + DBPaths::kAllTimeDeaths); // Get this value from persistent storage\n}\n}\n}\n\nvoid UDeathmatchScoreComponent::RecordKill(const FString Killer, const FString Victim)\n{\nif (Killer != Victim && PlayerScoreMap.Contains(Killer))\n{\n++PlayerScoreArray\[PlayerScoreMap\[Killer\]\].Kills;\n\n++PlayerScoreArray\[PlayerScoreMap\[Killer\]\].AllTimeKills;\n// Only use the Database Sync Worker if the entity exists.\nif (GameInstance->GetHierarchyServiceId() != 0)\n{\nRequestIncrement(DBPaths::kPlayersRoot + Killer + \".\" + DBPaths::kScoreFolder + \".\" + DBPaths::kAllTimeKills, 1);\t// Store this value in persistent storage\n}\n}\nif (PlayerScoreMap.Contains(Victim))\n{\n++PlayerScoreArray\[PlayerScoreMap\[Victim\]\].Deaths;\n\n++PlayerScoreArray\[PlayerScoreMap\[Victim\]\].AllTimeDeaths;\n// Only use the Database Sync Worker if the entity exists.\nif (GameInstance->GetHierarchyServiceId() != 0)\n{\nRequestIncrement(DBPaths::kPlayersRoot + Victim + \".\" + DBPaths::kScoreFolder + \".\" + DBPaths::kAllTimeDeaths, 1); // Store this value in persistent storage\n}\n}\n}",
      "language": "text"
    }
  ]
}
[/block]

With all these changes made, let's run the project and test our changes in game.

</br>
#### **> Next:** [4: Testing your changes]({{urlRoot}}/content/tutorials/dbsync/tutorial-dbsync-testing)
</br>


<br/>------<br/>
_2019-07-31 Page added with limited editorial review_
[//]: # (TODO: https://improbableio.atlassian.net/browse/DOC-1248)


