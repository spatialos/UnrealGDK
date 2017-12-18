// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialUpdateInterop.h"
#include "SpatialOS.h"
#include "SpatialNetDriver.h"
#include "SpatialActorChannel.h"

#include "Engine/PackageMapClient.h"
#include "Engine/StaticMeshActor.h"
#include "EngineUtils.h"

#include "Generated/SpatialUpdateInterop_Character.h"
#include "Generated/Usr/test/rpc/server_rpcs.h"
#include "Generated/Usr/test/rpc/client_rpcs.h"

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
			//return;
		}

		// Received replicated data from SpatialOS, pipe to Unreal.
		ReceiveUpdateFromSpatial_Character(ActorChannel, Op.Update);
	});

	TSharedPtr<worker::Connection> Connection = Instance->GetConnection().Pin();

	using ServerMoveCommand = test::rpc::ServerRpcs::Commands::ServerMove;
	View->OnCommandRequest<ServerMoveCommand>([this, Connection](const worker::CommandRequestOp<ServerMoveCommand>& Op) {
		TSharedPtr<FNetGUIDCache> GuidCache = NetDriver->GuidCache;
		UCharacterMovementComponent* MovementComponent = nullptr;

		// Hard coded actors for the moment.
		AActor* Actor = Op.EntityId == 2
			? Cast<AActor>(GuidCache->GetObjectFromNetGUID(FNetworkGUID(6), false))
			: Cast<AActor>(GuidCache->GetObjectFromNetGUID(FNetworkGUID(12), false));
		if (Actor) {
			MovementComponent = Actor->FindComponentByClass<UCharacterMovementComponent>();
		}

		if (MovementComponent && Actor->GetWorld()) {
			const improbable::Vector3f& Accel = Op.Request.field_in_accel();
			const improbable::Vector3f& Loc = Op.Request.field_client_loc();

			FNetworkGUID PrimitiveComponentGuid = Op.Request.field_client_movement_base();
			UPrimitiveComponent* Primitive = Op.Request.field_client_movement_base() > 0
				? static_cast<UPrimitiveComponent*>(GuidCache->GetObjectFromNetGUID(PrimitiveComponentGuid, false))
				: nullptr;

			MovementComponent->ServerMove_Implementation(
				Op.Request.field_time_stamp(), 
				FVector{ Accel.x(), Accel.y(), Accel.z() },
				FVector{ Loc.x(), Loc.y(), Loc.z() },
				static_cast<uint8>(Op.Request.field_compressed_move_flags()),
				static_cast<uint8>(Op.Request.field_client_roll()),
				Op.Request.field_view(),
				Primitive,
				FName{}, /*This seems to be "None" in all cases tested, so just using this for the prototype.*/
				static_cast<uint8>(Op.Request.field_client_movement_mode())
				);
		}
		Connection->SendCommandResponse<ServerMoveCommand>(Op.RequestId, typename ServerMoveCommand::Response{});
	});

	using ClientAckGoodMoveCommand = test::rpc::ClientRpcs::Commands::ClientAckGoodMove;
	View->OnCommandRequest<ClientAckGoodMoveCommand>([this, Connection](const worker::CommandRequestOp<ClientAckGoodMoveCommand>& Op) {
		UCharacterMovementComponent* MovementComponent = nullptr;
		// Hard coded actors for the moment.
		AActor* Actor = Op.EntityId == 2
			? Cast<AActor>(NetDriver->GuidCache->GetObjectFromNetGUID(FNetworkGUID(6), false))
			: Cast<AActor>(NetDriver->GuidCache->GetObjectFromNetGUID(FNetworkGUID(12), false));
		if (Actor) {
			MovementComponent = Actor->FindComponentByClass<UCharacterMovementComponent>();
		}
		if (MovementComponent && Actor->GetWorld()) {
			MovementComponent->ClientAckGoodMove_Implementation(Op.Request.field_time_stamp());
		}
		Connection->SendCommandResponse<ClientAckGoodMoveCommand>(Op.RequestId, typename ClientAckGoodMoveCommand::Response{});
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
			// We have the actor, wait for the Entity.
			auto& Entities = SpatialOSInstance->GetView().Pin()->Entities;
			if (Entities.find({2}) != Entities.end() && ActorChannel)
			{
				UE_LOG(LogTemp, Warning, TEXT("Actor with NetGUID 6 and EntityID 2 created. Actor Role: %d"), (int)Client1->Role.GetValue());
				EntityToClientActorChannel.Add({2}, ActorChannel);
			}
		}

		// Actor representing the player in the floating window.
		AActor* Client2 = Cast<AActor>(NetDriver->GuidCache->GetObjectFromNetGUID(FNetworkGUID(12), false));
		if (Client2 && EntityToClientActorChannel.Find({3}) == nullptr)
		{
			USpatialActorChannel* ActorChannel = Cast<USpatialActorChannel>(NetDriver->ServerConnection->ActorChannels[Client2]);
			// We have the actor, wait for the Entity.
			auto& Entities = SpatialOSInstance->GetView().Pin()->Entities;
			if (Entities.find({3}) != Entities.end() && ActorChannel)
			{
				UE_LOG(LogTemp, Warning, TEXT("Actor with NetGUID 12 and EntityID 3 created. Actor Role: %d"), (int)Client2->Role.GetValue());
				EntityToClientActorChannel.Add({3}, ActorChannel);
			}
		}
	}
}
