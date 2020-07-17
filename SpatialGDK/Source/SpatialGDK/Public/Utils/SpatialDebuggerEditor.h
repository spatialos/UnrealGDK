#pragma once

#include "CoreMinimal.h"
#include "Utils/SpatialDebugger.h"

#include "SpatialDebuggerEditor.generated.h"


/**
 * Extends the SpatialDebugger to visualise spatial information in the Editor
 */
UCLASS()
class SPATIALGDK_API ASpatialDebuggerEditor : public ASpatialDebugger
{
	GENERATED_UCLASS_BODY()

	virtual void BeginPlay() override;
	virtual void Destroyed() override;

	virtual void SpatialToggleDebugger() override;

protected:

	void InitialiseWorkerRegions();

};
