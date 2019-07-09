// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SpatialWorldTimeManager.h"

#include "SpatialWorldTimeComponent.generated.h"

UCLASS( SpatialType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SPATIALGDK_API USpatialWorldTimeComponent : public UActorComponent
{
	GENERATED_BODY()

	static constexpr float DEFAULT_WORLD_TIME_UPDATE_RATE{ 5.0f };

public:	
	USpatialWorldTimeComponent();

	/**
	 * Set a timer to execute delegate. Setting an existing timer will reset that timer with updated parameters.
	 * @param Object		Object that implements the delegate function. Defaults to self (this blueprint)
	 * @param FunctionName	Delegate function name. Can be a K2 function or a Custom Event.
	 * @param Time			How long to wait before executing the delegate, in seconds. Setting a timer to <= 0 seconds will clear it if it is set.
	 * @param bLooping		true to keep executing the delegate every Time seconds, false to execute delegate only once.
	 * @param FirstDelay	The time for the first iteration of a looping timer. If < 0.f inRate will be used.
	 * @return				The timer handle to pass to other timer functions to manipulate this timer.
	 */
	UFUNCTION(BlueprintCallable, meta=(DefaultToSelf = "Object"), Category = "Local Timers")
	FTimerHandle SetTimerByFunctionName(UObject* Object, FString FunctionName, float Time, bool bLooping = false, float FirstDelay = -1.f);

	/**
	 * Clears a set timer.
	 * @param Handle		The handle of the timer to clear.
	 */
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "Local Timers")
	void ClearAndInvalidateTimerHandle(UObject* WorldContextObject, UPARAM(ref) FTimerHandle& Handle);

	/**
	 * Gets the current time of the game world in the form of an FDateTime (yyyy.mm.dd-hh.mm.ss)
	 * @return the FDateTime representation of the current game time
	*/
	UFUNCTION(BlueprintCallable, Category = "World Time")
	FDateTime GetGameWorldDateTime() const;

	/**
	* Client RPC to set the component's World Time value
	*
	* @param DateTime The server's current World Time value
	*/
	UFUNCTION()
	void SetGameWorldTime(const FDateTime& DateTime);

	/** The number of seconds that elapse between each tick */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.1"))
	float WorldTimeUpdateRate = DEFAULT_WORLD_TIME_UPDATE_RATE;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	/**
	* Helper function to obtain the Spatial World Time Manager
	*
	* @return Pointer to the Spatial World Time Manager
	*/
	ASpatialWorldTimeManager* GetSpatialWorldTimeManager() const;

	/**
	* Callback for timers to request an update to the component's World Time
	*/
	UFUNCTION()
	void RequestGameWorldTimeUpdate() { UpdateGameWorldTime(this); }

	/**
	* RPC to request an update to the component's World Time
	*
	* @param TargetComponent The component which should receive the World Time update
	*/
	UFUNCTION(CrossServer, Reliable, WithValidation)
	void UpdateGameWorldTime(USpatialWorldTimeComponent* TargetComponent);

	/** The current in-game date and time. */
	UPROPERTY(Replicated)
	FDateTime GameWorldDateTime = FDateTime::MinValue();

	/** Handle for the timer responsible for world time update cadence. */
	FTimerHandle UpdateTimerHandle;
};
