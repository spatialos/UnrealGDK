// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/SpatialPackageMapClient.h"

#include "EngineUtils.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"

#include "EngineClasses/SpatialActorChannel.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Interop/SpatialReceiver.h"
#include "Interop/SpatialSender.h"
#include "Schema/UnrealObjectRef.h"
#include "SpatialConstants.h"
#include "Utils/EntityRegistry.h"
#include "Utils/SchemaOption.h"

DEFINE_LOG_CATEGORY(LogSpatialPackageMap);

void GetSubobjects(UObject* Object, TArray<UObject*>& InSubobjects)
{
	InSubobjects.Empty();
	ForEachObjectWithOuter(Object, [&InSubobjects](UObject* Object)
	{
		// Objects can only be allocated NetGUIDs if this is true.
		if (Object->IsSupportedForNetworking() && !Object->IsPendingKill() && !Object->IsEditorOnly())
		{
			// Walk up the outer chain and ensure that no object is PendingKill. This is required because although
			// EInternalObjectFlags::PendingKill prevents objects that are PendingKill themselves from getting added
			// to the list, it'll still add children of PendingKill objects. This then causes an assertion within 
			// FNetGUIDCache::RegisterNetGUID_Server where it again iterates up the object's owner chain, assigning
			// ids and ensuring that no object is set to PendingKill in the process.
			UObject* Outer = Object->GetOuter();
			while (Outer != nullptr)
			{
				if (Outer->IsPendingKill())
				{
					return;
				}
				Outer = Outer->GetOuter();
			}
			InSubobjects.Add(Object);
		}
	}, true, RF_NoFlags, EInternalObjectFlags::PendingKill);

	InSubobjects.StableSort([](UObject& A, UObject& B)
	{
		return A.GetName() < B.GetName();
	});
}

FNetworkGUID USpatialPackageMapClient::ResolveEntityActor(AActor* Actor, Worker_EntityId EntityId)
{
	FSpatialNetGUIDCache* SpatialGuidCache = static_cast<FSpatialNetGUIDCache*>(GuidCache.Get());
	FNetworkGUID NetGUID = SpatialGuidCache->GetNetGUIDFromEntityId(EntityId);

	// check we haven't already assigned a NetGUID to this object
	if (!NetGUID.IsValid())
	{
		NetGUID = SpatialGuidCache->AssignNewEntityActorNetGUID(Actor);
	}
	return NetGUID;
}

void USpatialPackageMapClient::RemoveEntityActor(Worker_EntityId EntityId)
{
	FSpatialNetGUIDCache* SpatialGuidCache = static_cast<FSpatialNetGUIDCache*>(GuidCache.Get());

	if (SpatialGuidCache->GetNetGUIDFromEntityId(EntityId).IsValid())
	{
		SpatialGuidCache->RemoveEntityNetGUID(EntityId);
	}
}

FNetworkGUID USpatialPackageMapClient::ResolveStablyNamedObject(UObject* Object)
{
	FSpatialNetGUIDCache* SpatialGuidCache = static_cast<FSpatialNetGUIDCache*>(GuidCache.Get());
	return SpatialGuidCache->AssignNewStablyNamedObjectNetGUID(Object);
}

FUnrealObjectRef USpatialPackageMapClient::GetUnrealObjectRefFromNetGUID(const FNetworkGUID & NetGUID) const
{
	FSpatialNetGUIDCache* SpatialGuidCache = static_cast<FSpatialNetGUIDCache*>(GuidCache.Get());
	return SpatialGuidCache->GetUnrealObjectRefFromNetGUID(NetGUID);
}

FNetworkGUID USpatialPackageMapClient::GetNetGUIDFromUnrealObjectRef(const FUnrealObjectRef & ObjectRef) const
{
	FSpatialNetGUIDCache* SpatialGuidCache = static_cast<FSpatialNetGUIDCache*>(GuidCache.Get());
	return SpatialGuidCache->GetNetGUIDFromUnrealObjectRef(ObjectRef);
}

FNetworkGUID USpatialPackageMapClient::GetNetGUIDFromEntityId(const Worker_EntityId& EntityId) const
{
	FSpatialNetGUIDCache* SpatialGuidCache = static_cast<FSpatialNetGUIDCache*>(GuidCache.Get());
	FUnrealObjectRef ObjectRef(EntityId, 0);
	return GetNetGUIDFromUnrealObjectRef(ObjectRef);
}

