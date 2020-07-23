// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/SpatialStatics.h"

#include "Engine/World.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "EngineClasses/SpatialWorldSettings.h"
#include "LoadBalancing/SpatialMultiWorkerSettings.h"
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

bool USpatialStatics::IsSpatialMultiWorkerEnabled(const UWorld* World)
{
	checkf(World != nullptr, TEXT("Called IsSpatialMultiWorkerEnabled with a nullptr World*"));

	const ASpatialWorldSettings* WorldSettings = Cast<ASpatialWorldSettings>(World->GetWorldSettings());
	return WorldSettings != nullptr && WorldSettings->IsMultiWorkerEnabled();
}

bool USpatialStatics::IsSpatialOffloadingEnabled(const UWorld* World)
{
	if (World != nullptr)
	{
		if (const ASpatialWorldSettings* WorldSettings = Cast<ASpatialWorldSettings>(World->GetWorldSettings()))
		{
			if (!IsSpatialNetworkingEnabled() || !WorldSettings->IsMultiWorkerEnabled())
			{
				return false;
			}

			const UAbstractSpatialMultiWorkerSettings* MultiWorkerSettings = WorldSettings->MultiWorkerSettingsClass->GetDefaultObject<UAbstractSpatialMultiWorkerSettings>();
			return MultiWorkerSettings->WorkerLayers.Num() > 1;
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

	if (World->IsNetMode(NM_Client))
	{
		return false;
	}

	if (const USpatialNetDriver* SpatialNetDriver = Cast<USpatialNetDriver>(World->GetNetDriver()))
	{
		// Calling IsActorGroupOwnerForClass before NotifyBeginPlay has been called (when NetDriver is ready) is invalid.
		if (!SpatialNetDriver->IsReady())
		{
			UE_LOG(LogSpatial, Error, TEXT("Called IsActorGroupOwnerForClass before NotifyBeginPlay has been called is invalid. Actor class: %s"), *GetNameSafe(ActorClass));
			return true;
		}

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
	if (Actor == nullptr)
	{
		return SpatialConstants::INVALID_ENTITY_ID;
	}

	if (const USpatialNetDriver* SpatialNetDriver = Cast<USpatialNetDriver>(Actor->GetNetDriver()))
	{
		return static_cast<int64>(SpatialNetDriver->PackageMap->GetEntityIdFromObject(Actor));
	}

	return SpatialConstants::INVALID_ENTITY_ID;
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
