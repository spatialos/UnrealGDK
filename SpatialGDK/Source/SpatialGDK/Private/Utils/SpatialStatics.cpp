// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/SpatialStatics.h"

#include "Engine/World.h"
#include "EngineClasses/SpatialGameInstance.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "EngineClasses/SpatialWorldSettings.h"
#include "GeneralProjectSettings.h"
#include "Interop/SpatialWorkerFlags.h"
#include "Kismet/KismetSystemLibrary.h"
#include "LoadBalancing/LayeredLBStrategy.h"
#include "LoadBalancing/SpatialMultiWorkerSettings.h"
#include "SpatialConstants.h"
#include "SpatialGDKSettings.h"
#include "Utils/InspectionColors.h"

DEFINE_LOG_CATEGORY(LogSpatial);

namespace
{
bool CanProcessActor(const AActor* Actor)
{
	if (Actor == nullptr)
	{
		UE_LOG(LogSpatial, Error, TEXT("Calling locking API functions on nullptr Actor is invalid."));
		return false;
	}

	const UNetDriver* NetDriver = Actor->GetWorld()->GetNetDriver();
	if (!NetDriver->IsServer())
	{
		UE_LOG(LogSpatial, Error, TEXT("Calling locking API functions on a client is invalid. Actor: %s"), *GetNameSafe(Actor));
		return false;
	}

	if (!Actor->HasAuthority())
	{
		UE_LOG(LogSpatial, Error, TEXT("Calling locking API functions on a non-auth Actor is invalid. Actor: %s."), *GetNameSafe(Actor));
		return false;
	}

	return true;
}
} // anonymous namespace

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

bool USpatialStatics::IsSpatialMultiWorkerEnabled(const UObject* WorldContextObject)
{
	checkf(WorldContextObject != nullptr, TEXT("Called IsSpatialMultiWorkerEnabled with a nullptr WorldContextObject*"));

	const UWorld* World = WorldContextObject->GetWorld();
	checkf(World != nullptr, TEXT("Called IsSpatialMultiWorkerEnabled with a nullptr World*"));

	const USpatialGDKSettings* SpatialGDKSettings = GetDefault<USpatialGDKSettings>();
	if (SpatialGDKSettings->bOverrideMultiWorker.IsSet())
	{
		return SpatialGDKSettings->bOverrideMultiWorker.GetValue();
	}

	const ASpatialWorldSettings* WorldSettings = Cast<ASpatialWorldSettings>(World->GetWorldSettings());
	return WorldSettings != nullptr && WorldSettings->IsMultiWorkerEnabledInWorldSettings();
}

bool USpatialStatics::IsSpatialOffloadingEnabled(const UWorld* World)
{
	if (World != nullptr)
	{
		if (const ASpatialWorldSettings* WorldSettings = Cast<ASpatialWorldSettings>(World->GetWorldSettings()))
		{
			if (!IsSpatialMultiWorkerEnabled(World))
			{
				return false;
			}

			const UAbstractSpatialMultiWorkerSettings* MultiWorkerSettings =
				WorldSettings->MultiWorkerSettingsClass->GetDefaultObject<UAbstractSpatialMultiWorkerSettings>();
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
			UE_LOG(LogSpatial, Error,
				   TEXT("Called IsActorGroupOwnerForClass before NotifyBeginPlay has been called is invalid. Actor class: %s"),
				   *GetNameSafe(ActorClass));
			return true;
		}

		if (const ULayeredLBStrategy* LBStrategy = Cast<ULayeredLBStrategy>(SpatialNetDriver->LoadBalanceStrategy))
		{
			return LBStrategy->CouldHaveAuthority(ActorClass);
		}
	}
	return true;
}

