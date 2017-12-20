// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialUpdateInterop.h"
#include "SpatialOS.h"
#include "SpatialNetDriver.h"
#include "SpatialActorChannel.h"

#include "Utils/BunchReader.h"

#include "Engine/PackageMapClient.h"

#include "Generated/SpatialUpdateInterop_Character.h"
#include "Generated/SpatialUpdateInterop_PlayerController.h"

// We assume that #define ENABLE_PROPERTY_CHECKSUMS exists in RepLayout.cpp:88 here.
#define ENABLE_PROPERTY_CHECKSUMS

namespace {
void RegisterInterop_Character(worker::View* View, UPackageMap* PackageMap, USpatialUpdateInterop* UpdateInterop)
{
	FTypeBinding Binding;

	Binding.SingleClientComponentId = improbable::unreal::UnrealCharacterSingleClientReplicatedData::ComponentId;

	// Register update callbacks.
	Binding.SingleClientUpdateCallback = View->OnComponentUpdate<improbable::unreal::UnrealCharacterSingleClientReplicatedData>([PackageMap, UpdateInterop](
		const worker::ComponentUpdateOp<improbable::unreal::UnrealCharacterSingleClientReplicatedData>& Op)
	{
		ReceiveUpdateFromSpatial_SingleClient_Character(UpdateInterop, PackageMap, Op);
	});
	Binding.MultiClientUpdateCallback = View->OnComponentUpdate<improbable::unreal::UnrealCharacterMultiClientReplicatedData>([PackageMap, UpdateInterop](
		const worker::ComponentUpdateOp<improbable::unreal::UnrealCharacterMultiClientReplicatedData>& Op)
	{
		ReceiveUpdateFromSpatial_MultiClient_Character(UpdateInterop, PackageMap, Op);
	});

	// Set spatial update function.
	Binding.SendSpatialUpdateFunction = [PackageMap](FOutBunch* BunchPtr, worker::Connection* Connection, const worker::EntityId& EntityId)
	{
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
		BunchReader.Parse(true, PackageMap, PropertyMap, RepDataHandler);

		// Send SpatialOS update.
		if (SingleClientUpdateChanged)
		{
			Connection->SendComponentUpdate<improbable::unreal::UnrealCharacterSingleClientReplicatedData>(EntityId, SingleClientUpdate);
		}
		if (MultiClientUpdateChanged)
		{
			Connection->SendComponentUpdate<improbable::unreal::UnrealCharacterMultiClientReplicatedData>(EntityId, MultiClientUpdate);
		}
	};

	UpdateInterop->RegisterInteropType(ACharacter::StaticClass(), Binding);
}
}

USpatialUpdateInterop::USpatialUpdateInterop()
{
}

void USpatialUpdateInterop::Init(bool bClient, USpatialOS* Instance, USpatialNetDriver* Driver)
{
	bIsClient = bClient;
	SpatialOSInstance = Instance;
	NetDriver = Driver;

	TSharedPtr<worker::View> View = SpatialOSInstance->GetView().Pin();
	RegisterInterop_Character(View.Get(), nullptr, this);
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

void USpatialUpdateInterop::RegisterInteropType(UClass* Class, FTypeBinding Binding)
{
	TypeBinding.Add(Class, Binding);
}

void USpatialUpdateInterop::UnregisterInteropType(UClass* Class)
{
	FTypeBinding* BindingIterator = TypeBinding.Find(Class);
	if (BindingIterator != nullptr)
	{
		TSharedPtr<worker::View> View = SpatialOSInstance->GetView().Pin();
		View->Remove(BindingIterator->SingleClientUpdateCallback);
		View->Remove(BindingIterator->MultiClientUpdateCallback);
		TypeBinding.Remove(Class);
	}
}

const FTypeBinding* USpatialUpdateInterop::GetTypeBindingByClass(UClass* Class) const
{
	for (const UClass* CurrentClass = Class; CurrentClass; CurrentClass = CurrentClass->GetSuperClass())
	{
		const FTypeBinding* Binding = TypeBinding.Find(CurrentClass);
		if (Binding)
		{
			return Binding;
		}
	}
	return nullptr;
}

void USpatialUpdateInterop::SendSpatialUpdate(USpatialActorChannel* Channel, FOutBunch* BunchPtr)
{
	const FTypeBinding* Binding = GetTypeBindingByClass(Channel->Actor->GetClass());
	if (!Binding)
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

	Binding->SendSpatialUpdateFunction(BunchPtr, WorkerConnection.Get(), Channel->GetEntityId());
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
		const FTypeBinding* Binding = GetTypeBindingByClass(ActorClass);
		if (Binding)
		{
			worker::Map<worker::ComponentId, worker::InterestOverride> Interest;
			Interest.emplace(Binding->SingleClientComponentId, worker::InterestOverride{true});
			SpatialOSInstance->GetConnection().Pin()->SendComponentInterest(EntityId, Interest);
			UE_LOG(LogTemp, Warning, TEXT("We are the owning client, therefore we want single client updates. Client ID: %s"),
				*SpatialOSInstance->GetWorkerConfiguration().GetWorkerId());
		}
	}
}
