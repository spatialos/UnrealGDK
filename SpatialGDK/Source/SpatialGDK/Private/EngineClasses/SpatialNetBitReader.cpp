// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/SpatialNetBitReader.h"

#include "UObject/WeakObjectPtr.h"

#include "EngineClasses/SpatialPackageMapClient.h"
#include "SpatialConstants.h"

DEFINE_LOG_CATEGORY(LogSpatialNetBitReader);

FSpatialNetBitReader::FSpatialNetBitReader(USpatialPackageMapClient* InPackageMap, uint8* Source, int64 CountBits, TSet<FUnrealObjectRef>& InDynamicRefs, TSet<FUnrealObjectRef>& InUnresolvedRefs)
	: FNetBitReader(InPackageMap, Source, CountBits)
	, DynamicRefs(InDynamicRefs)
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

	SerializeBits(&ObjectRef.bNoLoadOnClient, 1);
	SerializeBits(&ObjectRef.bUseSingletonClassPath, 1);
}

UObject* FSpatialNetBitReader::ReadObject(bool& bUnresolved)
{
	FUnrealObjectRef ObjectRef;

	DeserializeObjectRef(ObjectRef);
	check(ObjectRef != FUnrealObjectRef::UNRESOLVED_OBJECT_REF);

	UObject* Value = FUnrealObjectRef::ToObjectPtr(ObjectRef, Cast<USpatialPackageMapClient>(PackageMap), bUnresolved);

	if (bUnresolved)
	{
		UnresolvedRefs.Add(ObjectRef);
	}
	else if (Value && !Value->IsFullNameStableForNetworking())
	{
		DynamicRefs.Add(ObjectRef);
	}

	return Value;
}

FArchive& FSpatialNetBitReader::operator<<(UObject*& Value)
{
	bool bUnresolved = false;
	Value = ReadObject(bUnresolved);

	return *this;
}

FArchive& FSpatialNetBitReader::operator<<(FWeakObjectPtr& Value)
{
	UObject* Object;
	*this << Object;

	Value = Object;

	return *this;
}
