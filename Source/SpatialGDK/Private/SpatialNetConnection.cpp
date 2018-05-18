// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialNetConnection.h"
#include "SpatialNetDriver.h"
#include "SpatialPackageMapClient.h"
#include "Engine/LocalPlayer.h"
#include "UnrealEngine.h"

USpatialNetConnection::USpatialNetConnection(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	InternalAck = 1;
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

void USpatialNetConnection::InitLocalConnection(UNetDriver* InDriver, class FSocket* InSocket, const FURL& InURL, EConnectionState InState, int32 InMaxPacket, int32 InPacketOverhead)
{
	Super::InitLocalConnection(InDriver, InSocket, InURL, InState, InMaxPacket, InPacketOverhead);
}

// IMPROBABLE: MCS - workaround for UNR-236
void USpatialNetConnection::HandleClientPlayer(class APlayerController* PC, class UNetConnection* NetConnection)
{
	check(Driver->GetWorld());

	// Hook up the Viewport to the new player actor.
	ULocalPlayer*	LocalPlayer = NULL;
	for (FLocalPlayerIterator It(GEngine, Driver->GetWorld()); It; ++It)
	{
		LocalPlayer = *It;
		break;
	}

	// Detach old player if it's in the same level.
	check(LocalPlayer);
	if (LocalPlayer->PlayerController && LocalPlayer->PlayerController->GetLevel() == PC->GetLevel())
	{
		if (LocalPlayer->PlayerController->Role != ROLE_Authority)
		{
			// abort Super call as it will cause issues
			UE_LOG(LogTemp, Error, TEXT("Received bad HandleClientPlayer call! 0x%x"), this);
			return;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Received good HandleClientPlayer call! 0x%x"), this)
		}
	}

	Super::HandleClientPlayer(PC, NetConnection);
}

void USpatialNetConnection::InitRemoteConnection(UNetDriver* InDriver, class FSocket* InSocket, const FURL& InURL, const class FInternetAddr& InRemoteAddr, EConnectionState InState, int32 InMaxPacket, int32 InPacketOverhead)
{
	Super::InitRemoteConnection(InDriver, InSocket, InURL, InRemoteAddr, InState, InMaxPacket, InPacketOverhead);

}

bool USpatialNetConnection::ClientHasInitializedLevelFor(const AActor* TestActor) const
{
	check(Driver->IsServer());
	return true;
	//Intentionally does not call Super::
}

void USpatialNetConnection::LowLevelSend(void * Data, int32 CountBytes, int32 CountBits)
{
	//Intentionally does not call Super::
}

void USpatialNetConnection::Tick()
{
	// Since we're not receiving actual Unreal packets, Unreal may time out the connection. Timeouts are handled by SpatialOS, so we're setting these values here to keep Unreal happy.
	// Note that in the case of InternalAck (UnrealWorker) the engine does this (and more) in Super.
	if (!InternalAck)
	{
		LastReceiveTime = Driver->Time;
		LastReceiveRealtime = FPlatformTime::Seconds();
		LastGoodPacketRealtime = FPlatformTime::Seconds();
	}
	Super::Tick();
}
