#include "SpatialNetDriver.h"
#include "SpatialNetConnection.h"
#include "EntityRegistry.h"
#include "EntityPipeline.h"
#include "SimpleEntitySpawnerBlock.h"
#include "SpatialOS.h"
#include "SpatialOSComponentUpdater.h"
#include "Engine/ActorChannel.h"
#include "Net/RepLayout.h"
#include "Net/DataReplication.h"

// SpatialOS interop generated stuff.
#include "UnrealACharacterReplicatedDataComponent.h"
#include "Misc/Base64.h"

#define ENTITY_BLUEPRINTS_FOLDER "/Game/EntityBlueprints"

bool USpatialNetDriver::InitBase(bool bInitAsClient, FNetworkNotify* InNotify, const FURL& URL, bool bReuseAddressAndPort, FString& Error)
{
	if (!Super::InitBase(bInitAsClient, InNotify, URL, bReuseAddressAndPort, Error))
	{
		return false;
	}

	SpatialOSInstance = NewObject<USpatialOS>(this);

	SpatialOSInstance->OnConnectedDelegate.AddDynamic(this,
		&USpatialNetDriver::OnSpatialOSConnected);
	SpatialOSInstance->OnConnectionFailedDelegate.AddDynamic(
		this, &USpatialNetDriver::OnSpatialOSConnectFailed);
	SpatialOSInstance->OnDisconnectedDelegate.AddDynamic(
		this, &USpatialNetDriver::OnSpatialOSDisconnected);

	auto workerConfig = FSOSWorkerConfigurationData();

	workerConfig.Networking.UseExternalIp = false;
	workerConfig.SpatialOSApplication.WorkerPlatform =
		bInitAsClient ? TEXT("UnrealClient") : TEXT("UnrealWorker");

	SpatialOSInstance->ApplyConfiguration(workerConfig);
	SpatialOSInstance->Connect();

	SpatialOSComponentUpdater = NewObject<USpatialOSComponentUpdater>(this);

	EntityRegistry = NewObject<UEntityRegistry>(this);

	return true;
}

void USpatialNetDriver::OnSpatialOSConnected()
{
	UE_LOG(LogTemp, Warning, TEXT("Connected to SpatialOS."));
	ShadowActorPipelineBlock = NewObject<USpatialShadowActorPipelineBlock>();
	ShadowActorPipelineBlock->Init(EntityRegistry);
	SpatialOSInstance->GetEntityPipeline()->AddBlock(ShadowActorPipelineBlock);
	auto EntitySpawnerBlock = NewObject<USimpleEntitySpawnerBlock>();
	//EntitySpawnerBlock->Init(EntityRegistry);
	//SpatialOSInstance->GetEntityPipeline()->AddBlock(EntitySpawnerBlock);

	TArray<FString> BlueprintPaths;
	BlueprintPaths.Add(TEXT(ENTITY_BLUEPRINTS_FOLDER));

	EntityRegistry->RegisterEntityBlueprints(BlueprintPaths);
}

void USpatialNetDriver::OnSpatialOSDisconnected()
{
	UE_LOG(LogTemp, Warning, TEXT("Disconnected from SpatialOS."));
}

void USpatialNetDriver::OnSpatialOSConnectFailed()
{
	UE_LOG(LogTemp, Warning, TEXT("Could not connect to SpatialOS."));
}

