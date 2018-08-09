// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "CAPIPipelineBlock.h"

#include "improbable/c_worker.h"
#include "improbable/c_schema.h"

#include "DTBManager.generated.h"

class USpatialActorChannel;

UCLASS()
class SPATIALGDK_API UDTBManager : public UObject
{
	GENERATED_BODY()

public:
	UDTBManager();

	void InitClient();
	void InitServer();

	void OnCommandRequest(Worker_CommandRequestOp& Op);
	void OnCommandResponse(Worker_CommandResponseOp& Op);

	void SendReserveEntityIdRequest(USpatialActorChannel* Channel);

	void AddPendingActorRequest(Worker_RequestId RequestId, USpatialActorChannel* Channel);
	USpatialActorChannel* RemovePendingActorRequest(Worker_RequestId RequestId);

	void OnReserveEntityIdResponse(Worker_ReserveEntityIdResponseOp& Op);

	Worker_RequestId SendCreateEntityRequest(USpatialActorChannel* Channel, const FVector& Location, const FString& PlayerWorkerId, const TArray<uint16>& RepChanged, const TArray<uint16>& HandoverChanged);

	Worker_RequestId CreateActorEntity(const FString& ClientWorkerId, const FVector& Position, const FString& Metadata, const struct FPropertyChangeState& InitialChanges, USpatialActorChannel* Channel);

	void Tick();


	TMap<Worker_RequestId, USpatialActorChannel*> PendingActorRequests;

	TFunction<void()> OnSpawnRequest;

	Worker_Connection* Connection;

	class USpatialInterop* Interop;

	CAPIPipelineBlock PipelineBlock;
};
