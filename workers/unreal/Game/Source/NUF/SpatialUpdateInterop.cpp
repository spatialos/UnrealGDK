// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialUpdateInterop.h"

#include "SpatialActorChannel.h"
#include "SpatialNetConnection.h"
#include "SpatialNetDriver.h"
#include "SpatialOS.h"

#include "Utils/BunchReader.h"

#include "Engine/PackageMapClient.h"

#include "Generated/SpatialUpdateInterop_Character.h"
#include "Generated/SpatialUpdateInterop_PlayerController.h"

// We assume that #define ENABLE_PROPERTY_CHECKSUMS exists in RepLayout.cpp:88 here.
#define ENABLE_PROPERTY_CHECKSUMS

void FSpatialTypeBinding::Init(USpatialUpdateInterop* UpdateInterop, UPackageMap* PackageMap)
{
	this->PackageMap = PackageMap;
	this->UpdateInterop = UpdateInterop;
}

USpatialUpdateInterop::USpatialUpdateInterop()
{
}

void USpatialUpdateInterop::Init(bool bClient, USpatialOS* Instance, USpatialNetDriver* Driver)
{
	bIsClient = bClient;
	SpatialOSInstance = Instance;
	NetDriver = Driver;
	PackageMap = Driver->GetSpatialOSNetConnection()->PackageMap;

	RegisterInteropType(ACharacter::StaticClass(), TSharedPtr<FSpatialTypeBinding>(new FSpatialTypeBinding_Character()));
	RegisterInteropType(APlayerController::StaticClass(), TSharedPtr<FSpatialTypeBinding>(new FSpatialTypeBinding_PlayerController()));
}

