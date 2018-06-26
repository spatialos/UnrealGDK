// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialMemoryReader.h"

#include "SpatialPackageMapClient.h"
#include "WeakObjectPtr.h"

void FSpatialMemoryReader::DeserializeObjectRef(improbable::unreal::UnrealObjectRef& ObjectRef)
{
	*this << ObjectRef.entity();
	*this << ObjectRef.offset();

	bool bHasPath;
	*this << bHasPath;
	if (bHasPath)
	{
		FString Path;
		*this << Path;

		ObjectRef.path() = std::string(TCHAR_TO_UTF8(*Path));
	}

	bool bHasOuter;
	*this << bHasOuter;
	if (bHasOuter)
	{
		improbable::unreal::UnrealObjectRef Outer;
		DeserializeObjectRef(Outer);

		ObjectRef.outer() = Outer;
	}
}

FArchive& FSpatialMemoryReader::operator<<(UObject*& Value)
{
	improbable::unreal::UnrealObjectRef ObjectRef;

	DeserializeObjectRef(ObjectRef);

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
			// TODO: Handle unresolved object refs on the receiving end
			Value = nullptr;
		}
	}

	return *this;
}

FArchive& FSpatialMemoryReader::operator<<(FWeakObjectPtr& Value)
{
	UObject* Object;
	*this << Object;

	Value = Object;

	return *this;
}
