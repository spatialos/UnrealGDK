// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/SpatialNetConnection.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "Gameframework/PlayerController.h"
#include "Interop/Connection/SpatialWorkerConnection.h"

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

int32 USpatialNetConnection::IsNetReady(bool Saturate)
{
	// TODO: UNR-664 - Currently we do not report the number of bits sent when replicating, this means channel saturation cannot be checked properly.
	// This will always return true until we solve this.
	return true;
}

void USpatialNetConnection::UpdateLevelVisibility(const FName& PackageName, bool bIsVisible)
{
	UNetConnection::UpdateLevelVisibility(PackageName, bIsVisible);

	USpatialNetDriver* NetDriver = Cast<USpatialNetDriver>(Driver);
	USpatialClassInfoManager* ClassInfoManager = NetDriver->ClassInfoManager;
	USpatialPackageMapClient* PackageMapClient = Cast<USpatialPackageMapClient>(PackageMap);

	UPackage* TempPkg = FindPackage(nullptr, *PackageName.ToString());
	UWorld* LevelWorld = (UWorld*)FindObjectWithOuter(TempPkg, UWorld::StaticClass());
	uint32 ComponentId = ClassInfoManager->SchemaDatabase->LevelNameToComponentId[LevelWorld->GetName()];

	if (bIsVisible)
	{
		CurrentInterest.LoadedLevels.Add(ComponentId);
	}
	else
	{
		CurrentInterest.LoadedLevels.Remove(ComponentId);
	}

	const FClassInfo& Info = ClassInfoManager->GetOrCreateClassInfoByClass(PlayerController->GetClass());
	Interest NewInterest;
	NewInterest.ComponentInterest.Add(Info.SchemaComponents[SCHEMA_ClientRPC], CurrentInterest.CreateComponentInterest());
	Worker_ComponentUpdate Update = NewInterest.CreateInterestUpdate();

	Worker_EntityId EntityId = PackageMapClient->GetEntityIdFromObject(PlayerController);
	NetDriver->Connection->SendComponentUpdate(EntityId, &Update);
}
