// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include <improbable/worker.h>

#include "Engine/ActorChannel.h"
#include "DataReplication.h"
#include "RepLayout.h"
#include "SpatialNetDriver.h"
#include "SpatialTypeBinding.h"
#include "improbable/worker.h"
#include "improbable/c_worker.h"
#include "SpatialActorChannel.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialGDKActorChannel, Log, All);

class USpatialNetDriver;

// Storage for a changelist created by the replication system when replicating from the server.
struct FPropertyChangeState
{
	const uint8* RESTRICT SourceData;
	const TArray<uint16> RepChanged; // changed replicated properties
	TArray<FRepLayoutCmd>& RepCmds;
	TArray<FHandleToCmdIndex>& RepBaseHandleToCmdIndex;
	TArray<FRepParentCmd>& Parents;
	const TArray<uint16> HandoverChanged; // changed handover properties
};

// A structure containing information about a replicated property.
class FRepHandleData
{
public:
	FRepHandleData(UClass* Class, TArray<FName> PropertyNames, TArray<int32> PropertyIndices, ELifetimeCondition InCondition, ELifetimeRepNotifyCondition InRepNotifyCondition) :
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
			if (UStructProperty* StructProperty = Cast<UStructProperty>(CurProperty))
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
			const UProperty* CurProperty = PropertyChain[i];
			// Calculate the static array offset of this specific property, using its index and its parents indices.
			int32 IndexOffset = PropertyIndices[i] * CurProperty->ElementSize;
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

// A structure containing information about a handover property.
class FHandoverHandleData
{
public:
	FHandoverHandleData(UClass* Class, TArray<FName> PropertyNames, TArray<int32> InPropertyIndices) :
		SubobjectProperty(false),
		Offset(0),
		PropertyIndices(InPropertyIndices)
	{
		// Build property chain.
		check(PropertyNames.Num() > 0);
		check(PropertyNames.Num() == PropertyIndices.Num());
		UStruct* CurrentContainerType = Class;
		for (int i = 0; i < PropertyNames.Num(); ++i)
		{
			checkf(CurrentContainerType, TEXT("A property in the chain (except the end) is not a container."));
			const FName& PropertyName = PropertyNames[i];
			UProperty* CurProperty = CurrentContainerType->FindPropertyByName(PropertyName);
			check(CurProperty);
			PropertyChain.Add(CurProperty);
			if (!SubobjectProperty)
			{
				Offset += CurProperty->GetOffset_ForInternal();
				Offset += PropertyIndices[i] * CurProperty->ElementSize;
			}

			if (UStructProperty* StructProperty = Cast<UStructProperty>(CurProperty))
			{
				CurrentContainerType = StructProperty->Struct;
			}
			else
			{
				if (UObjectProperty* ObjectProperty = Cast<UObjectProperty>(CurProperty))
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
				Data += PropertyIndices[i] * PropertyChain[i]->ElementSize;

				// If we're not the last property in the chain.
				if (i < (PropertyChain.Num() - 1))
				{
					// Handover property chains can cross into subobjects, so we will need to deal with objects which are not inlined into the container.
					if (UObjectProperty* ObjectProperty = Cast<UObjectProperty>(PropertyChain[i]))
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
	TArray<int32> PropertyIndices;

private:
	bool SubobjectProperty;  // If this is true, then this property refers to a property within a subobject.
	uint32 Offset;
};

// A map from rep handle to rep handle data.
using FRepHandlePropertyMap = TMap<uint16, FRepHandleData>;

// A map from handover handle to handover handle data.
using FHandoverHandlePropertyMap = TMap<uint16, FHandoverHandleData>;

// A replacement actor channel that plugs into the Engine's replication system and works with SpatialOS
UCLASS(Transient)
class SPATIALGDK_API USpatialActorChannel : public UActorChannel
{
	GENERATED_BODY()

public:
	USpatialActorChannel(const FObjectInitializer & ObjectInitializer = FObjectInitializer::Get());

	// SpatialOS Entity ID.
	FORCEINLINE Worker_EntityId GetEntityId() const
	{
		return ActorEntityId;
	}

	FORCEINLINE void SetEntityId(Worker_EntityId ActorEntityId)
	{
		this->ActorEntityId = ActorEntityId;
	}

	FORCEINLINE bool IsReadyForReplication() const
	{
		// Wait until we've reserved an entity ID.		
		return ActorEntityId != 0;
	}

	// Called on the client when receiving an update.
	FORCEINLINE bool IsClientAutonomousProxy(worker::ComponentId ClientRPCsComponentId)
	{
		if (SpatialNetDriver->GetNetMode() != NM_Client)
		{
			return false;
		}

		// This will never fail because we can't have an actor channel without having checked out the entity.
		//auto& EntityAuthority = View->ComponentAuthority[ActorEntityId];
		//auto ComponentIterator = EntityAuthority.find(ClientRPCsComponentId);
		//if (ComponentIterator != EntityAuthority.end())
		//{
		//	return (*ComponentIterator).second == worker::Authority::kAuthoritative;
		//}

		return false;
	}

	FORCEINLINE FPropertyChangeState GetChangeState(const TArray<uint16>& RepChanged, const TArray<uint16>& HandoverChanged) const
	{
		return {
			(uint8*)Actor,
			RepChanged,
			ActorReplicator->RepLayout->Cmds,
			ActorReplicator->RepLayout->BaseHandleToCmdIndex,
			ActorReplicator->RepLayout->Parents,
			HandoverChanged
		};
	}

	FORCEINLINE FPropertyChangeState GetChangeStateForObject(UObject* Obj, FObjectReplicator* Replicator, const TArray<uint16>& RepChanged, const TArray<uint16>& HandoverChanged)
	{
		if (!Replicator)
		{
			auto WeakObjectPtr = TWeakObjectPtr<UObject>(Obj);
			check(ObjectHasReplicator(WeakObjectPtr));
			Replicator = &FindOrCreateReplicator(WeakObjectPtr).Get();
		}

		return {
			(uint8*)Obj,
			RepChanged,
			Replicator->RepLayout->Cmds,
			Replicator->RepLayout->BaseHandleToCmdIndex,
			Replicator->RepLayout->Parents,
			HandoverChanged
		};
	}

	FORCEINLINE FRepLayout& GetObjectRepLayout(UObject* Object)
	{
		auto WeakObjectPtr = TWeakObjectPtr<UObject>(Object);
		check(ObjectHasReplicator(WeakObjectPtr));
		return *FindOrCreateReplicator(WeakObjectPtr)->RepLayout;
	}

	FORCEINLINE FRepStateStaticBuffer& GetObjectStaticBuffer(UObject* Object)
	{
		auto WeakObjectPtr = TWeakObjectPtr<UObject>(Object);
		check(ObjectHasReplicator(WeakObjectPtr));
		return FindOrCreateReplicator(WeakObjectPtr)->RepState->StaticBuffer;
	}

	// UChannel interface
	virtual void Init(UNetConnection * InConnection, int32 ChannelIndex, bool bOpenedLocally) override;
	virtual void Close() override;
	//Requires source changes to be virtual in base class.
	virtual bool ReplicateActor() override;
	virtual void SetChannelActor(AActor* InActor) override;

	void SendReserveEntityIdRequest();
	void RegisterEntityId(const Worker_EntityId& ActorEntityId);
	bool ReplicateSubobject(UObject *Obj, const FReplicationFlags &RepFlags);
	FPropertyChangeState CreateSubobjectChangeState(UActorComponent* Component);
	TArray<uint16> GetAllPropertyHandles(FObjectReplicator& Replicator);

	// For an object that is replicated by this channel (i.e. this channel's actor or its component), find out whether a given handle is an array.
	bool IsDynamicArrayHandle(UObject* Object, uint16 Handle);

	// Called by SpatialInterop when receiving an update.
	FObjectReplicator& PreReceiveSpatialUpdate(UObject* TargetObject);
	void PostReceiveSpatialUpdate(UObject* TargetObject, const TArray<UProperty*>& RepNotifies);

	// Distinguishes between channels created for actors that went through the "old" pipeline vs actors that are triggered through SpawnActor() calls.
	//In the future we may not use an actor channel for non-core actors.
	UPROPERTY(transient)
	bool bCoreActor;

	//void OnReserveEntityIdResponse(const worker::ReserveEntityIdResponseOp& Op);
	//void OnCreateEntityResponse(const worker::CreateEntityResponseOp& Op);

	void OnReserveEntityIdResponse(const struct Worker_ReserveEntityIdResponseOp& Op);
	void OnCreateEntityResponse(const struct Worker_CreateEntityResponseOp& Op);

protected:
	// UChannel interface
	virtual bool CleanUp(const bool bForDestroy) override;

private:

	void DeleteEntityIfAuthoritative();

	// A critical entity is any entity built into the snapshot which should not be deleted by any worker.
	bool IsCriticalEntity();

	Worker_EntityId ActorEntityId;

	UPROPERTY(transient)
	USpatialNetDriver* SpatialNetDriver;

	FVector LastSpatialPosition;
	TArray<uint8> HandoverPropertyShadowData;

	// If this actor channel is responsible for creating a new entity, this will be set to true during initial replication.
	UPROPERTY(Transient)
	bool bCreatingNewEntity;

private:
	void UpdateSpatialPosition();

	static FVector GetActorSpatialPosition(AActor* Actor);
};
