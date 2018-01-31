// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include <improbable/worker.h>

#include "CoreMinimal.h"
#include "Net/RepLayout.h"
#include "SpatialTypeBinding.generated.h"

class USpatialUpdateInterop;
class USpatialPackageMapClient;
class USpatialActorChannel;

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

using FUntypedRequestId = decltype(worker::RequestId<void>::Id);
struct FRPCRequestResult
{
	UObject* UnresolvedObject;
	FUntypedRequestId RequestId;

	FRPCRequestResult(UObject* UnresolvedObject) : UnresolvedObject{UnresolvedObject}, RequestId{0} {}
	FRPCRequestResult(FUntypedRequestId RequestId) : UnresolvedObject{nullptr}, RequestId{RequestId} {}
};

// Storage for a command request.
class FCommandRequestContext
{
public:
	using FRequestFunction = TFunction<FRPCRequestResult()>;

	FCommandRequestContext(FRequestFunction SendCommandRequest) :
		SendCommandRequest{SendCommandRequest},
		NumFailures{0}
	{
	}

	FRequestFunction SendCommandRequest;
	uint32 NumFailures;
};

UCLASS()
class NUF_API USpatialTypeBinding : public UObject
{
	GENERATED_BODY()

public:
	virtual void Init(USpatialUpdateInterop* UpdateInterop, USpatialPackageMapClient* PackageMap);
	virtual void BindToView() PURE_VIRTUAL(USpatialTypeBinding::BindToView, );
	virtual void UnbindFromView() PURE_VIRTUAL(USpatialTypeBinding::UnbindFromView, );
	virtual worker::ComponentId GetReplicatedGroupComponentId(EReplicatedPropertyGroup Group) const PURE_VIRTUAL(USpatialTypeBinding::GetReplicatedGroupComponentId, return worker::ComponentId{}; );
	
	virtual worker::Entity CreateActorEntity(const FVector& Position, const FString& Metadata, const FPropertyChangeState& InitialChanges, USpatialActorChannel* Channel) const PURE_VIRTUAL(USpatialTypeBinding::CreateActorEntity, return worker::Entity{}; );
	virtual void SendComponentUpdates(const FPropertyChangeState& Changes, USpatialActorChannel* Channel, const worker::EntityId& EntityId) const PURE_VIRTUAL(USpatialTypeBinding::SendComponentUpdates, );
	virtual void SendRPCCommand(AActor* TargetActor, const UFunction* const Function, FFrame* const Frame, USpatialActorChannel* Channel) PURE_VIRTUAL(USpatialTypeBinding::SendRPCCommand, );
	
	virtual void ApplyQueuedStateToChannel(USpatialActorChannel* ActorChannel) PURE_VIRTUAL(USpatialTypeBinding::ApplyQueuedStateToActor, );
	void ResolvePendingRPCs(UObject* Object);

protected:
	template<class CommandType>
	void SendRPCResponse(const worker::CommandRequestOp<CommandType>& Op) const 
	{
		TSharedPtr<worker::Connection> PinnedConnection = UpdateInterop->GetSpatialOS()->GetConnection().Pin();
		PinnedConnection.Get()->SendCommandResponse<CommandType>(Op.RequestId, {});
	}

	void SendCommandRequest(FCommandRequestContext::FRequestFunction Function);

	UPROPERTY()
	USpatialUpdateInterop* UpdateInterop;

	UPROPERTY()
	USpatialPackageMapClient* PackageMap;

	// Pending RPCs.
	TMap<UObject*, TArray<FCommandRequestContext::FRequestFunction>> PendingRPCs;

	// Outgoing RPCs (for retry logic).
	TMap<FUntypedRequestId, FCommandRequestContext> OutgoingRPCs;
};
