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
	const uint8* RESTRICT SourceData;
	const TArray<uint16>& RepChanged; // changed replicated properties
	TArray<FRepLayoutCmd>& RepCmds;
	TArray<FHandleToCmdIndex>& RepBaseHandleToCmdIndex;
	const TArray<uint16>& MigChanged; // changed migratable properties
};

// A structure containing information about a replicated property.
class FRepHandleData
{
public:
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

	FORCEINLINE uint8* GetPropertyData(uint8* Container) const
	{
		return Container + Offset;
	}

	FORCEINLINE const uint8* GetPropertyData(const uint8* Container) const
	{
		return Container + Offset;
	}

	TArray<UProperty*> PropertyChain;
	UProperty* Property;
	ELifetimeCondition Condition;
	ELifetimeRepNotifyCondition RepNotifyCondition;
	int32 Offset;
};

// A structure containing information about a migratable property.
class FMigratableHandleData
{
public:
	FMigratableHandleData(UClass* Class, TArray<FName> PropertyNames)
	{
		// Build property chain.
		check(PropertyNames.Num() > 0);
		UStruct* CurrentContainerType = Class;
		for (FName PropertyName : PropertyNames)
		{
			checkf(CurrentContainerType, TEXT("A property in the chain (except the leaf) is not a struct property."));
			UProperty* Property = CurrentContainerType->FindPropertyByName(PropertyName);
			check(Property);
			PropertyChain.Add(Property);
			UStructProperty* StructProperty = Cast<UStructProperty>(Property);
			if (StructProperty)
			{
				CurrentContainerType = StructProperty->Struct;
			}
			else
			{
				UObjectProperty* ObjectProperty = Cast<UObjectProperty>(Property);
				if (ObjectProperty)
				{
					CurrentContainerType = ObjectProperty->PropertyClass;
				}
			}
		}
		Property = PropertyChain[PropertyChain.Num() - 1];
	}

	uint8* GetPropertyData(uint8* Container) const
	{
		uint8* Data = Container;
		for (int i = 0; i < PropertyChain.Num(); ++i)
		{
			Data += PropertyChain[i]->GetOffset_ForInternal();

			// If we're not the last property in the chain.
			if (i < (PropertyChain.Num() - 1))
			{
				// Migratable property chains can cross into subobjects, so we will need to deal with objects which are not inlined into the container.
				UObjectProperty* ObjectProperty = Cast<UObjectProperty>(PropertyChain[i]);
				if (ObjectProperty)
				{
					UObject* PropertyValue = ObjectProperty->GetObjectPropertyValue(Data);
					Data = (uint8*)PropertyValue;
				}
			}
		}
		return Data;
	}

	const uint8* GetPropertyData(const uint8* Container) const
	{
		const uint8* Data = Container;
		for (int i = 0; i < PropertyChain.Num(); ++i)
		{
			Data += PropertyChain[i]->GetOffset_ForInternal();

			// If we're not the last property in the chain.
			if (i < (PropertyChain.Num() - 1))
			{
				// Migratable property chains can cross into subobjects, so we will need to deal with objects which are not inlined into the container.
				UObjectProperty* ObjectProperty = Cast<UObjectProperty>(PropertyChain[i]);
				if (ObjectProperty)
				{
					UObject* PropertyValue = ObjectProperty->GetObjectPropertyValue(Data);
					Data = (uint8*)PropertyValue;
				}
			}
		}
		return Data;
	}

	TArray<UProperty*> PropertyChain;
	UProperty* Property;
};

// A map from rep handle to rep handle data.
using FRepHandlePropertyMap = TMap<uint16, FRepHandleData>;

// A map from migratable handle to migratable handle data.
using FMigratableHandlePropertyMap = TMap<uint16, FMigratableHandleData>;

UCLASS()
class NUF_API USpatialTypeBinding : public UObject
{
	GENERATED_BODY()

public:
	virtual const FRepHandlePropertyMap& GetRepHandlePropertyMap() const
		PURE_VIRTUAL(USpatialTypeBinding::GetRepHandlePropertyMap, static FRepHandlePropertyMap Map; return Map; );
	virtual const FMigratableHandlePropertyMap& GetMigratableHandlePropertyMap() const
		PURE_VIRTUAL(USpatialTypeBinding::GetMigratableHandlePropertyMap, static FMigratableHandlePropertyMap Map; return Map; );

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
