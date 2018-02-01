// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialPackageMapClient.h"
#include "EntityRegistry.h"
#include "SpatialNetDriver.h"
#include "SpatialUpdateInterop.h"
#include "SpatialActorChannel.h"
#include "SpatialTypeBinding.h"

DEFINE_LOG_CATEGORY(LogSpatialOSPackageMap);

const uint32 StaticObjectOffset = 0x80000000; // 2^31

struct FCompareComponentNames
{
	bool operator()(UObject& A, UObject& B) const 
	{
		return A.GetName() < B.GetName();
	}
};

void USpatialPackageMapClient::ResolveStaticObjectGUID(FNetworkGUID& NetGUID, FString& Path)
{
	// there should never be a case where a static object gets allocated in the dynamic region
	check(NetGUID.Value >= StaticObjectOffset);

	// check that we don't already have this object registered
	UObject* RegisteredObject = GuidCache->GetObjectFromNetGUID(NetGUID, false);
	if (RegisteredObject == nullptr)
	{
		FStringAssetReference AssetRef(*Path);
		UObject* Object = AssetRef.TryLoad();
		if (Object != nullptr)
		{
			FNetGuidCacheObject CacheObject;
			CacheObject.Object = Object;
			CacheObject.PathName = FName(*Path);
			static_cast<FSpatialNetGUIDCache*>(GuidCache.Get())->RegisterNetGUID_Internal(NetGUID, CacheObject);

		}
		else
		{
			UE_LOG(LogSpatialOSPackageMap, Warning, TEXT("Failed to resolve object with path: %s"), *Path);
		}
	}
}

FNetworkGUID USpatialPackageMapClient::ResolveEntityActor(AActor* Actor, FEntityId EntityId)
{
	FSpatialNetGUIDCache* SpatialGuidCache = static_cast<FSpatialNetGUIDCache*>(GuidCache.Get());
	FNetworkGUID NetGUID = SpatialGuidCache->GetNetGUIDFromEntityId(EntityId.ToSpatialEntityId());

	// check we haven't already assigned a NetGUID to this object
	if (!NetGUID.IsValid())
	{
		NetGUID = SpatialGuidCache->AssignNewEntityActorNetGUID(Actor);
	}
	return NetGUID;
}

bool USpatialPackageMapClient::SerializeNewActor(FArchive & Ar, UActorChannel * Channel, AActor *& Actor)
{
	bool bResult = Super::SerializeNewActor(Ar, Channel, Actor);
	//will remove the override if remains unused.
	return bResult;
}


improbable::unreal::UnrealObjectRef USpatialPackageMapClient::GetUnrealObjectRefFromNetGUID(const FNetworkGUID & NetGUID) const
{
	FSpatialNetGUIDCache* SpatialGuidCache = static_cast<FSpatialNetGUIDCache*>(GuidCache.Get());
	return SpatialGuidCache->GetUnrealObjectRefFromNetGUID(NetGUID);
}

FNetworkGUID USpatialPackageMapClient::GetNetGUIDFromUnrealObjectRef(const improbable::unreal::UnrealObjectRef & ObjectRef) const
{
	FSpatialNetGUIDCache* SpatialGuidCache = static_cast<FSpatialNetGUIDCache*>(GuidCache.Get());
	return SpatialGuidCache->GetNetGUIDFromUnrealObjectRef(ObjectRef);
}

FNetworkGUID USpatialPackageMapClient::GetNetGUIDFromEntityId(const worker::EntityId & EntityId) const
{
	FSpatialNetGUIDCache* SpatialGuidCache = static_cast<FSpatialNetGUIDCache*>(GuidCache.Get());
	improbable::unreal::UnrealObjectRef ObjectRef{EntityId, 0};
	return GetNetGUIDFromUnrealObjectRef(ObjectRef);
}

void USpatialPackageMapClient::AddPendingObjRef(UObject* Object, USpatialActorChannel* DependentChannel, uint16 Handle)
{
	UE_LOG(LogSpatialOSPackageMap, Log, TEXT("Added pending obj ref for object: %s, channel: %s, handle: %d."),
		*Object->GetName(), *DependentChannel->GetName(), Handle);
	FSpatialNetGUIDCache* SpatialGuidCache = static_cast<FSpatialNetGUIDCache*>(GuidCache.Get());
	SpatialGuidCache->AddPendingObjRef(Object, DependentChannel, Handle);
}

