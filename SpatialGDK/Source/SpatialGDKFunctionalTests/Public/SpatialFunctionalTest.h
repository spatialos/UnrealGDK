// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "FunctionalTest.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "SpatialFunctionalTestFlowControllerSpawner.h"
#include "SpatialFunctionalTestStep.h"
#include "SpatialFunctionalTest.generated.h"

namespace 
{
	typedef TFunction<bool(ASpatialFunctionalTest* NetTest)> FIsReadyEventFunc;
	typedef TFunction<void(ASpatialFunctionalTest* NetTest)> FStartEventFunc;
	typedef TFunction<void(ASpatialFunctionalTest* NetTest, float DeltaTime)> FTickEventFunc;

	// we need 2 values since the way we clean up tests is based on replication of variables,
	// so if the test fails to start, the cleanup process would never be triggered
	constexpr int SPATIAL_FUNCTIONAL_TEST_NOT_STARTED = -1; // represents test waiting to run
	constexpr int SPATIAL_FUNCTIONAL_TEST_FINISHED = -2;	// represents test already ran
}

/*
 * A Spatial Functional NetTest allows you to define a series of steps, and control which server/client context they execute on
 * Servers and Clients are registered as Test Players by the framework, and request individual steps to be executed in the correct Player
 */
UCLASS(Blueprintable, hidecategories = (Input, Movement, Collision, Rendering, Replication, LOD, "Utilities|Transformation"))
class SPATIALGDKFUNCTIONALTESTS_API ASpatialFunctionalTest : public AFunctionalTest
{
	GENERATED_BODY()

protected:
	TSubclassOf<ASpatialFunctionalTestFlowController> FlowControllerActorClass;

private:
	SpatialFunctionalTestFlowControllerSpawner FlowControllerSpawner;

	UPROPERTY(ReplicatedUsing=StartServerFlowControllerSpawn)
	uint8 bReadyToSpawnServerControllers : 1;

public:
	ASpatialFunctionalTest();

	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	virtual void OnAuthorityGained() override;

	virtual void RegisterAutoDestroyActor(AActor* ActorToAutoDestroy) override;

	// # Test APIs

	int GetNumRequiredClients() const { return NumRequiredClients; }

	// Starts being called after PrepareTest, until it returns true
	virtual bool IsReady_Implementation() override;

	// Called once after IsReady is true
	virtual void StartTest() override; 

	// Ends the Test, can be called from any place.
	virtual void FinishTest(EFunctionalTestResult TestResult, const FString& Message) override;

	UFUNCTION(CrossServer, Reliable)
	void CrossServerFinishTest(EFunctionalTestResult TestResult, const FString& Message);
	
	UFUNCTION(CrossServer, Reliable)
	void CrossServerNotifyStepFinished(ASpatialFunctionalTestFlowController* FlowController);

	// # FlowController related APIs
	
	void RegisterFlowController(ASpatialFunctionalTestFlowController* FlowController);

	// Get all the FlowControllers registered in this Test.
	const TArray<ASpatialFunctionalTestFlowController*>& GetFlowControllers() const { return FlowControllers; }

	UFUNCTION(BlueprintPure, Category = "Spatial Functional Test")
	ASpatialFunctionalTestFlowController* GetFlowController(ESpatialFunctionalTestFlowControllerType ControllerType, int InstanceId);

	// Get the FlowController that is Local to this instance
	UFUNCTION(BlueprintPure, Category = "Spatial Functional Test")
	ASpatialFunctionalTestFlowController* GetLocalFlowController();

	// # Step APIs 

	// Add Steps for Blueprints
	
	UFUNCTION(BlueprintCallable, Category = "Spatial Functional Test", meta = (AutoCreateRefTerm = "IsReadyEvent,StartEvent,TickEvent", ToolTip = "Adds a Step that runs on All Clients and Servers"))
	void AddUniversalStep(const FString& StepName, const FStepIsReadyDelegate& IsReadyEvent, const FStepStartDelegate& StartEvent, const FStepTickDelegate& TickEvent, float StepTimeLimit = 0.0f);

	UFUNCTION(BlueprintCallable, Category = "Spatial Functional Test", meta = (AutoCreateRefTerm = "IsReadyEvent,StartEvent,TickEvent", ClientId = "1", ToolTip = "Adds a Step that runs on Clients. Client Worker Ids start from 1.\n\nIf you pass 0 it will run on All the Clients (there's also a convenience function GetAllWorkersId())"))
	void AddClientStep(const FString& StepName, int ClientId, const FStepIsReadyDelegate& IsReadyEvent, const FStepStartDelegate& StartEvent, const FStepTickDelegate& TickEvent, float StepTimeLimit = 0.0f);

