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
#include "SpatialActorChannel.h"
#include "NoOpEntityPipelineBlock.h"

#include <test/rpc/client_rpcs.h>
#include <test/rpc/server_rpcs.h>

#include "EntityBuilder.h"

using namespace improbable;

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

	workerConfig.Networking.UseExternalIp = true;
	workerConfig.SpatialOSApplication.WorkerPlatform =
		bInitAsClient ? TEXT("UnrealClient") : TEXT("UnrealWorker");

	SpatialOSInstance->ApplyConfiguration(workerConfig);
	SpatialOSInstance->Connect();

	SpatialOSComponentUpdater = NewObject<USpatialOSComponentUpdater>(this);

	EntityRegistry = NewObject<UEntityRegistry>(this);

	UpdateInterop = NewObject<USpatialUpdateInterop>(this);

	return true;
}

void USpatialNetDriver::OnSpatialOSConnected()
{
	UE_LOG(LogTemp, Warning, TEXT("Connected to SpatialOS."));
	auto EntitySpawnerBlock = NewObject<UNoOpEntityPipelineBlock>();
	//EntitySpawnerBlock->Init(EntityRegistry);
	SpatialOSInstance->GetEntityPipeline()->AddBlock(EntitySpawnerBlock);

	TArray<FString> BlueprintPaths;
	BlueprintPaths.Add(TEXT(ENTITY_BLUEPRINTS_FOLDER));

	EntityRegistry->RegisterEntityBlueprints(BlueprintPaths);

	UpdateInterop->Init(GetNetMode() == NM_Client, SpatialOSInstance, this);

	// DEBUGGING, REMOVE LATER
	if (GetNetMode() == NM_Client)
	{
		TSharedPtr<worker::View> View = SpatialOSInstance->GetView().Pin();
		TSharedPtr<worker::Connection> Connection = SpatialOSInstance->GetConnection().Pin();
		View->OnReserveEntityIdResponse([this, Connection](const worker::ReserveEntityIdResponseOp& callback)
		{
			std::string ClientWorkerIdString = TCHAR_TO_UTF8(*SpatialOSInstance->GetWorkerConfiguration().GetWorkerId());
			WorkerAttributeSet ClientAttribute{ { "workerId:" + ClientWorkerIdString } };
			WorkerRequirementSet OwnClientOnly{ { ClientAttribute } };
			auto Entity = unreal::FEntityBuilder::Begin()
				.AddPositionComponent(Position::Data{ {10.0f, 10.0f, 10.0f} }, OwnClientOnly)
				.AddMetadataComponent(Metadata::Data{ "TEST" })
				.SetPersistence(true)
				.SetReadAcl(OwnClientOnly)
				.Build();

			Connection->SendCreateEntityRequest(Entity, callback.EntityId, 0);
		});
		Connection->SendReserveEntityIdRequest({});
	}
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
	return Super::ServerReplicateActors(DeltaSeconds);
}

void USpatialNetDriver::TickDispatch(float DeltaTime)
{
	Super::TickDispatch(DeltaTime);

	if (SpatialOSInstance != nullptr && SpatialOSInstance->GetEntityPipeline() != nullptr)
	{
		SpatialOSInstance->ProcessOps();
		SpatialOSInstance->GetEntityPipeline()->ProcessOps(SpatialOSInstance->GetView(), SpatialOSInstance->GetConnection(), GetWorld());
		SpatialOSComponentUpdater->UpdateComponents(EntityRegistry, DeltaTime);
		UpdateInterop->Tick(DeltaTime);
	}
}