void USpatialStatics::PrintStringSpatial(UObject* WorldContextObject, const FString& InString /*= FString(TEXT("Hello"))*/,
										 bool bPrintToScreen /*= true*/, FLinearColor TextColor /*= FLinearColor(0.0, 0.66, 1.0)*/,
										 float Duration /*= 2.f*/)
{
	// This will be logged in the SpatialOutput so we don't want to double log this, therefore bPrintToLog is false.
	UKismetSystemLibrary::PrintString(WorldContextObject, InString, bPrintToScreen, false /*bPrintToLog*/, TextColor, Duration);

	// By logging to LogSpatial we will print to the spatial os runtime.
	UE_LOG(LogSpatial, Log, TEXT("%s"), *InString);
}

void USpatialStatics::PrintTextSpatial(UObject* WorldContextObject, const FText InText /*= INVTEXT("Hello")*/,
									   bool bPrintToScreen /*= true*/, FLinearColor TextColor /*= FLinearColor(0.0, 0.66, 1.0)*/,
									   float Duration /*= 2.f*/)
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

FLockingToken USpatialStatics::AcquireLock(AActor* Actor, const FString& DebugString)
{
	if (!CanProcessActor(Actor) || !IsSpatialMultiWorkerEnabled(Actor))
	{
		return FLockingToken{ SpatialConstants::INVALID_ACTOR_LOCK_TOKEN };
	}

	UAbstractLockingPolicy* LockingPolicy = Cast<USpatialNetDriver>(Actor->GetWorld()->GetNetDriver())->LockingPolicy;

	const ActorLockToken LockToken = LockingPolicy->AcquireLock(Actor, DebugString);

	UE_LOG(LogSpatial, Verbose, TEXT("LockingComponent called AcquireLock. Actor: %s. Token: %lld. New lock count: %d"), *Actor->GetName(),
		   LockToken, LockingPolicy->GetActorLockCount(Actor));

	return FLockingToken{ LockToken };
}

bool USpatialStatics::IsLocked(const AActor* Actor)
{
	if (!CanProcessActor(Actor) || !IsSpatialMultiWorkerEnabled(Actor))
	{
		return false;
	}

	return Cast<USpatialNetDriver>(Actor->GetWorld()->GetNetDriver())->LockingPolicy->IsLocked(Actor);
}

void USpatialStatics::ReleaseLock(const AActor* Actor, FLockingToken LockToken)
{
	if (!CanProcessActor(Actor) || !IsSpatialMultiWorkerEnabled(Actor))
	{
		return;
	}

	UAbstractLockingPolicy* LockingPolicy = Cast<USpatialNetDriver>(Actor->GetWorld()->GetNetDriver())->LockingPolicy;
	LockingPolicy->ReleaseLock(LockToken.Token);

	UE_LOG(LogSpatial, Verbose, TEXT("LockingComponent called ReleaseLock. Actor: %s. Token: %lld. Resulting lock count: %d"),
		   *Actor->GetName(), LockToken.Token, LockingPolicy->GetActorLockCount(Actor));
}

FName USpatialStatics::GetLayerName(const UObject* WorldContextObject)
{
	const UWorld* World = WorldContextObject->GetWorld();
	if (World == nullptr)
	{
		UE_LOG(LogSpatial, Error, TEXT("World was nullptr when calling GetLayerName"));
		return NAME_None;
	}

	if (World->IsNetMode(NM_Client))
	{
		return SpatialConstants::DefaultClientWorkerType;
	}

	if (!IsSpatialNetworkingEnabled())
	{
		return SpatialConstants::DefaultLayer;
	}

	const USpatialNetDriver* SpatialNetDriver = Cast<USpatialNetDriver>(World->GetNetDriver());
	if (SpatialNetDriver == nullptr || !SpatialNetDriver->IsReady())
	{
		UE_LOG(LogSpatial, Error,
			   TEXT("Called GetLayerName before NotifyBeginPlay has been called is invalid. Worker doesn't know its layer yet"));
		return NAME_None;
	}

	const ULayeredLBStrategy* LBStrategy = Cast<ULayeredLBStrategy>(SpatialNetDriver->LoadBalanceStrategy);
	check(LBStrategy != nullptr);
	return LBStrategy->GetLocalLayerName();
}