void ApplyUpdateToSpatial_Character(AActor* Actor, int CmdIndex, UProperty* ParentProperty, UProperty* Property, UUnrealACharacterReplicatedDataComponent* ReplicatedData) {
	UObject* Container = Actor;
	switch (CmdIndex) {
	case 0:
	{
		auto& Value = *Property->ContainerPtrToValuePtr<uint8>(Container);
		ReplicatedData->FieldBhidden = Value != 0;
		break;
	}
	case 1:
	{
		auto& Value = *Property->ContainerPtrToValuePtr<uint8>(Container);
		ReplicatedData->FieldBreplicatemovement = Value != 0;
		break;
	}
	case 2:
	{
		auto& Value = *Property->ContainerPtrToValuePtr<uint8>(Container);
		ReplicatedData->FieldBtearoff = Value != 0;
		break;
	}
	case 3:
	{
		auto& Value = *Property->ContainerPtrToValuePtr<TEnumAsByte<ENetRole>>(Container);
		ReplicatedData->FieldRemoterole = int(Value);
		break;
	}
	case 4:
	{
		auto& Value = *Property->ContainerPtrToValuePtr<AActor*>(Container);
		// WEAK OBJECT REPLICATION - ReplicatedData->FieldOwner = Value;
		break;
	}
	case 5:
	{
		auto& Value = *Property->ContainerPtrToValuePtr<FRepMovement>(Container);
		TArray<uint8> Data;
		FMemoryWriter Writer(Data);
		bool Success;
		Value.NetSerialize(Writer, nullptr, Success);
		ReplicatedData->FieldReplicatedmovement = FBase64::Encode(Data);
		break;
	}
	case 6:
	{
		auto& Value = *ParentProperty->ContainerPtrToValuePtr<FRepAttachment>(Container);
		// WEAK OBJECT REPLICATION - ReplicatedData->FieldAttachmentreplicationAttachparent = Value.AttachParent;
		break;
	}
	case 7:
	{
		auto& Value = *ParentProperty->ContainerPtrToValuePtr<FRepAttachment>(Container);
		ReplicatedData->FieldAttachmentreplicationLocationoffset = Value.LocationOffset;
		break;
	}
	case 8:
	{
		auto& Value = *ParentProperty->ContainerPtrToValuePtr<FRepAttachment>(Container);
		ReplicatedData->FieldAttachmentreplicationRelativescale3d = Value.RelativeScale3D;
		break;
	}
	case 9:
	{
		auto& Value = *ParentProperty->ContainerPtrToValuePtr<FRepAttachment>(Container);
		auto& Rotator = ReplicatedData->FieldAttachmentreplicationRotationoffset;
		Rotator->SetPitch(Value.RotationOffset.Pitch);
		Rotator->SetYaw(Value.RotationOffset.Yaw);
		Rotator->SetRoll(Value.RotationOffset.Roll);
		break;
	}
	case 10:
	{
		auto& Value = *ParentProperty->ContainerPtrToValuePtr<FRepAttachment>(Container);
		ReplicatedData->FieldAttachmentreplicationAttachsocket = Value.AttachSocket.ToString();
		break;
	}
	case 11:
	{
		auto& Value = *ParentProperty->ContainerPtrToValuePtr<FRepAttachment>(Container);
		// WEAK OBJECT REPLICATION - ReplicatedData->FieldAttachmentreplicationAttachcomponent = Value.AttachComponent;
		break;
	}
	case 12:
	{
		auto& Value = *Property->ContainerPtrToValuePtr<TEnumAsByte<ENetRole>>(Container);
		ReplicatedData->FieldRole = int(Value);
		break;
	}
	case 13:
	{
		auto& Value = *Property->ContainerPtrToValuePtr<uint8>(Container);
		ReplicatedData->FieldBcanbedamaged = Value != 0;
		break;
	}
	case 14:
	{
		auto& Value = *Property->ContainerPtrToValuePtr<APawn*>(Container);
		// WEAK OBJECT REPLICATION - ReplicatedData->FieldInstigator = Value;
		break;
	}
	case 15:
	{
		auto& Value = *Property->ContainerPtrToValuePtr<APlayerState*>(Container);
		// WEAK OBJECT REPLICATION - ReplicatedData->FieldPlayerstate = Value;
		break;
	}
	case 16:
	{
		auto& Value = *Property->ContainerPtrToValuePtr<uint8>(Container);
		ReplicatedData->FieldRemoteviewpitch = int(Value);
		break;
	}
	case 17:
	{
		auto& Value = *Property->ContainerPtrToValuePtr<AController*>(Container);
		// WEAK OBJECT REPLICATION - ReplicatedData->FieldController = Value;
		break;
	}
	case 18:
	{
		auto& Value = *ParentProperty->ContainerPtrToValuePtr<FBasedMovementInfo>(Container);
		// WEAK OBJECT REPLICATION - ReplicatedData->FieldReplicatedbasedmovementMovementbase = Value.MovementBase;
		break;
	}
	case 19:
	{
		auto& Value = *ParentProperty->ContainerPtrToValuePtr<FBasedMovementInfo>(Container);
		ReplicatedData->FieldReplicatedbasedmovementBonename = Value.BoneName.ToString();
		break;
	}
	case 20:
	{
		auto& Value = *ParentProperty->ContainerPtrToValuePtr<FBasedMovementInfo>(Container);
		ReplicatedData->FieldReplicatedbasedmovementLocation = Value.Location;
		break;
	}
	case 21:
	{
		auto& Value = *ParentProperty->ContainerPtrToValuePtr<FBasedMovementInfo>(Container);
		auto& Rotator = ReplicatedData->FieldReplicatedbasedmovementRotation;
		Rotator->SetPitch(Value.Rotation.Pitch);
		Rotator->SetYaw(Value.Rotation.Yaw);
		Rotator->SetRoll(Value.Rotation.Roll);
		break;
	}
	case 22:
	{
		auto& Value = *ParentProperty->ContainerPtrToValuePtr<FBasedMovementInfo>(Container);
		ReplicatedData->FieldReplicatedbasedmovementBserverhasbasecomponent = Value.bServerHasBaseComponent != 0;
		break;
	}
	case 23:
	{
		auto& Value = *ParentProperty->ContainerPtrToValuePtr<FBasedMovementInfo>(Container);
		ReplicatedData->FieldReplicatedbasedmovementBrelativerotation = Value.bRelativeRotation != 0;
		break;
	}
	case 24:
	{
		auto& Value = *ParentProperty->ContainerPtrToValuePtr<FBasedMovementInfo>(Container);
		ReplicatedData->FieldReplicatedbasedmovementBserverhasvelocity = Value.bServerHasVelocity != 0;
		break;
	}
	case 25:
	{
		auto& Value = *Property->ContainerPtrToValuePtr<float>(Container);
		ReplicatedData->FieldAnimrootmotiontranslationscale = Value;
		break;
	}
	case 26:
	{
		auto& Value = *Property->ContainerPtrToValuePtr<float>(Container);
		ReplicatedData->FieldReplicatedserverlasttransformupdatetimestamp = Value;
		break;
	}
	case 27:
	{
		auto& Value = *Property->ContainerPtrToValuePtr<uint8>(Container);
		ReplicatedData->FieldReplicatedmovementmode = int(Value);
		break;
	}
	case 28:
	{
		auto& Value = *Property->ContainerPtrToValuePtr<uint8>(Container);
		ReplicatedData->FieldBiscrouched = Value != 0;
		break;
	}
	case 29:
	{
		auto& Value = *Property->ContainerPtrToValuePtr<float>(Container);
		ReplicatedData->FieldJumpmaxholdtime = Value;
		break;
	}
	case 30:
	{
		auto& Value = *Property->ContainerPtrToValuePtr<int32>(Container);
		ReplicatedData->FieldJumpmaxcount = Value;
		break;
	}
	case 31:
	{
		auto& Value = *ParentProperty->ContainerPtrToValuePtr<FRepRootMotionMontage>(Container);
		ReplicatedData->FieldReprootmotionBisactive = Value.bIsActive != 0;
		break;
	}
	case 32:
	{
		auto& Value = *ParentProperty->ContainerPtrToValuePtr<FRepRootMotionMontage>(Container);
		// WEAK OBJECT REPLICATION - ReplicatedData->FieldReprootmotionAnimmontage = Value.AnimMontage;
		break;
	}
	case 33:
	{
		auto& Value = *ParentProperty->ContainerPtrToValuePtr<FRepRootMotionMontage>(Container);
		ReplicatedData->FieldReprootmotionPosition = Value.Position;
		break;
	}
	case 34:
	{
		auto& Value = *ParentProperty->ContainerPtrToValuePtr<FRepRootMotionMontage>(Container);
		ReplicatedData->FieldReprootmotionLocation = Value.Location;
		break;
	}
	case 35:
	{
		auto& Value = *ParentProperty->ContainerPtrToValuePtr<FRepRootMotionMontage>(Container);
		auto& Rotator = ReplicatedData->FieldReprootmotionRotation;
		Rotator->SetPitch(Value.Rotation.Pitch);
		Rotator->SetYaw(Value.Rotation.Yaw);
		Rotator->SetRoll(Value.Rotation.Roll);
		break;
	}
	case 36:
	{
		auto& Value = *ParentProperty->ContainerPtrToValuePtr<FRepRootMotionMontage>(Container);
		// WEAK OBJECT REPLICATION - ReplicatedData->FieldReprootmotionMovementbase = Value.MovementBase;
		break;
	}
	case 37:
	{
		auto& Value = *ParentProperty->ContainerPtrToValuePtr<FRepRootMotionMontage>(Container);
		ReplicatedData->FieldReprootmotionMovementbasebonename = Value.MovementBaseBoneName.ToString();
		break;
	}
	case 38:
	{
		auto& Value = *ParentProperty->ContainerPtrToValuePtr<FRepRootMotionMontage>(Container);
		ReplicatedData->FieldReprootmotionBrelativeposition = Value.bRelativePosition != 0;
		break;
	}
	case 39:
	{
		auto& Value = *ParentProperty->ContainerPtrToValuePtr<FRepRootMotionMontage>(Container);
		ReplicatedData->FieldReprootmotionBrelativerotation = Value.bRelativeRotation != 0;
		break;
	}
	case 40:
	{
		auto& Value = *ParentProperty->ContainerPtrToValuePtr<FRepRootMotionMontage>(Container);
		{
		}
		{
		}
		{
		}
		{
		}
		{
		}
		break;
	}
	case 41:
	{
		auto& Value = *ParentProperty->ContainerPtrToValuePtr<FRepRootMotionMontage>(Container);
		ReplicatedData->FieldReprootmotionAcceleration = Value.Acceleration;
		break;
	}
	case 42:
	{
		auto& Value = *ParentProperty->ContainerPtrToValuePtr<FRepRootMotionMontage>(Container);
		ReplicatedData->FieldReprootmotionLinearvelocity = Value.LinearVelocity;
		break;
	}
	}
}


