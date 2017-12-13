// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialActorChannel.h"
#include "SpatialNetDriver.h"
#include "Engine/NetConnection.h"
#include "Net/DataBunch.h"
#include "Engine/PackageMapClient.h"
#include "SpatialNetConnection.h"
#include "SpatialOS.h"

#include "Utils/BunchReader.h"

#include "Generated/SpatialUpdateInterop_Character.h"

#include <improbable/standard_library.h>

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
	// If not a client, just call normal behaviour.
	if (!Connection->Driver->ServerConnection)
	{
		UActorChannel::ReceivedBunch(Bunch);
		return;
	}

	// Parse the bunch, and let through all non-replicated stuff, and replicated stuff that matches a few properties.
	auto& PropertyMap = GetHandlePropertyMap_Character();
	FBunchReader BunchReader(Bunch.GetData(), Bunch.GetNumBits());
	FBunchReader::RepDataHandler RepDataHandler = [](FNetBitReader& Reader, UPackageMap* PackageMap, int32 Handle, UProperty* Property) -> bool
	{
		// TODO: We can't parse UObjects or FNames here as we have no package map.
		if (Property->IsA(UObjectPropertyBase::StaticClass()) || Property->IsA(UNameProperty::StaticClass()))
		{
			UE_LOG(LogTemp, Warning, TEXT("<- Allowing through UObject/FName update."));
			return false;
		}

		// Filter Role (13) and RemoteRole (4)
		if (Handle == 13)
		{
			UE_LOG(LogTemp, Warning, TEXT("<- Allowing through Role update."));
			return false;
		}
		if (Handle == 4)
		{
			UE_LOG(LogTemp, Warning, TEXT("<- Allowing through RemoteRole update."));
			return false;
		}

		// Skip bytes.
		// Property data size: RepLayout.cpp:237
		TArray<uint8> PropertyData;
		PropertyData.AddZeroed(Property->ElementSize);
		Property->InitializeValue(PropertyData.GetData());
		Property->NetSerializeItem(Reader, PackageMap, PropertyData.GetData());
		return true;
	};
	BunchReader.Parse(Connection->Driver->IsServer(), nullptr, PropertyMap, RepDataHandler);
	if (BunchReader.HasError())
	{
		//UE_LOG(LogTemp, Warning, TEXT("<- Allowing through non actor bunch."));
		UActorChannel::ReceivedBunch(Bunch);
	}
	else if (!BunchReader.HasRepLayout() || !BunchReader.IsActor())
	{
		//UE_LOG(LogTemp, Warning, TEXT("<- Allowing through non replicated bunch."));
		UActorChannel::ReceivedBunch(Bunch);
	}
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

FPacketIdRange USpatialActorChannel::SendBunch(FOutBunch * BunchPtr, bool bMerge)
{
	// Run default actor channel code.
	// TODO(David) I believe that ReturnValue will always contain a PacketId which has the same First and Last value
	// if we don't break up bunches, which in our case we wont as there's no need to at this layer.
	auto ReturnValue = UActorChannel::SendBunch(BunchPtr, bMerge);
	
	// Get SpatialNetDriver.
	USpatialNetDriver* Driver = Cast<USpatialNetDriver>(Connection->Driver);
	TSharedPtr<worker::Connection> WorkerConnection = Driver->GetSpatialOS()->GetConnection().Pin();
	if (!WorkerConnection.Get() || !WorkerConnection->IsConnected())
	{
		UE_LOG(LogTemp, Warning, TEXT("SpatialOS is not connected yet."));
		return ReturnValue;
	}

	// Build SpatialOS update.
	improbable::unreal::UnrealCharacterReplicatedData::Update SpatialUpdate;
	auto& PropertyMap = GetHandlePropertyMap_Character();
	FBunchReader BunchReader(BunchPtr->GetData(), BunchPtr->GetNumBits());
	FBunchReader::RepDataHandler RepDataHandler = [&SpatialUpdate](FNetBitReader& Reader, UPackageMap* PackageMap, int32 Handle, UProperty* Property) -> bool
	{
		// TODO: We can't parse UObjects or FNames here as we have no package map.
		if (Property->IsA(UObjectPropertyBase::StaticClass()) || Property->IsA(UNameProperty::StaticClass()))
		{
			return false;
		}

		// Filter Role (13) and RemoteRole (4)
		if (Handle == 13 || Handle == 4)
		{
			return false;
		}

		UE_LOG(LogTemp, Log, TEXT("-> Handle: %d Property %s"), Handle, *Property->GetName());
		ApplyUpdateToSpatial_Character(Reader, Handle, Property, SpatialUpdate);

		return true;
	};
	BunchReader.Parse(Driver->IsServer(), nullptr, PropertyMap, RepDataHandler);

	FNetworkGUID NetGUID = Connection->PackageMap->GetNetGUIDFromObject(Actor);
	/*
	UE_LOG(LogTemp, Warning, TEXT("Actor: %s NetGUID: %s. RepLayout %d IsActor %d Return value: %d %d"),
		*Actor->GetName(),
		*NetGUID.ToString(),
		(int)BunchReader.HasRepLayout(),
		(int)BunchReader.IsActor(),
		ReturnValue.First,
		ReturnValue.Last);
		*/

	// TODO(david): Hacks :(
	worker::EntityId EntityId;
	if (NetGUID.Value == 6)
	{
		EntityId = {2};
	}
	else if (NetGUID.Value == 12)
	{
		EntityId = {3};
	}
	else
	{
		return ReturnValue;
	}

	// Send SpatialOS update.
	if (!BunchReader.HasError() && BunchReader.HasRepLayout())
	{
		WorkerConnection->SendComponentUpdate<improbable::unreal::UnrealCharacterReplicatedData>(EntityId, SpatialUpdate);
	}
	else
	{

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
	UActorChannel::ReceivedBunch(Bunch);
}
