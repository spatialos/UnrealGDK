// Fill out your copyright notice in the Description page of Project Settings.

#include "SpatialWorldTimeManager.h"

#include "SpatialWorldTimeComponent.h"
#include "UnrealNetwork.h"

DEFINE_LOG_CATEGORY(LogSpatialWorldTime);

ASpatialWorldTimeManager::ASpatialWorldTimeManager()
	: RealWorldStartTime(0),
	  GameWorldStartTime(0),
	  GameWorldTimeScale(1.f)
{
	PrimaryActorTick.bCanEverTick = true;
	SetReplicates(true);
}

void ASpatialWorldTimeManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ASpatialWorldTimeManager::BeginPlay()
{
	Super::BeginPlay();

	if (RealWorldStartTime == FDateTime{0})
	{
		RealWorldStartTime = FDateTime::Now();
	}
}

void ASpatialWorldTimeManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASpatialWorldTimeManager, RealWorldStartTime);
	DOREPLIFETIME(ASpatialWorldTimeManager, GameWorldStartTime);
	DOREPLIFETIME(ASpatialWorldTimeManager, GameWorldTimeScale);
}

bool ASpatialWorldTimeManager::GetGameWorldTime_Validate(USpatialWorldTimeComponent* TargetComponent)
{
	return ((TargetComponent != nullptr) && (GameWorldStartTime != FDateTime{0}));
}

void ASpatialWorldTimeManager::GetGameWorldTime_Implementation(USpatialWorldTimeComponent* TargetComponent)
{
	TargetComponent->SetGameWorldTime(GameWorldStartTime + ((FDateTime::Now() - RealWorldStartTime) * GameWorldTimeScale));
}

bool ASpatialWorldTimeManager::InitGameWorldTime_Validate(const FDateTime& StartTime, const float Scale)
{
	const bool IsUnset{ GameWorldStartTime == FDateTime{0} };
	const bool ValidInitTime{ StartTime.Validate(StartTime.GetYear(), StartTime.GetMonth(), StartTime.GetDay(), StartTime.GetHour(), StartTime.GetMinute(), StartTime.GetSecond(), StartTime.GetMillisecond()) };
	const bool ValidScale{ Scale > 0.f };
	
	return (IsUnset && ValidInitTime && ValidScale);
}

void ASpatialWorldTimeManager::InitGameWorldTime_Implementation(const FDateTime& StartTime, const float Scale)
{
	SetGameWorldTime(StartTime, Scale);
}

bool ASpatialWorldTimeManager::DebugSetGameWorldTime_Validate(const FDateTime& StartTime, const float Scale)
{
#if !UE_BUILD_SHIPPING
	const bool ValidInitTime{ StartTime.Validate(StartTime.GetYear(), StartTime.GetMonth(), StartTime.GetDay(), StartTime.GetHour(), StartTime.GetMinute(), StartTime.GetSecond(), StartTime.GetMillisecond()) };
	const bool ValidScale{ Scale > 0.f };

	return (ValidInitTime && ValidScale);
#else
	return false;
#endif
}

void ASpatialWorldTimeManager::DebugSetGameWorldTime_Implementation(const FDateTime& StartTime, const float Scale)
{
#if !UE_BUILD_SHIPPING
	SetGameWorldTime(StartTime, Scale);
#endif
}

void ASpatialWorldTimeManager::SetGameWorldTime(const FDateTime& StartTime, const float Scale)
{
	GameWorldStartTime = StartTime;
	GameWorldTimeScale = Scale;
}
