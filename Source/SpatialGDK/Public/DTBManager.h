// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "CAPIPipelineBlock.h"

#include "improbable/c_worker.h"
#include "improbable/c_schema.h"

#include "DTBManager.generated.h"

class USpatialActorChannel;

enum EAlsoRPCType
{
	ARPC_Client = 0,
	ARPC_Server,
	ARPC_CrossServer,
	ARPC_NetMulticast,
	ARPC_Count
};

struct RPCInfo
{
	EAlsoRPCType Type;
	uint32 Index;
};

struct ClassInfo
{
	TMap<EAlsoRPCType, TArray<UFunction*>> RPCs;
	TMap<UFunction*, RPCInfo> RPCInfoMap;

	Worker_ComponentId SingleClientComponent;
	Worker_ComponentId MultiClientComponent;
	Worker_ComponentId HandoverComponent;
	Worker_ComponentId RPCComponents[ARPC_Count];
};

UCLASS()
class SPATIALGDK_API UDTBManager : public UObject
{
	GENERATED_BODY()

public:
	UDTBManager();

	ClassInfo* FindClassInfoByClass(UClass* Class);
	void CreateTypebindings();

	void InitClient();
	void InitServer();

	bool DTBHasComponentAuthority(Worker_EntityId EntityId, Worker_ComponentId ComponentId);

	void OnCommandRequest(Worker_CommandRequestOp& Op);
	void OnCommandResponse(Worker_CommandResponseOp& Op);

	void OnDynamicData(Worker_ComponentData& Data, USpatialActorChannel* Channel, USpatialPackageMapClient* PackageMap);

	void OnComponentUpdate(Worker_ComponentUpdateOp& Op);

	void OnAuthorityChange(Worker_AuthorityChangeOp& Op);

	void SendReserveEntityIdRequest(USpatialActorChannel* Channel);

	void AddPendingActorRequest(Worker_RequestId RequestId, USpatialActorChannel* Channel);
	USpatialActorChannel* RemovePendingActorRequest(Worker_RequestId RequestId);

	void OnReserveEntityIdResponse(Worker_ReserveEntityIdResponseOp& Op);

	Worker_RequestId SendCreateEntityRequest(USpatialActorChannel* Channel, const FVector& Location, const FString& PlayerWorkerId, const TArray<uint16>& RepChanged, const TArray<uint16>& HandoverChanged);

	Worker_RequestId CreateActorEntity(const FString& ClientWorkerId, const FVector& Position, const FString& Metadata, const struct FPropertyChangeState& InitialChanges, USpatialActorChannel* Channel);

	void SendSpatialPositionUpdate(Worker_EntityId EntityId, const FVector& Location);

	void SendComponentUpdates(const struct FPropertyChangeState& Changes, USpatialActorChannel* Channel);

	void SendRPC(UObject* TargetObject, UFunction* Function, void* Parameters);

	void SendRPCCommand(UObject* TargetObject, UFunction* Function, void* Parameters, Worker_ComponentId ComponentId, Schema_FieldId CommandId);

	void Tick();

	TMap<UClass*, ClassInfo> ClassInfoMap;
	TMap<Worker_ComponentId, UClass*> ComponentToClassMap;

	TMap<Worker_EntityId, TMap<Worker_ComponentId, Worker_Authority>> ComponentAuthorityMap;

	TMap<Worker_RequestId, USpatialActorChannel*> PendingActorRequests;

	TMap<AActor*, FString> ActorToWorkerId;

	TFunction<AActor*()> OnSpawnRequest;

	Worker_Connection* Connection;

	class USpatialInterop* Interop;
	class USpatialPackageMapClient* PackageMap;

	CAPIPipelineBlock PipelineBlock;
};
