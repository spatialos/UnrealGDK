// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/SpatialNetConnection.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Interop/SpatialReceiver.h"
#include "Interop/SpatialSender.h"
#include "SpatialConstants.h"
#include "SpatialGDKSettings.h"

#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "TimerManager.h"

#include <WorkerSDK/improbable/c_schema.h>

DEFINE_LOG_CATEGORY(LogSpatialNetConnection);

DECLARE_CYCLE_STAT(TEXT("UpdateLevelVisibility"), STAT_SpatialNetConnectionUpdateLevelVisibility, STATGROUP_SpatialNet);

USpatialNetConnection::USpatialNetConnection(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, PlayerControllerEntity(SpatialConstants::INVALID_ENTITY_ID)
{
#if ENGINE_MINOR_VERSION <= 24
	InternalAck = 1;
#else
	SetInternalAck(true);
#endif
}

void USpatialNetConnection::BeginDestroy()
{
	DisableHeartbeat();
	
	Super::BeginDestroy();
}

void USpatialNetConnection::CleanUp()
{
	if (USpatialNetDriver* SpatialNetDriver = Cast<USpatialNetDriver>(Driver))
	{
		SpatialNetDriver->CleanUpClientConnection(this);
	}

	Super::CleanUp();
}

void USpatialNetConnection::InitBase(UNetDriver* InDriver, class FSocket* InSocket, const FURL& InURL, EConnectionState InState, int32 InMaxPacket /*= 0*/, int32 InPacketOverhead /*= 0*/)
{
	Super::InitBase(InDriver, InSocket, InURL, InState, InMaxPacket, InPacketOverhead);

	if (Cast<USpatialNetDriver>(InDriver)->PackageMap == nullptr)
	{
		// This should only happen if we're setting up the special "SpatialOS" connection.
		auto PackageMapClient = NewObject<USpatialPackageMapClient>(this);
		PackageMapClient->Initialize(this, InDriver->GuidCache);
		PackageMap = PackageMapClient;
		Cast<USpatialNetDriver>(InDriver)->PackageMap = PackageMapClient;
	}
	else
	{
		PackageMap = Cast<USpatialNetDriver>(InDriver)->PackageMap;
	}
}

void USpatialNetConnection::LowLevelSend(void* Data, int32 CountBits, FOutPacketTraits& Traits)
{
	//Intentionally does not call Super::
}

bool USpatialNetConnection::ClientHasInitializedLevelFor(const AActor* TestActor) const
{
	check(Driver->IsServer());
	return true;
	//Intentionally does not call Super::
}

int32 USpatialNetConnection::IsNetReady(bool Saturate)
{
	// TODO: UNR-664 - Currently we do not report the number of bits sent when replicating, this means channel saturation cannot be checked properly.
	// This will always return true until we solve this.
	return true;
}

#if ENGINE_MINOR_VERSION <= 23
void USpatialNetConnection::UpdateLevelVisibility(const FName& PackageName, bool bIsVisible)
#else
void USpatialNetConnection::UpdateLevelVisibility(const struct FUpdateLevelVisibilityLevelInfo& LevelVisibility)
#endif
{
	SCOPE_CYCLE_COUNTER(STAT_SpatialNetConnectionUpdateLevelVisibility);

#if ENGINE_MINOR_VERSION <= 23
	UNetConnection::UpdateLevelVisibility(PackageName, bIsVisible);
#else
	UNetConnection::UpdateLevelVisibility(LevelVisibility);
#endif

	// We want to update our interest as fast as possible
	// So we send an Interest update immediately.

	USpatialSender* Sender = Cast<USpatialNetDriver>(Driver)->Sender;

	bool bOwnerReady;
	Sender->UpdateInterestComponent(Cast<AActor>(PlayerController), bOwnerReady);
}

void USpatialNetConnection::FlushDormancy(AActor* Actor)
{
	Super::FlushDormancy(Actor);

	// This gets called from UNetDriver::FlushActorDormancyInternal for each connection. We inject our refresh
	// of dormancy component here. This is slightly backwards, but means we don't have to make an engine change.
	if (bReliableSpatialConnection)
	{
		const bool bMakeDormant = false;
		Cast<USpatialNetDriver>(Driver)->RefreshActorDormancy(Actor, bMakeDormant);
	}
}

void USpatialNetConnection::ClientNotifyClientHasQuit()
{
	if (PlayerControllerEntity != SpatialConstants::INVALID_ENTITY_ID)
	{
		if (!Cast<USpatialNetDriver>(Driver)->StaticComponentView->HasAuthority(PlayerControllerEntity, SpatialConstants::HEARTBEAT_COMPONENT_ID))
		{
			UE_LOG(LogSpatialNetConnection, Warning, TEXT("Quit the game but no authority over Heartbeat component: NetConnection %s, PlayerController entity %lld"), *GetName(), PlayerControllerEntity);
			return;
		}

		FWorkerComponentUpdate Update = {};
		Update.component_id = SpatialConstants::HEARTBEAT_COMPONENT_ID;
		Update.schema_type = Schema_CreateComponentUpdate();
		Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(Update.schema_type);

		Schema_AddBool(ComponentObject, SpatialConstants::HEARTBEAT_CLIENT_HAS_QUIT_ID, true);

		Cast<USpatialNetDriver>(Driver)->Connection->SendComponentUpdate(PlayerControllerEntity, &Update);
	}
	else
	{
		UE_LOG(LogSpatialNetConnection, Warning, TEXT("Quitting before Heartbeat component has been initialized: NetConnection %s"), *GetName());
	}
}

void USpatialNetConnection::InitHeartbeat(FTimerManager* InTimerManager, Worker_EntityId InPlayerControllerEntity)
{
	UE_LOG(LogSpatialNetConnection, Log, TEXT("Init Heartbeat component: NetConnection %s, PlayerController entity %lld"), *GetName(), InPlayerControllerEntity);

	checkf(PlayerControllerEntity == SpatialConstants::INVALID_ENTITY_ID, TEXT("InitHeartbeat: PlayerControllerEntity already set: %lld. New entity: %lld"), PlayerControllerEntity, InPlayerControllerEntity);
	PlayerControllerEntity = InPlayerControllerEntity;
	TimerManager = InTimerManager;

	if (Driver->IsServer())
	{
		SetHeartbeatTimeoutTimer();
	}
	else
	{
		SetHeartbeatEventTimer();
	}
}

void USpatialNetConnection::SetHeartbeatTimeoutTimer()
{
	float Timeout = GetDefault<USpatialGDKSettings>()->HeartbeatTimeoutSeconds;
#if WITH_EDITOR
	Timeout = GetDefault<USpatialGDKSettings>()->HeartbeatTimeoutWithEditorSeconds;
#endif

	TimerManager->SetTimer(HeartbeatTimer, [WeakThis = TWeakObjectPtr<USpatialNetConnection>(this)]()
	{
		if (USpatialNetConnection* Connection = WeakThis.Get())
		{
			// This client timed out. Disconnect it and trigger OnDisconnected logic.
			Connection->CleanUp();
		}
	}, Timeout, false);
}

void USpatialNetConnection::SetHeartbeatEventTimer()
{
	TimerManager->SetTimer(HeartbeatTimer, [WeakThis = TWeakObjectPtr<USpatialNetConnection>(this)]()
	{
		if (USpatialNetConnection* Connection = WeakThis.Get())
		{
			FWorkerComponentUpdate ComponentUpdate = {};

			ComponentUpdate.component_id = SpatialConstants::HEARTBEAT_COMPONENT_ID;
			ComponentUpdate.schema_type = Schema_CreateComponentUpdate();
			Schema_Object* EventsObject = Schema_GetComponentUpdateEvents(ComponentUpdate.schema_type);
			Schema_AddObject(EventsObject, SpatialConstants::HEARTBEAT_EVENT_ID);

			USpatialWorkerConnection* WorkerConnection = Cast<USpatialNetDriver>(Connection->Driver)->Connection;
			if (WorkerConnection != nullptr)
			{
				WorkerConnection->SendComponentUpdate(Connection->PlayerControllerEntity, &ComponentUpdate);
			}
		}
	}, GetDefault<USpatialGDKSettings>()->HeartbeatIntervalSeconds, true, 0.0f);
}

void USpatialNetConnection::DisableHeartbeat()
{
	// Remove the heartbeat callback
	if (TimerManager != nullptr && HeartbeatTimer.IsValid())
	{
		TimerManager->ClearTimer(HeartbeatTimer);
	}
	PlayerControllerEntity = SpatialConstants::INVALID_ENTITY_ID;
}

void USpatialNetConnection::OnHeartbeat()
{
	SetHeartbeatTimeoutTimer();
}
