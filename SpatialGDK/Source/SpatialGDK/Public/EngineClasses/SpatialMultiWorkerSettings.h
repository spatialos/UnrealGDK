// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "LoadBalancing/GridBasedLBStrategy.h"
#include "LoadBalancing/LayeredLBStrategy.h"
#include "LoadBalancing/OwnershipLockingPolicy.h"
#include "Utils/LayerInfo.h"

#include "Templates/SubclassOf.h"
#include "UObject/NameTypes.h"
#include "UObject/Object.h"

#include "SpatialMultiWorkerSettings.generated.h"

class UAbstractLockingPolicy;

namespace SpatialGDK
{
const FName DefaultLayerName = TEXT("Default");
const FLayerInfo DefaultLayerInfo = {DefaultLayerName, {AActor::StaticClass()}, UGridBasedLBStrategy::StaticClass()};
}

UCLASS(NotBlueprintable)
class SPATIALGDK_API UAbstractSpatialMultiWorkerSettings : public UObject
{
	GENERATED_BODY()

public:
	UAbstractSpatialMultiWorkerSettings() {};

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override
	{
		Super::PostEditChangeProperty(PropertyChangedEvent);

		// Use MemberProperty here so we report the correct member name for nested changes
		const FName Name = (PropertyChangedEvent.MemberProperty != nullptr) ? PropertyChangedEvent.MemberProperty->GetFName() : NAME_None;

		if (Name == GET_MEMBER_NAME_CHECKED(UAbstractSpatialMultiWorkerSettings, WorkerLayers))
		{
			ValidateNonEmptyWorkerLayers();
			ValidateSomeLayerHasActorClass();
			ValidateNoActorClassesDuplicatedAmongLayers();
			ValidateAllLayersHaveUniqueNonemptyNames();
		}
	};
#endif

protected:
	UAbstractSpatialMultiWorkerSettings(TArray<FLayerInfo> InWorkerLayers, TSubclassOf<UAbstractLockingPolicy> InLockingPolicy)
		: WorkerLayers(InWorkerLayers)
		, LockingPolicy(InLockingPolicy) {}

public:
	UPROPERTY(EditAnywhere, Category = "Multi-Worker")
		TArray<FLayerInfo> WorkerLayers;

	UPROPERTY(EditAnywhere, Category = "Multi-Worker")
		TSubclassOf<UAbstractLockingPolicy> LockingPolicy;

	uint32 GetMinimumRequiredWorkerCount() const
	{
		ULayeredLBStrategy* LbStrategy = NewObject<ULayeredLBStrategy>();
		LbStrategy->Init(this);
		return LbStrategy->GetMinimumRequiredWorkers();
	}

private:
	void ValidateNonEmptyWorkerLayers()
	{
		if (WorkerLayers.Num() == 0 )
		{
			WorkerLayers.Emplace(SpatialGDK::DefaultLayerInfo);
			FMessageDialog::Open(EAppMsgType::Ok, FText::Format(LOCTEXT("Having an empty worker layers map is invalid. "
				"Adding back the default layer. File: {0}"), GetNameSafe(this)));
		}
	}

	void ValidateSomeLayerHasActorClass()
	{
		bool bHasTopLevelActorClass = false;
		for (const FLayerInfo& Layer : WorkerLayers)
		{
			bHasTopLevelActorClass |= Layer.ActorClasses.Contains(AActor::StaticClass());
		}

		if (!bHasTopLevelActorClass)
		{
			WorkerLayers[0].ActorClasses.Add(AActor::StaticClass());
			FMessageDialog::Open(EAppMsgType::Ok, FText::Format(LOCTEXT("Some worker layer must contain the root Actor class. "
				"Adding AActor to the first worker layer entry. File: {0}"), GetNameSafe(this)));
		}
	}

	void ValidateNoActorClassesDuplicatedAmongLayers()
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
			FMessageDialog::Open(EAppMsgType::Ok, FText::Format(LOCTEXT("Defining the same Actor type across multiple layers is invalid. Removed all occurences after the first. "
                "File: {0}. Duplicate Actor types: {1}"), GetNameSafe(this), DuplicatedActorsList));
		}
	}

	void ValidateAllLayersHaveUniqueNonemptyNames()
	{
		TSet<FName> FoundLayerNames{};
		bool bSomeLayerNameWasChanged = false;

		uint32 LayerCount = 0;

		for (FLayerInfo& Layer : WorkerLayers)
		{
			LayerCount++;

			bool bLayerNameNeedsChanging = false;

			bLayerNameNeedsChanging |= Layer.Name.IsNone();

			bLayerNameNeedsChanging |= FoundLayerNames.Contains(Layer.Name);

            if (bLayerNameNeedsChanging)
            {
            	Layer.Name = LayerCount == 1 ? SpatialGDK::DefaultLayerName : FString::Printf(TEXT("Layer %d"), LayerCount);
            }

			bSomeLayerNameWasChanged |= bLayerNameNeedsChanging;

			FoundLayerNames.Add(Layer.Name);
		}

		if (bSomeLayerNameWasChanged)
		{
			FMessageDialog::Open(EAppMsgType::Ok, FText::Format(LOCTEXT("Found a worker layer with a duplicate or empty name. "
				"This has been fixed, please check your configuration. File: {0}"), GetNameSafe(this)));
		}
	}
};

UCLASS(Blueprintable)
class SPATIALGDK_API USpatialMultiWorkerSettings : public UAbstractSpatialMultiWorkerSettings
{
	GENERATED_BODY()

public:
	USpatialMultiWorkerSettings()
	: Super({SpatialGDK::DefaultLayerInfo}, UOwnershipLockingPolicy::StaticClass())
	{}
};
