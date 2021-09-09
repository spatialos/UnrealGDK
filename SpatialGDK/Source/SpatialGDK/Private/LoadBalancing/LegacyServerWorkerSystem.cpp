#include "LoadBalancing/LegacyServerWorkerSystem.h"
#include "LoadBalancing/LegacyLoadbalancingComponents.h"

ULegacyServerWorkerSystem::ULegacyServerWorkerSystem() = default;

TArray<SpatialGDK::ComponentData> ULegacyServerWorkerSystem::GetServerWorkerInitialData()
{
	TArray<SpatialGDK::ComponentData> Data;
	Data.Add(SpatialGDK::LegacyLB_CustomWorkerAssignments().CreateComponentData());

	return Data;
}
