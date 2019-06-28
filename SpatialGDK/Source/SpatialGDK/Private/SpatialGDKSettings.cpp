// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKSettings.h"
#include "Misc/MessageDialog.h"
#include "Misc/CommandLine.h"

USpatialGDKSettings::USpatialGDKSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, EntityPoolInitialReservationCount(3000)
	, EntityPoolRefreshThreshold(1000)
	, EntityPoolRefreshCount(2000)
	, HeartbeatIntervalSeconds(2.0f)
	, HeartbeatTimeoutSeconds(10.0f)
	, ActorReplicationRateLimit(0)
	, EntityCreationRateLimit(0)
	, OpsUpdateRate(1000.0f)
	, bEnableHandover(true)
	, bUsingQBI(true)
	, PositionUpdateFrequency(1.0f)
	, PositionDistanceThreshold(100.0f) // 1m (100cm)
	, bEnableMetrics(true)
	, bEnableMetricsDisplay(false)
	, MetricsReportRate(2.0f)
	, bUseFrameTimeAsLoad(false)
	, bCheckRPCOrder(false)
	, bBatchSpatialPositionUpdates(true)
	, MaxDynamicallyAttachedSubobjectsPerClass(3)
	, bEnableServerQBI(bUsingQBI)
	, bPackUnreliableRPCs(true)
	, bUseDevelopmentAuthenticationFlow(false)
{
}

void USpatialGDKSettings::PostInitProperties()
{
	Super::PostInitProperties();

	// Check any command line overrides for using QBI (after reading the config value):
	const TCHAR* CommandLine = FCommandLine::Get();
	FParse::Bool(CommandLine, TEXT("useQBI"), bUsingQBI);
}
