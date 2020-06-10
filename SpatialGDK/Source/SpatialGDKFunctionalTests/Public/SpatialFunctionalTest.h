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
}

/*
 * A Spatial Functional NetTest allows you to define a series of steps, and control which server/client context they execute on
 * Servers and Clients are registered as Test Players by the framework, and request individual steps to be executed in the correct Player
 */
UCLASS(Blueprintable, hidecategories = (Input, Movement, Collision, Rendering, Replication, Tick, LOD, "Utilities|Transformation"))
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
	
	UFUNCTION(BlueprintCallable, meta = (AutoCreateRefTerm = "IsReadyEvent,StartEvent,TickEvent"), Category = "Spatial Functional Test")
	void AddUniversalStep(FString StepName, const FStepIsReadyDelegate& IsReadyEvent, const FStepStartDelegate& StartEvent, const FStepTickDelegate& TickEvent, float StepTimeLimit = 0.0f);

	UFUNCTION(BlueprintCallable, meta = (AutoCreateRefTerm = "IsReadyEvent,StartEvent,TickEvent"), Category = "Spatial Functional Test")
	void AddClientStep(FString StepName, int ClientId, const FStepIsReadyDelegate& IsReadyEvent, const FStepStartDelegate& StartEvent, const FStepTickDelegate& TickEvent, float StepTimeLimit = 0.0f);

	UFUNCTION(BlueprintCallable, meta = (AutoCreateRefTerm = "IsReadyEvent,StartEvent,TickEvent"), Category = "Spatial Functional Test")
	void AddServerStep(FString StepName, int ServerId, const FStepIsReadyDelegate& IsReadyEvent, const FStepStartDelegate& StartEvent, const FStepTickDelegate& TickEvent, float StepTimeLimit = 0.0f);

	UFUNCTION(BlueprintCallable, Category = "Spatial Functional Test")
	void AddGenericStep(const FSpatialFunctionalTestStepDefinition& StepDefinition);

	// Add Steps for C++
	FSpatialFunctionalTestStepDefinition& AddUniversalStep(FString StepName, FIsReadyEventFunc IsReadyEvent = nullptr, FStartEventFunc StartEvent = nullptr, FTickEventFunc TickEvent = nullptr, float StepTimeLimit = 0.0f);

	FSpatialFunctionalTestStepDefinition& AddClientStep(FString StepName, int ClientId, FIsReadyEventFunc IsReadyEvent = nullptr, FStartEventFunc StartEvent = nullptr, FTickEventFunc TickEvent = nullptr, float StepTimeLimit = 0.0f);

	FSpatialFunctionalTestStepDefinition& AddServerStep(FString StepName, int ServerId, FIsReadyEventFunc IsReadyEvent = nullptr, FStartEventFunc StartEvent = nullptr, FTickEventFunc TickEvent = nullptr, float StepTimeLimit = 0.0f);

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

private:
	UPROPERTY(EditAnywhere, meta = (ClampMin = "2"), Category = "Spatial Functional Test") // @TODO currently cannot run tests with less than 2 clients
	int NumRequiredClients = 2;

	// number of servers that should be running in the world
	int NumExpectedServers = 0; 

	// FlowController which is locally owned
	ASpatialFunctionalTestFlowController* LocalFlowController = nullptr;

	TArray<FSpatialFunctionalTestStepDefinition> StepDefinitions;

	TArray<ASpatialFunctionalTestFlowController*> FlowControllersExecutingStep;

	// Time current step has been running for, used if Step Definition has TimeLimit >= 0
	float TimeRunningStep = 0.0f;

	// Current Step Index, -1 if not executing any
	UPROPERTY(ReplicatedUsing=OnReplicated_CurrentStepIndex, Transient)
	int CurrentStepIndex = -1;

	UFUNCTION()
	void OnReplicated_CurrentStepIndex();

	UPROPERTY(Replicated, Transient)
	TArray<ASpatialFunctionalTestFlowController*> FlowControllers;
		
	UFUNCTION()
	void StartServerFlowControllerSpawn();

	void SetupClientPlayerRegistrationFlow();

	UFUNCTION(CrossServer, Reliable)
	void CrossServerRegisterAutoDestroyActor(AActor* ActorToAutoDestroy);

	UFUNCTION(Server, Reliable)
	void ServerRegisterAutoDestroyActor(AActor* ActorToAutoDestroy);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastAutoDestroyActors(const TArray<AActor*>& ActorsToDestroy);
};
