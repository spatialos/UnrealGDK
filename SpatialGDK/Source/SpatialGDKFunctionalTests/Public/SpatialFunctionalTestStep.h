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

	UPROPERTY(BlueprintReadWrite, Category = "Spatial Functional Test Step Definition")
	FString StepName;

	bool bIsNativeDefinition;

	UPROPERTY(BlueprintReadWrite, Category = "Spatial Functional Test Step Definition")
	FStepIsReadyDelegate IsReadyEvent;
	UPROPERTY(BlueprintReadWrite, Category = "Spatial Functional Test Step Definition")
	FStepStartDelegate StartEvent;
	UPROPERTY(BlueprintReadWrite, Category = "Spatial Functional Test Step Definition")
	FStepTickDelegate TickEvent;

	FNativeStepIsReadyDelegate NativeIsReadyEvent;
	FNativeStepStartDelegate NativeStartEvent;
	FNativeStepTickDelegate NativeTickEvent;

	UPROPERTY(BlueprintReadWrite, Category = "Spatial Functional Test Step Definition")
	TArray<FWorkerDefinition> Workers;

	UPROPERTY(BlueprintReadWrite, Category = "Spatial Functional Test Step Definition")
	float TimeLimit;
};

UCLASS(BlueprintType, Blueprintable)
class USpatialFunctionalTestStepDefinitionWrapper : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Functional Test Step Definition")
	FString StepName;

	bool bIsNativeDefinition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Functional Test Step Definition")
	FStepIsReadyDelegate IsReadyEvent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Functional Test Step Definition")
	FStepStartDelegate StartEvent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Functional Test Step Definition")
	FStepTickDelegate TickEvent;
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