void USpatialPackageMapClient::AddPendingRPC(UObject* UnresolvedObject, FCommandRequestContext::FRequestFunction CommandSender)
{
	UE_LOG(LogSpatialOSPackageMap, Log, TEXT("Added pending RPC for object: %s."), *UnresolvedObject->GetName());
	FSpatialNetGUIDCache* SpatialGuidCache = static_cast<FSpatialNetGUIDCache*>(GuidCache.Get());
	SpatialGuidCache->AddPendingRPC(UnresolvedObject, CommandSender);
}

FSpatialNetGUIDCache::FSpatialNetGUIDCache(USpatialNetDriver* InDriver)
	: FNetGUIDCache(InDriver)
{
}

FNetworkGUID FSpatialNetGUIDCache::AssignNewEntityActorNetGUID(AActor* Actor)
{
	FEntityId EntityId = Cast<USpatialNetDriver>(Driver)->GetEntityRegistry()->GetEntityIdFromActor(Actor);
	check(EntityId.ToSpatialEntityId() > 0)

	UE_LOG(LogSpatialOSPackageMap, Log, TEXT("Assigning a NetGUID to entityid %d, actor: %s"), EntityId.ToSpatialEntityId(), *Actor->GetName());
	FNetworkGUID NetGUID = GetOrAssignNetGUID(Actor);
	//One major difference between how Unreal does NetGUIDs vs us is, we don't attempt to make them consistent across workers and client.
	// The function above might have returned without assigning new GUID, because we are the client.
	// Let's directly call the client function in that case.
	if (NetGUID == FNetworkGUID::GetDefault() && !IsNetGUIDAuthority() && IsDynamicObject(Actor)) //todo-giray: Support static objects
	{
		// Here we have to borrow from FNetGuidCache::\\\\\\\GUID_Server to avoid source change
#define COMPOSE_NET_GUID( Index, IsStatic )	( ( ( Index ) << 1 ) | ( IsStatic ) )
#define ALLOC_NEW_NET_GUID( IsStatic )		( COMPOSE_NET_GUID( ++UniqueNetIDs[ IsStatic ], IsStatic ) )

		// Generate new NetGUID and assign it
		const int32 IsStatic = IsDynamicObject(Actor) ? 0 : 1;

		const FNetworkGUID NewNetGuid(ALLOC_NEW_NET_GUID(IsStatic));
		RegisterNetGUID_Client(NewNetGuid, Actor);
		
		UE_LOG(LogSpatialOSPackageMap, Log, TEXT("NetGUID for %s was not found in the cache. Generated new NetGUID."), *Actor->GetName());
		NetGUID = NewNetGuid;
	}

	check(NetGUID.IsValid());
	check(!NetGUID.IsDefault());
	uint32 SubObjOffset = 0;
	improbable::unreal::UnrealObjectRef ObjRef{ EntityId.ToSpatialEntityId(), SubObjOffset };
	FUnrealObjectRefWrapper ObjRefWrapper;
	ObjRefWrapper.ObjectRef = ObjRef;
	NetGUIDToUnrealObjectRef.Emplace(NetGUID, ObjRefWrapper);
	UnrealObjectRefToNetGUID.Emplace(ObjRefWrapper, NetGUID);
	UE_LOG(LogSpatialOSPackageMap, Log, TEXT("Registered new objref for actor: %s, entityid: %d"), *Actor->GetName(), EntityId.ToSpatialEntityId());

	// Resolve pending replication updates and RPCs that reference this actor.
	ResolvePendingObjRefs(Actor);
	ResolvePendingRPCs(Actor);

	// Allocate GUIDs for each subobject too
	TArray<UObject*> DefaultSubobjects;
	Actor->GetDefaultSubobjects(DefaultSubobjects);

	// Sorting alphabetically to ensure stable references
	Sort(DefaultSubobjects.GetData(), DefaultSubobjects.Num(), FCompareComponentNames());

	for (UObject* Subobject : DefaultSubobjects)
	{
		SubObjOffset++;
		FNetworkGUID SubObjectNetGUID = GetOrAssignNetGUID(Subobject);
		ObjRefWrapper.ObjectRef.set_offset(SubObjOffset);
		NetGUIDToUnrealObjectRef.Emplace(SubObjectNetGUID, ObjRefWrapper);
		UnrealObjectRefToNetGUID.Emplace(ObjRefWrapper, SubObjectNetGUID);
		ResolvePendingObjRefs(Subobject);
		ResolvePendingRPCs(Subobject);
		check(SubObjectNetGUID.IsValid());
	}

	return NetGUID;
}

