// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Engine/PackageMapClient.h"
#include "Schema/UnrealMetadata.h"
#include "Schema/UnrealObjectRef.h"
#include "Utils/EntityPool.h"

#include "CoreMinimal.h"

#include <WorkerSDK/improbable/c_worker.h>

#include "SpatialPackageMapClient.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialPackageMap, Log, All);

class USpatialNetDriver;
class UEntityPool;
class FTimerManager;

UCLASS()
class SPATIALGDK_API USpatialPackageMapClient : public UPackageMapClient
{
	GENERATED_BODY()
public:
	void Init(USpatialNetDriver* NetDriver, FTimerManager* TimerManager);

	FEntityId AllocateEntityIdAndResolveActor(AActor* Actor);
	FNetworkGUID TryResolveObjectAsEntity(UObject* Value);

	bool IsEntityIdPendingCreation(FEntityId EntityId) const;
	void RemovePendingCreationEntityId(FEntityId EntityId);

	bool ResolveEntityActor(AActor* Actor, FEntityId EntityId);
	void ResolveSubobject(UObject* Object, const FUnrealObjectRef& ObjectRef);

	void RemoveEntityActor(FEntityId EntityId);
	void RemoveSubobject(const FUnrealObjectRef& ObjectRef);

	// This function is ONLY used in SpatialReceiver::GetOrCreateActor to undo
	// the unintended registering of objects when looking them up with static paths.
	void UnregisterActorObjectRefOnly(const FUnrealObjectRef& ObjectRef);

	FNetworkGUID ResolveStablyNamedObject(UObject* Object);

	FUnrealObjectRef GetUnrealObjectRefFromNetGUID(const FNetworkGUID& NetGUID) const;
	FNetworkGUID GetNetGUIDFromUnrealObjectRef(const FUnrealObjectRef& ObjectRef) const;
	FNetworkGUID GetNetGUIDFromEntityId(const FEntityId& EntityId) const;

	TWeakObjectPtr<UObject> GetObjectFromUnrealObjectRef(const FUnrealObjectRef& ObjectRef);
	TWeakObjectPtr<UObject> GetObjectFromEntityId(const FEntityId& EntityId);
	FUnrealObjectRef GetUnrealObjectRefFromObject(const UObject* Object);
	FEntityId GetEntityIdFromObject(const UObject* Object);

	AActor* GetUniqueActorInstanceByClassRef(const FUnrealObjectRef& ClassRef);
	AActor* GetUniqueActorInstanceByClass(UClass* Class) const;

	FNetworkGUID* GetRemovedDynamicSubobjectNetGUID(const FUnrealObjectRef& ObjectRef);
	void AddRemovedDynamicSubobjectObjectRef(const FUnrealObjectRef& ObjectRef, const FNetworkGUID& NetGUID);
	void ClearRemovedDynamicSubobjectObjectRefs(const FEntityId& InEntityId);

	// Expose FNetGUIDCache::CanClientLoadObject so we can include this info with UnrealObjectRef.
	bool CanClientLoadObject(UObject* Object);

	FEntityId AllocateEntityId();
	bool IsEntityPoolReady() const;
	FEntityPoolReadyEvent& GetEntityPoolReadyDelegate();

	virtual bool SerializeObject(FArchive& Ar, UClass* InClass, UObject*& Obj, FNetworkGUID* OutNetGUID = NULL) override;

	const FClassInfo* TryResolveNewDynamicSubobjectAndGetClassInfo(UObject* Object);

	// Pending object references, being asynchronously loaded.
	TSet<FNetworkGUID> PendingReferences;

private:
	UPROPERTY()
	UEntityPool* EntityPool;

	bool bIsServer = false;

	// Entities that have been assigned on this server and not created yet
	TSet<FEntityId> PendingCreationEntityIds;
	TMap<FUnrealObjectRef, FNetworkGUID> RemovedDynamicSubobjectObjectRefs;
};

class SPATIALGDK_API FSpatialNetGUIDCache : public FNetGUIDCache
{
public:
	FSpatialNetGUIDCache(class USpatialNetDriver* InDriver);

	FNetworkGUID AssignNewEntityActorNetGUID(AActor* Actor, FEntityId EntityId);
	void AssignNewSubobjectNetGUID(UObject* Subobject, const FUnrealObjectRef& SubobjectRef);

	void RemoveEntityNetGUID(FEntityId EntityId);
	void RemoveSubobjectNetGUID(const FUnrealObjectRef& SubobjectRef);

	FNetworkGUID AssignNewStablyNamedObjectNetGUID(UObject* Object);

	FNetworkGUID GetNetGUIDFromUnrealObjectRef(const FUnrealObjectRef& ObjectRef);
	FUnrealObjectRef GetUnrealObjectRefFromNetGUID(const FNetworkGUID& NetGUID) const;
	FNetworkGUID GetNetGUIDFromEntityId(FEntityId EntityId) const;

	void NetworkRemapObjectRefPaths(FUnrealObjectRef& ObjectRef, bool bReading) const;

	// This function is ONLY used in SpatialPackageMapClient::UnregisterActorObjectRefOnly
	// to undo the unintended registering of objects when looking them up with static paths.
	void UnregisterActorObjectRefOnly(const FUnrealObjectRef& ObjectRef);

private:
	FNetworkGUID GetNetGUIDFromUnrealObjectRefInternal(const FUnrealObjectRef& ObjectRef);

	FNetworkGUID GetOrAssignNetGUID_SpatialGDK(UObject* Object);
	void RegisterObjectRef(FNetworkGUID NetGUID, const FUnrealObjectRef& ObjectRef);

	FNetworkGUID RegisterNetGUIDFromPathForStaticObject(const FString& PathName, const FNetworkGUID& OuterGUID, bool bNoLoadOnClient);
	FNetworkGUID GenerateNewNetGUID(const int32 IsStatic);

	TMap<FNetworkGUID, FUnrealObjectRef> NetGUIDToUnrealObjectRef;
	TMap<FUnrealObjectRef, FNetworkGUID> UnrealObjectRefToNetGUID;
};
