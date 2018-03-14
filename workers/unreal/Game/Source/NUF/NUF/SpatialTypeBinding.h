// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include <improbable/worker.h>

#include "CoreMinimal.h"
#include "Net/RepLayout.h"
#include "AddComponentOpWrapperBase.h"
#include "EntityId.h"
#include "SpatialTypeBinding.generated.h"

class USpatialInterop;
class USpatialPackageMapClient;
class USpatialActorChannel;

enum EReplicatedPropertyGroup
{
	GROUP_SingleClient,
	GROUP_MultiClient
};

FORCEINLINE EReplicatedPropertyGroup GetGroupFromCondition(ELifetimeCondition Condition)
{
	switch (Condition)
	{
	case COND_AutonomousOnly:
	case COND_OwnerOnly:
		return GROUP_SingleClient;
	default:
		return GROUP_MultiClient;
	}
}

// Storage for a changelist created by the replication system when replicating from the server.
struct FPropertyChangeState
{
	const TArray<uint16>& Changed;
	const uint8* RESTRICT SourceData;
	TArray<FRepLayoutCmd>& Cmds;
	TArray<FHandleToCmdIndex>& BaseHandleToCmdIndex;
};

// A structure containing information about a replicated property.
struct FRepHandleData
{
	UProperty* Parent;
	UProperty* Property;
	ELifetimeCondition Condition;
	ELifetimeRepNotifyCondition RepNotifyCondition;
	int32 Offset;
};

// A map from rep handle to rep handle data.
using FRepHandlePropertyMap = TMap<int32, FRepHandleData>;

UCLASS()
class NUF_API USpatialTypeBinding : public UObject
{
	GENERATED_BODY()

public:
	virtual void Init(USpatialInterop* Interop, USpatialPackageMapClient* PackageMap);
	virtual void BindToView() PURE_VIRTUAL(USpatialTypeBinding::BindToView, );
	virtual void UnbindFromView() PURE_VIRTUAL(USpatialTypeBinding::UnbindFromView, );
	virtual worker::ComponentId GetReplicatedGroupComponentId(EReplicatedPropertyGroup Group) const PURE_VIRTUAL(USpatialTypeBinding::GetReplicatedGroupComponentId, return worker::ComponentId{}; );
	virtual UClass* GetBoundClass() const PURE_VIRTUAL(USpatialTypeBinding::GetBoundClass, return nullptr; );

	virtual worker::Entity CreateActorEntity(const FString& ClientWorkerId, const FVector& Position, const FString& Metadata, const FPropertyChangeState& InitialChanges, USpatialActorChannel* Channel) const PURE_VIRTUAL(USpatialTypeBinding::CreateActorEntity, return worker::Entity{}; );
	virtual void SendComponentUpdates(const FPropertyChangeState& Changes, USpatialActorChannel* Channel, const FEntityId& EntityId) const PURE_VIRTUAL(USpatialTypeBinding::SendComponentUpdates, );
	virtual void SendRPCCommand(UObject* TargetObject, const UFunction* const Function, FFrame* const Frame) PURE_VIRTUAL(USpatialTypeBinding::SendRPCCommand, );
	
	virtual void ReceiveAddComponent(USpatialActorChannel* Channel, UAddComponentOpWrapperBase* AddComponentOp) const PURE_VIRTUAL(USpatialTypeBinding::ReceiveAddComponent, );
	virtual void ApplyQueuedStateToChannel(USpatialActorChannel* ActorChannel) PURE_VIRTUAL(USpatialTypeBinding::ApplyQueuedStateToActor, );

protected:
	UPROPERTY()
	USpatialInterop* Interop;

	UPROPERTY()
	USpatialPackageMapClient* PackageMap;
};
