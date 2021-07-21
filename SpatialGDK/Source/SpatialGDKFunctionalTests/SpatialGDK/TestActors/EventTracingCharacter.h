// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EventTracingCharacter.generated.h"

UCLASS()
class AEventTracingCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AEventTracingCharacter();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "EventTracing")
	bool bUseEventTracing = true;

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	UPROPERTY(ReplicatedUsing = OnRepTestInt)
	int32 TestInt;

	FTimerHandle MoveCharacterTimerHandle;
	FTimerHandle ToggleTestIntTimerHandle;

	UFUNCTION()
	void ToggleTestInt();

	UFUNCTION()
	void SendRPC();

	UFUNCTION(Server, Reliable)
	void RPC();

	UFUNCTION()
	void OnRepTestInt();
};
