// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKSettings.h"
#include "Misc/MessageDialog.h"
#include "Misc/CommandLine.h"

#if WITH_EDITOR
#include "Modules/ModuleManager.h"
#include "ISettingsModule.h"
#endif

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
	, bUsingQBI(true)
	, PositionUpdateFrequency(1.0f)
	, PositionDistanceThreshold(100.0f) // 1m (100cm)
	, bEnableMetrics(true)
	, MetricsReportRate(2.0f)
	, bCheckRPCOrder(false)
{
}

void USpatialGDKSettings::PostInitProperties()
{
	Super::PostInitProperties();

	// Check any command line overrides for using QBI (after reading the config value):
	const TCHAR* CommandLine = FCommandLine::Get();
	FParse::Bool(CommandLine, TEXT("useQBI"), bUsingQBI);
}

#if WITH_EDITOR
// Add a pop-up to warn users to update their config upon changing the using QBI property.
void USpatialGDKSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.Property == nullptr)
	{
		return;
	}
	const FName PropertyName = PropertyChangedEvent.Property->GetFName();

	if (PropertyName == GET_MEMBER_NAME_CHECKED(USpatialGDKSettings, bUsingQBI))
	{
		const EAppReturnType::Type Result = FMessageDialog::Open(EAppMsgType::YesNo,
			FText::FromString(FString::Printf(TEXT("You must set the value of the \"enable_chunk_interest\" Legacy flag to \"%s\" in your launch configuration file for this to work.\n\nIf you are using an auto-generated launch config, you can set this value from within Unreal Editor by going to Edit > Project Settings > SpatialOS GDK for Unreal > Settings.\n\nDo you want to configure your launch config settings now?"),
				bUsingQBI ? TEXT("false") : TEXT("true"))));

		if (Result == EAppReturnType::Yes)
		{
			FModuleManager::LoadModuleChecked<ISettingsModule>("Settings").ShowViewer("Project", "SpatialGDKEditor", "Editor Settings");
		}
	}
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif
