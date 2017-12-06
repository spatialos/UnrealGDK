// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialUpdateInterop.h"
#include "SpatialOS.h"
#include "SpatialNetDriver.h"
#include "SpatialActorChannel.h"

#include "Engine/PackageMapClient.h"

#include "Generated/SpatialUpdateInterop_Character.h"

using improbable::unreal::UnrealCharacterReplicatedData;
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
	ComponentUpdateCallback = View->OnComponentUpdate<UnrealCharacterReplicatedData>([this](const worker::ComponentUpdateOp<UnrealCharacterReplicatedData>& Op)
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
			//UE_LOG(LogTemp, Warning, TEXT("Actor is not simulated proxy on %s. It's role is %d (SP=1)"), *SpatialOSInstance->GetWorkerConfiguration().GetWorkerId(), (int)ActorChannel->Actor->Role);
			return;
		}

		// Received replicated data from SpatialOS, pipe to Unreal.
		ReceiveUpdateFromSpatial_Character(ActorChannel, Op.Update);
	});

	// TODO: Remove once the stuff inside USpatialUpdateInterop::Tick is removed.
	WaitingForGuid = true;
}

void USpatialUpdateInterop::Tick(float DeltaTime)
{
	// TODO: Remove the WaitingForGuid variable and all this logic once we actually integrate with Unreals actor system properly.
	// i.e. When Girays stuff is done.
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
