// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Interop/EntityCommandHandler.h"
#include "Interop/EntityQueryHandler.h"
#include "Schema/PlayerSpawner.h"
#include "SpatialCommonTypes.h"

#include "GameFramework/OnlineReplStructs.h"
#include "Templates/UniquePtr.h"
#include "UObject/NoExportTypes.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

#include "SpatialPlayerSpawner.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialPlayerSpawner, Log, All);

class FTimerManager;
class USpatialNetDriver;

DECLARE_DELEGATE_OneParam(FOnPlayerSpawnFailed, const FString&);

UCLASS()
class SPATIALGDK_API USpatialPlayerSpawner : public UObject
{
	GENERATED_BODY()

public:
	void Init(USpatialNetDriver* NetDriver);

	void Advance(const TArray<Worker_Op>& Ops);
	void OnPlayerSpawnCommandReceived(const Worker_Op& Op, const Worker_CommandRequestOp& CommandRequestOp);
	void OnPlayerSpawnResponseReceived(const Worker_Op& Op, const Worker_CommandResponseOp& CommandResponseOp);
	void OnForwardedPlayerSpawnCommandReceived(const Worker_Op& Op, const Worker_CommandRequestOp& CommandRequestOp);
	void OnForwardedPlayerSpawnResponseReceived(const Worker_Op& Op, const Worker_CommandResponseOp& CommandResponseOp);

	// Client
	void SendPlayerSpawnRequest();
	void ReceivePlayerSpawnResponseOnClient(const Worker_CommandResponseOp& Op);

	FOnPlayerSpawnFailed OnPlayerSpawnFailed;

	// Authoritative server worker
	void ReceivePlayerSpawnRequestOnServer(const Worker_CommandRequestOp& Op);
	void ReceiveForwardPlayerSpawnResponse(const Worker_CommandResponseOp& Op);

	// Non-authoritative server worker
	void ReceiveForwardedPlayerSpawnRequest(const Worker_CommandRequestOp& Op);

private:
	struct ForwardSpawnRequestDeleter
	{
		void operator()(Schema_CommandRequest* Request) const noexcept
		{
			if (Request == nullptr)
			{
				return;
			}
			Schema_DestroyCommandRequest(Request);
		}
	};

	// Client
	SpatialGDK::SpawnPlayerRequest ObtainPlayerParams() const;

	// Authoritative server worker
	void FindPlayerStartAndProcessPlayerSpawn(Schema_Object* Request, const FSpatialEntityId& ClientWorkerId);
	void ForwardSpawnRequestToStrategizedServer(const Schema_Object* OriginalPlayerSpawnRequest, AActor* PlayerStart,
												const FSpatialEntityId& ClientWorkerId, const VirtualWorkerId SpawningVirtualWorker);
	void RetryForwardSpawnPlayerRequest(const FSpatialEntityId EntityId, const Worker_RequestId RequestId,
										const bool bShouldTryDifferentPlayerStart = false);

	// Any server
	void PassSpawnRequestToNetDriver(const Schema_Object* PlayerSpawnData, AActor* PlayerStart);

	UPROPERTY()
	USpatialNetDriver* NetDriver;

	TMap<Worker_RequestId_Key, TUniquePtr<Schema_CommandRequest, ForwardSpawnRequestDeleter>> OutgoingForwardPlayerSpawnRequests;

	SpatialGDK::EntityQueryHandler QueryHandler;
	SpatialGDK::EntityCommandRequestHandler RequestHandler;
	SpatialGDK::EntityCommandResponseHandler ResponseHandler;

	TSet<Worker_EntityId_Key> WorkersWithPlayersSpawned;
};
