// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialMemoryWriter.h"

#include "SpatialPackageMapClient.h"
#include "WeakObjectPtr.h"
#include "SchemaHelpers.h"
#include "Schema/UnrealObjectRef.h"

void FSpatialMemoryWriter::SerializeObjectRef(UnrealObjectRef& ObjectRef)
{
	*this << ObjectRef.Entity;
	*this << ObjectRef.Offset;

	uint8 HasPath = !ObjectRef.Path.empty();
	SerializeBits(&HasPath, 1);
	if (HasPath)
	{
		FString Path = FString(UTF8_TO_TCHAR(ObjectRef.Path->c_str()));
		*this << Path;
	}

	uint8 HasOuter = !ObjectRef.Outer.empty();
	SerializeBits(&HasOuter, 1);
	if (HasOuter)
	{
		SerializeObjectRef(*ObjectRef.Outer);
	}
}

FArchive& FSpatialMemoryWriter::operator<<(UObject*& Value)
{
	UnrealObjectRef ObjectRef;
	if (Value != nullptr)
	{
		FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromObject(Value);
		ObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(NetGUID);
		if (ObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UnresolvedObjects.Add(Value);
			ObjectRef = SpatialConstants::NULL_OBJECT_REF;
		}
	}
	else
	{
		ObjectRef = SpatialConstants::NULL_OBJECT_REF;
	}

	SerializeObjectRef(ObjectRef);

	return *this;
}

FArchive& FSpatialMemoryWriter::operator<<(FWeakObjectPtr& Value)
{
	UObject* Object = Value.Get(true);
	*this << Object;

	return *this;
}

void FSpatialNetBitWriter::SerializeObjectRef(UnrealObjectRef& ObjectRef)
{
	*this << ObjectRef.Entity;
	*this << ObjectRef.Offset;

	uint8 HasPath = !ObjectRef.Path.empty();
	SerializeBits(&HasPath, 1);
	if (HasPath)
	{
		FString Path = FString(UTF8_TO_TCHAR(ObjectRef.Path->c_str()));
		*this << Path;
	}

	uint8 HasOuter = !ObjectRef.Outer.empty();
	SerializeBits(&HasOuter, 1);
	if (HasOuter)
	{
		SerializeObjectRef(*ObjectRef.Outer);
	}
}

FArchive& FSpatialNetBitWriter::operator<<(UObject*& Value)
{
	UnrealObjectRef ObjectRef;
	if (Value != nullptr)
	{
		auto PackageMapClient = Cast<USpatialPackageMapClient>(PackageMap);
		FNetworkGUID NetGUID = PackageMapClient->GetNetGUIDFromObject(Value);
		if (!NetGUID.IsValid())
		{
			if (Value->IsFullNameStableForNetworking())
			{
				NetGUID = PackageMapClient->ResolveStablyNamedObject(Value);
			}
		}
		ObjectRef = UnrealObjectRef(PackageMapClient->GetUnrealObjectRefFromNetGUID(NetGUID));
		if (ObjectRef == UNRESOLVED_OBJECT_REF)
		{
			UnresolvedObjects.Add(Value);
			ObjectRef = NULL_OBJECT_REF;
		}
	}
	else
	{
		ObjectRef = NULL_OBJECT_REF;
	}

	SerializeObjectRef(ObjectRef);

	return *this;
}

FArchive& FSpatialNetBitWriter::operator<<(FWeakObjectPtr& Value)
{
	UObject* Object = Value.Get(true);
	*this << Object;

	return *this;
}
