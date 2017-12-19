// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialActorChannel.h"
#include "SpatialNetDriver.h"
#include "Engine/NetConnection.h"
#include "Engine/PackageMapClient.h"
#include "SpatialNetConnection.h"
#include "SpatialOS.h"
#include "SpatialUpdateInterop.h"

#include "Utils/BunchReader.h"

// TODO(David): Required until we sever the unreal connection.
#include "Generated/SpatialUpdateInterop_Character.h"

USpatialActorChannel::USpatialActorChannel(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	: Super(ObjectInitializer)
{
	ChType = CHTYPE_Actor;
}

worker::EntityId USpatialActorChannel::GetEntityId() const
{
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
	if (NetGUID.Value == 6)
	{
		return {2};
	}
	else if (NetGUID.Value == 12)
	{
		return {3};
	}
	else
	{
		check(false);
		return {};
	}
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
	// TODO(david): Remove this when we sever the Unreal connection.
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

	// Read bunch and build up SpatialOS component updates.
	auto& PropertyMap = GetHandlePropertyMap_Character();
	FBunchReader BunchReader(BunchPtr->GetData(), BunchPtr->GetNumBits());
	FBunchReader::RepDataHandler RepDataHandler = [&](FNetBitReader& Reader, UPackageMap* PackageMap, int32 Handle, UProperty* Property) -> bool
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

		// Skip bytes.
		// Property data size: RepLayout.cpp:237
		TArray<uint8> PropertyData;
		PropertyData.AddZeroed(Property->ElementSize);
		Property->InitializeValue(PropertyData.GetData());
		Property->NetSerializeItem(Reader, PackageMap, PropertyData.GetData());
		return true;
	};
	BunchReader.Parse(true, nullptr, PropertyMap, RepDataHandler);

	// Pass bunch to update interop.
	USpatialUpdateInterop* UpdateInterop = Cast<USpatialNetDriver>(Connection->Driver)->GetSpatialUpdateInterop();
	if (!UpdateInterop)
	{
		UE_LOG(LogTemp, Error, TEXT("Update Interop object not set up. This should never happen"));
	}
	UpdateInterop->SendSpatialUpdate(this, BunchPtr);
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
