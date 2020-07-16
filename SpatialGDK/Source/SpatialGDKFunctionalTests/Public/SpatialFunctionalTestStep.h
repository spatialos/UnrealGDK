// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "SpatialFunctionalTestStep.generated.h"

class ASpatialFunctionalTest;
class ASpatialFunctionalTestFlowController;

// Blueprint Delegates
DECLARE_DYNAMIC_DELEGATE_RetVal(bool, FStepIsReadyDelegate);
DECLARE_DYNAMIC_DELEGATE(FStepStartDelegate);
DECLARE_DYNAMIC_DELEGATE_OneParam(FStepTickDelegate, float, DeltaTime);

// C++ Delegates
DECLARE_DELEGATE_RetVal_OneParam(bool, FNativeStepIsReadyDelegate, ASpatialFunctionalTest*);
DECLARE_DELEGATE_OneParam(FNativeStepStartDelegate, ASpatialFunctionalTest*);
DECLARE_DELEGATE_TwoParams(FNativeStepTickDelegate, ASpatialFunctionalTest*, float /*DeltaTime*/);

UENUM()
enum class ESpatialFunctionalTestFlowControllerType : uint8
{
	Server,
	Client
};


USTRUCT(BlueprintType)
struct FWorkerDefinition
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category="Spatial Functional Test")
	ESpatialFunctionalTestFlowControllerType ControllerType;
	UPROPERTY(BlueprintReadWrite, Category = "Spatial Functional Test")
	int WorkerId;

	static const int ALL_WORKERS_ID = 0;
};

USTRUCT(BlueprintType)
struct FSpatialFunctionalTestStepDefinition
{
	GENERATED_BODY()

	FSpatialFunctionalTestStepDefinition()
		: bIsNativeDefinition(false)
		, TimeLimit(0.0f)
	{
	}

	UPROPERTY()
	FString StepName;

	bool bIsNativeDefinition;

	UPROPERTY()
	FStepIsReadyDelegate IsReadyEvent;
	UPROPERTY()
	FStepStartDelegate StartEvent;
	UPROPERTY()
	FStepTickDelegate TickEvent;

	FNativeStepIsReadyDelegate NativeIsReadyEvent;
	FNativeStepStartDelegate NativeStartEvent;
	FNativeStepTickDelegate NativeTickEvent;

	UPROPERTY()
	TArray<FWorkerDefinition> Workers;

	UPROPERTY()
	float TimeLimit;
};


class SpatialFunctionalTestStep
{
public:
	SpatialFunctionalTestStep();

	void Start(FSpatialFunctionalTestStepDefinition NewStepDefinition);

	void Tick(float DeltaTime);

	void Reset();

	bool HasReadyEvent();

	ASpatialFunctionalTest* Owner;
	bool bIsRunning;
	bool bIsReady;
	
	FSpatialFunctionalTestStepDefinition StepDefinition;
};