TWeakObjectPtr<UObject> USpatialPackageMapClient::GetObjectFromUnrealObjectRef(const FUnrealObjectRef& ObjectRef)
{
	FNetworkGUID NetGUID = GetNetGUIDFromUnrealObjectRef(ObjectRef);
	if (NetGUID.IsValid() && !NetGUID.IsDefault())
	{
		return GetObjectFromNetGUID(NetGUID, true);
	}

	return nullptr;
}

FUnrealObjectRef USpatialPackageMapClient::GetUnrealObjectRefFromObject(UObject* Object)
{
	if (Object == nullptr)
	{
		return FUnrealObjectRef::NULL_OBJECT_REF;
	}

	FNetworkGUID NetGUID = GetNetGUIDFromObject(Object);

	return GetUnrealObjectRefFromNetGUID(NetGUID);
}

bool USpatialPackageMapClient::SerializeObject(FArchive& Ar, UClass* InClass, UObject*& Obj, FNetworkGUID *OutNetGUID)
{
	// Super::SerializeObject is not called here on purpose
	Ar << Obj;

	return true;
}

FSpatialNetGUIDCache::FSpatialNetGUIDCache(USpatialNetDriver* InDriver)
	: FNetGUIDCache(InDriver)
{
}

FNetworkGUID FSpatialNetGUIDCache::AssignNewEntityActorNetGUID(AActor* Actor)
{
	USpatialNetDriver* SpatialNetDriver = Cast<USpatialNetDriver>(Driver);

	Worker_EntityId EntityId = SpatialNetDriver->GetEntityRegistry()->GetEntityIdFromActor(Actor);
	check(EntityId > 0);

	USpatialReceiver* Receiver = SpatialNetDriver->Receiver;

	FNetworkGUID NetGUID;
	FUnrealObjectRef EntityObjectRef(EntityId, 0);

	// Valid if Actor is stably named. Used for stably named subobject assignment further below
	FUnrealObjectRef StablyNamedRef;

	if (Actor->IsNameStableForNetworking())
	{
		// Startup Actors have two valid UnrealObjectRefs: the entity id and the path.
		// AssignNewStablyNamedObjectNetGUID will register the path ref.
		NetGUID = AssignNewStablyNamedObjectNetGUID(Actor);

		// We register the entity id ref here.
		UnrealObjectRefToNetGUID.Emplace(EntityObjectRef, NetGUID);

		// Once we have an entity id, we should always be using it to refer to entities.
		// Since the path ref may have been registered previously, we first try to remove it
		// and then register the entity id ref.
		StablyNamedRef = NetGUIDToUnrealObjectRef[NetGUID];
		NetGUIDToUnrealObjectRef.Emplace(NetGUID, EntityObjectRef);
	}
	else
	{
		NetGUID = GetOrAssignNetGUID_SpatialGDK(Actor);
		RegisterObjectRef(NetGUID, EntityObjectRef);
	}

	UE_LOG(LogSpatialPackageMap, Verbose, TEXT("Registered new object ref for actor: %s. NetGUID: %s, entity ID: %lld"),
		*Actor->GetName(), *NetGUID.ToString(), EntityId);

	// This will be null when being used in the snapshot generator
#if WITH_EDITOR
	if (Receiver != nullptr)
#endif
	{
		Receiver->ResolvePendingOperations(Actor, EntityObjectRef);
	}

	const FClassInfo& Info = SpatialNetDriver->ClassInfoManager->GetOrCreateClassInfoByClass(Actor->GetClass());
	const SubobjectToOffsetMap& SubobjectToOffset = improbable::CreateOffsetMapFromActor(Actor, Info);

	for (auto& Pair : SubobjectToOffset)
	{
		UObject* Subobject = Pair.Key;
		uint32 Offset = Pair.Value;

		// AssignNewStablyNamedObjectNetGUID is not used due to using the wrong ObjectRef as the outer of the subobject.
		// So it is ok to use RegisterObjectRef in both cases since no prior bookkeeping was done (unlike Actors)
		FNetworkGUID SubobjectNetGUID = GetOrAssignNetGUID_SpatialGDK(Subobject);
		FUnrealObjectRef EntityIdSubobjectRef(EntityId, Offset);

		if (Subobject->IsNameStableForNetworking())
		{
			if (Subobject->GetFName().ToString().Equals(TEXT("PersistentLevel")) && !Subobject->IsA<ULevel>())
			{
				UE_LOG(LogSpatialPackageMap, Fatal, TEXT("Found object called PersistentLevel which isn't a Level! This is not allowed when using the GDK"));
			}

			// Using StablyNamedRef for the outer since referencing ObjectRef in the map
			// will have the EntityId
			FUnrealObjectRef StablyNamedSubobjectRef(0, 0, Subobject->GetFName().ToString(), StablyNamedRef);

			// This is the only extra object ref that has to be registered for the subobject.
			UnrealObjectRefToNetGUID.Emplace(StablyNamedSubobjectRef, SubobjectNetGUID);
		}

		RegisterObjectRef(SubobjectNetGUID, EntityIdSubobjectRef);

		UE_LOG(LogSpatialPackageMap, Verbose, TEXT("Registered new object ref for subobject %s inside actor %s. NetGUID: %s, object ref: %s"),
			*Subobject->GetName(), *Actor->GetName(), *SubobjectNetGUID.ToString(), *EntityIdSubobjectRef.ToString());

			// This will be null when being used in the snapshot generator
#if WITH_EDITOR
			if (Receiver != nullptr)
#endif
			{
				Receiver->ResolvePendingOperations(Subobject, EntityIdSubobjectRef);
			}
	}

	return NetGUID;
}

