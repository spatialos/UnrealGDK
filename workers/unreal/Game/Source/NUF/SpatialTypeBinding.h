// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include <improbable/worker.h>

#include "CoreMinimal.h"
#include "Net/RepLayout.h"
#include "SpatialTypeBinding.generated.h"

class USpatialUpdateInterop;
class UPackageMap;
class USpatialPackageMapClient;

using FRPCSender = TFunction<void(worker::Connection* const, struct FFrame* const, const worker::EntityId&, USpatialPackageMapClient*)>; 

enum EReplicatedPropertyGroup
{
	GROUP_SingleClient,
	GROUP_MultiClient
};

inline EReplicatedPropertyGroup GetGroupFromCondition(ELifetimeCondition Condition)
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

// Storage for a changelist created by the replication system.
struct FPropertyChangeState
{
	const TArray<uint16>& Changed;
	const uint8* RESTRICT SourceData;
	TArray<FRepLayoutCmd>& Cmds;
	TArray<FHandleToCmdIndex>& BaseHandleToCmdIndex;
};

UCLASS()
class NUF_API USpatialTypeBinding : public UObject
{
	GENERATED_BODY()
public:
	virtual void Init(USpatialUpdateInterop* UpdateInterop, UPackageMap* PackageMap);
	virtual void BindToView() PURE_VIRTUAL(USpatialTypeBinding::BindToView, );
	virtual void UnbindFromView() PURE_VIRTUAL(USpatialTypeBinding::UnbindFromView, );
	virtual worker::ComponentId GetReplicatedGroupComponentId(EReplicatedPropertyGroup Group) const PURE_VIRTUAL(USpatialTypeBinding::GetReplicatedGroupComponentId, return worker::ComponentId{}; );
	virtual void SendComponentUpdates(const FPropertyChangeState& Changes, const worker::EntityId& EntityId) const PURE_VIRTUAL(USpatialTypeBinding::SendComponentUpdates, );
	virtual void SendRPCCommand(const UFunction* const Function, FFrame* const RPCFrame, const worker::EntityId& Target) const PURE_VIRTUAL(USpatialTypeBinding::SendRPCCommand, );
	virtual worker::Entity CreateActorEntity(const FVector& Position, const FString& Metadata, const FPropertyChangeState& InitialChanges) const PURE_VIRTUAL(USpatialTypeBinding::CreateActorEntity, return worker::Entity{}; );

protected:
	template<class CommandType>
	void SendRPCResponse(const worker::CommandRequestOp<CommandType>& Op) const 
	{
		TSharedPtr<worker::Connection> PinnedConnection = UpdateInterop->GetSpatialOS()->GetConnection().Pin();
		PinnedConnection.Get()->SendCommandResponse<CommandType>(Op.RequestId, {});
	}

	UPROPERTY()
	USpatialUpdateInterop* UpdateInterop;

	UPROPERTY()
	UPackageMap* PackageMap;
};