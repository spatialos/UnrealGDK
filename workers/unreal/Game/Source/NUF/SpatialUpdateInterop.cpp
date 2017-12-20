// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialUpdateInterop.h"
#include "SpatialNetDriver.h"
#include "SpatialOS.h"
#include "SpatialActorChannel.h"

#include "Utils/BunchReader.h"

#include "Engine/PackageMapClient.h"
#include "Engine/StaticMeshActor.h"
#include "EngineUtils.h"

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

	RegisterInteropType(ACharacter::StaticClass(), TSharedPtr<FSpatialTypeBinding>(new FSpatialTypeBinding_Character()));
	//RegisterInteropType(PlayerController::StaticClass(), TSharedPtr<FSpatialTypeBinding>(new FSpatialTypeBinding_PlayerController()));
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

void USpatialUpdateInterop::RegisterInteropType(UClass* Class, TSharedPtr<FSpatialTypeBinding> Binding)
{
	Binding->Init(this, nullptr);
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

	Binding->SendComponentUpdates(OutgoingBunch, Channel->GetEntityId());
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

void USpatialUpdateInterop::HandleRPCInvocation(AActor* TargetActor, UFunction* Function, FFrame* DuplicateFrame, worker::EntityId Target)
{
	TargetActor->StaticClass();

	if (TargetActor->IsA(ACharacter::StaticClass()))
	{
		
		//ApplyUpdateToSpatial_MultiClient_Character
	}
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
