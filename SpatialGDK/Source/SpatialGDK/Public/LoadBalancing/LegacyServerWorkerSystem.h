#pragma once

#include "EngineClasses/SpatialServerWorkerSystem.h"
#include "LoadBalancing/LegacyLoadbalancingComponents.h"
#include "LegacyServerWorkerSystem.generated.h"

UCLASS()
class ULegacyServerWorkerSystem : public USpatialServerWorkerSystem
{
	GENERATED_BODY()
public:
	ULegacyServerWorkerSystem();

private:
	TArray<SpatialGDK::ComponentData> GetServerWorkerInitialData() override;
};