// Recursively assign netguids to the outer chain of a UObject. Then associate them with their Spatial representation (FUnrealObjectRef)
// This is required in order to be able to refer to a non-replicated stably named UObject.
// Dynamically spawned actors and references to their subobjects do not go through this codepath.
FNetworkGUID FSpatialNetGUIDCache::AssignNewStablyNamedObjectNetGUID(UObject* Object)
{
	FNetworkGUID NetGUID = GetOrAssignNetGUID_SpatialGDK(Object);
	FUnrealObjectRef ExistingObjRef = GetUnrealObjectRefFromNetGUID(NetGUID);
	if (ExistingObjRef != FUnrealObjectRef::UNRESOLVED_OBJECT_REF)
	{
		return NetGUID;
	}

	FNetworkGUID OuterGUID;
	UObject* OuterObject = Object->GetOuter();

	if (OuterObject)
	{
		OuterGUID = AssignNewStablyNamedObjectNetGUID(OuterObject);
	}


	if (Object->GetFName().ToString().Equals(TEXT("PersistentLevel")) && !Object->IsA<ULevel>())
	{
		UE_LOG(LogSpatialPackageMap, Fatal, TEXT("Found object called PersistentLevel which isn't a Level! This is not allowed when using the GDK"));
	}

	FUnrealObjectRef StablyNamedObjRef(0, 0, Object->GetFName().ToString(), (OuterGUID.IsValid() && !OuterGUID.IsDefault()) ? GetUnrealObjectRefFromNetGUID(OuterGUID) : FUnrealObjectRef());
	RegisterObjectRef(NetGUID, StablyNamedObjRef);

	return NetGUID;
}

