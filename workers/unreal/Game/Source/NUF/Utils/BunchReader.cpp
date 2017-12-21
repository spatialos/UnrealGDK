// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "BunchReader.h"
#include "Net/DataBunch.h"
#include "Engine/PackageMapClient.h"

// We assume that #define ENABLE_PROPERTY_CHECKSUMS exists in RepLayout.cpp:88 here.
#define ENABLE_PROPERTY_CHECKSUMS
// This is assumed to match PackageMapClient.cpp:35
#define INTERNAL_LOAD_OBJECT_RECURSION_LIMIT 16

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

static void ReadHeaderSubobject(FNetBitReader& Bunch, int RecursionCount)
{
	// This function heavily mirrors UPackageMapClient::InternalLoadObject, which is called
	// by UPackageMapClient::SerializeObject.
	if (RecursionCount > INTERNAL_LOAD_OBJECT_RECURSION_LIMIT)
	{
		return;
	}

	FNetworkGUID NetGUID;
	FExportFlags ExportFlags;

	Bunch << NetGUID; // UE4.16 PackageMapClient.cpp:702
	if (!NetGUID.IsValid())
	{
		return;
	}

	//UE_LOG(LogTemp, Log, TEXT("Header: Subobject NetGUID %s"), *NetGUID.ToString());

	// Attempt to resolve NetGUID.
	if (NetGUID.IsValid() && !NetGUID.IsDefault()) // UE4.16 PackageMapClient.cpp:720
	{
		// PackageMapClient->GetObjectFromNetGUID(NetGUID)
		// TODO(David): I guess this would just read from this clients "local" NetGUID cache.
	}

	// Read NetGUID in full.
	if (NetGUID.IsDefault())
	{
		Bunch << ExportFlags.Value; // UE4.16 PackageMapClient.cpp:732
	}
	if (ExportFlags.bHasPath)
	{
		ReadHeaderSubobject(Bunch, RecursionCount + 1); // UE4.16 PackageMapClient.cpp:752

		FString PathName;
		uint32 NetworkChecksum;

		Bunch << PathName; // UE4.16 PackageMapClient.cpp:757
		if (ExportFlags.bHasNetworkChecksum)
		{
			Bunch << NetworkChecksum; // UE4.16 PackageMapClient.cpp:771
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
	// This function heavily mirrors UActorChannel::ReadContentBlockHeader.
	bHasRepLayout = Bunch.ReadBit() != 0; // UE4.16 DataChannel.cpp:2765
	bIsActor = Bunch.ReadBit() != 0; // UE4.16 DataChannel.cpp:2773
	if (bIsActor) // UE4.16 DataChannel.cpp:2781
	{
		return;
	}

	ReadHeaderSubobject(Bunch, 0); // UE4.16 DataChannel.cpp:2796
	if (bIsServer) // UE4.16 DataChannel.cpp:2838
	{
		return;
	}

	const bool bStablyNamed = Bunch.ReadBit() != 0; // UE4.16 DataChannel.cpp:2851
	if (bStablyNamed) // UE4.16 DataChannel.cpp:2859
	{
		return;
	}
	ReadHeaderSubobject(Bunch, 0); // UE4.16 DataChannel.cpp:2881
}

bool FBunchReader::Parse(bool bIsServer, UPackageMap* PackageMap, const FRepHandlePropertyMap& PropertyMap, RepDataHandler RepDataHandlerFunc)
{
	// Parse header.
	ReadHeader(bIsServer);
	if (HasError())
	{
		return false;
	}

	//UE_LOG(LogTemp, Warning, TEXT("Header: HasRepLayout %d IsActor %d"), (int)bHasRepLayout, (int)bIsActor);

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
