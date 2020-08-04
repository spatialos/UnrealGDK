// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Engine/ActorChannel.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Interop/SpatialClassInfoManager.h"
#include "Interop/SpatialStaticComponentView.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Schema/StandardLibrary.h"
#include "Schema/RPCPayload.h"
#include "SpatialCommonTypes.h"
#include "SpatialGDKSettings.h"
#include "Utils/GDKPropertyMacros.h"
#include "Utils/RepDataUtils.h"
#include "Utils/SpatialStatics.h"

#include <WorkerSDK/improbable/c_worker.h>

#include "SpatialActorChannel.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialActorChannel, Log, All);

struct FObjectReferences
{
	FObjectReferences() = default;
	FObjectReferences(FObjectReferences&& Other)
		: MappedRefs(MoveTemp(Other.MappedRefs))
		, UnresolvedRefs(MoveTemp(Other.UnresolvedRefs))
		, bSingleProp(Other.bSingleProp)
		, bFastArrayProp(Other.bFastArrayProp)
		, Buffer(MoveTemp(Other.Buffer))
		, NumBufferBits(Other.NumBufferBits)
		, Array(MoveTemp(Other.Array))
		, ShadowOffset(Other.ShadowOffset)
		, ParentIndex(Other.ParentIndex)
		, Property(Other.Property) {}

	// Single property constructor
	FObjectReferences(const FUnrealObjectRef& InObjectRef, bool bUnresolved, int32 InCmdIndex, int32 InParentIndex, GDK_PROPERTY(Property)* InProperty)
		: bSingleProp(true), bFastArrayProp(false), ShadowOffset(InCmdIndex), ParentIndex(InParentIndex), Property(InProperty)
	{
		if (bUnresolved)
		{
			UnresolvedRefs.Add(InObjectRef);
		}
		else
		{
			MappedRefs.Add(InObjectRef);
		}
	}

	// Struct (memory stream) constructor
	FObjectReferences(const TArray<uint8>& InBuffer, int32 InNumBufferBits, TSet<FUnrealObjectRef>&& InDynamicRefs, TSet<FUnrealObjectRef>&& InUnresolvedRefs, int32 InCmdIndex, int32 InParentIndex, GDK_PROPERTY(Property)* InProperty, bool InFastArrayProp = false)
		: MappedRefs(MoveTemp(InDynamicRefs)), UnresolvedRefs(MoveTemp(InUnresolvedRefs)), bSingleProp(false), bFastArrayProp(InFastArrayProp), Buffer(InBuffer), NumBufferBits(InNumBufferBits), ShadowOffset(InCmdIndex), ParentIndex(InParentIndex), Property(InProperty) {}

	// Array constructor
	FObjectReferences(FObjectReferencesMap* InArray, int32 InCmdIndex, int32 InParentIndex, GDK_PROPERTY(Property)* InProperty)
		: bSingleProp(false), bFastArrayProp(false), Array(InArray), ShadowOffset(InCmdIndex), ParentIndex(InParentIndex), Property(InProperty) {}

	TSet<FUnrealObjectRef>				MappedRefs;
	TSet<FUnrealObjectRef>				UnresolvedRefs;

	bool								bSingleProp;
	bool								bFastArrayProp;
	TArray<uint8>						Buffer;
	int32								NumBufferBits;

	TUniquePtr<FObjectReferencesMap>	Array;
	int32								ShadowOffset;
	int32								ParentIndex;
	GDK_PROPERTY(Property)*				Property;
};

struct FPendingSubobjectAttachment
{
	const FClassInfo* Info;
	TWeakObjectPtr<UObject> Subobject;

	TSet<Worker_ComponentId> PendingAuthorityDelegations;
};

// Utility class to manage mapped and unresolved references.
// Reproduces what is happening with FRepState::GuidReferencesMap, but with FUnrealObjectRef instead of FNetworkGUID
class FSpatialObjectRepState
{
public:

	FSpatialObjectRepState(FChannelObjectPair InThisObj) : ThisObj(InThisObj) {}

	void UpdateRefToRepStateMap(FObjectToRepStateMap& ReplicatorMap);
	bool MoveMappedObjectToUnmapped(const FUnrealObjectRef& ObjRef);
	bool HasUnresolved() const { return UnresolvedRefs.Num() == 0; }

	const FChannelObjectPair& GetChannelObjectPair() const { return ThisObj; }

	FObjectReferencesMap ReferenceMap;
	TSet< FUnrealObjectRef > ReferencedObj;
	TSet< FUnrealObjectRef > UnresolvedRefs;

private:
	bool MoveMappedObjectToUnmapped_r(const FUnrealObjectRef& ObjRef, FObjectReferencesMap& ObjectReferencesMap);
	void GatherObjectRef(TSet<FUnrealObjectRef>& OutReferenced, TSet<FUnrealObjectRef>& OutUnresolved, const FObjectReferences& References) const;

