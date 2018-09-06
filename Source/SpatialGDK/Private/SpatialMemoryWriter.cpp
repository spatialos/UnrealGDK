// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialMemoryWriter.h"

#include "SpatialPackageMapClient.h"
#include "WeakObjectPtr.h"

void FSpatialMemoryWriter::SerializeObjectRef(improbable::unreal::UnrealObjectRef& ObjectRef)
{
	*this << ObjectRef.entity();
	*this << ObjectRef.offset();

	uint8 HasPath = !ObjectRef.path().empty();
	SerializeBits(&HasPath, 1);
	if (HasPath)
	{
		FString Path = FString(UTF8_TO_TCHAR(ObjectRef.path()->c_str()));
		*this << Path;
	}

	uint8 HasOuter = !ObjectRef.outer().empty();
	SerializeBits(&HasOuter, 1);
	if (HasOuter)
	{
		SerializeObjectRef(*ObjectRef.outer());
	}
}

FArchive& FSpatialMemoryWriter::operator<<(UObject*& Value)
{
	improbable::unreal::UnrealObjectRef ObjectRef;
	if (Value != nullptr)
	{
		FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromObject(Value);
		if (!NetGUID.IsValid() && Value->IsFullNameStableForNetworking())
		{
			NetGUID = PackageMap->ResolveStablyNamedObject(Value);
		}
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