void USpatialUpdateInterop::Tick(float DeltaTime)
{
	//Leaving it here for now, we'll remove if it ends up unused.
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

void USpatialUpdateInterop::RegisterInteropType(UClass* Class, TSharedPtr<FSpatialTypeBinding> Binding)
{
	Binding->Init(this, PackageMap);
	Binding->BindToView();
	TypeBinding.Add(Class, Binding);
}

void USpatialUpdateInterop::UnregisterInteropType(UClass* Class)
{
	TSharedPtr<FSpatialTypeBinding>* BindingIterator = TypeBinding.Find(Class);
	if (BindingIterator != nullptr)
	{
		TSharedPtr<FSpatialTypeBinding> Binding = *BindingIterator;
		Binding->UnbindFromView();
		TypeBinding.Remove(Class);
	}
}

const FSpatialTypeBinding* USpatialUpdateInterop::GetTypeBindingByClass(UClass* Class) const
{
	for (const UClass* CurrentClass = Class; CurrentClass; CurrentClass = CurrentClass->GetSuperClass())
	{
		const TSharedPtr<FSpatialTypeBinding>* BindingIterator = TypeBinding.Find(CurrentClass);
		if (BindingIterator)
		{
			return BindingIterator->Get();
		}
	}
	return nullptr;
}

void USpatialUpdateInterop::SendSpatialUpdate(USpatialActorChannel* Channel, FOutBunch* OutgoingBunch)
{
	// WILL DELETE THIS.
	const FSpatialTypeBinding* Binding = GetTypeBindingByClass(Channel->Actor->GetClass());
	if (!Binding)
	{
		//UE_LOG(LogTemp, Warning, TEXT("SpatialUpdateInterop: Trying to send Spatial update on unsupported class %s."),
		//	*Channel->Actor->GetClass()->GetName());
		return;
	}

	// Check that SpatialOS is connected.
	// TODO(David): This function should never get called until SpatialOS _is_ connected.
	TSharedPtr<worker::Connection> WorkerConnection = SpatialOSInstance->GetConnection().Pin();
	if (!WorkerConnection.Get() || !WorkerConnection->IsConnected())
	{
		UE_LOG(LogTemp, Warning, TEXT("SpatialOS is not connected yet."));
		return;
	}

	TArray<uint8> Buffer;
	int64 NumBits;

	if (Channel->bSendingInitialBunch)
	{
		check(Channel->IncrementalUpdateMark.GetNumBits() > 0);
		Channel->IncrementalUpdateMark.Copy(*OutgoingBunch, Buffer);
		NumBits = OutgoingBunch->GetNumBits() - Channel->IncrementalUpdateMark.GetNumBits();
		if (Channel->Actor->IsA(APlayerController::StaticClass()))
		{
			//This is a horrible looking hack, however it seems to be the best way to avoid copying 100+ lines of code.
			//We want to seek to the beginning of actor replication in the bunch. Most of the time, we have the right number saved through
			// USpatialPackageMapClient::SerializeNewActor(). However, only for player controllers, there is 1 byte of additional data saved in
			// UPlayerController::OnSerializeNewActor(). After that point there is no virtual function to override on Spatial side to account for it cleanly.
			// So we do this. The alternative would have been to copy-paste pretty much all of UActorChannel::ReplicateActor() into USpatialActorChannel.
			Buffer.RemoveAt(0); //this is O(N), do it better.
			NumBits -= 8;
		}
	}
	else
	{
		Buffer = MoveTemp(*OutgoingBunch->GetBuffer());
		NumBits = OutgoingBunch->GetNumBits();
	}

	FInBunch InBunch(Channel->Connection, Buffer.GetData(), NumBits);

	Binding->SendComponentUpdates(&InBunch, Channel->GetEntityId());
}

void USpatialUpdateInterop::SendSpatialUpdate(USpatialActorChannel* Channel, const TArray<uint16>& Changed)
{
	const FSpatialTypeBinding* Binding = GetTypeBindingByClass(Channel->Actor->GetClass());
	if (!Binding)
	{
		//UE_LOG(LogTemp, Warning, TEXT("SpatialUpdateInterop: Trying to send Spatial update on unsupported class %s."),
		//	*Channel->Actor->GetClass()->GetName());
		return;
	}

	const uint8* SourceData = (uint8*)Channel->Actor;

	// Check that SpatialOS is connected.
	// TODO(David): This function should never get called until SpatialOS _is_ connected.
	TSharedPtr<worker::Connection> WorkerConnection = SpatialOSInstance->GetConnection().Pin();
	if (!WorkerConnection.Get() || !WorkerConnection->IsConnected())
	{
		UE_LOG(LogTemp, Warning, TEXT("SpatialOS is not connected yet."));
		return;
	}

	Binding->SendComponentUpdates(Changed,
		SourceData,
		Channel->ActorReplicator->RepLayout->Cmds,
		Channel->ActorReplicator->RepLayout->BaseHandleToCmdIndex,
		Channel->GetEntityId());
}

void USpatialUpdateInterop::ReceiveSpatialUpdate(USpatialActorChannel* Channel, FNetBitWriter& IncomingPayload)
{
	// Add null terminator to payload.
	uint32 Terminator = 0;
	IncomingPayload.SerializeIntPacked(Terminator);

	// Build bunch data to send to the actor channel.
	FNetBitWriter BunchData(nullptr, 0);
	// Write header.
	BunchData.WriteBit(1); // bHasRepLayout
	BunchData.WriteBit(1); // bIsActor
	// Write property info.
	uint32 PayloadSize = IncomingPayload.GetNumBits() + 1; // extra bit for bDoChecksum
	BunchData.SerializeIntPacked(PayloadSize);
#ifdef ENABLE_PROPERTY_CHECKSUMS
	BunchData.WriteBit(0); // bDoChecksum
#endif
	BunchData.SerializeBits(IncomingPayload.GetData(), IncomingPayload.GetNumBits());

	// Create bunch and send to actor channel.
	FInBunch Bunch(Channel->Connection, BunchData.GetData(), BunchData.GetNumBits());
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
		const FSpatialTypeBinding* Binding = GetTypeBindingByClass(ActorClass);
		if (Binding)
		{
			worker::Map<worker::ComponentId, worker::InterestOverride> Interest;
			Interest.emplace(Binding->GetReplicatedGroupComponentId(GROUP_SingleClient), worker::InterestOverride{true});
			SpatialOSInstance->GetConnection().Pin()->SendComponentInterest(EntityId, Interest);
			UE_LOG(LogTemp, Warning, TEXT("We are the owning client, therefore we want single client updates. Client ID: %s"),
				*SpatialOSInstance->GetWorkerConfiguration().GetWorkerId());
		}
	}
}