void USpatialNetDriver::ProcessRemoteFunction(
	AActor* Actor, 
	UFunction* Function, 
	void* Parameters, 
	FOutParmRec* OutParms, 
	FFrame* Stack, 
	UObject* SubObject) 
{
	UE_LOG(LogTemp, Warning, TEXT("Function: %s, actor: %s"), *Function->GetName(), *Actor->GetName())

	auto* Connection = Actor->GetNetConnection();
	bool CorrectActor = false;
	if (Connection) 
	{
		FNetworkGUID NetGuid = Connection->PackageMap->GetNetGUIDFromObject(Actor).Value == 6;
		CorrectActor = NetGuid == 6 || NetGuid == 12;
	}

	if (Function->FunctionFlags & FUNC_Net && CorrectActor) 
	{
		if (Function->GetName().Equals("ServerMove")) 
		{
			ProcessServerMove(Function, Parameters);
			return;
		}
		else if (Function->GetName().Equals("ClientAckGoodMove"))
		{
			ProcessClientAckGoodMove(Function, Parameters);
			return;
		}
	}
	UIpNetDriver::ProcessRemoteFunction(Actor, Function, Parameters, OutParms, Stack, SubObject);
}

void USpatialNetDriver::ProcessServerMove(UFunction* Function, void* Parameters) 
{
	test::rpc::ServerMoveRequest Request;
	uint32 BytesRead = 0;
	float Timestamp = *static_cast<float*>(static_cast<void*>(static_cast<char*>(Parameters) + BytesRead));
	Request.set_field_time_stamp(Timestamp);
	BytesRead += sizeof(float);

	FVector Accel = *static_cast<FVector*>(static_cast<void*>(static_cast<char*>(Parameters) + BytesRead));
	Request.set_field_in_accel(improbable::Vector3f{ Accel.X, Accel.Y, Accel.Z });
	BytesRead += sizeof(FVector);

	FVector Loc = *static_cast<FVector*>(static_cast<void*>(static_cast<char*>(Parameters) + BytesRead));
	Request.set_field_client_loc(improbable::Vector3f{ Loc.X, Loc.Y, Loc.Z });
	BytesRead += sizeof(FVector);

	uint8 MoveFlags = *static_cast<uint8*>(static_cast<void*>(static_cast<char*>(Parameters) + BytesRead));
	Request.set_field_compressed_move_flags(MoveFlags);
	BytesRead += sizeof(uint8);

	uint8 Roll = *static_cast<uint8*>(static_cast<void*>(static_cast<char*>(Parameters) + BytesRead));
	Request.set_field_client_roll(Roll);
	BytesRead += sizeof(uint8);

	uint32 View = *static_cast<uint32*>(static_cast<void*>(static_cast<char*>(Parameters) + BytesRead));
	Request.set_field_view(View);
	BytesRead += sizeof(uint32);

	// This is the odd one. Given 14 bytes instead of 8. The first two are usually set, the next 4 aren't. The last 8 are the pointer in question.
	UPrimitiveComponent* MovementBase = *static_cast<UPrimitiveComponent**>(static_cast<void*>(static_cast<char*>(Parameters) + BytesRead + 6));
	if (MovementBase) 
	{
		Request.set_field_client_movement_base(TCHAR_TO_UTF8(*MovementBase->GetOwner()->GetName()));
	}
	else 
	{
		Request.set_field_client_movement_base(std::string{ "" });
	}
	BytesRead += sizeof(UPrimitiveComponent*) + 6;

	FName BoneName = *static_cast<FName*>(static_cast<void*>(static_cast<char*>(Parameters) + BytesRead));
	Request.set_field_client_base_bone_name(TCHAR_TO_UTF8(*BoneName.ToString()));
	BytesRead += sizeof(FName);

	uint8 MovementMode = *static_cast<uint8*>(static_cast<void*>(static_cast<char*>(Parameters) + BytesRead));
	Request.set_field_client_movement_mode(MovementMode);
	BytesRead += sizeof(uint8);

	GetSpatialOS()->GetConnection().Pin()->SendCommandRequest<test::rpc::ServerRpcs::Commands::ServerMove>(2, Request, 0);
}

void USpatialNetDriver::ProcessClientAckGoodMove(UFunction* Function, void* Parameters) 
{
	test::rpc::ClientAckGoodMoveRequest Request;
	float Timestamp = *static_cast<float*>(Parameters);
	Request.set_field_time_stamp(Timestamp);

	GetSpatialOS()->GetConnection().Pin()->SendCommandRequest<test::rpc::ClientRpcs::Commands::ClientAckGoodMove>(2, Request, 0);
}
