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

	auto connection = Instance->GetConnection().Pin();

	using ServerMoveCommand = test::rpc::ServerRpcs::Commands::ServerMove;
	View->OnCommandRequest<ServerMoveCommand>([this, connection](const worker::CommandRequestOp<ServerMoveCommand>& op) {
		UCharacterMovementComponent* component = nullptr;
		AActor* Actor = Cast<AActor>(NetDriver->GuidCache->GetObjectFromNetGUID(FNetworkGUID(6), false));
		if (Actor) {
			component = Actor->FindComponentByClass<UCharacterMovementComponent>();
		}

		if (component && Actor->GetWorld()) {
			const improbable::Vector3f& accel = op.Request.field_in_accel();
			const improbable::Vector3f& loc = op.Request.field_client_loc();

			UPrimitiveComponent* primitive = nullptr;
			if (op.Request.field_client_movement_mode() == 1) {

				for (TActorIterator<AStaticMeshActor> ActorItr(Actor->GetWorld()); ActorItr; ++ActorItr) {
					if (ActorItr->GetName().Equals(op.Request.field_client_movement_base().c_str())) {
						primitive = ActorItr->FindComponentByClass<UPrimitiveComponent>();
						break;
					}
				}
			}

			UE_LOG(LogTemp, Warning, TEXT("Timestamp: %f  Time elapsed: %f"), op.Request.field_time_stamp(), Actor->GetWorld()->GetTimeSeconds())
			component->ServerMove_Implementation(
				op.Request.field_time_stamp(), 
				FVector{ accel.x(), accel.y(), accel.z() },
				FVector{ loc.x(), loc.y(), loc.z() },
				static_cast<uint8>(op.Request.field_compressed_move_flags()),
				static_cast<uint8>(op.Request.field_client_roll()),
				op.Request.field_view(),
				primitive,
				FName{},
				static_cast<uint8>(op.Request.field_client_movement_mode())
				);
		}
		connection->SendCommandResponse<ServerMoveCommand>(op.RequestId, typename ServerMoveCommand::Response{});
	});

	using ClientAckGoodMoveCommand = test::rpc::ClientRpcs::Commands::ClientAckGoodMove;
	View->OnCommandRequest<ClientAckGoodMoveCommand>([this, connection](const worker::CommandRequestOp<ClientAckGoodMoveCommand>& op) {
		UCharacterMovementComponent* component = nullptr;
		AActor* Actor = Cast<AActor>(NetDriver->GuidCache->GetObjectFromNetGUID(FNetworkGUID(6), false));
		if (Actor) {
			component = Actor->FindComponentByClass<UCharacterMovementComponent>();
		}
		if (Component && Actor->GetWorld()) {
			Component->ClientAckGoodMove_Implementation(Op.Request.field_time_stamp());
		}
		connection->SendCommandResponse<ClientAckGoodMoveCommand>(op.RequestId, typename ClientAckGoodMoveCommand::Response{});
	});

	// TODO: Remove once the stuff inside USpatialUpdateInterop::Tick is removed.
	WaitingForGuid = true;
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
