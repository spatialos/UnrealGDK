// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialNetConnection.h"
#include "SpatialNetDriver.h"
#include "SpatialPackageMapClient.h"

USpatialNetConnection::USpatialNetConnection(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void USpatialNetConnection::InitBase(UNetDriver* InDriver, class FSocket* InSocket, const FURL& InURL, EConnectionState InState, int32 InMaxPacket /*= 0*/, int32 InPacketOverhead /*= 0*/)
{
	Super::InitBase(InDriver, InSocket, InURL, InState, InMaxPacket, InPacketOverhead);

	auto PackageMapClient = NewObject<USpatialPackageMapClient>(this);
	PackageMapClient->Initialize(this, InDriver->GuidCache);

	PackageMap = PackageMapClient;
}

void USpatialNetConnection::InitLocalConnection(UNetDriver* InDriver, class FSocket* InSocket, const FURL& InURL, EConnectionState InState, int32 InMaxPacket, int32 InPacketOverhead)
{
	Super::InitLocalConnection(InDriver, InSocket, InURL, InState, InMaxPacket, InPacketOverhead);

	// Set up the UpdateInterop on the client.
	USpatialNetDriver* SpatialNetDriver = Cast<USpatialNetDriver>(InDriver);
	check(SpatialNetDriver);
	SpatialNetDriver->GetSpatialInterop()->Init(true, SpatialNetDriver->GetSpatialOS(), SpatialNetDriver, &SpatialNetDriver->GetTimerManager());
}

void USpatialNetConnection::InitRemoteConnection(UNetDriver* InDriver, class FSocket* InSocket, const FURL& InURL, const class FInternetAddr& InRemoteAddr, EConnectionState InState, int32 InMaxPacket, int32 InPacketOverhead)
{
	// TODO(David): InternalAck=1 was moved to here from the constructor, to ensure that clients receiving SpatialOS updates have
	// InternalAck set to 0. This avoids a crash. Once we remove bunch handling, we can move InternalAck = 1 back to the constructor.
	InternalAck = 1;
	Super::InitRemoteConnection(InDriver, InSocket, InURL, InRemoteAddr, InState, InMaxPacket, InPacketOverhead);
}

bool USpatialNetConnection::ClientHasInitializedLevelFor(const UObject* TestObject) const
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
