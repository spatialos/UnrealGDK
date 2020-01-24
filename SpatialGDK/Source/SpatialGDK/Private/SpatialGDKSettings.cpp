// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKSettings.h"
#include "Improbable/SpatialEngineConstants.h"
#include "Misc/MessageDialog.h"
#include "Misc/CommandLine.h"
#include "SpatialConstants.h"

#if WITH_EDITOR
#include "Settings/LevelEditorPlaySettings.h"
#endif

DEFINE_LOG_CATEGORY(LogSpatialGDKSettings);

USpatialGDKSettings::USpatialGDKSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, EntityPoolInitialReservationCount(3000)
	, EntityPoolRefreshThreshold(1000)
	, EntityPoolRefreshCount(2000)
	, HeartbeatIntervalSeconds(2.0f)
	, HeartbeatTimeoutSeconds(10.0f)
	, HeartbeatTimeoutWithEditorSeconds(10000.0f)
	, ActorReplicationRateLimit(0)
	, EntityCreationRateLimit(0)
	, UseIsActorRelevantForConnection(false)
	, OpsUpdateRate(1000.0f)
	, bEnableHandover(true)
	, MaxNetCullDistanceSquared(900000000.0f) // Set to twice the default Actor NetCullDistanceSquared (300m)
	, QueuedIncomingRPCWaitTime(1.0f)
	, PositionUpdateFrequency(1.0f)
	, PositionDistanceThreshold(100.0f) // 1m (100cm)
	, bEnableMetrics(true)
	, bEnableMetricsDisplay(false)
	, MetricsReportRate(2.0f)
	, bUseFrameTimeAsLoad(false)
	, bBatchSpatialPositionUpdates(false)
	, MaxDynamicallyAttachedSubobjectsPerClass(3)
	, bEnableServerQBI(true)
	, bEnableClientResultTypes(false)
	, bPackRPCs(false)
	, bUseDevelopmentAuthenticationFlow(false)
	, DefaultWorkerType(FWorkerType(SpatialConstants::DefaultServerWorkerType))
	, bEnableOffloading(false)
	, ServerWorkerTypes({ SpatialConstants::DefaultServerWorkerType })
	, WorkerLogLevel(ESettingsWorkerLogVerbosity::Warning)
	, bEnableUnrealLoadBalancer(false)
	, bUseRPCRingBuffers(false)
	, DefaultRPCRingBufferSize(8)
	, MaxRPCRingBufferSize(32)
	// TODO - UNR 2514 - These defaults are not necessarily optimal - readdress when we have better data
	, bTcpNoDelay(false)
	, UdpServerUpstreamUpdateIntervalMS(1)
	, UdpServerDownstreamUpdateIntervalMS(1)
	, UdpClientUpstreamUpdateIntervalMS(1)
	, UdpClientDownstreamUpdateIntervalMS(1)
	// TODO - end
	, bAsyncLoadNewClassesOnEntityCheckout(false)
{
	DefaultReceptionistHost = SpatialConstants::LOCAL_HOST;
}

void USpatialGDKSettings::PostInitProperties()
{
	Super::PostInitProperties();

	// Check any command line overrides for using QBI, Offloading (after reading the config value):
	const TCHAR* CommandLine = FCommandLine::Get();

	if (FParse::Param(CommandLine, TEXT("OverrideSpatialOffloading")))
	{
		bEnableOffloading = true;
	}
	else
	{
		FParse::Bool(CommandLine, TEXT("OverrideSpatialOffloading="), bEnableOffloading);
	}
	UE_LOG(LogSpatialGDKSettings, Log, TEXT("Offloading is %s."), bEnableOffloading ? TEXT("enabled") : TEXT("disabled"));

	if (FParse::Param(CommandLine, TEXT("OverrideServerInterest")))
	{
		bEnableServerQBI = true;
	}
	else
	{
		FParse::Bool(CommandLine, TEXT("OverrideServerInterest="), bEnableServerQBI);
	}
	UE_LOG(LogSpatialGDKSettings, Log, TEXT("Server interest is %s."), bEnableServerQBI ? TEXT("enabled") : TEXT("disabled"));

	if (FParse::Param(CommandLine, TEXT("OverrideHandover")))
	{
		bEnableHandover = true;
	}
	else
	{
		FParse::Bool(CommandLine, TEXT("OverrideHandover="), bEnableHandover);
	}
	UE_LOG(LogSpatialGDKSettings, Log, TEXT("Handover is %s."), bEnableHandover ? TEXT("enabled") : TEXT("disabled"));

	if (FParse::Param(CommandLine, TEXT("OverrideLoadBalancer")))
	{
		bEnableUnrealLoadBalancer = true;
	}
	else
	{
		FParse::Bool(CommandLine, TEXT("OverrideLoadBalancer="), bEnableUnrealLoadBalancer);
	}
	UE_LOG(LogSpatialGDKSettings, Log, TEXT("Unreal load balancing is %s."), bEnableUnrealLoadBalancer ? TEXT("enabled") : TEXT("disabled"));

	if (bEnableUnrealLoadBalancer)
	{
		if (bEnableServerQBI == false)
		{
			UE_LOG(LogSpatialGDKSettings, Warning, TEXT("Unreal load balancing is enabled, but server interest is disabled."));
		}
		if (bEnableHandover == false)
		{
			UE_LOG(LogSpatialGDKSettings, Warning, TEXT("Unreal load balancing is enabled, but handover is disabled."));
		}
	}

#if WITH_EDITOR
	ULevelEditorPlaySettings* PlayInSettings = GetMutableDefault<ULevelEditorPlaySettings>();
	PlayInSettings->bEnableOffloading = bEnableOffloading;
	PlayInSettings->DefaultWorkerType = DefaultWorkerType.WorkerTypeName;
#endif
}

#if WITH_EDITOR
void USpatialGDKSettings::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Use MemberProperty here so we report the correct member name for nested changes
	const FName Name = (PropertyChangedEvent.MemberProperty != nullptr) ? PropertyChangedEvent.MemberProperty->GetFName() : NAME_None;

	if (Name == GET_MEMBER_NAME_CHECKED(USpatialGDKSettings, bEnableOffloading))
	{
		GetMutableDefault<ULevelEditorPlaySettings>()->bEnableOffloading = bEnableOffloading;
	}
	else if (Name == GET_MEMBER_NAME_CHECKED(USpatialGDKSettings, DefaultWorkerType))
	{
		GetMutableDefault<ULevelEditorPlaySettings>()->DefaultWorkerType = DefaultWorkerType.WorkerTypeName;
	}
	else if (Name == GET_MEMBER_NAME_CHECKED(USpatialGDKSettings, MaxDynamicallyAttachedSubobjectsPerClass))
	{
		FMessageDialog::Open(EAppMsgType::Ok,
			FText::FromString(FString::Printf(TEXT("You MUST regenerate schema using the full scan option after changing the number of max dynamic subobjects. "
				"Failing to do will result in unintended behavior or crashes!"))));
	}
}
#endif

uint32 USpatialGDKSettings::GetRPCRingBufferSize(ERPCType RPCType) const
{
	if (const uint32* Size = RPCRingBufferSizeMap.Find(RPCType))
	{
		return *Size;
	}

	return DefaultRPCRingBufferSize;
}
