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
	if (!Connection)
	{
		UIpNetDriver::ProcessRemoteFunction(Actor, Function, Parameters, OutParms, Stack, SubObject);
		return;
	}

	FNetworkGUID NetGuid = Connection->PackageMap->GetNetGUIDFromObject(Actor).Value;
	// Hard coded to up to two actors. The first actor will have NetGuid 6 and entitiy ID 2. The second will be 12 and 3.
	// Only send a command if it was one of these actors which sent the rpc.
	bool CorrectActor = NetGuid == 6 || NetGuid == 12;
	if (Function->FunctionFlags & FUNC_Net && CorrectActor)
	{
		worker::EntityId entityId = NetGuid == 6 ? 2 : 3;
		// The rpc might have been called by an actor directly, or by a subobject on that actor (e.g. UCharacterMovementComponent)
		UObject* CallingObject = SubObject ? Actor : SubObject;
		// Reading properties from an FFrame changes the FFrame (internal pointer to the last property read). So we need to make a new one.
		FFrame TempRpcFrameForReading{ CallingObject, Function, Parameters, nullptr, Function->Children };
		if (Function->GetName().Equals("ServerMove"))
		{
			ProcessServerMove(&TempRpcFrameForReading, entityId, Connection->PackageMap);
			return;
		}
		else if (Function->GetName().Equals("ClientAckGoodMove"))
		{
			ProcessClientAckGoodMove(&TempRpcFrameForReading, entityId);
			return;
		}
	}
	UIpNetDriver::ProcessRemoteFunction(Actor, Function, Parameters, OutParms, Stack, SubObject);
}

void USpatialNetDriver::ProcessServerMove(FFrame* TempRpcFrameForReading, worker::EntityId entityId, UPackageMap* PackageMap) 
{
	// The FFrame being read has to be called "Stack" for the macros to work.
	FFrame& Stack = *TempRpcFrameForReading;
	P_GET_PROPERTY(UFloatProperty, Timestamp);
	P_GET_STRUCT(FVector_NetQuantize10, InAccel);
	P_GET_STRUCT(FVector_NetQuantize100, ClientLoc);
	P_GET_PROPERTY(UByteProperty, CompressedMoveFlags);
	P_GET_PROPERTY(UByteProperty, ClientRoll);
	P_GET_PROPERTY(UUInt32Property, View);
	P_GET_OBJECT(UPrimitiveComponent, ClientMovementBase);
	P_GET_PROPERTY(UNameProperty, ClientBaseBoneName);
	P_GET_PROPERTY(UByteProperty, ClientMovementMode);

	test::rpc::ServerMoveRequest Request;
	Request.set_field_time_stamp(Timestamp);
	Request.set_field_in_accel(improbable::Vector3f{ InAccel.X, InAccel.Y, InAccel.Z });
	Request.set_field_client_loc(improbable::Vector3f{ ClientLoc.X, ClientLoc.Y, ClientLoc.Z });
	Request.set_field_compressed_move_flags(CompressedMoveFlags);
	Request.set_field_client_roll(ClientRoll);
	Request.set_field_view(View);

	// This argument is a pointer to the UPrimitiveComponent the actor is standing on.
	// Normally this would be serialised as a NetGuid, but note this will not work in general here.
	if (ClientMovementBase) 
	{
		Request.set_field_client_movement_base(PackageMap->GetNetGUIDFromObject(ClientMovementBase).Value);
	}
	Request.set_field_client_base_bone_name(TCHAR_TO_UTF8(*ClientBaseBoneName.ToString()));
	Request.set_field_client_movement_mode(ClientMovementMode);

	GetSpatialOS()->GetConnection().Pin()->SendCommandRequest<test::rpc::ServerRpcs::Commands::ServerMove>(entityId, Request, 0);
}

void USpatialNetDriver::ProcessClientAckGoodMove(FFrame* TempRpcFrameForReading, worker::EntityId entityId) 
{
	// The FFrame being read has to be called "Stack" for the macros to work.
	FFrame& Stack = *TempRpcFrameForReading;
	P_GET_PROPERTY(UFloatProperty, Timestamp);

	test::rpc::ClientAckGoodMoveRequest Request;
	Request.set_field_time_stamp(Timestamp);

	GetSpatialOS()->GetConnection().Pin()->SendCommandRequest<test::rpc::ClientRpcs::Commands::ClientAckGoodMove>(entityId, Request, 0);
}