int32 USpatialNetDriver::ServerReplicateActors(float DeltaSeconds)
{

	if (!ShadowActorPipelineBlock)
	{
		return 0;
		//return RetVal;
	}

#if WITH_SERVER_CODE
    for (int32 ClientId = 0; ClientId < ClientConnections.Num(); ClientId++)
    {
        UNetConnection* NetConnection = ClientConnections[ClientId];
        for (int32 ChannelId = 0; ChannelId < NetConnection->OpenChannels.Num(); ChannelId++)
        {
            UActorChannel* ActorChannel = Cast<UActorChannel>(NetConnection->OpenChannels[ChannelId]);
            if (!ActorChannel)
            {
                continue;
            }

			// TODO: Remove this once we are using our entity pipeline.
			if (!ActorChannel->Actor->IsA<ACharacter>()) {
				continue;
			}

            // Get FRepState.
            auto RepData = ActorChannel->ActorReplicator;
            FRepState* RepState = RepData->RepState;
            if (!RepState)
            {
                continue;
            }

			// Get entity ID.
			FEntityId EntityId = 2;//EntityRegistry->GetEntityIdFromActor(ActorChannel->Actor);
			if (EntityId == FEntityId())
			{
				// TODO: Is this an error?
				continue;
			}

			// Get shadow actor.
			ASpatialShadowActor* ShadowActor = ShadowActorPipelineBlock->GetShadowActor(EntityId);
			if (!ShadowActor)
			{
				UE_LOG(LogTemp, Warning, TEXT("Actor channel has no corresponding shadow actor. That means the entity hasn't been checked out yet."));
				continue;
			}

            // Write changed properties to SpatialOS.
            auto RepLayout = RepState->RepLayout;
            for (int k = RepState->HistoryStart; k <= RepState->HistoryEnd; k++)
            {
                auto Changed = RepState->ChangeHistory[k].Changed;
                if (Changed.Num() > 0)
                {
                    for (int idx = 0; idx < Changed.Num(); idx++)
                    {
                        if (Changed[idx] == 0)
                        {
                            continue;
                        }
                        int CmdIndex = Changed[idx] - 1;

                        // Ignore dynamic arrays.
                        auto Type = RepLayout->Cmds[CmdIndex].Type;
                        if (Type == REPCMD_Return || Type == REPCMD_DynamicArray)
                        {
                            continue;
                        }

						UProperty* Property = RepLayout->Cmds[CmdIndex].Property;
						UProperty* ParentProperty = RepLayout->Parents[RepLayout->Cmds[CmdIndex].ParentIndex].Property;
						ApplyUpdateToSpatial_Character(ActorChannel->Actor, CmdIndex, ParentProperty, Property, ShadowActor->ReplicatedData);

						FString ChangedProp = Property->GetNameCPP();
                        UE_LOG(LogTemp, Warning, TEXT("Actor: %s, cmd %s"), *GetNameSafe(ActorChannel->Actor), *ChangedProp);
                    }
                }
            }
        }
    }
#endif

	int32 RetVal = Super::ServerReplicateActors(DeltaSeconds);
    return RetVal;
}

void USpatialNetDriver::TickDispatch(float DeltaTime)
{
	Super::TickDispatch(DeltaTime);

	if (SpatialOSInstance != nullptr && SpatialOSInstance->GetEntityPipeline() != nullptr)
	{
		SpatialOSInstance->ProcessOps();
		SpatialOSInstance->GetEntityPipeline()->ProcessOps(SpatialOSInstance->GetView(), SpatialOSInstance->GetConnection(), GetWorld());
		SpatialOSComponentUpdater->UpdateComponents(EntityRegistry, DeltaTime);
		if (ShadowActorPipelineBlock)
		{
			ShadowActorPipelineBlock->ReplicateShadowActorChanges(DeltaTime);
		}
	}
}