FNetworkGUID FSpatialNetGUIDCache::GetNetGUIDFromUnrealObjectRef(const improbable::unreal::UnrealObjectRef& ObjectRef) const
{
	FUnrealObjectRefWrapper ObjRefWrapper;
	ObjRefWrapper.ObjectRef = ObjectRef;
	const FNetworkGUID* NetGUID = UnrealObjectRefToNetGUID.Find(ObjRefWrapper);
	return (NetGUID == nullptr ? FNetworkGUID(0) : *NetGUID);
}

improbable::unreal::UnrealObjectRef FSpatialNetGUIDCache::GetUnrealObjectRefFromNetGUID(const FNetworkGUID& NetGUID) const
{
	const FUnrealObjectRefWrapper* ObjRefWrapper = NetGUIDToUnrealObjectRef.Find(NetGUID);

	// If not found, return entity id 0 as it's not a valid entity id.
	return ObjRefWrapper ? ObjRefWrapper->ObjectRef : improbable::unreal::UnrealObjectRef{0, 0};
}

FNetworkGUID FSpatialNetGUIDCache::GetNetGUIDFromEntityId(const worker::EntityId& EntityId) const
{
	FUnrealObjectRefWrapper ObjRefWrapper;
	improbable::unreal::UnrealObjectRef ObjRef{EntityId, 0};
	ObjRefWrapper.ObjectRef = ObjRef;
	const FNetworkGUID* NetGUID = UnrealObjectRefToNetGUID.Find(ObjRefWrapper);
	return (NetGUID == nullptr ? FNetworkGUID(0) : *NetGUID);
}

void FSpatialNetGUIDCache::AddPendingObjRef(UObject* Object, USpatialActorChannel* DependentChannel, uint16 Handle)
{
	if (Object == nullptr)
	{
		return;
	}

	TArray<USpatialActorChannel*>& Channels = ChannelsAwaitingObjRefResolve.FindOrAdd(Object);
	Channels.AddUnique(DependentChannel);

	TArray<uint16>& Handles = PendingObjRefHandles.FindOrAdd(DependentChannel);
	Handles.AddUnique(Handle);
}

void FSpatialNetGUIDCache::AddPendingRPC(UObject* UnresolvedObject, FCommandRequestContext::FRequestFunction CommandSender)
{
	if (UnresolvedObject == nullptr)
	{
		return;
	}

	PendingRPCs.FindOrAdd(UnresolvedObject).Add(CommandSender);
}

FNetworkGUID FSpatialNetGUIDCache::AssignNewNetGUID(const UObject* Object)
{
	FNetworkGUID GUID = (0x80000000 - 1) & ++UniqueNetIDs[0];
	FNetGuidCacheObject CacheObject;
	CacheObject.Object = Object;

	RegisterNetGUID_Internal(GUID, CacheObject);
	return GUID;
}

void FSpatialNetGUIDCache::ResolvePendingObjRefs(const UObject* Object)
{
	TArray<USpatialActorChannel*>* DependentChannels = ChannelsAwaitingObjRefResolve.Find(Object);
	if (DependentChannels == nullptr)
	{
		return;
	}

	USpatialUpdateInterop* UpdateInterop = Cast<USpatialNetDriver>(Driver)->GetSpatialUpdateInterop();
	check(UpdateInterop);

	for (auto DependentChannel : *DependentChannels)
	{
		TArray<uint16>* Handles = PendingObjRefHandles.Find(DependentChannel);
		if (Handles && Handles->Num() > 0)
		{
			// Changelists always have a 0 at the end.
			Handles->Add(0);

			UpdateInterop->SendSpatialUpdate(DependentChannel, *Handles);
			PendingObjRefHandles.Remove(DependentChannel);
		}
	}
	DependentChannels->Reset();
	ChannelsAwaitingObjRefResolve.Remove(Object);
}

void FSpatialNetGUIDCache::ResolvePendingRPCs(UObject* Object)
{
	TArray<FCommandRequestContext::FRequestFunction>* RPCList = PendingRPCs.Find(Object);
	if (RPCList)
	{
		USpatialUpdateInterop* UpdateInterop = Cast<USpatialNetDriver>(Driver)->GetSpatialUpdateInterop();
		check(UpdateInterop);
		for (auto& RequestFunc : *RPCList)
		{
			// We can guarantee that SendCommandRequest won't populate PendingRPCs[Actor], because Actor has
			// been resolved when we call ResolvePendingRPCs.
			UpdateInterop->SendCommandRequest(RequestFunc);
		}
		PendingRPCs.Remove(Object);
	}
}
