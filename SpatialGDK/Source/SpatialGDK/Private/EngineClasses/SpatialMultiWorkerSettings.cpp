// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "EngineClasses/SpatialMultiWorkerSettings.h"

#include "LoadBalancing/GridBasedLBStrategy.h"
#include "LoadBalancing/LayeredLBStrategy.h"
#include "LoadBalancing/OwnershipLockingPolicy.h"
#include "Utils/LayerInfo.h"

#include "Misc/MessageDialog.h"

#define LOCTEXT_NAMESPACE "SpatialMultiWorkerSettings"

#if WITH_EDITOR
void UAbstractSpatialMultiWorkerSettings::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName Name = (PropertyChangedEvent.MemberProperty != nullptr) ? PropertyChangedEvent.MemberProperty->GetFName() : NAME_None;

	if (Name == GET_MEMBER_NAME_CHECKED(UAbstractSpatialMultiWorkerSettings, WorkerLayers))
	{
		ValidateNonEmptyWorkerLayers();
		ValidateSomeLayerHasActorClass();
		ValidateNoActorClassesDuplicatedAmongLayers();
		ValidateAllLayersHaveUniqueNonemptyNames();
		ValidateAllLayersHaveLoadBalancingStrategy();
	}
	else if (Name == GET_MEMBER_NAME_CHECKED(UAbstractSpatialMultiWorkerSettings, LockingPolicy))
	{
		ValidateLockingPolicyIsSet();
	}
};
#endif

uint32 UAbstractSpatialMultiWorkerSettings::GetMinimumRequiredWorkerCount() const
{
	ULayeredLBStrategy* LbStrategy = NewObject<ULayeredLBStrategy>();
	LbStrategy->Init(this);
	return LbStrategy->GetMinimumRequiredWorkers();
}

void UAbstractSpatialMultiWorkerSettings::ValidateNonEmptyWorkerLayers()
{
	if (WorkerLayers.Num() == 0 )
	{
		WorkerLayers.Emplace(SpatialGDK::DefaultLayerInfo);
		FMessageDialog::Open(EAppMsgType::Ok,
			FText::Format(LOCTEXT("EmptyWorkerLayer_Prompt", "Having an empty worker layers map is invalid. "
			"Adding back the default layer. File: {0}"), FText::FromString(GetNameSafe(this))));
	}
}

void UAbstractSpatialMultiWorkerSettings::ValidateSomeLayerHasActorClass()
{
	bool bHasTopLevelActorClass = false;
	for (const FLayerInfo& Layer : WorkerLayers)
	{
		bHasTopLevelActorClass |= Layer.ActorClasses.Contains(AActor::StaticClass());
	}

	if (!bHasTopLevelActorClass)
	{
		WorkerLayers[0].ActorClasses.Add(AActor::StaticClass()); FMessageDialog::Open(EAppMsgType::Ok,
			FText::Format(LOCTEXT("MissingActorLayer_Prompt", "Some worker layer must contain the root Actor class. "
			"Adding AActor to the first worker layer entry. File: {0}"), FText::FromString(GetNameSafe(this))));
	}
}

void UAbstractSpatialMultiWorkerSettings::ValidateNoActorClassesDuplicatedAmongLayers()
{
	TSet<TSoftClassPtr<AActor>> FoundActorClasses;
	TSet<TSoftClassPtr<AActor>> DuplicatedActorClasses;

	for (FLayerInfo& Layer : WorkerLayers)
	{
		for (const TSoftClassPtr<AActor> LayerClass : Layer.ActorClasses)
		{
			if (FoundActorClasses.Contains(LayerClass))
			{
				DuplicatedActorClasses.Add(LayerClass);
			}
			FoundActorClasses.Add(LayerClass);
		}

		for (const TSoftClassPtr<AActor> DuplicatedClass : DuplicatedActorClasses)
		{
			Layer.ActorClasses.Remove(DuplicatedClass);
		}
	}

	if (DuplicatedActorClasses.Num() > 0)
	{
		FString DuplicatedActorsList = "";
		for (const TSoftClassPtr<AActor> DuplicatedClass : DuplicatedActorClasses)
		{
			DuplicatedActorsList.Append(FString::Printf(TEXT("%s, "), *DuplicatedClass.GetAssetName()));
		}
		FMessageDialog::Open(EAppMsgType::Ok,FText::Format(LOCTEXT("MultipleActorLayers_Prompt",
			"Defining the same Actor type across multiple layers is invalid. Removed all occurences after the first. "
            "File: {0}. Duplicate Actor types: {1}"), FText::FromString(GetNameSafe(this)), FText::FromString(DuplicatedActorsList)));
	}
}

void UAbstractSpatialMultiWorkerSettings::ValidateAllLayersHaveUniqueNonemptyNames()
{
	TSet<FName> FoundLayerNames{};
	bool bSomeLayerNameWasChanged = false;

	uint32 LayerCount = 1;

	// We currently rely on this for rendering debug information.
	WorkerLayers[0].Name = SpatialConstants::DefaultLayer;

	for (FLayerInfo& Layer : WorkerLayers)
	{
		const FName FallbackLayerName = *FString::Printf(TEXT("Layer %d"), LayerCount);

		if (Layer.Name.IsNone() || FoundLayerNames.Contains(Layer.Name))
		{
			Layer.Name = FallbackLayerName;
		}

        if (FoundLayerNames.Contains(Layer.Name))
        {
			bSomeLayerNameWasChanged |= true;
        }

		FoundLayerNames.Add(Layer.Name);
		LayerCount++;
	}

	if (bSomeLayerNameWasChanged)
	{
		FMessageDialog::Open(EAppMsgType::Ok,
			FText::Format(LOCTEXT("BadLayerName_Prompt", "Found a worker layer with a duplicate name. "
				"This has been fixed, please check your configuration. File: {0}"), FText::FromString(GetNameSafe(this))));
	}
}

void UAbstractSpatialMultiWorkerSettings::ValidateAllLayersHaveLoadBalancingStrategy()
{
	bool bSomeLayerNameWasChanged = false;

	for (FLayerInfo& Layer : WorkerLayers)
	{
		if (*Layer.LoadBalanceStrategy == nullptr)
		{
			bSomeLayerNameWasChanged |= true;
			Layer.LoadBalanceStrategy = UGridBasedLBStrategy::StaticClass();
		}
	}

	if (bSomeLayerNameWasChanged)
	{
		FMessageDialog::Open(EAppMsgType::Ok,
			FText::Format(LOCTEXT("UnsetLoadBalancingStrategy_Prompt", "Found a worker layer with an unset load balancing strategy. "
				"Defaulting to a 1x1 grid. File: {0}"), FText::FromString(GetNameSafe(this))));
	}
}

void UAbstractSpatialMultiWorkerSettings::ValidateLockingPolicyIsSet()
{
	if (*LockingPolicy == nullptr)
	{
		LockingPolicy = UOwnershipLockingPolicy::StaticClass();
		FMessageDialog::Open(EAppMsgType::Ok,
			FText::Format(LOCTEXT("UnsetLockingPolicy_Prompt", "Locking policy must be set. Resetting to default policy."
				"File: {0}"), FText::FromString(GetNameSafe(this))));
	}
}
