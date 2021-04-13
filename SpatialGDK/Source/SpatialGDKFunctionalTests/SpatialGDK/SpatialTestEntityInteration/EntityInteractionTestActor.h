// Copyright (c) Improbable Worl*ds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EntityInteractionTestActor.generated.h"

class USceneComponent;

UCLASS(Blueprintable)
class AEntityInteractionTestActor : public AActor
{
	GENERATED_BODY()

public:
	static const FString s_NetWriteFenceName;
	static const FString s_ReliableName;
	static const FString s_UnreliableName;
	static const FString s_UnorderedName;
	static const FString s_NoLoopbackName;

	AEntityInteractionTestActor();

	UFUNCTION(BlueprintCallable, NetWriteFence, Reliable)
	void TestNetWriteFence(int32 Param);

	UFUNCTION(BlueprintCallable, CrossServer, Reliable)
	void TestReliable(int32 Param);

	UFUNCTION(BlueprintCallable, CrossServer, Unreliable)
	void TestUnreliable(int32 Param);

	UFUNCTION(BlueprintCallable, CrossServer, Reliable, Unordered)
	void TestUnordered(int32 Param);

	UFUNCTION(BlueprintCallable, CrossServer, Reliable, NetWriteFence)
	void TestNoLoopback(int32 Param);

	UPROPERTY()
	uint32 Index;

	TMap<int32, FString> Steps;

	UPROPERTY()
	USceneComponent* SceneComponent;
};
