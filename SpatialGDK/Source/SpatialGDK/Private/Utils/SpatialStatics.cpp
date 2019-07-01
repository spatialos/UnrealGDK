// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/SpatialStatics.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "GeneralProjectSettings.h"
#include "SpatialConstants.h"
#include "SpatialGDKSettings.h"
#include "Utils/ActorGroupManager.h"

DEFINE_LOG_CATEGORY(LogSpatialStatics);

bool USpatialStatics::SpatialNetworkingEnabled()
{
    return GetDefault<UGeneralProjectSettings>()->bSpatialNetworking;
}

bool USpatialStatics::SpatialOffloadingEnabled()
{
    return SpatialNetworkingEnabled() && GetDefault<USpatialGDKSettings>()->bEnableOffloading;
}

bool USpatialStatics::IsAuthoritativeWorkerType(const AActor* Actor)
{
    if (Actor == nullptr)
    {
        UE_LOG(LogSpatialStatics, Warning, TEXT("Actor was nullptr in USpatialStatics::IsAuthoritativeWorkerType"));
        return false;
    }

    if (!SpatialNetworkingEnabled() || !SpatialOffloadingEnabled()) 
    {
        return Actor->HasAuthority();
    }

    UWorld* World = Actor->GetWorld();
    if (!World)
    {
        UE_LOG(LogSpatialStatics, Warning, TEXT("World was nullptr in USpatialStatics::IsAuthoritativeWorkerType for actor: %s", *Actor->GetName()));
        return false;
    }

	USpatialNetDriver* SpatialNetDriver = Cast<USpatialNetDriver>(World->GetNetDriver());
	if (!SpatialNetDriver)
	{
		UE_LOG(LogSpatialStatics, Warning, TEXT("SpatialNetDriver was nullptr in USpatialStatics::IsAuthoritativeWorkerType for actor: %s", *Actor->GetName()));
		return false;
	}

    UActorGroupManager* ActorGroupManager = SpatialNetDriver->ActorGroupManager;
    return ActorGroupManager->GetWorkerTypeForClass(Actor->GetClass()).Compare(World->GetGameInstance()->GetSpatialWorkerType()) == 0;
}

bool USpatialStatics::IsDefaultWorker(const UObject* WorldContextObject)
{
    UWorld* World = WorldContextObject->GetWorld();
    if (!World)
    {
        UE_LOG(LogSpatialStatics, Warning, TEXT("World was nullptr in USpatialStatics::IsDefaultWorker"));
        return false;
    }

    return World->GetGameInstance()->GetSpatialWorkerType().Compare(SpatialConstants::DefaultServerWorkerType) == 0;
}
