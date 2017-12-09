// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialNetConnection.h"
#include "SpatialPackageMapClient.h"

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

	// Note that this is called from the client where there is only one connection.
	bVanillaUnrealConnection = true;
}

void USpatialNetConnection::InitRemoteConnection(UNetDriver* InDriver, class FSocket* InSocket, const FURL& InURL, const class FInternetAddr& InRemoteAddr, EConnectionState InState, int32 InMaxPacket, int32 InPacketOverhead)
{
	Super::InitRemoteConnection(InDriver, InSocket, InURL, InRemoteAddr, InState, InMaxPacket, InPacketOverhead);

	bVanillaUnrealConnection = InSocket ? true : false;
	if (!bVanillaUnrealConnection)
	{
		InternalAck = true;
	}
	bFakeSpatialClient = true;
}

void USpatialNetConnection::LowLevelSend(void* Data, int32 CountBytes, int32 CountBits)
{
	if (bVanillaUnrealConnection)
	{
		// Not sending anything anymore.
		//Super::LowLevelSend(Data, CountBytes, CountBits);
	}
}

bool USpatialNetConnection::ClientHasInitializedLevelFor(const UObject* TestObject) const
{
	if (bFakeSpatialClient)
		return true;

	return Super::ClientHasInitializedLevelFor(TestObject);	
}
