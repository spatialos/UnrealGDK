// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/PackageMapClient.h"
#include <improbable/c_worker.h>
#include "SchemaHelpers.h"

#include "SpatialPackageMapClient.generated.h"

class USpatialActorChannel;
struct UnrealObjectRef;

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialOSPackageMap, Log, All);

using SubobjectToOffsetMap = TMap<FString, std::uint32_t>;

/**
 * 
 */
UCLASS()
class SPATIALGDK_API USpatialPackageMapClient : public UPackageMapClient
{
	GENERATED_BODY()		
public:
	FNetworkGUID ResolveEntityActor(AActor* Actor, Worker_EntityId EntityId, const SubobjectToOffsetMap& SubobjectToOffset);
	void RemoveEntityActor(const Worker_EntityId& EntityId);

	void RemoveEntitySubobjects(const Worker_EntityId& EntityId, const SubobjectToOffsetMap& SubobjectToOffset);
	FNetworkGUID ResolveStablyNamedObject(const UObject* Object);
	
	UnrealObjectRef GetUnrealObjectRefFromNetGUID(const FNetworkGUID& NetGUID) const;
	FNetworkGUID GetNetGUIDFromUnrealObjectRef(const UnrealObjectRef& ObjectRef) const;
	FNetworkGUID GetNetGUIDFromEntityId(const Worker_EntityId& EntityId) const;
	FNetworkGUID GetNetGUIDFromStablyNamedObject(const UObject* Object) const;

	virtual bool SerializeObject(FArchive& Ar, UClass* InClass, UObject*& Obj, FNetworkGUID *OutNetGUID = NULL) override;

	void ResetTrackedObjectRefs(bool bShouldTrack)
	{
		TrackedUnresolvedRefs.Empty();
		bShouldTrackUnresolvedRefs = bShouldTrack;
	}

	const TSet<UnrealObjectRef>& GetTrackedUnresolvedRefs() const
	{
		return TrackedUnresolvedRefs;
	}

private:
	friend class FSpatialNetBitReader;

	bool					bShouldTrackUnresolvedRefs;
	TSet<UnrealObjectRef>	TrackedUnresolvedRefs;
};

class SPATIALGDK_API FSpatialNetGUIDCache : public FNetGUIDCache
{
public:
	FSpatialNetGUIDCache(class USpatialNetDriver* InDriver);
		
	FNetworkGUID AssignNewEntityActorNetGUID(AActor* Actor, const SubobjectToOffsetMap& SubobjectToOffset);
	void RemoveEntityNetGUID(Worker_EntityId EntityId);
	void RemoveEntitySubobjectsNetGUIDs(Worker_EntityId EntityId, const SubobjectToOffsetMap& SubobjectToOffset);
	void RemoveNetGUID(const FNetworkGUID& NetGUID);

	FNetworkGUID AssignNewStablyNamedObjectNetGUID(const UObject* Object);
	
	FNetworkGUID GetNetGUIDFromUnrealObjectRef(const UnrealObjectRef& ObjectRef);
	UnrealObjectRef GetUnrealObjectRefFromNetGUID(const FNetworkGUID& NetGUID) const;
	FNetworkGUID GetNetGUIDFromEntityId(Worker_EntityId EntityId) const;

private:
	FNetworkGUID GetOrAssignNetGUID_SpatialGDK(const UObject* Object);
	void RegisterObjectRef(FNetworkGUID NetGUID, const UnrealObjectRef& ObjectRef);
	
	FNetworkGUID RegisterNetGUIDFromPath(const FString& PathName, const FNetworkGUID& OuterGUID);
	FNetworkGUID GenerateNewNetGUID(const int32 IsStatic);

	TMap<FNetworkGUID, UnrealObjectRef> NetGUIDToUnrealObjectRef;
	TMap<UnrealObjectRef, FNetworkGUID> UnrealObjectRefToNetGUID;
};

