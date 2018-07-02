// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include <improbable/view.h>
#include <improbable/worker.h>

#include "CoreMinimal.h"
#include "AddComponentOpWrapperBase.h"
#include "EntityId.h"
#include "Net/RepLayout.h"
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

// TODO: Remove once we've upgraded to 14.0 and can disable component short circuiting. See TIG-137.
FORCEINLINE bool HasComponentAuthority(TWeakPtr<worker::View> View, const worker::EntityId EntityId, const worker::ComponentId ComponentId)
{
	TSharedPtr<worker::View> PinnedView = View.Pin();
	if (PinnedView.IsValid())
	{
		auto It = PinnedView->ComponentAuthority.find(EntityId);
		if (It != PinnedView->ComponentAuthority.end())
		{
			auto ComponentIt = (*It).second.find(ComponentId);
			if (ComponentIt != (*It).second.end())
			{
				return (*ComponentIt).second == worker::Authority::kAuthoritative;
			}
		}
	}
	return false;
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
	FRepHandleData(UClass* Class, TArray<FName> PropertyNames, TArray<int32> PropertyIndicies, ELifetimeCondition InCondition, ELifetimeRepNotifyCondition InRepNotifyCondition) :
		Condition(InCondition),
		RepNotifyCondition(InRepNotifyCondition),
		Offset(0)
	{
		// Build property chain.
		check(PropertyNames.Num() > 0);
		UStruct* CurrentContainerType = Class;
		for (FName PropertyName : PropertyNames)
		{
			checkf(CurrentContainerType, TEXT("A property in the chain (except the end) is not a container."));
			UProperty* CurProperty = CurrentContainerType->FindPropertyByName(PropertyName);
			PropertyChain.Add(CurProperty);

			UStructProperty* StructProperty = Cast<UStructProperty>(CurProperty);
			if (StructProperty)
			{
				CurrentContainerType = StructProperty->Struct;
			}
			else
			{
				CurrentContainerType = nullptr;
			}
		}
		Property = PropertyChain[PropertyChain.Num() - 1];

		// Calculate offset by summing the offsets of each property in the chain.
		for (int i = 0; i < PropertyChain.Num(); i++)
		{
			UProperty* CurProperty = PropertyChain[i];
			// Calculate the static array offset of this specific property, using it's index and it's parents indicies.
			int32 IndexOffset = PropertyIndicies[i] * CurProperty->ElementSize;
			Offset += CurProperty->GetOffset_ForInternal();
			Offset += IndexOffset;
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

private:
	int32 Offset;
};

// A structure containing information about a migratable property.
class FMigratableHandleData
{
public:
	FMigratableHandleData(UClass* Class, TArray<FName> PropertyNames) :
    SubobjectProperty(false),
    Offset(0)
	{
		// Build property chain.
		check(PropertyNames.Num() > 0);
		UStruct* CurrentContainerType = Class;
		for (FName PropertyName : PropertyNames)
		{
			checkf(CurrentContainerType, TEXT("A property in the chain (except the end) is not a container."));
			UProperty* CurProperty = CurrentContainerType->FindPropertyByName(PropertyName);
			check(CurProperty);
			PropertyChain.Add(CurProperty);
			if (!SubobjectProperty)
			{
				Offset += CurProperty->GetOffset_ForInternal();
			}
			UStructProperty* StructProperty = Cast<UStructProperty>(CurProperty);
			if (StructProperty)
			{
				CurrentContainerType = StructProperty->Struct;
			}
			else
			{
				UObjectProperty* ObjectProperty = Cast<UObjectProperty>(CurProperty);
				if (ObjectProperty)
				{
					CurrentContainerType = ObjectProperty->PropertyClass;
					SubobjectProperty = true; // We are now recursing into a subobjects properties.
					Offset = 0;
				}
				else
				{
					// We should only encounter a non-container style property if this is the final property in the chain.
					// Otherwise, the above check will be hit.
					CurrentContainerType = nullptr;
				}
			}
		}
		Property = PropertyChain[PropertyChain.Num() - 1];
	}

	FORCEINLINE uint8* GetPropertyData(uint8* Container) const
	{
		if (SubobjectProperty)
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
		else
		{
			return Container + Offset;
		}
	}

	FORCEINLINE const uint8* GetPropertyData(const uint8* Container) const
	{
		return GetPropertyData((uint8*)Container);
	}

	TArray<UProperty*> PropertyChain;
	UProperty* Property;

private:
	bool SubobjectProperty;  // If this is true, then this property refers to a property within a subobject.
	uint32 Offset;
};

// A map from rep handle to rep handle data.
using FRepHandlePropertyMap = TMap<uint16, FRepHandleData>;

// A map from migratable handle to migratable handle data.
using FMigratableHandlePropertyMap = TMap<uint16, FMigratableHandleData>;

UCLASS()
class SPATIALGDK_API USpatialTypeBinding : public UObject
{
	GENERATED_BODY()

public:
	virtual const FRepHandlePropertyMap& GetRepHandlePropertyMap() const
		PURE_VIRTUAL(USpatialTypeBinding::GetRepHandlePropertyMap, static FRepHandlePropertyMap Map; return Map; );
	virtual const FMigratableHandlePropertyMap& GetMigratableHandlePropertyMap() const
		PURE_VIRTUAL(USpatialTypeBinding::GetMigratableHandlePropertyMap, static FMigratableHandlePropertyMap Map; return Map; );

	virtual void Init(USpatialInterop* Interop, USpatialPackageMapClient* PackageMap);
	virtual void BindToView(bool bIsClient) PURE_VIRTUAL(USpatialTypeBinding::BindToView, );
	virtual void UnbindFromView() PURE_VIRTUAL(USpatialTypeBinding::UnbindFromView, );
	virtual UClass* GetBoundClass() const PURE_VIRTUAL(USpatialTypeBinding::GetBoundClass, return nullptr; );

	virtual worker::Entity CreateActorEntity(const FString& ClientWorkerId, const FVector& Position, const FString& Metadata, const FPropertyChangeState& InitialChanges, USpatialActorChannel* Channel) const PURE_VIRTUAL(USpatialTypeBinding::CreateActorEntity, return worker::Entity{}; );
	virtual void SendComponentUpdates(const FPropertyChangeState& Changes, USpatialActorChannel* Channel, const FEntityId& EntityId) const PURE_VIRTUAL(USpatialTypeBinding::SendComponentUpdates, );
	virtual void SendRPCCommand(UObject* TargetObject, const UFunction* const Function, void* Parameters) PURE_VIRTUAL(USpatialTypeBinding::SendRPCCommand, );

	virtual void ReceiveAddComponent(USpatialActorChannel* Channel, UAddComponentOpWrapperBase* AddComponentOp) const PURE_VIRTUAL(USpatialTypeBinding::ReceiveAddComponent, );
	virtual worker::Map<worker::ComponentId, worker::InterestOverride> GetInterestOverrideMap(bool bIsClient, bool bAutonomousProxy) const PURE_VIRTUAL(USpatialTypeBinding::GetInterestOverrideMap, return {}; );

protected:
	UPROPERTY()
	USpatialInterop* Interop;

	UPROPERTY()
	USpatialPackageMapClient* PackageMap;
};
