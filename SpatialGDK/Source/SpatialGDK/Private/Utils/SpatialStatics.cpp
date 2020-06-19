// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/SpatialStatics.h"

#include "Engine/World.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "EngineClasses/SpatialWorldSettings.h"
#include "GeneralProjectSettings.h"
#include "Interop/SpatialWorkerFlags.h"
#include "Kismet/KismetSystemLibrary.h"
#include "SpatialConstants.h"
#include "EngineClasses/SpatialGameInstance.h"
#include "LoadBalancing/LayeredLBStrategy.h"
#include "SpatialGDKSettings.h"
#include "Utils/InspectionColors.h"

DEFINE_LOG_CATEGORY(LogSpatial);

bool USpatialStatics::IsSpatialNetworkingEnabled()
{
    return GetDefault<UGeneralProjectSettings>()->UsesSpatialNetworking();
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

bool USpatialStatics::GetWorkerFlag(const UObject* WorldContext, const FString& InFlagName, FString& OutFlagValue)
{
	if (const UWorld* World = WorldContext->GetWorld())
	{
		if (const USpatialNetDriver* SpatialNetDriver = Cast<USpatialNetDriver>(World->GetNetDriver()))
		{
			if (const USpatialWorkerFlags* SpatialWorkerFlags = SpatialNetDriver->SpatialWorkerFlags) 
			{
				return SpatialWorkerFlags->GetWorkerFlag(InFlagName, OutFlagValue);
			}
		}
	}

	return false;
}

TArray<FDistanceFrequencyPair> USpatialStatics::GetNCDDistanceRatios()
{
	return GetDefault<USpatialGDKSettings>()->InterestRangeFrequencyPairs;
}

float USpatialStatics::GetFullFrequencyNetCullDistanceRatio()
{
	return GetDefault<USpatialGDKSettings>()->FullFrequencyNetCullDistanceRatio;
}

FColor USpatialStatics::GetInspectorColorForWorkerName(const FString& WorkerName)
{
	return SpatialGDK::GetColorForWorkerName(WorkerName);
}

bool USpatialStatics::IsSpatialOffloadingEnabled(const UWorld* World)
{
	if (World != nullptr)
	{
		const ASpatialWorldSettings* WorldSettings = Cast<ASpatialWorldSettings>(World->GetWorldSettings());
		if (WorldSettings != nullptr)
		{
			return IsSpatialNetworkingEnabled() && WorldSettings->WorkerLayers.Num() > 0;
		}
	}

	return false;
}

bool USpatialStatics::IsActorGroupOwnerForActor(const AActor* Actor)
{
	if (Actor == nullptr)
	{
		return false;
	}

	// Offloading using the Unreal Load Balancing always load balances based on the owning actor.
	const AActor* RootOwner = Actor;
	while (RootOwner->GetOwner() != nullptr && RootOwner->GetOwner()->GetIsReplicated())
	{
		RootOwner = RootOwner->GetOwner();
	}

	return IsActorGroupOwnerForClass(RootOwner, RootOwner->GetClass());
}

bool USpatialStatics::IsActorGroupOwnerForClass(const UObject* WorldContextObject, const TSubclassOf<AActor> ActorClass)
{
	const UWorld* World = WorldContextObject->GetWorld();
	if (World == nullptr)
	{
		return false;
	}

	if (const USpatialNetDriver* SpatialNetDriver = Cast<USpatialNetDriver>(World->GetNetDriver()))
	{
		if (const ULayeredLBStrategy* LBStrategy = Cast<ULayeredLBStrategy>(SpatialNetDriver->LoadBalanceStrategy))
		{
			return LBStrategy->CouldHaveAuthority(ActorClass);
		}
	}
	return true;
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

int64 USpatialStatics::GetActorEntityId(const AActor* Actor)
{
	check(Actor);
	if (const USpatialNetDriver* SpatialNetDriver = Cast<USpatialNetDriver>(Actor->GetNetDriver()))
	{
		return static_cast<int64>(SpatialNetDriver->PackageMap->GetEntityIdFromObject(Actor));
	}
	return 0;
}

FString USpatialStatics::EntityIdToString(int64 EntityId)
{
	if (EntityId <= SpatialConstants::INVALID_ENTITY_ID)
	{
		return FString("Invalid");
	}

	return FString::Printf(TEXT("%lld"), EntityId);
}

FString USpatialStatics::GetActorEntityIdAsString(const AActor* Actor)
{
	return EntityIdToString(GetActorEntityId(Actor));
}
