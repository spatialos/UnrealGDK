// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpatialWorldTimeManager.generated.h"

class USpatialWorldTimeComponent;

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialWorldTime, Log, All);

UCLASS(SpatialType=(Singleton, ServerOnly))
class SPATIALGDK_API ASpatialWorldTimeManager : public AActor
{
	GENERATED_BODY()

public:	
	ASpatialWorldTimeManager();

	virtual void Tick(float DeltaTime) override;

	UFUNCTION(Server, Reliable, WithValidation, Category = "World Time Manager")
	void GetGameWorldTime(USpatialWorldTimeComponent* TargetComponent);

	UFUNCTION(Server, Reliable, WithValidation, Category = "World Time Manager")
	void InitGameWorldTime(const FDateTime& StartTime, const float Scale);

	UFUNCTION(Server, Reliable, WithValidation)
	void DebugSetGameWorldTime(const FDateTime& StartTime, const float Scale);

protected:
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:

	void SetGameWorldTime(const FDateTime& StartTime, const float Scale);

	// Real-world time the game world launched, in seconds.
	UPROPERTY(Replicated)
	FDateTime RealWorldStartTime;

	// In-game day/time when the server launched
	UPROPERTY(Replicated)
	FDateTime GameWorldStartTime;

	// Relative speed of in-game time to real-world time
	UPROPERTY(Replicated)
	float GameWorldTimeScale;
};
