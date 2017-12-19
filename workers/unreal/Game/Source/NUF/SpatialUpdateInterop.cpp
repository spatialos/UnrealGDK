// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialUpdateInterop.h"
#include "SpatialOS.h"
#include "SpatialNetDriver.h"
#include "SpatialActorChannel.h"

#include "Utils/BunchReader.h"

#include "Engine/PackageMapClient.h"

#include "Generated/SpatialUpdateInterop_Character.h"

// We assume that #define ENABLE_PROPERTY_CHECKSUMS exists in RepLayout.cpp:88 here.
#define ENABLE_PROPERTY_CHECKSUMS

using improbable::unreal::UnrealCharacterSingleClientReplicatedData;
using improbable::unreal::UnrealCharacterMultiClientReplicatedData;
using improbable::unreal::UnrealCharacterCompleteData;

USpatialUpdateInterop::USpatialUpdateInterop()
{
}

void USpatialUpdateInterop::Init(bool bClient, USpatialOS* Instance, USpatialNetDriver* Driver)
{
	bIsClient = bClient;
	SpatialOSInstance = Instance;
	NetDriver = Driver;

	TSharedPtr<worker::View> View = SpatialOSInstance->GetView().Pin();
	ComponentUpdateCallback = View->OnComponentUpdate<UnrealCharacterSingleClientReplicatedData>([this](const worker::ComponentUpdateOp<UnrealCharacterSingleClientReplicatedData>& Op)
	{
		// TODO(David): Give this a package map.
		ReceiveUpdateFromSpatial_SingleClient_Character(this, nullptr, Op);
	});
	ComponentUpdateCallback = View->OnComponentUpdate<UnrealCharacterMultiClientReplicatedData>([this](const worker::ComponentUpdateOp<UnrealCharacterMultiClientReplicatedData>& Op)
	{
		// TODO(David): Give this a package map.
		ReceiveUpdateFromSpatial_MultiClient_Character(this, nullptr, Op);
	});
}

void USpatialUpdateInterop::Tick(float DeltaTime)
{
	// TODO: Remove the WaitingForGuid variable and all this logic once we actually integrate with Unreals actor system properly.
	// i.e. When Girays stuff is done.
	if (NetDriver && bIsClient)
	{
		// Actor representing the player in editor.
		AActor* Client1 = Cast<AActor>(NetDriver->GuidCache->GetObjectFromNetGUID(FNetworkGUID(6), false));
		if (Client1 && EntityToClientActorChannel.Find({2}) == nullptr)
		{
			USpatialActorChannel* ActorChannel = Cast<USpatialActorChannel>(NetDriver->ServerConnection->ActorChannels[Client1]);
			// We have the actor, wait for the Entity and role assignment.
			auto& Entities = SpatialOSInstance->GetView().Pin()->Entities;
			if (Client1->Role != ROLE_Authority && Entities.find({2}) != Entities.end() && ActorChannel)
			{
				UE_LOG(LogTemp, Warning, TEXT("Actor with NetGUID 6 and EntityID 2 created. Actor Role: %d"), (int)Client1->Role.GetValue());
				EntityToClientActorChannel.Add({2}, ActorChannel);
				SetComponentInterests(ActorChannel, {2});
			}
		}

		// Actor representing the player in the floating window.
		AActor* Client2 = Cast<AActor>(NetDriver->GuidCache->GetObjectFromNetGUID(FNetworkGUID(12), false));
		if (Client2 && EntityToClientActorChannel.Find({3}) == nullptr)
		{
			USpatialActorChannel* ActorChannel = Cast<USpatialActorChannel>(NetDriver->ServerConnection->ActorChannels[Client2]);
			// We have the actor, wait for the Entity.
			auto& Entities = SpatialOSInstance->GetView().Pin()->Entities;
			if (Client2->Role != ROLE_Authority && Entities.find({3}) != Entities.end() && ActorChannel)
			{
				UE_LOG(LogTemp, Warning, TEXT("Actor with NetGUID 12 and EntityID 3 created. Actor Role: %d"), (int)Client2->Role.GetValue());
				EntityToClientActorChannel.Add({3}, ActorChannel);
				SetComponentInterests(ActorChannel, {3});
			}
		}
	}
}

USpatialActorChannel* USpatialUpdateInterop::GetClientActorChannel(const worker::EntityId & EntityId) const
{
	// Get actor channel.
	USpatialActorChannel* const* ActorChannelIt = EntityToClientActorChannel.Find(EntityId);
	if (!ActorChannelIt)
	{
		// Can't find actor channel for this entity, give up.
		return nullptr;
	}
	return *ActorChannelIt;
}

