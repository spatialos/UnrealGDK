// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "EngineUtils.h"
//#include "Kismet/BlueprintFunctionLibrary.h"
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

USTRUCT(BlueprintType, meta = (HasNativeMake = "SpatialGDKFunctionalTests.USpatialFunctionalTestBlueprintLibrary.MakeStepDefinition"))
struct FSpatialFunctionalTestStepDefinition
{
	GENERATED_BODY()

	/**
	 * bIsNative defines that this StepDefinition is meant to be used in C++, so when
	 * defining native StepDefinitions make sure you pass True.
	 */
	FSpatialFunctionalTestStepDefinition(bool bIsNative = false)
		: bIsNativeDefinition(bIsNative)
		, TimeLimit(0.0f)
	{
	}

	UPROPERTY(BlueprintReadWrite, Category = "Spatial Functional Test")
	FString StepName;

	bool bIsNativeDefinition;

	UPROPERTY(BlueprintReadWrite, Category = "Spatial Functional Test")
	FStepIsReadyDelegate IsReadyEvent;
	UPROPERTY(BlueprintReadWrite, Category = "Spatial Functional Test")
	FStepStartDelegate StartEvent;
	UPROPERTY(BlueprintReadWrite, Category = "Spatial Functional Test")
	FStepTickDelegate TickEvent;

	FNativeStepIsReadyDelegate NativeIsReadyEvent;
	FNativeStepStartDelegate NativeStartEvent;
	FNativeStepTickDelegate NativeTickEvent;

	UPROPERTY(BlueprintReadWrite, Category = "Spatial Functional Test")
	TArray<FWorkerDefinition> Workers;

	UPROPERTY(BlueprintReadWrite, Category = "Spatial Functional Test")
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
//
//UCLASS(meta = (ScriptName = "SpatialFunctionalTestLibrary"))
//class USpatialFunctionalTestBlueprintLibrary : public UBlueprintFunctionLibrary
//{
//	GENERATED_BODY()
//
//public:
//
//	UFUNCTION(BlueprintPure, Category = "Spatial Functional Test", meta = (AutoCreateRefTerm = "IsReadyEvent,StartEvent,TickEvent,Workers"))
//	static FSpatialFunctionalTestStepDefinition MakeStepDefinition(const FString& StepName, const FStepIsReadyDelegate& IsReadyEvent, const FStepStartDelegate& StartEvent, const FStepTickDelegate& TickEvent, const TArray<FWorkerDefinition>& Workers, const float StepTimeLimit)
//	{
//		FSpatialFunctionalTestStepDefinition StepDefinition;
//		StepDefinition.StepName = StepName;
//		StepDefinition.IsReadyEvent = IsReadyEvent;
//		StepDefinition.StartEvent = StartEvent;
//		StepDefinition.TickEvent = TickEvent;
//		StepDefinition.Workers.Append(Workers);
//		StepDefinition.TimeLimit = StepTimeLimit;
//		return StepDefinition;
//	}
//};
