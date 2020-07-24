// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Utils/SpatialDebugger.h"

#include "CoreMinimal.h"

#include "SpatialDebuggerEditor.generated.h"


/**
 * Extends the SpatialDebugger to visualise spatial information in the Editor
 */
UCLASS(Transient)
class SPATIALGDK_API ASpatialDebuggerEditor : public ASpatialDebugger
{
	GENERATED_UCLASS_BODY()

	virtual void Destroyed() override;

	void ShowWorkerRegions(bool bEnabled);
	void RefreshWorkerRegions();

	bool AllowWorkerBoundaries() const;
protected:

	void InitialiseWorkerRegions();
};
