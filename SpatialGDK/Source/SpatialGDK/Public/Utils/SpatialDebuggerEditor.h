#pragma once

#include "CoreMinimal.h"
#include "Utils/SpatialDebugger.h"

#include "SpatialDebuggerEditor.generated.h"


/**
 * Extends the SpatialDebugger to visualise spatial information in the Editor
 */
UCLASS(Transient)
class SPATIALGDK_API ASpatialDebuggerEditor : public ASpatialDebugger
{
	GENERATED_UCLASS_BODY()

	virtual void BeginPlay() override;
	virtual void Destroyed() override;

	void OnPieBeginEvent(bool bIsSimulating);
	void OnPieEndEvent(bool bIsSimulating);

	void ShowWorkerRegions(bool bEnabled);
	void RefreshWorkerRegions();

protected:

	FDelegateHandle OnBeginPieHandle;
	FDelegateHandle OnEndPieHandle;

	void InitialiseWorkerRegions();
};
