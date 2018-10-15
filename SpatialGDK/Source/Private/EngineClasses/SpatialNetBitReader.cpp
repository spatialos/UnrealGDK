// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialNetBitReader.h"

#include "UObject/WeakObjectPtr.h"

#include "EngineClasses/SpatialPackageMapClient.h"
#include "SpatialConstants.h"

FSpatialNetBitReader::FSpatialNetBitReader(USpatialPackageMapClient* InPackageMap, uint8* Source, int64 CountBits, TSet<FUnrealObjectRef>& InUnresolvedRefs)
	: FNetBitReader(InPackageMap, Source, CountBits)
	, UnresolvedRefs(InUnresolvedRefs) {}

void FSpatialNetBitReader::DeserializeObjectRef(FUnrealObjectRef& ObjectRef)
{
	*this << ObjectRef.Entity;
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

	check(ObjectRef != SpatialConstants::UNRESOLVED_OBJECT_REF);
	if (ObjectRef == SpatialConstants::NULL_OBJECT_REF)
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
			checkf(Value, TEXT("An object ref %s should map to a valid object."), *ObjectRef.ToString());
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
