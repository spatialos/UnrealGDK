// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialActorChannel.h"
#include "SpatialNetDriver.h"
#include "Engine/NetConnection.h"
#include "Net/DataBunch.h"

#include "Generated/SpatialShadowActor_Character.h"

// We assume that #define ENABLE_PROPERTY_CHECKSUMS exists in RepLayout.cpp:88 here.
#define ENABLE_PROPERTY_CHECKSUMS

USpatialActorChannel::USpatialActorChannel(const FObjectInitializer & objectInitializer /*= FObjectInitializer::Get()*/)
	: Super(objectInitializer)
{
	ChType = CHTYPE_Actor;
}

void USpatialActorChannel::Init(UNetConnection* connection, int32 channelIndex, bool bOpenedLocally)
{
	UActorChannel::Init(connection, channelIndex, bOpenedLocally);
}

void USpatialActorChannel::SetClosingFlag()
{
	UActorChannel::SetClosingFlag();
}

void USpatialActorChannel::Close()
{
	UActorChannel::Close();
}

void USpatialActorChannel::ReceivedBunch(FInBunch &Bunch)
{
	UActorChannel::ReceivedBunch(Bunch);
}

void USpatialActorChannel::ReceivedNak(int32 PacketId)
{
	UActorChannel::ReceivedNak(PacketId);
}

void USpatialActorChannel::Tick()
{
	UActorChannel::Tick();
}

bool USpatialActorChannel::CanStopTicking() const
{
	return UActorChannel::CanStopTicking();
}

void USpatialActorChannel::AppendExportBunches(TArray<FOutBunch *> & outExportBunches)
{
	UActorChannel::AppendExportBunches(outExportBunches);
}

void USpatialActorChannel::AppendMustBeMappedGuids(FOutBunch * bunch)
{
	UActorChannel::AppendMustBeMappedGuids(bunch);
}

// TODO: Epic should expose this.
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

static bool ParseHeader(FNetBitReader& Bunch, bool bIsServer, bool& bHasRepLayout, bool& bIsActor)
{
	bHasRepLayout = Bunch.ReadBit() != 0;
	bIsActor = Bunch.ReadBit() != 0;
	if (!bIsActor)
	{
		// For now, just give up.
		return false;

		ParseHeader_Subobject(Bunch);
		if (!bIsServer)
		{
			// bStablyNamed.
			Bunch.ReadBit();
		}
	}
	// If we have enough space to read another byte, then we should continue.
	return (Bunch.GetPosBits() + 8 <= Bunch.GetNumBits());
}

FPacketIdRange USpatialActorChannel::SendBunch(FOutBunch * BunchPtr, bool bMerge)
{
	FNetBitReader BunchReader(nullptr, BunchPtr->GetData(), BunchPtr->GetNumBits());

	// Run default actor channel code.
	auto ReturnValue = UActorChannel::SendBunch(BunchPtr, bMerge);

	// Read bunch.
	bool bHasRepLayout;
	bool bIsActor;
	if (!ParseHeader(BunchReader, Connection->Driver->IsServer(), bHasRepLayout, bIsActor))
	{
		return ReturnValue;
	}

	// Get payload data.
	uint32 PayloadBitCount;
	BunchReader.SerializeIntPacked(PayloadBitCount);
	FNetBitReader Reader;
	Reader.SetData(BunchReader, PayloadBitCount);

	if (!bHasRepLayout)
	{
		return ReturnValue;
	}

	UE_LOG(LogTemp, Log, TEXT("-> Header: HasRepLayout %d IsActor %d PayloadBitCount %d"), (int)bHasRepLayout, (int)bIsActor, (int)PayloadBitCount);

	// Get SpatialNetDriver.
	USpatialNetDriver* Driver = Cast<USpatialNetDriver>(Connection->Driver);
	if (!Driver->ShadowActorPipelineBlock)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpatialOS is not connected yet."));
		return ReturnValue;
	}

	// Get entity ID.
	// TODO: Replace with EntityId from registry corresponding to this replicated actor.
	FEntityId EntityId = 2;//EntityRegistry->GetEntityIdFromActor(ActorChannel->Actor);
	if (EntityId == FEntityId())
	{
		return ReturnValue;
	}

	// Get shadow actor.
	ASpatialShadowActor_Character* ShadowActor = Cast<ASpatialShadowActor_Character>(Driver->ShadowActorPipelineBlock->GetShadowActor(EntityId));
	if (!ShadowActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("Actor channel has no corresponding shadow actor. That means the entity hasn't been checked out yet."));
		return ReturnValue;
	}

	// Parse payload.
#ifdef ENABLE_PROPERTY_CHECKSUMS
	bool bDoChecksum = Reader.ReadBit() != 0;
#else
	bool bDoChecksum = false;
#endif
	while (Reader.GetBitsLeft() > 0)
	{
		uint32 Handle;
		Reader.SerializeIntPacked(Handle);
		if (bDoChecksum)
		{
			// Skip checksum bits.
			uint32 Checksum;
			Reader << Checksum;
		}

		// Ignore null terminator handle.
		if (Handle == 0)
		{
			break;
		}

		// Get property and apply update to SpatialOS.
		auto PropertyMap = ShadowActor->GetHandlePropertyMap();
		UProperty* Property = PropertyMap[Handle].Property;
		UE_LOG(LogTemp, Log, TEXT("-> Handle: %d Property %s"), Handle, *Property->GetName());
		ShadowActor->ApplyUpdateToSpatial(Reader, Handle, Property);

		// TODO: Currently, object references are not handled properly in ApplyUpdateToSpatial.
		if (Property->IsA(UObjectPropertyBase::StaticClass()))
		{
			UE_LOG(LogTemp, Warning, TEXT("-> Skipping object reference."));
			break;
		}

		// Skip checksum bits.
		if (bDoChecksum)
		{
			// Skip.
			uint32 Checksum;
			Reader << Checksum;
			if (!Property->IsA(UObjectPropertyBase::StaticClass()))
			{
				Reader << Checksum;
			}
		}
	}

	return ReturnValue;
}

void USpatialActorChannel::StartBecomingDormant()
{
	UActorChannel::StartBecomingDormant();
}

void USpatialActorChannel::BecomeDormant()
{
	UActorChannel::BecomeDormant();
}

bool USpatialActorChannel::CleanUp(const bool bForDestroy)
{
	return UActorChannel::CleanUp(bForDestroy);
}

void USpatialActorChannel::SpatialReceivePropertyUpdate(FNetBitWriter& Payload)
{
	FNetBitWriter OutBunchData(nullptr, 0);
	
	// Write header.
	OutBunchData.WriteBit(1); // bHasRepLayout
	OutBunchData.WriteBit(1); // bIsActor

	// Add null terminator to payload.
	uint32 Terminator = 0;
	Payload.SerializeIntPacked(Terminator);

	// Write property info.
	uint32 PayloadSize = Payload.GetNumBits() + 1; // extra bit for bDoChecksum
	OutBunchData.SerializeIntPacked(PayloadSize);
#ifdef ENABLE_PROPERTY_CHECKSUMS
	OutBunchData.WriteBit(0); // bDoChecksum
#endif
	OutBunchData.SerializeBits(Payload.GetData(), Payload.GetNumBits());

	FInBunch Bunch(Connection, OutBunchData.GetData(), OutBunchData.GetNumBits());
	Bunch.ChIndex = ChIndex;
	Bunch.bHasMustBeMappedGUIDs = false;
	Bunch.bIsReplicationPaused = false;
	ReceivedBunch(Bunch);
}
