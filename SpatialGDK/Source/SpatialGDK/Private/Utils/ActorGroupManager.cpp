// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/ActorGroupManager.h"
#include "SpatialGDKSettings.h"

void UActorGroupManager::Init()
{
	if (const USpatialGDKSettings* Settings = GetDefault<USpatialGDKSettings>())
	{
		DefaultWorkerType = Settings->DefaultWorkerType.WorkerTypeName;
		if (Settings->bEnableOffloading)
		{
			for (const TPair<FName, FActorGroupInfo>& ActorGroup : Settings->ActorGroups)
			{
				ActorGroupToWorkerType.Add(ActorGroup.Key, ActorGroup.Value.OwningWorkerType.WorkerTypeName);

				for (const TSoftClassPtr<AActor>& ClassPtr : ActorGroup.Value.ActorClasses)
				{
					ClassPathToActorGroup.Add(ClassPtr, ActorGroup.Key);
				}
			}
		}
	}
}

FName UActorGroupManager::GetActorGroupForClass(const TSubclassOf<AActor> Class)
{
	if (Class == nullptr)
	{
		return NAME_None;
	}

	UClass* FoundClass = Class;
	TSoftClassPtr<AActor> ClassPtr = TSoftClassPtr<AActor>(FoundClass);

	while (FoundClass != nullptr && FoundClass->IsChildOf(AActor::StaticClass()))
	{
		if (const FName* ActorGroup = ClassPathToActorGroup.Find(ClassPtr))
		{
			if (FoundClass != Class)
			{
				ClassPathToActorGroup.Add(TSoftClassPtr<AActor>(Class), *ActorGroup);
			}
			return *ActorGroup;
		}

		FoundClass = FoundClass->GetSuperClass();
		ClassPtr = TSoftClassPtr<AActor>(FoundClass);
	}

	// No mapping found so set and return default actor group.
	ClassPathToActorGroup.Add(TSoftClassPtr<AActor>(Class), SpatialConstants::DefaultActorGroup);
	return SpatialConstants::DefaultActorGroup;
}

FName UActorGroupManager::GetWorkerTypeForClass(const TSubclassOf<AActor> Class)
{
	const FName ActorGroup = GetActorGroupForClass(Class);

	if (const FName* WorkerType = ActorGroupToWorkerType.Find(ActorGroup))
	{
		return *WorkerType;
	}

	return DefaultWorkerType;
}

FName UActorGroupManager::GetWorkerTypeForActorGroup(const FName& ActorGroup) const
{
	if (const FName* WorkerType = ActorGroupToWorkerType.Find(ActorGroup))
	{
		return *WorkerType;
	}

	return DefaultWorkerType;
}

bool UActorGroupManager::IsSameWorkerType(const AActor* ActorA, const AActor* ActorB)
{
	if (ActorA == nullptr || ActorB == nullptr)
	{
		return false;
	}

	const FName& WorkerTypeA = GetWorkerTypeForClass(ActorA->GetClass());
	const FName& WorkerTypeB = GetWorkerTypeForClass(ActorB->GetClass());

	return (WorkerTypeA == WorkerTypeB);
}
