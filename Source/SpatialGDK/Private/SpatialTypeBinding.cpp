// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTypeBinding.h"
#include "SpatialPackageMapClient.h"
#include "SpatialInterop.h"
#include "SpatialNetDriver.h"

void USpatialTypeBinding::Init(USpatialInterop* InInterop, USpatialPackageMapClient* InPackageMap)
{
	check(InInterop);
	check(InPackageMap);
	Interop = InInterop;
	PackageMap = InPackageMap;
}

void USpatialTypeBinding::SerializeStruct(UStruct* Struct, FArchive& Ar, void* Data) const
{
	TSharedPtr<FRepLayout> RepLayout = Interop->GetNetDriver()->GetStructRepLayout(Struct);
	bool bHasUnmapped = false;
	SerializePropertiesForStruct(*RepLayout.Get(), Ar, Data, bHasUnmapped);
}

void USpatialTypeBinding::SerializeProperties_DynamicArray(FRepLayout& RepLayout, FArchive& Ar, UPackageMap* Map, const int32 CmdIndex, uint8* Data, bool& bHasUnmapped) const
{
	const FRepLayoutCmd& Cmd = RepLayout.Cmds[CmdIndex];

	FScriptArray* Array = (FScriptArray*)Data;

	uint16 OutArrayNum = Array->Num();
	Ar << OutArrayNum;

	// If loading from the archive, OutArrayNum will contain the number of elements.
	// Otherwise, use the input number of elements.
	const int32 ArrayNum = Ar.IsLoading() ? (int32)OutArrayNum : Array->Num();

	// When loading, we may need to resize the array to properly fit the number of elements.
	if (Ar.IsLoading() && OutArrayNum != Array->Num())
	{
		FScriptArrayHelper ArrayHelper((UArrayProperty*)Cmd.Property, Data);
		ArrayHelper.Resize(OutArrayNum);
	}

	Data = (uint8*)Array->GetData();

	for (int32 i = 0; i < Array->Num() && !Ar.IsError(); i++)
	{
		SerializeProperties(RepLayout, Ar, Map, CmdIndex + 1, Cmd.EndCmd - 1, Data + i * Cmd.ElementSize, bHasUnmapped);
	}
}

void USpatialTypeBinding::SerializeProperties(FRepLayout& RepLayout, FArchive& Ar, UPackageMap* Map, const int32 CmdStart, const int32 CmdEnd, void* Data, bool& bHasUnmapped) const
{
	for (int32 CmdIndex = CmdStart; CmdIndex < CmdEnd && !Ar.IsError(); CmdIndex++)
	{
		const FRepLayoutCmd& Cmd = RepLayout.Cmds[CmdIndex];

		check(Cmd.Type != REPCMD_Return);

		if (Cmd.Type == REPCMD_DynamicArray)
		{
			SerializeProperties_DynamicArray(RepLayout, Ar, Map, CmdIndex, (uint8*)Data + Cmd.Offset, bHasUnmapped);
			CmdIndex = Cmd.EndCmd - 1;		// The -1 to handle the ++ in the for loop
			continue;
		}

		if (!Cmd.Property->NetSerializeItem(Ar, Map, (void*)((uint8*)Data + Cmd.Offset)))
		{
			bHasUnmapped = true;
		}
	}
}

void USpatialTypeBinding::SerializePropertiesForStruct(FRepLayout& RepLayout, FArchive& Ar, void* Data, bool& bHasUnmapped) const
{
	TArray<FRepParentCmd>& Parents = RepLayout.Parents;
	for (int32 i = 0; i < Parents.Num(); i++)
	{
		SerializeProperties(RepLayout, Ar, PackageMap, Parents[i].CmdStart, Parents[i].CmdEnd, Data, bHasUnmapped);

		if (Ar.IsError())
		{
			return;
		}
	}
}
