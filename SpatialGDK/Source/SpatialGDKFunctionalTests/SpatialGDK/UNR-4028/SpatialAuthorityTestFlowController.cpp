// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialAuthorityTestFlowController.h"
#include "SpatialAuthorityTest.h"

void ASpatialAuthorityTestFlowController::ServerSetGameStateAuthority_Implementation(int Authority)
{
	ASpatialAuthorityTest* Test = Cast<ASpatialAuthorityTest>(OwningTest);

	if(Test == nullptr)
	{
		OwningTest->FinishTest(EFunctionalTestResult::Failed, TEXT("ASpatialAuthorityTestFlowController is meant to be used with ASpatialAuthorityTest"));
		return;
	}

	Test->CrossServerSetGameStateAuthorityFromWorker(WorkerDefinition, Authority);
}
