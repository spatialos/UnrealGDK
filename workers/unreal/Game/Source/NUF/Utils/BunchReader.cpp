// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "BunchReader.h"
#include "Net/DataBunch.h"
#include "Engine/PackageMapClient.h"

// We assume that #define ENABLE_PROPERTY_CHECKSUMS exists in RepLayout.cpp:88 here.
#define ENABLE_PROPERTY_CHECKSUMS

namespace
{
struct FExportFlags
{
	union
	{
		struct
		{
			uint8 bHasPath : 1;
			uint8 bNoLoad : 1;
			uint8 bHasNetworkChecksum : 1;
		};

		uint8	Value;
	};

	FExportFlags()
	{
		Value = 0;
	}
};

static void ParseHeader_Subobject(FNetBitReader& Bunch)
{
	FNetworkGUID NetGUID;
	FExportFlags ExportFlags;
	Bunch << NetGUID;
	Bunch << ExportFlags.Value;
	if (ExportFlags.bHasPath)
	{
		ParseHeader_Subobject(Bunch);

		FString PathName;
		Bunch << PathName;
		uint32 NetworkChecksum;
		if (ExportFlags.bHasNetworkChecksum)
		{
			Bunch << NetworkChecksum;
		}
	}
}
} // ::

FBunchReader::FBunchReader(uint8* Data, int NumBits) :
	bError(false),
	bHasRepLayout(false),
	bIsActor(false),
	Bunch(nullptr, Data, NumBits)
{
}

void FBunchReader::ReadHeader(bool bIsServer)
{
	bHasRepLayout = Bunch.ReadBit() != 0;
	bIsActor = Bunch.ReadBit() != 0;
	if (!bIsActor)
	{
		// For now, just give up.
		//bError = true;
		return;

		ParseHeader_Subobject(Bunch);
		if (!bIsServer)
		{
			// bStablyNamed.
			Bunch.ReadBit();
		}
	}
}

bool FBunchReader::Parse(bool bIsServer, UPackageMap* PackageMap, const TMap<int32, RepHandleData>& PropertyMap, RepDataHandler RepDataHandlerFunc)
{
	// Parse header.
	ReadHeader(bIsServer);
	if (HasError())
	{
		return false;
	}

	//UE_LOG(LogTemp, Log, TEXT("Header: HasRepLayout %d IsActor %d"), (int)bHasRepLayout, (int)bIsActor);

	// If this bunch has subobject info, quit as we didn't parse the header properly yet.
	if (!bIsActor)
	{
		return false;
	}

	// If this bunch is not a replicated bunch, finish parsing here.
	if (!bHasRepLayout)
	{
		return true;
	}

	// Get payload data.
	uint32 PayloadBitCount;
	Bunch.SerializeIntPacked(PayloadBitCount);
	FNetBitReader Payload;
	Payload.SetData(Bunch, PayloadBitCount);

	// Read replicated properties.
#ifdef ENABLE_PROPERTY_CHECKSUMS
	bool bDoChecksum = Payload.ReadBit() != 0;
#else
	bool bDoChecksum = false;
#endif
	while (Payload.GetBitsLeft() > 0)
	{
		uint32 Handle;
		Payload.SerializeIntPacked(Handle);
		if (bDoChecksum)
		{
			// Skip checksum bits.
			uint32 Checksum;
			Payload << Checksum;
		}

		// Ignore null terminator handle.
		if (Handle == 0)
		{
			break;
		}

		// Look up property and call replicated data handler function.
		if (Handle >= (uint32)PropertyMap.Num())
		{
			UE_LOG(LogTemp, Error, TEXT("BunchReader: Read invalid handle %d."), Handle);
			break;
		}
		UProperty* Property = PropertyMap[Handle].Property;
		int32 BitsBeforeRead = Payload.GetPosBits();
		if (!RepDataHandlerFunc(Payload, PackageMap, Handle, Property))
		{
			bError = true;
			return false;
		}
		if (BitsBeforeRead == Payload.GetPosBits())
		{
			UE_LOG(LogTemp, Error, TEXT("BunchReader: Read no bits whilst parsing property %s (%d)."), *Property->GetNameCPP(), Handle);
			bError = true;
			return false;
		}

		// Skip checksum bits.
		if (bDoChecksum)
		{
			// Skip.
			uint32 Checksum;
			Payload << Checksum;
			if (!Property->IsA(UObjectPropertyBase::StaticClass()))
			{
				Payload << Checksum;
			}
		}
	}
	return true;
}

bool FBunchReader::HasError() const
{
	return bError;
}

bool FBunchReader::HasRepLayout() const
{
	return bHasRepLayout;
}

bool FBunchReader::IsActor() const
{
	return bIsActor;
}
