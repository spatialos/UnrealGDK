// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

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
#include "SpatialPackageMapClient.h"
#include "SpatialActorChannel.h"

//#include "Generated/SpatialInteropCharacter.h"

#define ENTITY_BLUEPRINTS_FOLDER "/Game/EntityBlueprints"

bool USpatialNetDriver::InitBase(bool bInitAsClient, FNetworkNotify* InNotify, const FURL& URL, bool bReuseAddressAndPort, FString& Error)
{
	if (!Super::InitBase(bInitAsClient, InNotify, URL, bReuseAddressAndPort, Error))
	{
		return false;
	}

	// make absolutely sure that the actor channel that we are using is our Spatial actor channel
	UChannel::ChannelClasses[CHTYPE_Actor] = USpatialActorChannel::StaticClass();

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
	ShadowActorPipelineBlock = NewObject<USpatialShadowActorPipelineBlock>(this);
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

int32 USpatialNetDriver::ServerReplicateActors(float DeltaSeconds)
{
	int32 RetVal = Super::ServerReplicateActors(DeltaSeconds);

	if (!ShadowActorPipelineBlock)
	{
		return RetVal;
	}

#if WITH_SERVER_CODE
	/*
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
			if (!ActorChannel->Actor->IsA<ACharacter>())
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
			// TODO: Replace with EntityId from registry corresponding to this replicated actor.
			FEntityId EntityId = 2;//EntityRegistry->GetEntityIdFromActor(ActorChannel->Actor);
			if (EntityId == FEntityId())
			{
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
						//ApplyUpdateToSpatial_Character(ActorChannel->Actor, CmdIndex, ParentProperty, Property, ShadowActor->ReplicatedData);

						FString ChangedProp = Property->GetNameCPP();
						UE_LOG(LogTemp, Warning, TEXT("Actor: %s, cmd %s"), *GetNameSafe(ActorChannel->Actor), *ChangedProp);
					}
				}
			}
		}
	}
	*/
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
		if (ShadowActorPipelineBlock)
		{
			ShadowActorPipelineBlock->ReplicateShadowActorChanges(DeltaTime);
		}
	}
}

void USpatialNetDriver::PostInitProperties()
{
	Super::PostInitProperties();

	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		GuidCache = TSharedPtr<FSpatialNetGUIDCache>(new FSpatialNetGUIDCache(this));
	}
}

UEntityRegistry* USpatialNetDriver::GetEntityRegistry()
{
	return EntityRegistry;
}

