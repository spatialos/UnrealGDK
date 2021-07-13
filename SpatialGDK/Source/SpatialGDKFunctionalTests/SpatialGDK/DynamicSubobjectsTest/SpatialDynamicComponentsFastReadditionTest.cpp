#include "SpatialGDK/DynamicSubobjectsTest/SpatialDynamicComponentsFastReadditionTest.h"

#include "Algo/Count.h"
#include "Engine/World.h"
#include "EngineUtils.h"

/*
 * This tests UNR-5818: A Spatial component is duplicated if an Unreal component is
 * recreated in between USpatialActorChannel::ReplicateActor calls.
 *
 * The steps are:
 * * Create a test replicated actor
 * * Add a special self-recreating dynamic component which would recreate itself after the first replication.
 * * Wait to confirm only one component is present (and to catch any errors that happen in the meantime)
 */
ASpatialDynamicComponentsFastReadditionTest::ASpatialDynamicComponentsFastReadditionTest()
{
	Author = TEXT("Dmitrii Kozlov <dmitriikozlov@improbable.io>");
}

void ASpatialDynamicComponentsFastReadditionTest::PrepareTest()
{
	Super::PrepareTest();

	AddStep(TEXT("Spawn the test actor"), FWorkerDefinition::Server(1), nullptr, [this] {
		ADynamicComponentsTestActor* TestActor = GetWorld()->SpawnActor<ADynamicComponentsTestActor>();
		if (!AssertIsValid(TestActor, TEXT("Spawned test actor successfully")))
		{
			return;
		}
		USelfRecreatingDynamicComponent* DynamicComponent = NewObject<USelfRecreatingDynamicComponent>(TestActor, TEXT("Initial"));
		if (!AssertIsValid(DynamicComponent, TEXT("Spawned initial component successfully")))
		{
			return;
		}
		DynamicComponent->RegisterComponent();
		FinishStep();
	});

	AddStep(
		TEXT("Wait to ensure only one component is observed"), FWorkerDefinition::AllServers, nullptr,
		[this] {
			StepTimeCounter = 0.0f;
		},
		[this](float DeltaTime) {
			StepTimeCounter += DeltaTime;
			const ADynamicComponentsTestActor* TestActor = FindTestActor();
			if (RequireTrue(IsValid(TestActor), TEXT("Should have the test actor")))
			{
				const int32 DiscoveredRecreatingComponents =
					Algo::CountIf(TestActor->GetComponents(), [](const UActorComponent* Component) {
						return Component->IsA<USelfRecreatingDynamicComponent>();
					});
				RequireEqual_Int(DiscoveredRecreatingComponents, 1, TEXT("Should only have one recreating component"));
			}
			RequireCompare_Float(StepTimeCounter, EComparisonMethod::Greater_Than, 1.0f, TEXT("Waited for 1 second"));
			FinishStep();
		});
}

ADynamicComponentsTestActor* ASpatialDynamicComponentsFastReadditionTest::FindTestActor()
{
	for (ADynamicComponentsTestActor* Actor : TActorRange<ADynamicComponentsTestActor>(GetWorld()))
	{
		return Actor;
	}
	return nullptr;
}

ADynamicComponentsTestActor::ADynamicComponentsTestActor()
{
	bReplicates = true;
	bAlwaysRelevant = true;
}

USelfRecreatingDynamicComponent::USelfRecreatingDynamicComponent()
	: bNeedsToRecreateItself(true)
{
	SetIsReplicatedByDefault(true);
}

void USelfRecreatingDynamicComponent::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
{
	Super::PreReplication(ChangedPropertyTracker);
	if (!bNeedsToRecreateItself)
	{
		return;
	}

	bNeedsToRecreateItself = false;

	TSharedPtr<FDelegateHandle> DelegateHandle = MakeShared<FDelegateHandle>();
	auto OnFrame = [this, DelegateHandle] {
		// Set another component up.
		USelfRecreatingDynamicComponent* Another =
			NewObject<USelfRecreatingDynamicComponent>(GetOwner(), *FString::Printf(TEXT("Recreated")));
		Another->bNeedsToRecreateItself = false;

		Another->RegisterComponent();

		// Destroy self.
		DestroyComponent();

		FCoreDelegates::OnEndFrame.Remove(*DelegateHandle);
	};
	*DelegateHandle = FCoreDelegates::OnEndFrame.Add(FSimpleDelegate::CreateLambda(OnFrame));
}
