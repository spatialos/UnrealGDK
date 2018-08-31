#pragma once

#include "Platform.h"
#include "RepLayout.h"

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

		check(Cmd.Type != REPCMD_Return);

		if (Cmd.Type == REPCMD_DynamicArray)
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
	TArray<FRepParentCmd>& Parents = RepLayout.Parents;
	for (int32 i = 0; i < Parents.Num(); i++)
	{
		RepLayout_SerializeProperties(RepLayout, Ar, Map, Parents[i].CmdStart, Parents[i].CmdEnd, Data, bHasUnmapped);

		if (Ar.IsError())
		{
			return;
		}
	}
}

inline void RepLayout_SendPropertiesForRPC(FRepLayout& RepLayout, FNetBitWriter& Writer, void* Data)
{
	TArray<FRepParentCmd>& Parents = RepLayout.Parents;

	for (int32 i = 0; i < Parents.Num(); i++)
	{
		bool Send = true;

		if (!Cast<UBoolProperty>(Parents[i].Property))
		{
			// check for a complete match, including arrays
			// (we're comparing against zero data here, since 
			// that's the default.)
			Send = !Parents[i].Property->Identical_InContainer(Data, NULL, Parents[i].ArrayIndex);

			Writer.WriteBit(Send ? 1 : 0);
		}

		if (Send)
		{
			bool bHasUnmapped = false;
			RepLayout_SerializeProperties(RepLayout, Writer, Writer.PackageMap, Parents[i].CmdStart, Parents[i].CmdEnd, Data, bHasUnmapped);
		}
	}
}

inline void RepLayout_ReceivePropertiesForRPC(FRepLayout& RepLayout, FNetBitReader& Reader, void* Data)
{
	TArray<FRepParentCmd>& Parents = RepLayout.Parents;

	for (int32 i = 0; i < Parents.Num(); i++)
	{
		if (Parents[i].ArrayIndex == 0 && (Parents[i].Property->PropertyFlags & CPF_ZeroConstructor) == 0)
		{
			// If this property needs to be constructed, make sure we do that
			Parents[i].Property->InitializeValue((uint8*)Data + Parents[i].Property->GetOffset_ForUFunction());
		}
	}

	for (int32 i = 0; i < Parents.Num(); i++)
	{
		if (Cast<UBoolProperty>(Parents[i].Property) || Reader.ReadBit())
		{
			bool bHasUnmapped = false;

			RepLayout_SerializeProperties(RepLayout, Reader, Reader.PackageMap, Parents[i].CmdStart, Parents[i].CmdEnd, Data, bHasUnmapped);

			if (Reader.IsError())
			{
				return;
			}

			if (bHasUnmapped)
			{
				//UE_LOG(LogTemp, Log, TEXT("Unable to resolve RPC parameter. Object[%d] %s. Function %s. Parameter %s."),
				//	Channel->ChIndex, *Object->GetName(), *Function->GetName(), *Parents[i].Property->GetName());
			}
		}
	}
}