	FChannelObjectPair ThisObj;
};


UCLASS(Transient)
class SPATIALGDK_API USpatialActorChannel : public UActorChannel
{
	GENERATED_BODY()

public:
	USpatialActorChannel(const FObjectInitializer & ObjectInitializer = FObjectInitializer::Get());

	// SpatialOS Entity ID.
	FORCEINLINE Worker_EntityId GetEntityId() const
	{
		return EntityId;
	}

	FORCEINLINE void SetEntityId(Worker_EntityId InEntityId)
	{
		EntityId = InEntityId;
	}

	FORCEINLINE bool IsReadyForReplication()
	{
		// Make sure we have authority
		if (Actor->Role != ROLE_Authority)
		{
			return false;
		}

		if (EntityId != SpatialConstants::INVALID_ENTITY_ID)
		{
			// If the entity already exists, make sure we have spatial authority before we replicate.
			if (!bCreatingNewEntity && !NetDriver->StaticComponentView->HasAuthority(EntityId, SpatialConstants::POSITION_COMPONENT_ID))
			{
				return false;
			}

			return true;
		}

		// This could happen if we've run out of entity ids at the time we called SetChannelActor.
		// If that is the case, keep trying to allocate an entity ID until we succeed.
		return TryResolveActor();
	}

	// Called on the client when receiving an update.
	FORCEINLINE bool IsClientAutonomousProxy()
	{
		if (NetDriver->GetNetMode() != NM_Client)
		{
			return false;
		}

		return NetDriver->StaticComponentView->HasAuthority(EntityId, SpatialConstants::GetClientAuthorityComponent(GetDefault<USpatialGDKSettings>()->UseRPCRingBuffer()));
	}

	inline void SetClientAuthority(const bool IsAuth)
	{
		bIsAuthClient = IsAuth;
	}


	// Indicates whether this client worker has "ownership" (authority over Client endpoint) over the entity corresponding to this channel.
	inline bool IsAuthoritativeClient() const
	{
		return bIsAuthClient;
	}

	// Sets the server and client authorities for this SpatialActorChannel based on the StaticComponentView
	inline void RefreshAuthority()
	{
		if (NetDriver->IsServer())
		{
			SetServerAuthority(NetDriver->StaticComponentView->HasAuthority(EntityId, SpatialConstants::POSITION_COMPONENT_ID));
		}
		else
		{
			SetClientAuthority(NetDriver->StaticComponentView->HasAuthority(EntityId, SpatialConstants::GetClientAuthorityComponent(GetDefault<USpatialGDKSettings>()->UseRPCRingBuffer())));
		}
	}

	void SetServerAuthority(const bool IsAuth)
	{
		if (IsAuth && !bIsAuthServer)
		{
			AuthorityReceivedTimestamp = FPlatformTime::Cycles64();
		}
		bIsAuthServer = IsAuth;
	}

	uint64 GetAuthorityReceivedTimestamp() const
	{
		return AuthorityReceivedTimestamp;
	}

	inline bool IsAuthoritativeServer() const
	{
		return bIsAuthServer;
	}

	FORCEINLINE FRepLayout& GetObjectRepLayout(UObject* Object)
	{
		check(ObjectHasReplicator(Object));
		return *FindOrCreateReplicator(Object)->RepLayout;
	}

	FORCEINLINE FRepStateStaticBuffer& GetObjectStaticBuffer(UObject* Object)
	{
		check(ObjectHasReplicator(Object));
		return FindOrCreateReplicator(Object)->RepState->GetReceivingRepState()->StaticBuffer;
	}

	// Begin UChannel interface
	virtual void Init(UNetConnection * InConnection, int32 ChannelIndex, EChannelCreateFlags CreateFlag) override;
	virtual int64 Close(EChannelCloseReason Reason) override;
	// End UChannel interface

	// Begin UActorChannel interface
	virtual int64 ReplicateActor() override;
	virtual void SetChannelActor(AActor* InActor, ESetChannelActorFlags Flags) override;
	virtual bool ReplicateSubobject(UObject* Obj, FOutBunch& Bunch, const FReplicationFlags& RepFlags) override;
	virtual bool ReadyForDormancy(bool suppressLogs = false) override;
	// End UActorChannel interface

	bool TryResolveActor();

	bool ReplicateSubobject(UObject* Obj, const FReplicationFlags& RepFlags);

	TMap<UObject*, const FClassInfo*> GetHandoverSubobjects();

	FRepChangeState CreateInitialRepChangeState(TWeakObjectPtr<UObject> Object);
	FHandoverChangeState CreateInitialHandoverChangeState(const FClassInfo& ClassInfo);

	// For an object that is replicated by this channel (i.e. this channel's actor or its component), find out whether a given handle is an array.
	bool IsDynamicArrayHandle(UObject* Object, uint16 Handle);

	FObjectReplicator* PreReceiveSpatialUpdate(UObject* TargetObject);
	void PostReceiveSpatialUpdate(UObject* TargetObject, const TArray<GDK_PROPERTY(Property)*>& RepNotifies);

