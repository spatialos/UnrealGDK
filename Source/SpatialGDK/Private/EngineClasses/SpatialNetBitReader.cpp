// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialNetBitReader.h"

#include "SpatialPackageMapClient.h"
#include "WeakObjectPtr.h"
#include "SchemaHelpers.h"

void FSpatialNetBitReader::DeserializeObjectRef(UnrealObjectRef& ObjectRef)
{
	*this << ObjectRef.Entity;
	*this << ObjectRef.Offset;

	uint8 HasPath;
	SerializeBits(&HasPath, 1);
	if (HasPath)
	{
		FString Path;
		*this << Path;

		ObjectRef.Path = std::string(TCHAR_TO_UTF8(*Path));
	}

	uint8 HasOuter;
	SerializeBits(&HasOuter, 1);
	if (HasOuter)
	{
		ObjectRef.Outer = UnrealObjectRef();
		DeserializeObjectRef(*ObjectRef.Outer);
	}
}

FArchive& FSpatialNetBitReader::operator<<(UObject*& Value)
{
	UnrealObjectRef ObjectRef;

	DeserializeObjectRef(ObjectRef);

	check(ObjectRef != UNRESOLVED_OBJECT_REF);
	if (ObjectRef == NULL_OBJECT_REF)
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
