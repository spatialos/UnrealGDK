// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include <improbable/worker.h>

#include "CoreMinimal.h"
#include "Net/RepLayout.h"
#include "AddComponentOpWrapperBase.h"
#include "SpatialEntityId.h"
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
	FRepHandleData(UClass* Class, TArray<FName> PropertyNames, ELifetimeCondition Condition, ELifetimeRepNotifyCondition RepNotifyCondition) :
		Condition(Condition),
		RepNotifyCondition(RepNotifyCondition),
		Offset(0)
	{
		// Build property chain.
		check(PropertyNames.Num() > 0);
		UStruct* CurrentContainerType = Class;
		for (FName PropertyName : PropertyNames)
		{
			checkf(CurrentContainerType, TEXT("A property in the chain (except the leaf) is not a struct property."));
			UProperty* Property = CurrentContainerType->FindPropertyByName(PropertyName);
			PropertyChain.Add(Property);
			UStructProperty* StructProperty = Cast<UStructProperty>(Property);
			if (StructProperty)
			{
				CurrentContainerType = StructProperty->Struct;
			}
		}
		Property = PropertyChain[PropertyChain.Num() - 1];

		// Calculate offset by summing the offsets of each property in the chain.
		for (UProperty* Property : PropertyChain)
		{
			Offset += Property->GetOffset_ForInternal();
		}
	}

	TArray<UProperty*> PropertyChain;
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
