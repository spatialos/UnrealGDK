// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "CrossServerAndClientOrchestrationFlowController.h"

#include "CrossServerAndClientOrchestrationTest.h"

void ACrossServerAndClientOrchestrationFlowController::ServerClientReadValueAck_Implementation()
{
	ACrossServerAndClientOrchestrationTest* Test = Cast<ACrossServerAndClientOrchestrationTest>(OwningTest);
	Test->CrossServerSetTestValue(ESpatialFunctionalTestWorkerType::Client, WorkerDefinition.Id);
}
