// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/SpatialFastArrayNetSerialize.h"

#include "EngineClasses/SpatialNetBitReader.h"
#include "EngineClasses/SpatialNetBitWriter.h"

namespace SpatialGDK
{

bool FSpatialNetDeltaSerializeInfo::DeltaSerializeRead(USpatialNetDriver* NetDriver, FSpatialNetBitReader& Reader, UObject* Object, int32 ArrayIndex, UProperty* ParentProperty, UScriptStruct* NetDeltaStruct)
{
	FSpatialNetDeltaSerializeInfo NetDeltaInfo;

	SpatialFastArrayNetSerializeCB SerializeCB(NetDriver);

	NetDeltaInfo.Reader = &Reader;
	NetDeltaInfo.Map = Reader.PackageMap;
	NetDeltaInfo.NetSerializeCB = &SerializeCB;

	UStructProperty* ParentStruct = Cast<UStructProperty>(ParentProperty);
	check(ParentStruct);
	void* Destination = ParentStruct->ContainerPtrToValuePtr<void>(Object, ArrayIndex);

	UScriptStruct::ICppStructOps* CppStructOps = NetDeltaStruct->GetCppStructOps();
	check(CppStructOps);

	return CppStructOps->NetDeltaSerialize(NetDeltaInfo, Destination);
}

bool FSpatialNetDeltaSerializeInfo::DeltaSerializeWrite(USpatialNetDriver* NetDriver, FSpatialNetBitWriter& Writer, UObject* Object, int32 ArrayIndex, UProperty* ParentProperty, UScriptStruct* NetDeltaStruct)
{
	FSpatialNetDeltaSerializeInfo NetDeltaInfo;

	SpatialFastArrayNetSerializeCB SerializeCB(NetDriver);

	NetDeltaInfo.Writer = &Writer;
	NetDeltaInfo.Map = Writer.PackageMap;
	NetDeltaInfo.NetSerializeCB = &SerializeCB;

	UStructProperty* ParentStruct = Cast<UStructProperty>(ParentProperty);
	check(ParentStruct);
	void* Source = ParentStruct->ContainerPtrToValuePtr<void>(Object, ArrayIndex);

	UScriptStruct::ICppStructOps* CppStructOps = NetDeltaStruct->GetCppStructOps();
	check(CppStructOps);

	return CppStructOps->NetDeltaSerialize(NetDeltaInfo, Source);
}

void SpatialFastArrayNetSerializeCB::NetSerializeStruct(UScriptStruct* Struct, FBitArchive& Ar, UPackageMap* PackageMap, void* Data, bool& bHasUnmapped)
{
	// Check if struct has custom NetSerialize function, otherwise call standard struct replication
	if (Struct->StructFlags & STRUCT_NetSerializeNative)
	{
		UScriptStruct::ICppStructOps* CppStructOps = Struct->GetCppStructOps();
		check(CppStructOps); // else should not have STRUCT_NetSerializeNative
		bool bSuccess = true;
		if (!CppStructOps->NetSerialize(Ar, PackageMap, bSuccess, reinterpret_cast<uint8*>(Data)))
		{
			bHasUnmapped = true;
		}

		// Check the success of the serialization and print a warning if it failed. This is how native handles failed serialization.
		if (!bSuccess)
		{
			UE_LOG(LogSpatialNetSerialize, Warning, TEXT("SpatialFastArrayNetSerialize: NetSerialize %s failed."), *Struct->GetFullName());
		}
	}
	else
	{
		TSharedPtr<FRepLayout> RepLayout = NetDriver->GetStructRepLayout(Struct);

		RepLayout_SerializePropertiesForStruct(*RepLayout, Ar, PackageMap, reinterpret_cast<uint8*>(Data), bHasUnmapped);
	}
}

} // namespace SpatialGDK
