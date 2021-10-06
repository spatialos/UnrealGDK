// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TestUnrealComponentsActor.generated.h"

class UDataOnlyComponent;
class UDataAndHandoverComponent;
class UDataAndOwnerOnlyComponent;
class UDataAndInitialOnlyComponent;

/**
 * A replicated, always relevant Actor used to test Unreal components.
 */

UCLASS()
class ATestUnrealComponentsActor : public AActor
{
	GENERATED_BODY()

public:
	ATestUnrealComponentsActor();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void SpawnDynamicComponents();
	bool AreAllDynamicComponentsValid() const;

	UPROPERTY(Replicated)
	UDataOnlyComponent* StaticDataOnlyComponent;

	UPROPERTY(Replicated)
	UDataOnlyComponent* DynamicDataOnlyComponent;

	UPROPERTY(Replicated)
	UDataAndHandoverComponent* StaticDataAndHandoverComponent;

	UPROPERTY(Replicated)
	UDataAndHandoverComponent* DynamicDataAndHandoverComponent;

	UPROPERTY(Replicated)
	UDataAndOwnerOnlyComponent* StaticDataAndOwnerOnlyComponent;

	UPROPERTY(Replicated)
	UDataAndOwnerOnlyComponent* DynamicDataAndOwnerOnlyComponent;

	UPROPERTY(Replicated)
	UDataAndInitialOnlyComponent* StaticDataAndInitialOnlyComponent;

	UPROPERTY(Replicated)
	UDataAndInitialOnlyComponent* DynamicDataAndInitialOnlyComponent;

private:
	template <class T>
	T* SpawnDynamicComponent(FName Name)
	{
		T* NewComponent = NewObject<T>(this, Name);
		NewComponent->SetupAttachment(GetRootComponent());
		NewComponent->RegisterComponent();
		return NewComponent;
	}
};
