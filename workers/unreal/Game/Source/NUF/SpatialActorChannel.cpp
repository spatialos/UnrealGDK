// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialActorChannel.h"
#include "SpatialNetDriver.h"
#include "Engine/NetConnection.h"
#include "Net/DataBunch.h"
#include "Generated/SpatialInteropCharacter.h"

USpatialActorChannel::USpatialActorChannel(const FObjectInitializer & objectInitializer /*= FObjectInitializer::Get()*/)
	: Super(objectInitializer)
{
	ChannelClasses[CHTYPE_Actor] = GetClass();
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

void USpatialActorChannel::ReceivedBunch(FInBunch & bunch)
{
	UActorChannel::ReceivedBunch(bunch);
}

void USpatialActorChannel::ReceivedNak(int32 packetId)
{
	UActorChannel::ReceivedNak(packetId);
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

	// Get property map.
	// TODO: Store this in the shadow actor.
	auto PropertyMap = CreateHandleToPropertyMap_Character();

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

	// Parse payload.
	bool bDoChecksum = Reader.ReadBit() != 0;
	uint32 Handle;
	Reader.SerializeIntPacked(Handle);
	if (bDoChecksum)
	{
		uint32 Checksum;
		Reader << Checksum;
	}

	if (Handle == 0)
	{
		return ReturnValue;
	}

	UProperty* Property = PropertyMap[Handle].Property;
	UE_LOG(LogTemp, Log, TEXT("-> Handle: %d Property %s"), Handle, *Property->GetName());

	// Get SpatialNetDriver.
	USpatialNetDriver* Driver = Cast<USpatialNetDriver>(Connection->Driver);

	// Get entity ID.
	// TODO: Replace with EntityId from registry corresponding to this replicated actor.
	FEntityId EntityId = 2;//EntityRegistry->GetEntityIdFromActor(ActorChannel->Actor);
	if (EntityId == FEntityId())
	{
		return ReturnValue;
	}

	// Get shadow actor.
	ASpatialShadowActor* ShadowActor = Driver->ShadowActorPipelineBlock->GetShadowActor(EntityId);
	if (!ShadowActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("Actor channel has no corresponding shadow actor. That means the entity hasn't been checked out yet."));
		return ReturnValue;
	}

	ApplyUpdateToSpatial_Character(Reader, Handle, Property, ShadowActor->ReplicatedData);

	// Receive property
	if (bDoChecksum)
	{
		// Skip.
		//uint32 Checksum;
		//Reader << Checksum;
		//Reader << Checksum;
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