	void OnCreateEntityResponse(const Worker_CreateEntityResponseOp& Op);

	void RemoveRepNotifiesWithUnresolvedObjs(TArray<GDK_PROPERTY(Property)*>& RepNotifies, const FRepLayout& RepLayout, const FObjectReferencesMap& RefMap, UObject* Object);

	void UpdateShadowData();
	void UpdateSpatialPositionWithFrequencyCheck();
	void UpdateSpatialPosition();

	void ServerProcessOwnershipChange();
	void ClientProcessOwnershipChange(bool bNewNetOwned);

	FORCEINLINE void MarkInterestDirty() { bInterestDirty = true; }
	FORCEINLINE bool GetInterestDirty() const { return bInterestDirty; }

	bool IsListening() const;

	// Call when a subobject is deleted to unmap its references and cleanup its cached informations.
	void OnSubobjectDeleted(const FUnrealObjectRef& ObjectRef, UObject* Object);

	static void ResetShadowData(FRepLayout& RepLayout, FRepStateStaticBuffer& StaticBuffer, UObject* TargetObject);

	void SetNeedOwnerInterestUpdate(bool bInNeedOwnerInterestUpdate)
	{
		bNeedOwnerInterestUpdate = bInNeedOwnerInterestUpdate;
	}

	bool NeedOwnerInterestUpdate() const
	{
		return bNeedOwnerInterestUpdate;
	}

protected:
	// Begin UChannel interface
	virtual bool CleanUp(const bool bForDestroy, EChannelCloseReason CloseReason) override;
	// End UChannel interface

private:
	void DynamicallyAttachSubobject(UObject* Object);

	void RetireEntityIfAuthoritative();

	void SendPositionUpdate(AActor* InActor, Worker_EntityId InEntityId, const FVector& NewPosition);

	void InitializeHandoverShadowData(TArray<uint8>& ShadowData, UObject* Object);
	FHandoverChangeState GetHandoverChangeList(TArray<uint8>& ShadowData, UObject* Object);

public:
	// If this actor channel is responsible for creating a new entity, this will be set to true once the entity creation request is issued.
	bool bCreatedEntity;

	// If this actor channel is responsible for creating a new entity, this will be set to true during initial replication.
	bool bCreatingNewEntity;

	TSet<TWeakObjectPtr<UObject>> PendingDynamicSubobjects;

	TMap<TWeakObjectPtr<UObject>, FSpatialObjectRepState> ObjectReferenceMap;

private:
	Worker_EntityId EntityId;
	bool bInterestDirty;

	bool bIsAuthServer;
	bool bIsAuthClient;

	// Used on the client to track gaining/losing ownership.
	bool bNetOwned;

	// Used on the server
	// Tracks the client worker ID corresponding to the owning connection.
	// If no owning client connection exists, this will be an empty string.
	FString SavedConnectionOwningWorkerId;

	// Used on the server
	// Tracks the interest bucket component ID for the relevant Actor.
	Worker_ComponentId SavedInterestBucketComponentID;

	UPROPERTY(transient)
	USpatialNetDriver* NetDriver;

	UPROPERTY(transient)
	class USpatialSender* Sender;

	UPROPERTY(transient)
	class USpatialReceiver* Receiver;

	FVector LastPositionSinceUpdate;
	double TimeWhenPositionLastUpdated;

	uint8 FramesTillDormancyAllowed = 0;

	// This is incremented in ReplicateActor. It represents how many bytes are sent per call to ReplicateActor.
	// ReplicationBytesWritten is reset back to 0 at the start of ReplicateActor.
	uint32 ReplicationBytesWritten = 0;

	// Shadow data for Handover properties.
	// For each object with handover properties, we store a blob of memory which contains
	// the state of those properties at the last time we sent them, and is used to detect
	// when those properties change.
	TArray<uint8>* ActorHandoverShadowData;
	TMap<TWeakObjectPtr<UObject>, TSharedRef<TArray<uint8>>> HandoverShadowDataMap;

	// Band-aid until we get Actor Sets.
	// Used on server-side workers only.
	// Record when this worker receives SpatialOS Position component authority over the Actor.
	// Tracking this helps prevent authority thrashing which can happen due a replication race
	// between hierarchy Actors. This happens because hierarchy Actor migration using the
	// default load-balancing strategy depends on the position of the hierarchy root Actor,
	// or its controlled pawn. If the hierarchy Actor data is replicated to the new worker
	// before the actor holding the position for all the hierarchy, it can immediately attempt to migrate back.
	// Using this timestamp, we can back off attempting migrations for a while.
	uint64 AuthorityReceivedTimestamp;

	// In case the actor's owner did not have an entity ID when trying to set interest to it
	// We set this flag in order to try to add interest as soon as possible.
	bool bNeedOwnerInterestUpdate;
};
