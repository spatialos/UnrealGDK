// Fill out your copyright notice in the Description page of Project Settings.

#include "SpatialNetConnection.h"
#include "SpatialPackageMapClient.h"

void USpatialNetConnection::InitBase(UNetDriver* InDriver, class FSocket* InSocket, const FURL& InURL, EConnectionState InState, int32 InMaxPacket /*= 0*/, int32 InPacketOverhead /*= 0*/)
{
	Super::InitBase(InDriver, InSocket, InURL, InState, InMaxPacket, InPacketOverhead);

	auto PackageMapClient = NewObject<USpatialPackageMapClient>(this);
	PackageMapClient->Initialize(this, InDriver->GuidCache);
	PackageMap = PackageMapClient;
}