void FSpatialNetGUIDCache::RemoveEntityNetGUID(Worker_EntityId EntityId)
{
	// Remove actor subobjects.
	USpatialNetDriver* SpatialNetDriver = Cast<USpatialNetDriver>(Driver);

	AActor* Actor = SpatialNetDriver->EntityRegistry->GetActorFromEntityId(EntityId);
	if (Actor == nullptr)
	{
		UE_LOG(LogSpatialPackageMap, Warning, TEXT("Trying to clean up Actor for EntityId %lld but Actor does not exist! Will not cleanup subobjects for this Entity"), EntityId);
		return;
	}

	UClass* Class = Actor->GetClass();
	const FClassInfo& Info = SpatialNetDriver->ClassInfoManager->GetOrCreateClassInfoByClass(Class);

	improbable::UnrealMetadata* UnrealMetadata = SpatialNetDriver->StaticComponentView->GetComponentData<improbable::UnrealMetadata>(EntityId);

	// There are times when the Editor is quitting out of PIE that this is nullptr.
	if (UnrealMetadata == nullptr)
	{
		return;
	}

	improbable::TSchemaOption<FUnrealObjectRef>& StablyNamedRefOption = UnrealMetadata->StablyNamedRef;

	for (auto& SubobjectInfoPair : Info.SubobjectInfo)
	{
		FUnrealObjectRef SubobjectRef(EntityId, SubobjectInfoPair.Key);
		if (FNetworkGUID* SubobjectNetGUID = UnrealObjectRefToNetGUID.Find(SubobjectRef))
		{
			NetGUIDToUnrealObjectRef.Remove(*SubobjectNetGUID);
			UnrealObjectRefToNetGUID.Remove(SubobjectRef);

			if (StablyNamedRefOption.IsSet())
			{
				UnrealObjectRefToNetGUID.Remove(FUnrealObjectRef(0, 0, SubobjectInfoPair.Value->SubobjectName.ToString(), StablyNamedRefOption.GetValue()));
			}
		}
	}

	// Remove actor.
	FNetworkGUID EntityNetGUID = GetNetGUIDFromEntityId(EntityId);
	// TODO: Figure out why NetGUIDToUnrealObjectRef might not have this GUID. UNR-989
	if (FUnrealObjectRef* ActorRef = NetGUIDToUnrealObjectRef.Find(EntityNetGUID))
	{
		UnrealObjectRefToNetGUID.Remove(*ActorRef);
	}
	NetGUIDToUnrealObjectRef.Remove(EntityNetGUID);
	if (StablyNamedRefOption.IsSet())
	{
		UnrealObjectRefToNetGUID.Remove(StablyNamedRefOption.GetValue());
	}
}

FNetworkGUID FSpatialNetGUIDCache::GetNetGUIDFromUnrealObjectRef(const FUnrealObjectRef& ObjectRef)
{
	return GetNetGUIDFromUnrealObjectRefInternal(ObjectRef);
}

FNetworkGUID FSpatialNetGUIDCache::GetNetGUIDFromUnrealObjectRefInternal(const FUnrealObjectRef& ObjectRef)
{
	FNetworkGUID* CachedGUID = UnrealObjectRefToNetGUID.Find(ObjectRef);
	FNetworkGUID NetGUID = CachedGUID ? *CachedGUID : FNetworkGUID{};
	if (!NetGUID.IsValid() && ObjectRef.Path.IsSet())
	{
		FNetworkGUID OuterGUID;
		if (ObjectRef.Outer.IsSet())
		{
			OuterGUID = GetNetGUIDFromUnrealObjectRef(ObjectRef.Outer.GetValue());
		}
		NetGUID = RegisterNetGUIDFromPathForStaticObject(ObjectRef.Path.GetValue(), OuterGUID);
		RegisterObjectRef(NetGUID, ObjectRef);
	}
	return NetGUID;
}

void FSpatialNetGUIDCache::NetworkRemapObjectRefPaths(FUnrealObjectRef& ObjectRef, bool bReading) const
{
	// If we have paths, network-sanitize all of them (e.g. removing PIE prefix).
	if (!ObjectRef.Path.IsSet())
	{
		return;
	}

	FUnrealObjectRef* Iterator = &ObjectRef;
	while (true)
	{
		if (Iterator->Path.IsSet())
		{
			FString TempPath(*Iterator->Path);
			GEngine->NetworkRemapPath(Driver, TempPath, bReading);
			Iterator->Path = TempPath;
		}
		if (!Iterator->Outer.IsSet())
		{
			break;
		}
		Iterator = &Iterator->Outer.GetValue();
	}
}

FUnrealObjectRef FSpatialNetGUIDCache::GetUnrealObjectRefFromNetGUID(const FNetworkGUID& NetGUID) const
{
	const FUnrealObjectRef* ObjRef = NetGUIDToUnrealObjectRef.Find(NetGUID);
	return ObjRef ? (FUnrealObjectRef)*ObjRef : FUnrealObjectRef::UNRESOLVED_OBJECT_REF;
}

FNetworkGUID FSpatialNetGUIDCache::GetNetGUIDFromEntityId(Worker_EntityId EntityId) const
{
	FUnrealObjectRef ObjRef(EntityId, 0);
	const FNetworkGUID* NetGUID = UnrealObjectRefToNetGUID.Find(ObjRef);
	return (NetGUID == nullptr ? FNetworkGUID(0) : *NetGUID);
}

