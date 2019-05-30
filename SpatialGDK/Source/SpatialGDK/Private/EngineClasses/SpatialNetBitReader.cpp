// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/SpatialNetBitReader.h"

#include "UObject/WeakObjectPtr.h"

#include "EngineClasses/SpatialPackageMapClient.h"
#include "SpatialConstants.h"

DEFINE_LOG_CATEGORY(LogSpatialNetBitReader);

FSpatialNetBitReader::FSpatialNetBitReader(USpatialPackageMapClient* InPackageMap, uint8* Source, int64 CountBits, TSet<FUnrealObjectRef>& InUnresolvedRefs)
	: FNetBitReader(InPackageMap, Source, CountBits)
	, UnresolvedRefs(InUnresolvedRefs) {}

void FSpatialNetBitReader::DeserializeObjectRef(FUnrealObjectRef& ObjectRef)
{
	int64 EntityId;
	*this << EntityId;
	ObjectRef.Entity = EntityId;
	*this << ObjectRef.Offset;

	uint8 HasPath;
	SerializeBits(&HasPath, 1);
	if (HasPath)
	{
		FString Path;
		*this << Path;

		ObjectRef.Path = Path;
	}

	uint8 HasOuter;
	SerializeBits(&HasOuter, 1);
	if (HasOuter)
	{
		ObjectRef.Outer = FUnrealObjectRef();
		DeserializeObjectRef(*ObjectRef.Outer);
	}
}

FArchive& FSpatialNetBitReader::operator<<(UObject*& Value)
{
	FUnrealObjectRef ObjectRef;

	DeserializeObjectRef(ObjectRef);

	check(ObjectRef != FUnrealObjectRef::UNRESOLVED_OBJECT_REF);
	if (ObjectRef == FUnrealObjectRef::NULL_OBJECT_REF)
	{
		Value = nullptr;
	}
	else
	{
		auto PackageMapClient = Cast<USpatialPackageMapClient>(PackageMap);
		FNetworkGUID NetGUID = PackageMapClient->GetNetGUIDFromUnrealObjectRef(ObjectRef);
		if (NetGUID.IsValid())
		{
			Value = PackageMapClient->GetObjectFromNetGUID(NetGUID, true);
			if (Value == nullptr)
			{
				// At this point, we're unable to resolve a stably-named actor by path. This likely means either the actor doesn't exist, or
				// it's part of a streaming level that hasn't been streamed in. Native Unreal networking sets reference to nullptr and continues.
				// So we do the same.
				FString FullPath;
				SpatialGDK::GetFullPathFromUnrealObjectReference(ObjectRef, FullPath);
				UE_LOG(LogSpatialNetBitReader, Verbose, TEXT("Object ref did not map to valid object. Streaming level not loaded or actor deleted. Will be set to nullptr: %s %s"),
					*ObjectRef.ToString(), FullPath.IsEmpty() ? TEXT("[NO PATH]") : *FullPath);
			}
		}
		else
		{
			UnresolvedRefs.Add(ObjectRef);
			Value = nullptr;
		}
	}

	return *this;
}

FArchive& FSpatialNetBitReader::operator<<(FWeakObjectPtr& Value)
{
	UObject* Object;
	*this << Object;

	Value = Object;

	return *this;
}
