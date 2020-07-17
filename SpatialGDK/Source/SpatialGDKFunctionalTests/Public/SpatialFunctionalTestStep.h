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
	Client,
	All		// Special type that allows you to reference all the Servers and Clients
};


USTRUCT(BlueprintType)
struct FWorkerDefinition
{
	GENERATED_BODY()

	// Type of Worker, usually Server or Client.
	UPROPERTY(BlueprintReadWrite, Category="Spatial Functional Test")
	ESpatialFunctionalTestFlowControllerType Type = ESpatialFunctionalTestFlowControllerType::Server;

	// Ids of Workers start from 1.
	UPROPERTY(BlueprintReadWrite, Category = "Spatial Functional Test")
	int Id = 1;

	// Id that represents all workers, useful when you want to run a Step on all Clients or Servers.
	static const int ALL_WORKERS_ID = 0;

	// Definition that represents all workers (both Client and Server).
	static const FWorkerDefinition AllWorkers;

	// Definition that represents all Server workers
	static const FWorkerDefinition AllServers;

	// Definition that represents all Client workers
	static const FWorkerDefinition AllClients;

	// Helper for Server Worker Definition
	static FWorkerDefinition Server(int ServerId);

	// Helper for Client Worker Definition
	static FWorkerDefinition Client(int ClientId);
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

	// Description so that in the logs you can clearly identify Test Steps
	UPROPERTY()
	FString StepName;

	// Given that we support different delegate types for C++ and BP
	// this is a variable to distinguish what kind you're running
	bool bIsNativeDefinition;

	// BP Delegates
	UPROPERTY()
	FStepIsReadyDelegate IsReadyEvent;
	UPROPERTY()
	FStepStartDelegate StartEvent;
	UPROPERTY()
	FStepTickDelegate TickEvent;

	// C++ Delegates
	FNativeStepIsReadyDelegate NativeIsReadyEvent;
	FNativeStepStartDelegate NativeStartEvent;
	FNativeStepTickDelegate NativeTickEvent;

	// Workers the Test Step should run on
	UPROPERTY()
	TArray<FWorkerDefinition> Workers;

	// Maximum time it can take to finish this Step; if <= 0 it falls back to the time limit of the whole Test
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
