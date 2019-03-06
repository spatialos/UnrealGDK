// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/PackageMapClient.h"

#include "Schema/UnrealMetadata.h"
#include "Schema/UnrealObjectRef.h"

#include <WorkerSDK/improbable/c_worker.h>

#include "SpatialPackageMapClient.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialPackageMap, Log, All);

class USpatialClassInfoManager;

UCLASS()
class SPATIALGDK_API USpatialPackageMapClient : public UPackageMapClient
{
	GENERATED_BODY()		
public:
	FNetworkGUID ResolveEntityActor(AActor* Actor, Worker_EntityId EntityId);
	void RemoveEntityActor(Worker_EntityId EntityId);

	FNetworkGUID ResolveStablyNamedObject(UObject* Object);
	
	FUnrealObjectRef GetUnrealObjectRefFromNetGUID(const FNetworkGUID& NetGUID) const;
	FNetworkGUID GetNetGUIDFromUnrealObjectRef(const FUnrealObjectRef& ObjectRef) const;
	FNetworkGUID GetNetGUIDFromEntityId(const Worker_EntityId& EntityId) const;

	TWeakObjectPtr<UObject> GetObjectFromUnrealObjectRef(const FUnrealObjectRef& ObjectRef);
	FUnrealObjectRef GetUnrealObjectRefFromObject(UObject* Object);

	virtual bool SerializeObject(FArchive& Ar, UClass* InClass, UObject*& Obj, FNetworkGUID *OutNetGUID = NULL) override;

private:
	UPROPERTY()
	USpatialClassInfoManager* ClassInfoManager;
};

class SPATIALGDK_API FSpatialNetGUIDCache : public FNetGUIDCache
{
public:
	FSpatialNetGUIDCache(class USpatialNetDriver* InDriver);
		
	FNetworkGUID AssignNewEntityActorNetGUID(AActor* Actor);
	void RemoveEntityNetGUID(Worker_EntityId EntityId);
	void RemoveNetGUID(const FNetworkGUID& NetGUID);

	FNetworkGUID AssignNewStablyNamedObjectNetGUID(UObject* Object);
	
	FNetworkGUID GetNetGUIDFromUnrealObjectRef(const FUnrealObjectRef& ObjectRef);
	FUnrealObjectRef GetUnrealObjectRefFromNetGUID(const FNetworkGUID& NetGUID) const;
	FNetworkGUID GetNetGUIDFromEntityId(Worker_EntityId EntityId) const;

	void NetworkRemapObjectRefPaths(FUnrealObjectRef& ObjectRef, bool bReading) const;

private:
	FNetworkGUID GetNetGUIDFromUnrealObjectRefInternal(const FUnrealObjectRef& ObjectRef);

	FNetworkGUID GetOrAssignNetGUID_SpatialGDK(UObject* Object);
	void RegisterObjectRef(FNetworkGUID NetGUID, const FUnrealObjectRef& ObjectRef);
	
	FNetworkGUID RegisterNetGUIDFromPathForStaticObject(const FString& PathName, const FNetworkGUID& OuterGUID, bool bNoLoadOnClient);
	FNetworkGUID GenerateNewNetGUID(const int32 IsStatic);

	TMap<FNetworkGUID, FUnrealObjectRef> NetGUIDToUnrealObjectRef;
	TMap<FUnrealObjectRef, FNetworkGUID> UnrealObjectRefToNetGUID;
};

