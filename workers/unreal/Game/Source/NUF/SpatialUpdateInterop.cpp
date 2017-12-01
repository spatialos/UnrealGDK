// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialUpdateInterop.h"
#include "SpatialOS.h"
#include "SpatialNetDriver.h"
#include "SpatialActorChannel.h"

#include "Engine/PackageMapClient.h"

#include "Generated/SpatialShadowActor_Character.h"

// TODO: Remove when HandleSpatialUpdate_Character is moved
#include "Misc/Base64.h" 

using improbable::unreal::UnrealACharacterReplicatedData;
using improbable::unreal::UnrealACharacterCompleteData;

USpatialUpdateInterop::USpatialUpdateInterop()
{
}

void USpatialUpdateInterop::Init(bool bClient, USpatialOS* Instance, USpatialNetDriver* Driver)
{
	bIsClient = bClient;
	SpatialOSInstance = Instance;
	NetDriver = Driver;

	TSharedPtr<worker::View> View = SpatialOSInstance->GetView().Pin();
	// TODO: Don't hardcode UCharacter interop.
	AddComponentCallback = View->OnAddComponent<UnrealACharacterReplicatedData>([this](const worker::AddComponentOp<UnrealACharacterReplicatedData>& Op)
	{
		// TODO: This section will get used once Patricks stuff is merged in (so we definitely have an actor channel by the time the entity is created).
		/*
		if (!bIsClient)
		{
			return;
		}

		// TODO: Don't hard code the entity ID.
		if (Op.EntityId != 2)
		{
			return;
		}
		UE_LOG(LogTemp, Warning, TEXT("Adding ReplicatedDataComponent for Entity ID 2."));

		// Look up the actor by hard coding the NetGUID for testing.
		// TODO: This assumes that the actor has already been created via the normal Unreal spawn flow.
		// Will need to replace with Patricks solution at a later stage.
		AActor* Actor = Cast<AActor>(NetDriver->GuidCache->GetObjectFromNetGUID(FNetworkGUID(6), false));
		if (!Actor)
		{
			UE_LOG(LogTemp, Warning, TEXT("Unable to find Actor with NetGUID 6."));
			return;
		}

		// TODO: This assumes the actor channel has already been created (which will be true until
		// patricks spawn entity stuff is in).
		USpatialActorChannel* ActorChannel = Cast<USpatialActorChannel>(NetDriver->ServerConnection->ActorChannels[Actor]);
		if (!ActorChannel)
		{
			UE_LOG(LogTemp, Warning, TEXT("Unable to find ActorChannel for Actor with NetGUID 6."));
			return;
		}

		EntityToClientActorChannel.Add(Op.EntityId, ActorChannel);
		UE_LOG(LogTemp, Warning, TEXT("Added ActorChannel for EntityID 2. Actor Role: %d"), (int)Actor->Role.GetValue());
		*/
	});
	RemoveComponentCallback = View->OnRemoveComponent<UnrealACharacterReplicatedData>([this](const worker::RemoveComponentOp& Op)
	{
	});
	ComponentUpdateCallback = View->OnComponentUpdate<UnrealACharacterReplicatedData>([this](const worker::ComponentUpdateOp<UnrealACharacterReplicatedData>& Op)
	{
		// Get actor channel.
		USpatialActorChannel** ActorChannelIt = EntityToClientActorChannel.Find(Op.EntityId);
		if (!ActorChannelIt)
		{
			// Can't find actor channel for this entity, give up.
			return;
		}
		USpatialActorChannel* ActorChannel = *ActorChannelIt;

		// Temporarily ignore AutonomousProxy unless we're assigning the role.
		if (ActorChannel->Actor->Role != ROLE_SimulatedProxy)
		{
			UE_LOG(LogTemp, Warning, TEXT("Actor is not simulated proxy on %s. It's role is %d (SP=1)"), *SpatialOSInstance->GetWorkerConfiguration().GetWorkerId(), (int)ActorChannel->Actor->Role);
			return;
		}

		// Received replicated data from SpatialOS, pipe to Unreal.
		const UnrealACharacterReplicatedData::Update& Update = Op.Update;
		ReceiveUpdateFromSpatial_Character(ActorChannel, Op.Update);
	});

	// TODO: Remove
	WaitingForGuid = true;
}

void USpatialUpdateInterop::Tick(float DeltaTime)
{
	if (WaitingForGuid && NetDriver && bIsClient)
	{
		AActor* Actor = Cast<AActor>(NetDriver->GuidCache->GetObjectFromNetGUID(FNetworkGUID(6), false));
		if (Actor)
		{
			USpatialActorChannel* ActorChannel = Cast<USpatialActorChannel>(NetDriver->ServerConnection->ActorChannels[Actor]);
			// We have the actor, wait for the Entity.
			auto& Entities = SpatialOSInstance->GetView().Pin()->Entities;
			if (Entities.find({2}) != Entities.end() && ActorChannel)
			{
				UE_LOG(LogTemp, Warning, TEXT("Actor with NetGUID 6 and EntityID 2 created. Actor Role: %d"), (int)Actor->Role.GetValue());
				WaitingForGuid = false;
				EntityToClientActorChannel.Add({2}, ActorChannel);
			}
		}
	}
}
