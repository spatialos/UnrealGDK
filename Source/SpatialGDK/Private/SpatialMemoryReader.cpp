// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialMemoryReader.h"

#include "SpatialPackageMapClient.h"

FArchive& FSpatialMemoryReader::operator<<(UObject*& Value)
{
	improbable::unreal::UnrealObjectRef ObjectRef;

	*this << ObjectRef.entity();
	*this << ObjectRef.offset();

	check(ObjectRef != SpatialConstants::UNRESOLVED_OBJECT_REF);
	if (ObjectRef == SpatialConstants::NULL_OBJECT_REF)
	{
		Value = nullptr;
	}
	else
	{
		FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(ObjectRef);
		if (NetGUID.IsValid())
		{
			Value = PackageMap->GetObjectFromNetGUID(NetGUID, true);
			checkf(Value, TEXT("An object ref %s should map to a valid object."), *ObjectRefToString(ObjectRef));
		}
		else
		{
			UnresolvedObjectRefs.Add(ObjectRef);
			Value = nullptr;
		}
	}

	return *this;
}
