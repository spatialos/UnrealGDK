// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKSettings.h"
#include "Misc/MessageDialog.h"
#include "Misc/CommandLine.h"
#include "GameFramework/Actor.h"

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
	, bEnableServerQBI(bUsingQBI)
	, bPackUnreliableRPCs(true)
	, ActorGroups(UActorGroupManager::DefaultActorGroups())
	, WorkerTypes(UActorGroupManager::DefaultWorkerTypes())
	, WorkerAssociation(UActorGroupManager::DefaultWorkerAssociation())
{
}

void USpatialGDKSettings::PostInitProperties()
{
	Super::PostInitProperties();

	// Check any command line overrides for using QBI (after reading the config value):
	const TCHAR* CommandLine = FCommandLine::Get();
	FParse::Bool(CommandLine, TEXT("useQBI"), bUsingQBI);

	OldActorGroups = ActorGroups;
	OldWorkerTypes = WorkerTypes;
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

	if (PropertyName == GET_MEMBER_NAME_CHECKED(USpatialGDKSettings, ActorGroups))
	{
		ValidateOffloadingSettings();
	}

	if (PropertyName == GET_MEMBER_NAME_CHECKED(USpatialGDKSettings, WorkerTypes))
	{
		ValidateOffloadingSettings();
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}

// Returns the first key in A that is NOT in B.
bool GetFirstDifferentValue(TArray<FName> A, TArray<FName> B, FName& OutDifferent)
{
	for (const FName AName : A)
	{
		if (!B.Contains(AName))
		{
			OutDifferent = AName;
			return true;
		}
	}
	return false;
}

void USpatialGDKSettings::ValidateOffloadingSettings()
{
	if (ActorGroups.Num() == 0)
	{
		ActorGroups = UActorGroupManager::DefaultActorGroups();
	}

	// Check for empty Worker Types
	if (WorkerTypes.Num() == 0)
	{
		WorkerTypes = UActorGroupManager::DefaultWorkerTypes();
	}

	TArray<FName> ActorGroupKeys;
	ActorGroups.GetKeys(ActorGroupKeys);

	TArray<FName> OldActorGroupKeys;
	OldActorGroups.GetKeys(OldActorGroupKeys);

	// Check for renamed Actor Group
	if (ActorGroups.Num() == OldActorGroups.Num())
	{
		FName FromActorGroup, ToActorGroup;
		if (GetFirstDifferentValue(OldActorGroupKeys, ActorGroupKeys, FromActorGroup)
			&& GetFirstDifferentValue(ActorGroupKeys, OldActorGroupKeys, ToActorGroup))
		{
			if (WorkerAssociation.ActorGroupToWorker.Contains(FromActorGroup))
			{
				WorkerAssociation.ActorGroupToWorker.Add(ToActorGroup, WorkerAssociation.ActorGroupToWorker.FindRef(FromActorGroup));
				WorkerAssociation.ActorGroupToWorker.Remove(FromActorGroup);
			}
		}
	}

	// Check for renamed WorkerType
	if (WorkerTypes.Num() == OldWorkerTypes.Num())
	{
		FName FromWorkerType, ToWorkerType;
		if (GetFirstDifferentValue(OldWorkerTypes.Array(), WorkerTypes.Array(), FromWorkerType) &&
			GetFirstDifferentValue(WorkerTypes.Array(), OldWorkerTypes.Array(), ToWorkerType))
		{
			for (auto Entry = WorkerAssociation.ActorGroupToWorker.CreateConstIterator(); Entry; ++Entry)
			{
				if (Entry->Value == FromWorkerType)
				{
					WorkerAssociation.ActorGroupToWorker.Add(Entry->Key, ToWorkerType);
				}
			}
		}
	}
	
	// Remove any keys for deleted actor groups.
	TArray<FName> Keys;
	WorkerAssociation.ActorGroupToWorker.GetKeys(Keys);
	for (FName Key : Keys)
	{
		if (!ActorGroupKeys.Contains(Key))
		{
			WorkerAssociation.ActorGroupToWorker.Remove(Key);
		}
	}

	FName FirstWorkerType = WorkerTypes.Array()[0];

	// Add default key for any new actor groups.
	for (FName ActorGroup : ActorGroupKeys)
	{
		if (!WorkerAssociation.ActorGroupToWorker.Contains(ActorGroup))
		{
			WorkerAssociation.ActorGroupToWorker.Add(ActorGroup, FirstWorkerType);
		}
	}

	// Replace any now invalid Worker Types with FirstWorkerType.
	for (auto Entry = WorkerAssociation.ActorGroupToWorker.CreateConstIterator(); Entry; ++Entry)
	{
		if (!WorkerTypes.Contains(Entry->Value))
		{
			WorkerAssociation.ActorGroupToWorker.Add(Entry->Key, FirstWorkerType);
		}
	}

	OldActorGroups = ActorGroups;
	OldWorkerTypes = WorkerTypes;
}

#endif
