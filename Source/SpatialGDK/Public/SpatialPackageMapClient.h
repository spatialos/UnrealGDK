// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/PackageMapClient.h"
#include "EntityId.h"
#include "SpatialInterop.h"
#include "SpatialUnrealObjectRef.h"

#include <improbable/unreal/gdk/level_data.h>

#include "SpatialPackageMapClient.generated.h"

class USpatialActorChannel;

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialOSPackageMap, Log, All);

using SubobjectToOffsetMap = ::worker::Map< std::string, std::uint32_t >;

/**
 * 
 */
UCLASS()
class SPATIALGDK_API USpatialPackageMapClient : public UPackageMapClient
{
	GENERATED_BODY()		
public:
	FNetworkGUID ResolveEntityActor(AActor* Actor, FEntityId EntityId, const SubobjectToOffsetMap& SubobjectToOffset);
	void RemoveEntityActor(const FEntityId& EntityId);

	FNetworkGUID ResolveStablyNamedObject(const UObject* Object);
	void RemoveStablyNamedObject(const UObject* Object);

	improbable::unreal::UnrealObjectRef GetUnrealObjectRefFromNetGUID(const FNetworkGUID& NetGUID) const;
	FNetworkGUID GetNetGUIDFromUnrealObjectRef(const improbable::unreal::UnrealObjectRef& ObjectRef) const;
	FNetworkGUID GetNetGUIDFromEntityId(const worker::EntityId& EntityId) const;
	FNetworkGUID GetNetGUIDFromStablyNamedObject(const UObject* Object) const;

	virtual bool SerializeObject(FArchive& Ar, UClass* InClass, UObject*& Obj, FNetworkGUID *OutNetGUID = NULL) override;

private:

};

class SPATIALGDK_API FSpatialNetGUIDCache : public FNetGUIDCache
{
public:
	FSpatialNetGUIDCache(class USpatialNetDriver* InDriver);
		
	FNetworkGUID AssignNewEntityActorNetGUID(AActor* Actor, const SubobjectToOffsetMap& SubobjectToOffset);
	void RemoveEntityNetGUID(worker::EntityId EntityId);
	void RemoveNetGUID(FNetworkGUID NetGUID);

	FNetworkGUID AssignNewStablyNamedObjectNetGUID(const UObject* Object);
	
	FNetworkGUID GetNetGUIDFromUnrealObjectRef(const improbable::unreal::UnrealObjectRef& ObjectRef);
	improbable::unreal::UnrealObjectRef GetUnrealObjectRefFromNetGUID(const FNetworkGUID& NetGUID) const;
	FNetworkGUID GetNetGUIDFromEntityId(worker::EntityId EntityId) const;

private:
	FNetworkGUID GetOrAssignNetGUID_SpatialGDK(const UObject* Object);
	void RegisterObjectRef(FNetworkGUID NetGUID, const improbable::unreal::UnrealObjectRef& ObjectRef);
	
	FNetworkGUID RegisterNetGUIDFromPath(const FString& PathName, const FNetworkGUID& OuterGUID);
	FNetworkGUID GenerateNewNetGUID(const int32 IsStatic);

	TMap<FNetworkGUID, FHashableUnrealObjectRef> NetGUIDToUnrealObjectRef;
	TMap<FHashableUnrealObjectRef, FNetworkGUID> UnrealObjectRefToNetGUID;
};