	UFUNCTION(BlueprintCallable, Category = "Spatial Functional Test", meta = (AutoCreateRefTerm = "IsReadyEvent,StartEvent,TickEvent", ServerId = "1", ToolTip = "Adds a Step that runs on Servers. Server Worker Ids start from 1.\n\nIf you pass 0 it will run on All the Servers (there's also a convenience function GetAllWorkersId())"))
	void AddServerStep(const FString& StepName, int ServerId, const FStepIsReadyDelegate& IsReadyEvent, const FStepStartDelegate& StartEvent, const FStepTickDelegate& TickEvent, float StepTimeLimit = 0.0f);

	UFUNCTION(BlueprintCallable, Category = "Spatial Functional Test")
	void AddGenericStep(const FSpatialFunctionalTestStepDefinition& StepDefinition);

	// Add Steps for C++
	FSpatialFunctionalTestStepDefinition& AddUniversalStep(const FString& StepName, FIsReadyEventFunc IsReadyEvent = nullptr, FStartEventFunc StartEvent = nullptr, FTickEventFunc TickEvent = nullptr, float StepTimeLimit = 0.0f);

	FSpatialFunctionalTestStepDefinition& AddClientStep(const FString& StepName, int ClientId, FIsReadyEventFunc IsReadyEvent = nullptr, FStartEventFunc StartEvent = nullptr, FTickEventFunc TickEvent = nullptr, float StepTimeLimit = 0.0f);

	FSpatialFunctionalTestStepDefinition& AddServerStep(const FString& StepName, int ServerId, FIsReadyEventFunc IsReadyEvent = nullptr, FStartEventFunc StartEvent = nullptr, FTickEventFunc TickEvent = nullptr, float StepTimeLimit = 0.0f);

	// Start Running a Step
	void StartStep(const int StepIndex);

	// Terminate current Running Step (called once per FlowController executing it)
	UFUNCTION(BlueprintCallable, Category = "Spatial Functional Test")
	void FinishStep();

	const FSpatialFunctionalTestStepDefinition GetStepDefinition(int StepIndex) const;
	
	int GetCurrentStepIndex() { return CurrentStepIndex; }

	// Convenience function that goes over all FlowControllers and counts how many are Servers
	UFUNCTION(BlueprintPure, Category = "Spatial Functional Test")
	int GetNumberOfServerWorkers();

	// Convenience function that goes over all FlowControllers and counts how many are Clients
	UFUNCTION(BlueprintPure, Category = "Spatial Functional Test")
	int GetNumberOfClientWorkers();

	// Convenience function that returns the Id used for executing steps on all Servers / Clients
	UFUNCTION(BlueprintPure, meta = (ToolTip = "Returns the Id (0) that represents all Workers (ie Server / Client), useful for when you want to have a Server / Client Step run on all of them"), Category = "Spatial Functional Test")
	int GetAllWorkersId() { return FWorkerDefinition::ALL_WORKERS_ID; }

	virtual void GatherRelevantActors(TArray<AActor*>& OutActors) const override;

protected:
	void SetNumRequiredClients(int NewNumRequiredClients) { NumRequiredClients = FMath::Max(NewNumRequiredClients, 0); }
	int GetNumExpectedServers() const { return NumExpectedServers; }
	void DeleteActorsRegisteredForAutoDestroy();

private:
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0"), Category = "Spatial Functional Test")
	int NumRequiredClients = 2;

	// number of servers that should be running in the world
	int NumExpectedServers = 0; 

	// FlowController which is locally owned
	ASpatialFunctionalTestFlowController* LocalFlowController = nullptr;

	TArray<FSpatialFunctionalTestStepDefinition> StepDefinitions;

	TArray<ASpatialFunctionalTestFlowController*> FlowControllersExecutingStep;

	// Time current step has been running for, used if Step Definition has TimeLimit >= 0
	float TimeRunningStep = 0.0f;

	// Current Step Index, < 0 if not executing any, check consts at the top
	UPROPERTY(ReplicatedUsing=OnReplicated_CurrentStepIndex, Transient)
	int CurrentStepIndex = SPATIAL_FUNCTIONAL_TEST_NOT_STARTED;

	UFUNCTION()
	void OnReplicated_CurrentStepIndex();

	UPROPERTY(Replicated, Transient)
	TArray<ASpatialFunctionalTestFlowController*> FlowControllers;
		
	UFUNCTION()
	void StartServerFlowControllerSpawn();

	void SetupClientPlayerRegistrationFlow();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastAutoDestroyActors();
};
