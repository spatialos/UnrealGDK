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
	EntitySpawnerBlock->Init(EntityRegistry);
	SpatialOSInstance->GetEntityPipeline()->AddBlock(EntitySpawnerBlock);

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

void ApplyUpdateToSpatial(AActor* Actor, int cmdIndex, UProperty* Property, UUnrealACharacterReplicatedDataComponent* NativeComponent)
{
	AActor* Container = Actor;
	{
		switch (cmdIndex) {
		case 0:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<uint8>(Container);
			NativeComponent->FieldBhidden = Value != 0;
			break;
		}
		case 1:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<uint8>(Container);
			NativeComponent->FieldBreplicatemovement = Value != 0;
			break;
		}
		case 2:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<uint8>(Container);
			NativeComponent->FieldBtearoff = Value != 0;
			break;
		}
		case 3:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<TEnumAsByte<ENetRole>>(Container);
			NativeComponent->FieldRemoterole = int(Value);
			break;
		}
		case 4:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<AActor*>(Container);
			// WEAK OBJECT REPLICATION - NativeComponent->FieldOwner = Value;
			break;
		}
		case 5:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<FRepMovement>(Container);
			TArray<uint8> Data;
			FMemoryWriter Writer(Data);
			bool Success;
			Value.NetSerialize(Writer, nullptr, Success);
			NativeComponent->FieldReplicatedmovement = FBase64::Encode(Data);
			break;
		}
		case 6:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<AActor*>(Container);
			// WEAK OBJECT REPLICATION - NativeComponent->FieldAttachmentreplicationAttachparent = Value;
			break;
		}
		case 7:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<FVector_NetQuantize100>(Container);
			NativeComponent->FieldAttachmentreplicationLocationoffset = Value;
			break;
		}
		case 8:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<FVector_NetQuantize100>(Container);
			NativeComponent->FieldAttachmentreplicationRelativescale3d = Value;
			break;
		}
		case 9:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<FRotator>(Container);
			auto& Rotator = NativeComponent->FieldAttachmentreplicationRotationoffset;
			Rotator->SetPitch(Value.Pitch);
			Rotator->SetYaw(Value.Yaw);
			Rotator->SetRoll(Value.Roll);
			break;
		}
		case 10:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<FName>(Container);
			NativeComponent->FieldAttachmentreplicationAttachsocket = Value.ToString();
			break;
		}
		case 11:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<USceneComponent*>(Container);
			// WEAK OBJECT REPLICATION - NativeComponent->FieldAttachmentreplicationAttachcomponent = Value;
			break;
		}
		case 12:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<TEnumAsByte<ENetRole>>(Container);
			NativeComponent->FieldRole = int(Value);
			break;
		}
		case 13:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<uint8>(Container);
			NativeComponent->FieldBcanbedamaged = Value != 0;
			break;
		}
		case 14:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<APawn*>(Container);
			// WEAK OBJECT REPLICATION - NativeComponent->FieldInstigator = Value;
			break;
		}
		case 15:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<APlayerState*>(Container);
			// WEAK OBJECT REPLICATION - NativeComponent->FieldPlayerstate = Value;
			break;
		}
		case 16:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<uint8>(Container);
			NativeComponent->FieldRemoteviewpitch = int(Value);
			break;
		}
		case 17:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<AController*>(Container);
			// WEAK OBJECT REPLICATION - NativeComponent->FieldController = Value;
			break;
		}
		case 18:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<UPrimitiveComponent*>(Container);
			// WEAK OBJECT REPLICATION - NativeComponent->FieldReplicatedbasedmovementMovementbase = Value;
			break;
		}
		case 19:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<FName>(Container);
			NativeComponent->FieldReplicatedbasedmovementBonename = Value.ToString();
			break;
		}
		case 20:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<FVector_NetQuantize100>(Container);
			NativeComponent->FieldReplicatedbasedmovementLocation = Value;
			break;
		}
		case 21:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<FRotator>(Container);
			auto& Rotator = NativeComponent->FieldReplicatedbasedmovementRotation;
			Rotator->SetPitch(Value.Pitch);
			Rotator->SetYaw(Value.Yaw);
			Rotator->SetRoll(Value.Roll);
			break;
		}
		case 22:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<bool>(Container);
			NativeComponent->FieldReplicatedbasedmovementBserverhasbasecomponent = Value != 0;
			break;
		}
		case 23:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<bool>(Container);
			NativeComponent->FieldReplicatedbasedmovementBrelativerotation = Value != 0;
			break;
		}
		case 24:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<bool>(Container);
			NativeComponent->FieldReplicatedbasedmovementBserverhasvelocity = Value != 0;
			break;
		}
		case 25:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<float>(Container);
			NativeComponent->FieldAnimrootmotiontranslationscale = Value;
			break;
		}
		case 26:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<float>(Container);
			NativeComponent->FieldReplicatedserverlasttransformupdatetimestamp = Value;
			break;
		}
		case 27:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<uint8>(Container);
			NativeComponent->FieldReplicatedmovementmode = int(Value);
			break;
		}
		case 28:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<uint8>(Container);
			NativeComponent->FieldBiscrouched = Value != 0;
			break;
		}
		case 29:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<float>(Container);
			NativeComponent->FieldJumpmaxholdtime = Value;
			break;
		}
		case 30:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<int32>(Container);
			NativeComponent->FieldJumpmaxcount = Value;
			break;
		}
		case 31:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<bool>(Container);
			NativeComponent->FieldReprootmotionBisactive = Value != 0;
			break;
		}
		case 32:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<UAnimMontage*>(Container);
			// WEAK OBJECT REPLICATION - NativeComponent->FieldReprootmotionAnimmontage = Value;
			break;
		}
		case 33:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<float>(Container);
			NativeComponent->FieldReprootmotionPosition = Value;
			break;
		}
		case 34:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<FVector_NetQuantize100>(Container);
			NativeComponent->FieldReprootmotionLocation = Value;
			break;
		}
		case 35:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<FRotator>(Container);
			auto& Rotator = NativeComponent->FieldReprootmotionRotation;
			Rotator->SetPitch(Value.Pitch);
			Rotator->SetYaw(Value.Yaw);
			Rotator->SetRoll(Value.Roll);
			break;
		}
		case 36:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<UPrimitiveComponent*>(Container);
			// WEAK OBJECT REPLICATION - NativeComponent->FieldReprootmotionMovementbase = Value;
			break;
		}
		case 37:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<FName>(Container);
			NativeComponent->FieldReprootmotionMovementbasebonename = Value.ToString();
			break;
		}
		case 38:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<bool>(Container);
			NativeComponent->FieldReprootmotionBrelativeposition = Value != 0;
			break;
		}
		case 39:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<bool>(Container);
			NativeComponent->FieldReprootmotionBrelativerotation = Value != 0;
			break;
		}
		case 40:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<FRootMotionSourceGroup>(Container);
			/*
			{
				NativeComponent->FieldReprootmotionAuthoritativerootmotionBhasadditivesources = Value.bHasAdditiveSources != 0;
			}
			{
				NativeComponent->FieldReprootmotionAuthoritativerootmotionBhasoverridesources = Value.bHasOverrideSources != 0;
			}
			{
				NativeComponent->FieldReprootmotionAuthoritativerootmotionLastpreadditivevelocity = Value.LastPreAdditiveVelocity;
			}
			{
				NativeComponent->FieldReprootmotionAuthoritativerootmotionBisadditivevelocityapplied = Value.bIsAdditiveVelocityApplied != 0;
			}
			{
				{
					NativeComponent->FieldReprootmotionAuthoritativerootmotionLastaccumulatedsettingsFlags = int(Value.LastAccumulatedSettings.Flags);
				}
			}
			*/
			break;
		}
		case 41:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<FVector_NetQuantize10>(Container);
			NativeComponent->FieldReprootmotionAcceleration = Value;
			break;
		}
		case 42:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<FVector_NetQuantize10>(Container);
			NativeComponent->FieldReprootmotionLinearvelocity = Value;
			break;
		}
		}
	}

}

int32 USpatialNetDriver::ServerReplicateActors(float DeltaSeconds)
{
	int32 RetVal = Super::ServerReplicateActors(DeltaSeconds);

	if (!ShadowActorPipelineBlock)
	{
		return RetVal;
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

            // Get FRepState.
            auto RepData = ActorChannel->ActorReplicator;
            FRepState* RepState = RepData->RepState;
            if (!RepState)
            {
                continue;
            }

			// Get entity ID.
			FEntityId EntityId = EntityRegistry->GetEntityIdFromActor(ActorChannel->Actor);
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
						ApplyUpdateToSpatial(ActorChannel->Actor, CmdIndex, Property, ShadowActor->ReplicatedData);

						FString ChangedProp = Property->GetNameCPP();
                        UE_LOG(LogTemp, Warning, TEXT("Actor: %s, cmd %s"), *GetNameSafe(ActorChannel->Actor), *ChangedProp);
                    }
                }
            }
        }
    }
#endif

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
	}
}
