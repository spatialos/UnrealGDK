// Fill out your copyright notice in the Description page of Project Settings.

#include "SpatialWorldTimeComponent.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "UnrealNetwork.h"


USpatialWorldTimeComponent::USpatialWorldTimeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicated(true);
}

void USpatialWorldTimeComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USpatialWorldTimeComponent, GameWorldDateTime);
}

void USpatialWorldTimeComponent::BeginPlay()
{
	Super::BeginPlay();

	GetWorld()->GetTimerManager().SetTimer(UpdateTimerHandle, this, &USpatialWorldTimeComponent::RequestGameWorldTimeUpdate, WorldTimeUpdateRate, true, 0.0f);
}

void USpatialWorldTimeComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	GetWorld()->GetTimerManager().ClearTimer(UpdateTimerHandle);
	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
}

FTimerHandle USpatialWorldTimeComponent::SetTimerByFunctionName(UObject* Object, FString FunctionName, float Time, bool bLooping, float FirstDelay)
{
	FTimerHandle Handle;

	if (GetOwner()->Role != ROLE_Authority)
	{
		UE_LOG(LogSpatialWorldTime, Error, TEXT("SetTimer called by an actor without proper authority [%s]."), *GetNameSafe(Object));
		return Handle;
	}

	// Remainder of this function comes from the kismet system library, K2_SetTimer.
	// We are just wrapping it to facilitate the handling of authority transfers at the component level.
	FName const FunctionFName(*FunctionName);

	if (Object)
	{
		UFunction* const Func = Object->FindFunction(FunctionFName);
		if (Func && (Func->ParmsSize > 0))
		{
			// User passed in a valid function, but one that takes parameters
			// FTimerDynamicDelegate expects zero parameters and will choke on execution if it tries
			// to execute a mismatched function
			UE_LOG(LogSpatialWorldTime, Warning, TEXT("SetTimer passed a function (%s) that expects parameters."), *FunctionName);
			return Handle;
		}
	}

	FTimerDynamicDelegate Delegate;
	Delegate.BindUFunction(Object, FunctionFName);

	if (Delegate.IsBound())
	{
		const UWorld* const World = GEngine->GetWorldFromContextObject(Delegate.GetUObject(), EGetWorldErrorMode::LogAndReturnNull);
		if (World)
		{
			FTimerManager& TimerManager = World->GetTimerManager();
			Handle = TimerManager.K2_FindDynamicTimerHandle(Delegate);
			TimerManager.SetTimer(Handle, Delegate, Time, bLooping, FirstDelay);
		}
	}
	else
	{
		UE_LOG(LogBlueprintUserMessages, Warning,
			TEXT("SetTimer passed a bad function (%s) or object (%s)"),
			*Delegate.GetFunctionName().ToString(), *GetNameSafe(Delegate.GetUObject()));
	}

	return Handle;
}

void USpatialWorldTimeComponent::ClearAndInvalidateTimerHandle(UObject* WorldContextObject, UPARAM(ref) FTimerHandle& Handle)
{
	if (GetOwner()->Role != ROLE_Authority)
	{
		UE_LOG(LogBlueprintUserMessages, Error, TEXT("ClearAndInvalidateTimerHandle called by an actor without proper authority [%s]."), *GetNameSafe(GetOwner()));
		return;
	}

	if (Handle.IsValid())
	{
		UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
		if (World)
		{
			World->GetTimerManager().ClearTimer(Handle);
		}
	}
}

FDateTime USpatialWorldTimeComponent::GetGameWorldDateTime() const
{
	return GameWorldDateTime;
}

bool USpatialWorldTimeComponent::UpdateGameWorldTime_Validate(USpatialWorldTimeComponent* TargetComponent)
{
	return ((TargetComponent != nullptr) && (GetOwner()->Role == ROLE_Authority));
}

void USpatialWorldTimeComponent::UpdateGameWorldTime_Implementation(USpatialWorldTimeComponent* TargetComponent)
{
	ASpatialWorldTimeManager* TimerManager{ GetSpatialWorldTimeManager() };

	if (TimerManager != nullptr)
	{
		TimerManager->GetGameWorldTime(TargetComponent);
	}
}

void USpatialWorldTimeComponent::SetGameWorldTime(const FDateTime& DateTime)
{
	GameWorldDateTime = DateTime;
}

ASpatialWorldTimeManager* USpatialWorldTimeComponent::GetSpatialWorldTimeManager() const
{
	TArray<AActor*> TimeManagers;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpatialWorldTimeManager::StaticClass(), TimeManagers);
	if (TimeManagers.Num() != 1)
	{
		UE_LOG(LogSpatialWorldTime, Error, TEXT("Only one TimeManager expected, found %d"), TimeManagers.Num());
		return nullptr;
	}

	return Cast<ASpatialWorldTimeManager>(TimeManagers[0]);
}
