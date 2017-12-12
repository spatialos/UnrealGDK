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

	auto* connection = Actor->GetNetConnection();
	bool correctActor = false;
	if (connection) {
		correctActor = connection->PackageMap->GetNetGUIDFromObject(Actor).Value == 6;
	}

	if (Function->FunctionFlags & FUNC_Net && correctActor) 
	{
		UE_LOG(LogTemp, Warning, TEXT("Function: %s, actor: %s"), *Function->GetName(), *Actor->GetName())
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
	test::rpc::ServerMoveRequest request;
	UField* argIter = Function->Children;
	uint32 bytesRead = 0;
	//UE_LOG(LogTemp, Log, TEXT("args: %s"), *Cast<UProperty>(argIter)->GetCPPType())
	float timestamp = *static_cast<float*>(static_cast<void*>(static_cast<char*>(Parameters) + bytesRead));
	request.set_field_time_stamp(timestamp);
	//UE_LOG(LogTemp, Log, TEXT("value: %f"), timestamp) 
	argIter = argIter->Next;
	bytesRead += sizeof(float);

	//UE_LOG(LogTemp, Log, TEXT("args: %s"), *Cast<UProperty>(argIter)->GetCPPType())
	FVector accel = *static_cast<FVector*>(static_cast<void*>(static_cast<char*>(Parameters) + bytesRead));
	request.set_field_in_accel(improbable::Vector3f{ accel.X, accel.Y, accel.Z });
	//UE_LOG(LogTemp, Log, TEXT("value: %s"), *accel.ToString())
	argIter = argIter->Next;
	bytesRead += sizeof(FVector);

	//UE_LOG(LogTemp, Log, TEXT("args: %s"), *Cast<UProperty>(argIter)->GetCPPType())
	FVector loc = *static_cast<FVector*>(static_cast<void*>(static_cast<char*>(Parameters) + bytesRead));
	request.set_field_client_loc(improbable::Vector3f{ loc.X, loc.Y, loc.Z });
	//UE_LOG(LogTemp, Log, TEXT("value: %s"), *loc.ToString())
	argIter = argIter->Next;
	bytesRead += sizeof(FVector);

	//UE_LOG(LogTemp, Log, TEXT("args: %s"), *Cast<UProperty>(argIter)->GetCPPType())
	uint8 moveFlags = *static_cast<uint8*>(static_cast<void*>(static_cast<char*>(Parameters) + bytesRead));
	request.set_field_compressed_move_flags(moveFlags);
	//UE_LOG(LogTemp, Log, TEXT("value: %d"), moveFlags)
	argIter = argIter->Next;
	bytesRead += sizeof(uint8);

	//UE_LOG(LogTemp, Log, TEXT("args: %s"), *Cast<UProperty>(argIter)->GetCPPType())
	uint8 roll = *static_cast<uint8*>(static_cast<void*>(static_cast<char*>(Parameters) + bytesRead));
	request.set_field_client_roll(roll);
	//UE_LOG(LogTemp, Log, TEXT("value: %d"), roll)
	argIter = argIter->Next;
	bytesRead += sizeof(uint8);

	//UE_LOG(LogTemp, Log, TEXT("args: %s"), *Cast<UProperty>(argIter)->GetCPPType())
	uint32 view = *static_cast<uint32*>(static_cast<void*>(static_cast<char*>(Parameters) + bytesRead));
	request.set_field_view(view);
	//UE_LOG(LogTemp, Log, TEXT("value: %d"), )
	argIter = argIter->Next;
	bytesRead += sizeof(uint32);

	//UE_LOG(LogTemp, Log, TEXT("args: %s"), *Cast<UProperty>(argIter)->GetCPPType())
	UObject* movementBase = *static_cast<UObject**>(static_cast<void*>(static_cast<char*>(Parameters) + bytesRead + 6));
	//UE_LOG(LogTemp, Log, TEXT("value: %p"), movementBase)
	argIter = argIter->Next;
	uint8 objPtr[14];
	for (int i = 0; i < 14; ++i) {
		objPtr[i] = *static_cast<uint8*>(static_cast<void*>(static_cast<char*>(Parameters) + bytesRead + i));
	}
	UPrimitiveComponent* x = Cast<UPrimitiveComponent>(movementBase);
	if (x) {
		request.set_field_client_movement_base(TCHAR_TO_UTF8(*x->GetOwner()->GetName()));
	}
	else {
		request.set_field_client_movement_base(std::string{ "" });
	}
	bytesRead += sizeof(UObject*) + 6;

	//UE_LOG(LogTemp, Log, TEXT("args: %s"), *Cast<UProperty>(argIter)->GetCPPType())
	FName boneName = *static_cast<FName*>(static_cast<void*>(static_cast<char*>(Parameters) + bytesRead));
	request.set_field_client_base_bone_name(TCHAR_TO_UTF8(*boneName.ToString()));
	//UE_LOG(LogTemp, Log, TEXT("value: %s"), *boneName.ToString())
	argIter = argIter->Next;
	bytesRead += sizeof(FName);

	//UE_LOG(LogTemp, Log, TEXT("args: %s"), *Cast<UProperty>(argIter)->GetCPPType())
	uint8 movementMode = *static_cast<uint8*>(static_cast<void*>(static_cast<char*>(Parameters) + bytesRead));
	request.set_field_client_movement_mode(movementMode);
	//UE_LOG(LogTemp, Log, TEXT("value: %d"), movementMode)
	argIter = argIter->Next;
	bytesRead += sizeof(uint8);

	GetSpatialOS()->GetConnection().Pin()->SendCommandRequest<test::rpc::ServerRpcs::Commands::ServerMove>(2, request, 0);
}

void USpatialNetDriver::ProcessClientAckGoodMove(UFunction* Function, void* Parameters) 
{
	test::rpc::ClientAckGoodMoveRequest request;
	float timestamp = *static_cast<float*>(Parameters);
	request.set_field_time_stamp(timestamp);

	GetSpatialOS()->GetConnection().Pin()->SendCommandRequest<test::rpc::ClientRpcs::Commands::ClientAckGoodMove>(2, request, 0);
}
