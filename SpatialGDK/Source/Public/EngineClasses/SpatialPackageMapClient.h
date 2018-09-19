// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/PackageMapClient.h"

#include "Schema/UnrealMetadata.h"
#include "Schema/UnrealObjectRef.h"

#include <improbable/c_worker.h>

#include "SpatialPackageMapClient.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialOSPackageMap, Log, All);

UCLASS()
class SPATIALGDK_API USpatialPackageMapClient : public UPackageMapClient
{
	GENERATED_BODY()		
public:
	FNetworkGUID ResolveEntityActor(AActor* Actor, Worker_EntityId EntityId, const SubobjectToOffsetMap& SubobjectToOffset);
	void RemoveEntityActor(Worker_EntityId EntityId);
	void RemoveEntitySubobjects(Worker_EntityId EntityId);

	FNetworkGUID ResolveStablyNamedObject(const UObject* Object);
	
	improbable::UnrealObjectRef GetUnrealObjectRefFromNetGUID(const FNetworkGUID& NetGUID) const;
	FNetworkGUID GetNetGUIDFromUnrealObjectRef(const improbable::UnrealObjectRef& ObjectRef) const;
	FNetworkGUID GetNetGUIDFromEntityId(const Worker_EntityId& EntityId) const;

	virtual bool SerializeObject(FArchive& Ar, UClass* InClass, UObject*& Obj, FNetworkGUID *OutNetGUID = NULL) override;
};

class SPATIALGDK_API FSpatialNetGUIDCache : public FNetGUIDCache
{
public:
	FSpatialNetGUIDCache(class USpatialNetDriver* InDriver);
		
	FNetworkGUID AssignNewEntityActorNetGUID(AActor* Actor, const SubobjectToOffsetMap& SubobjectToOffset);
	void RemoveEntityNetGUID(Worker_EntityId EntityId);
	void RemoveEntitySubobjectsNetGUIDs(Worker_EntityId EntityId);
	void RemoveNetGUID(const FNetworkGUID& NetGUID);

	FNetworkGUID AssignNewStablyNamedObjectNetGUID(const UObject* Object);
	
	FNetworkGUID GetNetGUIDFromUnrealObjectRef(const improbable::UnrealObjectRef& ObjectRef);
	improbable::UnrealObjectRef GetUnrealObjectRefFromNetGUID(const FNetworkGUID& NetGUID) const;
	FNetworkGUID GetNetGUIDFromEntityId(Worker_EntityId EntityId) const;

private:
	FNetworkGUID GetOrAssignNetGUID_SpatialGDK(const UObject* Object);
	void RegisterObjectRef(FNetworkGUID NetGUID, const improbable::UnrealObjectRef& ObjectRef);
	
	FNetworkGUID RegisterNetGUIDFromPath(const FString& PathName, const FNetworkGUID& OuterGUID);
	FNetworkGUID GenerateNewNetGUID(const int32 IsStatic);

	TMap<FNetworkGUID, improbable::UnrealObjectRef> NetGUIDToUnrealObjectRef;
	TMap<improbable::UnrealObjectRef, FNetworkGUID> UnrealObjectRefToNetGUID;
};

