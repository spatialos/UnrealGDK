// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/SpatialStatics.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "GeneralProjectSettings.h"
#include "SpatialConstants.h"
#include "SpatialGDKSettings.h"
#include "Utils/ActorGroupManager.h"
#include "Engine/World.h"

DEFINE_LOG_CATEGORY(LogSpatialStatics);

bool USpatialStatics::IsSpatialNetworkingEnabled()
{
    return GetDefault<UGeneralProjectSettings>()->bSpatialNetworking;
}

bool USpatialStatics::IsSpatialOffloadingEnabled()
{
    return IsSpatialNetworkingEnabled() && GetDefault<USpatialGDKSettings>()->bEnableOffloading;
}

bool USpatialStatics::IsActorGroupOwnerForActor(const AActor* Actor)
{
	if (Actor == nullptr)
	{
		return false;
	}

	if (const UWorld* World = Actor->GetWorld())
	{
		if (const USpatialNetDriver* SpatialNetDriver = Cast<USpatialNetDriver>(World->GetNetDriver()))
		{
			if (UActorGroupManager* ActorGroupManager = SpatialNetDriver->ActorGroupManager)
			{
				UClass* ActorClass = Actor->GetClass();
				const FName ClassWorkerType = ActorGroupManager->GetWorkerTypeForClass(ActorClass);
				const FName CurrentWorkerType = World->GetGameInstance()->GetSpatialWorkerType();
				UE_LOG(LogTemp, Display, TEXT("ClassWorkerType [%s], CurrentWorkerType [%s]"), *ClassWorkerType.ToString(), *CurrentWorkerType.ToString())
				return (ClassWorkerType == CurrentWorkerType);
			}
		}		
	}

	return Actor->HasAuthority();
}

bool USpatialStatics::IsActorGroupOwnerForClass(const UObject* WorldContextObject, const TSubclassOf<AActor> ActorClass)
{
	if (const UWorld* World = WorldContextObject->GetWorld())
	{
		if (const USpatialNetDriver* SpatialNetDriver = Cast<USpatialNetDriver>(World->GetNetDriver()))
		{
			if (UActorGroupManager* ActorGroupManager = SpatialNetDriver->ActorGroupManager)
			{
				const FName ClassWorkerType = ActorGroupManager->GetWorkerTypeForClass(ActorClass);
				const FName CurrentWorkerType = World->GetGameInstance()->GetSpatialWorkerType();
				UE_LOG(LogTemp, Display, TEXT("ClassWorkerType [%s], CurrentWorkerType [%s]"), *ClassWorkerType.ToString(), *CurrentWorkerType.ToString())
				return (ClassWorkerType == CurrentWorkerType);
			}
		}
		return (World->GetNetMode() != NM_Client);
	}

	return false;
}

bool USpatialStatics::IsActorGroupOwner(const UObject* WorldContextObject, const FName ActorGroup)
{
	if (const UWorld* World = WorldContextObject->GetWorld())
	{
		if (const USpatialNetDriver* SpatialNetDriver = Cast<USpatialNetDriver>(World->GetNetDriver()))
		{
			if (UActorGroupManager* ActorGroupManager = SpatialNetDriver->ActorGroupManager)
			{
				const FName ActorGroupWorkerType = ActorGroupManager->GetWorkerTypeForActorGroup(ActorGroup);
				const FName CurrentWorkerType = World->GetGameInstance()->GetSpatialWorkerType();
				UE_LOG(LogTemp, Display, TEXT("ActorGroupWorkerType [%s], CurrentWorkerType [%s]"), *ActorGroupWorkerType.ToString(), *CurrentWorkerType.ToString())
				return (ActorGroupWorkerType == CurrentWorkerType);
			}
		}
		return (World->GetNetMode() != NM_Client);
	}

	return false;
}
