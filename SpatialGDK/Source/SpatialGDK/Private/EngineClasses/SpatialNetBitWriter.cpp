// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/SpatialNetBitWriter.h"

#include "UObject/WeakObjectPtr.h"

#include "EngineClasses/SpatialPackageMapClient.h"
#include "Schema/UnrealObjectRef.h"
#include "SpatialConstants.h"

FSpatialNetBitWriter::FSpatialNetBitWriter(USpatialPackageMapClient* InPackageMap, TSet<TWeakObjectPtr<const UObject>>& InUnresolvedObjects)
	: FNetBitWriter(InPackageMap, 0)
	, UnresolvedObjects(InUnresolvedObjects)
{}

void FSpatialNetBitWriter::SerializeObjectRef(FUnrealObjectRef& ObjectRef)
{
	*this << ObjectRef.Entity;
	*this << ObjectRef.Offset;

	uint8 HasPath = ObjectRef.Path.IsSet();
	SerializeBits(&HasPath, 1);
	if (HasPath)
	{
		*this << ObjectRef.Path.GetValue();
	}

	uint8 HasOuter = ObjectRef.Outer.IsSet();
	SerializeBits(&HasOuter, 1);
	if (HasOuter)
	{
		SerializeObjectRef(*ObjectRef.Outer);
	}
}

FArchive& FSpatialNetBitWriter::operator<<(UObject*& Value)
{
	FUnrealObjectRef ObjectRef;
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
		ObjectRef = FUnrealObjectRef(PackageMapClient->GetUnrealObjectRefFromNetGUID(NetGUID));
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

FArchive& FSpatialNetBitWriter::operator<<(FWeakObjectPtr& Value)
{
	UObject* Object = Value.Get(true);
	*this << Object;

	return *this;
}
