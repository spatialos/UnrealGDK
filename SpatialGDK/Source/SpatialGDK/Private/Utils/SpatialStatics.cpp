// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/SpatialStatics.h"

#include "Engine/World.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "GeneralProjectSettings.h"
#include "Kismet/KismetSystemLibrary.h"
#include "SpatialConstants.h"
#include "SpatialGDKSettings.h"
#include "Utils/ActorGroupManager.h"

DEFINE_LOG_CATEGORY(LogSpatial);

bool USpatialStatics::IsSpatialNetworkingEnabled()
{
    return GetDefault<UGeneralProjectSettings>()->UsesSpatialNetworking();
}

UActorGroupManager* USpatialStatics::GetActorGroupManager(const UObject* WorldContext)
{
	if (const UWorld* World = WorldContext->GetWorld())
	{
		if (const USpatialNetDriver* SpatialNetDriver = Cast<USpatialNetDriver>(World->GetNetDriver()))
		{
			return SpatialNetDriver->ActorGroupManager;
		}
	}
	return nullptr;
}

FName USpatialStatics::GetCurrentWorkerType(const UObject* WorldContext)
{
	if (const UWorld* World = WorldContext->GetWorld())
	{
		if (const UGameInstance* GameInstance = World->GetGameInstance())
		{
			return GameInstance->GetSpatialWorkerType();
		}
	}

	return NAME_None;
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

	return IsActorGroupOwnerForClass(Actor, Actor->GetClass());
}

bool USpatialStatics::IsActorGroupOwnerForClass(const UObject* WorldContextObject, const TSubclassOf<AActor> ActorClass)
{
	if (UActorGroupManager* ActorGroupManager = GetActorGroupManager(WorldContextObject))
	{
		const FName ClassWorkerType = ActorGroupManager->GetWorkerTypeForClass(ActorClass);
		const FName CurrentWorkerType = GetCurrentWorkerType(WorldContextObject);
		return ClassWorkerType == CurrentWorkerType;
	}

	if (const UWorld* World = WorldContextObject->GetWorld())
	{
		return World->GetNetMode() != NM_Client;
	}

	return false;
}

bool USpatialStatics::IsActorGroupOwner(const UObject* WorldContextObject, const FName ActorGroup)
{
	if (UActorGroupManager* ActorGroupManager = GetActorGroupManager(WorldContextObject))
	{
		const FName ActorGroupWorkerType = ActorGroupManager->GetWorkerTypeForActorGroup(ActorGroup);
		const FName CurrentWorkerType = GetCurrentWorkerType(WorldContextObject);
		return ActorGroupWorkerType == CurrentWorkerType;
	}

	if (const UWorld* World = WorldContextObject->GetWorld())
	{
		return World->GetNetMode() != NM_Client;
	}

	return false;
}

FName USpatialStatics::GetActorGroupForActor(const AActor* Actor)
{
	if (UActorGroupManager* ActorGroupManager = GetActorGroupManager(Actor))
	{
		UClass* ActorClass = Actor->GetClass();
		return ActorGroupManager->GetActorGroupForClass(ActorClass);
	}

	return SpatialConstants::DefaultActorGroup;
}

FName USpatialStatics::GetActorGroupForClass(const UObject* WorldContextObject, const TSubclassOf<AActor> ActorClass)
{
	if (UActorGroupManager* ActorGroupManager = GetActorGroupManager(WorldContextObject))
	{
		return ActorGroupManager->GetActorGroupForClass(ActorClass);
	}

	return SpatialConstants::DefaultActorGroup;
}

void USpatialStatics::PrintStringSpatial(UObject* WorldContextObject, const FString& InString /*= FString(TEXT("Hello"))*/, bool bPrintToScreen /*= true*/, FLinearColor TextColor /*= FLinearColor(0.0, 0.66, 1.0)*/, float Duration /*= 2.f*/)
{
	// This will be logged in the SpatialOutput so we don't want to double log this, therefore bPrintToLog is false.
	UKismetSystemLibrary::PrintString(WorldContextObject, InString, bPrintToScreen, false /*bPrintToLog*/, TextColor, Duration);

	// By logging to LogSpatial we will print to the spatial os runtime.
	UE_LOG(LogSpatial, Log, TEXT("%s"), *InString);
}

void USpatialStatics::PrintTextSpatial(UObject* WorldContextObject, const FText InText /*= INVTEXT("Hello")*/, bool bPrintToScreen /*= true*/, FLinearColor TextColor /*= FLinearColor(0.0, 0.66, 1.0)*/, float Duration /*= 2.f*/)
{
	PrintStringSpatial(WorldContextObject, InText.ToString(), bPrintToScreen, TextColor, Duration);
}
