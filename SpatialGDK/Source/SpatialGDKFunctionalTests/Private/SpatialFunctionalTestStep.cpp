// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialFunctionalTestStep.h"

const FWorkerDefinition FWorkerDefinition::AllWorkers =
	FWorkerDefinition{ ESpatialFunctionalTestWorkerType::All, FWorkerDefinition::ALL_WORKERS_ID };
const FWorkerDefinition FWorkerDefinition::AllServers =
	FWorkerDefinition{ ESpatialFunctionalTestWorkerType::Server, FWorkerDefinition::ALL_WORKERS_ID };
const FWorkerDefinition FWorkerDefinition::AllClients =
	FWorkerDefinition{ ESpatialFunctionalTestWorkerType::Client, FWorkerDefinition::ALL_WORKERS_ID };

FWorkerDefinition FWorkerDefinition::Server(int ServerId)
{
	return FWorkerDefinition{ ESpatialFunctionalTestWorkerType::Server, ServerId };
}

FWorkerDefinition FWorkerDefinition::Client(int ClientId)
{
	return FWorkerDefinition{ ESpatialFunctionalTestWorkerType::Client, ClientId };
}

SpatialFunctionalTestStep::SpatialFunctionalTestStep()
	: bIsRunning(false)
	, bIsReady(false)
{
}

void SpatialFunctionalTestStep::Start(FSpatialFunctionalTestStepDefinition NewStepDefinition)
{
	StepDefinition = FSpatialFunctionalTestStepDefinition(NewStepDefinition);

	bIsRunning = true;

	bIsReady = false;
}

void SpatialFunctionalTestStep::Tick(float DeltaTime)
{
	if (!bIsRunning)
	{
		return;
	}

	if (!bIsReady)
	{
		if (HasReadyEvent())
		{
			if (StepDefinition.bIsNativeDefinition)
			{
				bIsReady = StepDefinition.NativeIsReadyEvent.Execute(Owner);
			}
			else
			{
				bIsReady = StepDefinition.IsReadyEvent.Execute();
			}
		}
		else
		{
			bIsReady = true;
		}

		if (bIsReady)
		{
			if (StepDefinition.bIsNativeDefinition)
			{
				StepDefinition.NativeStartEvent.ExecuteIfBound(Owner);
			}
			else
			{
				StepDefinition.StartEvent.ExecuteIfBound();
			}
		}
	}

	if (bIsReady)
	{
		if (StepDefinition.bIsNativeDefinition)
		{
			StepDefinition.NativeTickEvent.ExecuteIfBound(Owner, DeltaTime);
		}
		else
		{
			StepDefinition.TickEvent.ExecuteIfBound(DeltaTime);
		}
	}
}

void SpatialFunctionalTestStep::Reset()
{
	bIsRunning = false;
	bIsReady = false;
	StepDefinition = FSpatialFunctionalTestStepDefinition();
}

bool SpatialFunctionalTestStep::HasReadyEvent()
{
	return StepDefinition.NativeIsReadyEvent.IsBound() || StepDefinition.IsReadyEvent.IsBound();
}