void USpatialUpdateInterop::SendSpatialUpdate(USpatialActorChannel* Channel, FOutBunch* BunchPtr)
{
	if (!Channel->Actor->GetClass()->IsChildOf(ACharacter::StaticClass()))
	{
		//UE_LOG(LogTemp, Warning, TEXT("SpatialUpdateInterop: Trying to send Spatial update on unsupported class %s."),
		//	*Channel->Actor->GetClass()->GetName());
		return;
	}

	// Check that SpatialOS is connected.
	// TODO(David): This function should never get called until SpatialOS _is_ connected.
	TSharedPtr<worker::Connection> WorkerConnection = NetDriver->GetSpatialOS()->GetConnection().Pin();
	if (!WorkerConnection.Get() || !WorkerConnection->IsConnected())
	{
		UE_LOG(LogTemp, Warning, TEXT("SpatialOS is not connected yet."));
		return;
	}

	// Build SpatialOS updates.
	improbable::unreal::UnrealCharacterSingleClientReplicatedData::Update SingleClientUpdate;
	bool SingleClientUpdateChanged = false;
	improbable::unreal::UnrealCharacterMultiClientReplicatedData::Update MultiClientUpdate;
	bool MultiClientUpdateChanged = false;

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

		auto& Data = PropertyMap[Handle];
		UE_LOG(LogTemp, Log, TEXT("-> Handle: %d Property %s (group: %s)"), Handle, *Property->GetName(),
			GetGroupFromCondition(Data.Condition) == GROUP_SingleClient ? TEXT("Single") : TEXT("Multi"));
		if (GetGroupFromCondition(Data.Condition) == GROUP_SingleClient)
		{
			ApplyUpdateToSpatial_SingleClient_Character(Reader, Handle, Property, PackageMap, SingleClientUpdate);
			SingleClientUpdateChanged = true;
		}
		else
		{
			ApplyUpdateToSpatial_MultiClient_Character(Reader, Handle, Property, PackageMap, MultiClientUpdate);
			MultiClientUpdateChanged = true;
		}

		return true;
	};
	// TODO(David): Give this a package map.
	BunchReader.Parse(NetDriver->IsServer(), nullptr, PropertyMap, RepDataHandler);

	// Send SpatialOS update.
	worker::EntityId EntityId = Channel->GetEntityId();
	if (SingleClientUpdateChanged)
	{
		WorkerConnection->SendComponentUpdate<improbable::unreal::UnrealCharacterSingleClientReplicatedData>(EntityId, SingleClientUpdate);
	}
	if (MultiClientUpdateChanged)
	{
		WorkerConnection->SendComponentUpdate<improbable::unreal::UnrealCharacterMultiClientReplicatedData>(EntityId, MultiClientUpdate);
	}
}

void USpatialUpdateInterop::ReceiveSpatialUpdate(USpatialActorChannel* Channel, FNetBitWriter& Payload)
{
	// Build bunch data to send to the actor channel.
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

	// Create bunch and send to actor channel.
	FInBunch Bunch(Channel->Connection, OutBunchData.GetData(), OutBunchData.GetNumBits());
	Bunch.ChIndex = Channel->ChIndex;
	Bunch.bHasMustBeMappedGUIDs = false;
	Bunch.bIsReplicationPaused = false;
	Channel->UActorChannel::ReceivedBunch(Bunch);
}

void USpatialUpdateInterop::SetComponentInterests(USpatialActorChannel* ActorChannel, const worker::EntityId& EntityId)
{
	UClass* ActorClass = ActorChannel->Actor->GetClass();
	// Are we the autonomous proxy?
	if (ActorChannel->Actor->Role == ROLE_AutonomousProxy)
	{
		// We want to receive single client updates.
		worker::ComponentId ComponentId;
		if (ActorClass->IsChildOf(ACharacter::StaticClass()))
		{
			ComponentId = UnrealCharacterSingleClientReplicatedData::ComponentId;
		}
		// TODO: Support multiple actor types.
		worker::Map<worker::ComponentId, worker::InterestOverride> Interest;
		Interest.emplace(ComponentId, worker::InterestOverride{true});
		SpatialOSInstance->GetConnection().Pin()->SendComponentInterest(EntityId, Interest);
		UE_LOG(LogTemp, Warning, TEXT("We are the owning client, therefore we want single client updates. Client ID: %s"),
			*SpatialOSInstance->GetWorkerConfiguration().GetWorkerId());
	}
}
