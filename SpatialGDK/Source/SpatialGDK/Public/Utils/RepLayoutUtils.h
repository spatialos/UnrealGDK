// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "HAL/Platform.h"
#include "Net/RepLayout.h"

#include "EngineClasses/SpatialNetBitReader.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"

namespace improbable
{

void RepLayout_SerializeProperties(FRepLayout& RepLayout, FArchive& Ar, UPackageMap* Map, const int32 CmdStart, const int32 CmdEnd, void* Data, bool& bHasUnmapped);

inline void RepLayout_SerializeProperties_DynamicArray(FRepLayout& RepLayout, FArchive& Ar, UPackageMap* Map, const int32 CmdIndex, uint8* Data, bool& bHasUnmapped)
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
		RepLayout_SerializeProperties(RepLayout, Ar, Map, CmdIndex + 1, Cmd.EndCmd - 1, Data + i * Cmd.ElementSize, bHasUnmapped);
	}
}

inline void RepLayout_SerializeProperties(FRepLayout& RepLayout, FArchive& Ar, UPackageMap* Map, const int32 CmdStart, const int32 CmdEnd, void* Data, bool& bHasUnmapped)
{
	for (int32 CmdIndex = CmdStart; CmdIndex < CmdEnd && !Ar.IsError(); CmdIndex++)
	{
		const FRepLayoutCmd& Cmd = RepLayout.Cmds[CmdIndex];

		check(Cmd.Type != ERepLayoutCmdType::Return);

		if (Cmd.Type == ERepLayoutCmdType::DynamicArray)
		{
			RepLayout_SerializeProperties_DynamicArray(RepLayout, Ar, Map, CmdIndex, (uint8*)Data + Cmd.Offset, bHasUnmapped);
			CmdIndex = Cmd.EndCmd - 1;		// The -1 to handle the ++ in the for loop
			continue;
		}

		if (!Cmd.Property->NetSerializeItem(Ar, Map, (void*)((uint8*)Data + Cmd.Offset)))
		{
			bHasUnmapped = true;
		}
	}
}

inline void RepLayout_SerializePropertiesForStruct(FRepLayout& RepLayout, FArchive& Ar, UPackageMap* Map, void* Data, bool& bHasUnmapped)
{
	for (auto& Parent : RepLayout.Parents)
	{
		RepLayout_SerializeProperties(RepLayout, Ar, Map, Parent.CmdStart, Parent.CmdEnd, Data, bHasUnmapped);

		if (Ar.IsError())
		{
			return;
		}
	}
}

inline void RepLayout_SendPropertiesForRPC(FRepLayout& RepLayout, FNetBitWriter& Writer, void* Data)
{
	for (auto& Parent : RepLayout.Parents)
	{
		bool bSend = true;

		if (!Cast<UBoolProperty>(Parent.Property))
		{
			// check for a complete match, including arrays
			// (we're comparing against zero data here, since 
			// that's the default.)
			bSend = !Parent.Property->Identical_InContainer(Data, NULL, Parent.ArrayIndex);

			Writer.WriteBit(bSend ? 1 : 0);
		}

		if (bSend)
		{
			bool bHasUnmapped = false;
			RepLayout_SerializeProperties(RepLayout, Writer, Writer.PackageMap, Parent.CmdStart, Parent.CmdEnd, Data, bHasUnmapped);
		}
	}
}

inline void RepLayout_ReceivePropertiesForRPC(FRepLayout& RepLayout, FNetBitReader& Reader, void* Data)
{
	for (auto& Parent : RepLayout.Parents)
	{
		if (Parent.ArrayIndex == 0 && (Parent.Property->PropertyFlags & CPF_ZeroConstructor) == 0)
		{
			// If this property needs to be constructed, make sure we do that
			Parent.Property->InitializeValue((uint8*)Data + Parent.Property->GetOffset_ForUFunction());
		}
	}

	for (auto& Parent : RepLayout.Parents)
	{
		if (Cast<UBoolProperty>(Parent.Property) || Reader.ReadBit())
		{
			bool bHasUnmapped = false;

			RepLayout_SerializeProperties(RepLayout, Reader, Reader.PackageMap, Parent.CmdStart, Parent.CmdEnd, Data, bHasUnmapped);

			if (Reader.IsError())
			{
				return;
			}
		}
	}
}

inline void ReadStructProperty(FSpatialNetBitReader& Reader, UStructProperty* Property, USpatialNetDriver* NetDriver, uint8* Data, bool& bOutHasUnmapped)
{
	UScriptStruct* Struct = Property->Struct;

	if (Struct->StructFlags & STRUCT_NetSerializeNative)
	{
		UScriptStruct::ICppStructOps* CppStructOps = Struct->GetCppStructOps();
		check(CppStructOps); // else should not have STRUCT_NetSerializeNative
		bool bSuccess = true;
		if (!CppStructOps->NetSerialize(Reader, NetDriver->PackageMap, bSuccess, Data))
		{
			bOutHasUnmapped = true;
		}
		checkf(bSuccess, TEXT("NetSerialize on %s failed."), *Struct->GetStructCPPName());
	}
	else
	{
		TSharedPtr<FRepLayout> RepLayout = NetDriver->GetStructRepLayout(Struct);

		RepLayout_SerializePropertiesForStruct(*RepLayout, Reader, NetDriver->PackageMap, Data, bOutHasUnmapped);
	}
}

inline TArray<UFunction*> GetClassRPCFunctions(const UClass* Class)
{
	// Get all remote functions from the class. This includes parents super functions and child override functions.
	TArray<UFunction*> AllClassFunctions;

	for (TFieldIterator<UFunction> RemoteFunction(Class); RemoteFunction; ++RemoteFunction)
	{
		if (RemoteFunction->FunctionFlags & FUNC_NetClient ||
			RemoteFunction->FunctionFlags & FUNC_NetServer ||
			RemoteFunction->FunctionFlags & FUNC_NetCrossServer ||
			RemoteFunction->FunctionFlags & FUNC_NetMulticast)
		{
			AllClassFunctions.Add(*RemoteFunction);
		}
	}

	TArray<UFunction*> RelevantClassFunctions = AllClassFunctions;

	// Remove parent super functions from the class RPC list so we only use the overridden functions in this class.
	for (int i = 0; i < AllClassFunctions.Num(); i++)
	{
		UFunction* CurrentFunction = AllClassFunctions[i];

		for (int j = 0; j < AllClassFunctions.Num(); j++)
		{
			UFunction* PotentialParentFunction = AllClassFunctions[j];
			if (CurrentFunction->GetSuperFunction() == PotentialParentFunction)
			{
				// Remove the parent function.
				RelevantClassFunctions.Remove(PotentialParentFunction);
			}
		}
	}

	return RelevantClassFunctions;
}

}