FNetworkGUID FSpatialNetGUIDCache::RegisterNetGUIDFromPathForStaticObject(const FString& PathName, const FNetworkGUID& OuterGUID)
{
	// Put the PIE prefix back (if applicable) so that the correct object can be found.
	FString TempPath = PathName;
	GEngine->NetworkRemapPath(Driver, TempPath, true);

	// This function should only be called for stably named object references, not dynamic ones.
	FNetGuidCacheObject CacheObject;
	CacheObject.PathName = FName(*TempPath);
	CacheObject.OuterGUID = OuterGUID;
	CacheObject.bNoLoad = false;				// allow worker to attempt to load object
	CacheObject.bIgnoreWhenMissing = true;		// ensure we give workers time to load non-loaded assets
	FNetworkGUID NetGUID = GenerateNewNetGUID(0);
	RegisterNetGUID_Internal(NetGUID, CacheObject);
	return NetGUID;
}

FNetworkGUID FSpatialNetGUIDCache::GenerateNewNetGUID(const int32 IsStatic)
{
	// Here we have to borrow from FNetGuidCache::AssignNewNetGUID_Server to avoid a source change.
#define COMPOSE_NET_GUID(Index, IsStatic)	(((Index) << 1) | (IsStatic) )
#define ALLOC_NEW_NET_GUID(IsStatic)		(COMPOSE_NET_GUID(++UniqueNetIDs[IsStatic], IsStatic))

	// Generate new NetGUID and assign it
	FNetworkGUID NetGUID = FNetworkGUID(ALLOC_NEW_NET_GUID(IsStatic));
	return NetGUID;
}

FNetworkGUID FSpatialNetGUIDCache::GetOrAssignNetGUID_SpatialGDK(UObject* Object)
{
	FNetworkGUID NetGUID = GetOrAssignNetGUID(Object);

	// One major difference between how Unreal does NetGUIDs vs us is, we don't attempt to make them consistent across workers and client.
	// The function above might have returned without assigning new GUID, because we are the client.
	// Let's directly call the client function in that case.
	if (Object != nullptr && NetGUID == FNetworkGUID::GetDefault() && !IsNetGUIDAuthority())
	{
		NetGUID = GenerateNewNetGUID(IsDynamicObject(Object) ? 0 : 1);

		FNetGuidCacheObject CacheObject;
		CacheObject.Object = MakeWeakObjectPtr(const_cast<UObject*>(Object));
		CacheObject.PathName = Object->GetFName();
		CacheObject.OuterGUID = GetOrAssignNetGUID_SpatialGDK(Object->GetOuter());
		CacheObject.bIgnoreWhenMissing = true;
		RegisterNetGUID_Internal(NetGUID, CacheObject);

		UE_LOG(LogSpatialPackageMap, Log, TEXT("%s: NetGUID for object %s was not found in the cache. Generated new NetGUID %s."),
			*Cast<USpatialNetDriver>(Driver)->Connection->GetWorkerId(),
			*Object->GetPathName(),
			*NetGUID.ToString());
	}

	check((NetGUID.IsValid() && !NetGUID.IsDefault()) || Object == nullptr);
	return NetGUID;
}

void FSpatialNetGUIDCache::RegisterObjectRef(FNetworkGUID NetGUID, const FUnrealObjectRef& ObjectRef)
{
	// Registered ObjectRefs should never have PIE.
	FUnrealObjectRef RemappedObjectRef = ObjectRef;
	NetworkRemapObjectRefPaths(RemappedObjectRef, false /*bIsReading*/);

	checkSlow(!NetGUIDToUnrealObjectRef.Contains(NetGUID) || (NetGUIDToUnrealObjectRef.Contains(NetGUID) && NetGUIDToUnrealObjectRef.FindChecked(NetGUID) == RemappedObjectRef));
	checkSlow(!UnrealObjectRefToNetGUID.Contains(RemappedObjectRef) || (UnrealObjectRefToNetGUID.Contains(RemappedObjectRef) && UnrealObjectRefToNetGUID.FindChecked(RemappedObjectRef) == NetGUID));
	NetGUIDToUnrealObjectRef.Emplace(NetGUID, RemappedObjectRef);
	UnrealObjectRefToNetGUID.Emplace(RemappedObjectRef, NetGUID);
}
