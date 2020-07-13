// Copyright (c) Improbable Worlds Ltd, All Rights Reserved


#include "TestReplicatedStartupActor.h"
#include "SpatialFunctionalTestFlowController.h"
#include "Kismet/GameplayStatics.h"
#include "ReplicatedActor.h"
#include "SpatialFunctionalTestFlowController.h"
#include "Net/UnrealNetwork.h"


/**
 * This test automates the ReplicatedStartupActor gym. The gym was used for:
 * - QA workflows Test Replicated startup actor are correctly spawned on all clients
 * - To support QA test case "C1944 Replicated startup actors are correctly spawned on all clients"
 * 
 * The test includes a single server and two client workers. The client workers begin with a player controller and their default pawns, which they initially possess.
 * The flow is as follows:
 * - Test:
 *  - After two seconds each client asserts that the ReplicatedActor is visible.
 */

ATestReplicatedStartupActor::ATestReplicatedStartupActor()
	: Super()
{
	Author = "Andrei";
	Description = TEXT("Test Replicated Startup Actor");
}

void ATestReplicatedStartupActor::ServerToClientRPC_Implementation(AActor* ReplicatedActor, ATestReplicatedStartupActor* Test)
{
	UE_LOG(LogTemp, Warning, TEXT(" Pe client mergi a?"));

	if (IsValid(ReplicatedActor))
	{
		Test->bIsValidReference = true;
	}
}

void ATestReplicatedStartupActor::ClientToServerRPC_Implementation(AActor* ReplicatedActor, ATestReplicatedStartupActor* Test)
{
	UE_LOG(LogTemp, Warning, TEXT(" HELLO I AM HERE BEFORE THE IF!"));
	if (IsValid(ReplicatedActor))
	{
		UE_LOG(LogTemp, Warning, TEXT(" HELLO I AM HERE Inside THE IF!"))
		Test->bIsValidReference = true;
	}
}


void ATestReplicatedStartupActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATestReplicatedStartupActor, bIsValidReference);
}

void ATestReplicatedStartupActor::BeginPlay()
{
	Super::BeginPlay();

	AddUniversalStep(TEXT("TestReplicatedStartupActorUniversalSetup"), nullptr, [](ASpatialFunctionalTest* NetTest) {
		ATestReplicatedStartupActor* Test = Cast<ATestReplicatedStartupActor>(NetTest);

		Test->SetOwner(Test->GetLocalFlowController()->GetOwner());

		Test->ElapsedTime = 0.0f;
		Test->bIsValidReference = false;

		Test->FinishStep();
		});

	AddClientStep(TEXT("TestReplicatedStarupActorClientCheckStep"), 1, nullptr, nullptr, [](ASpatialFunctionalTest* NetTest, float DeltaTime) {
		ATestReplicatedStartupActor* Test = Cast<ATestReplicatedStartupActor>(NetTest);

	
		TArray<AActor*> ReplicatedActors;
		UGameplayStatics::GetAllActorsOfClass(Test->GetWorld(), AReplicatedActor::StaticClass(), ReplicatedActors);

		checkf(ReplicatedActors.Num() == 1, TEXT("There should be exactly 1 replicated actor"));

		if(Test->ElapsedTime < 0.1f)
		{ 
			Test->ClientToServerRPC(ReplicatedActors[0], Test);
		}	

		if (Test->ElapsedTime > 1.5f)
		{
			Test->AssertTrue(Test->bIsValidReference, TEXT("The server has a valid reference to this client's replicated actor"));
			//Test->FinishStep();
		}

		Test->ElapsedTime += DeltaTime;

	});

	AddServerStep(TEXT("TestReplicatedStartupActorServerCheckStep"), 1, nullptr, [](ASpatialFunctionalTest* NetTest) {
		ATestReplicatedStartupActor* Test = Cast<ATestReplicatedStartupActor>(NetTest);

		TArray<AActor*> ReplicatedActors;
		UGameplayStatics::GetAllActorsOfClass(Test->GetWorld(), AReplicatedActor::StaticClass(), ReplicatedActors);

		checkf(ReplicatedActors.Num() == 1, TEXT("There should be exactly 1 replicated actor"));


		Test->ServerToClientRPC(ReplicatedActors[0], Test);

		Test->AssertTrue(Test->bIsValidReference, TEXT("The client has a valid reference to the server's replicated actor"));

		Test->FinishStep();


		});
}
