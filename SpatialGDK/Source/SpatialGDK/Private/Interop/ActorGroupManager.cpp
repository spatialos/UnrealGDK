#include "ActorGroupManager.h"
#include "SpatialGDKSettings.h"

UActorGroupManager::UActorGroupManager()
{
	InitFromSettings();
}

UActorGroupManager* UActorGroupManager::GetInstance()
{
	UActorGroupManager* Manager = NewObject<UActorGroupManager>();
	Manager->InitFromSettings();
	return Manager;
}

void UActorGroupManager::InitFromSettings()
{
	const USpatialGDKSettings* Settings = GetDefault<USpatialGDKSettings>();

	ActorGroupSet.Empty();
	ClassPathToActorGroup.Empty();

	for (TPair<FName, FActorClassSet> ActorGroup : Settings->ActorGroups)
	{
		ActorGroupSet.Add(ActorGroup.Key);

		for (TSoftClassPtr<AActor> ClassPtr : ActorGroup.Value.ActorClasses)
		{
			ClassPathToActorGroup.Add(ClassPtr, ActorGroup.Key);
		}
	}
}

void UActorGroupManager::DumpActorGroups()
{
	UE_LOG(LogTemp, Log, TEXT("-- Actor Groups --"))
	for (FName ActorGroup : ActorGroupSet)
	{
		UE_LOG(LogTemp, Log, TEXT("[%s]"), *ActorGroup.ToString())
	}

	UE_LOG(LogTemp, Log, TEXT("-- Actor Group Mappings --"))
	for (TPair<TSoftClassPtr<AActor>, FName> Mapping : ClassPathToActorGroup)
	{
		UE_LOG(LogTemp, Log, TEXT("[%s] => [%s]"), *Mapping.Key.ToString(), *Mapping.Value.ToString())
	}
}

TSet<FName> UActorGroupManager::GetActorGroups()
{
	return ActorGroupSet;
}

FName UActorGroupManager::GetActorGroupForClass(UClass* Class)
{
	if (Class == nullptr || !Class->IsChildOf<AActor>())
	{
		return NAME_None;
	}

	UClass* FoundClass = Class;
	TSoftClassPtr<AActor> ClassPtr = TSoftClassPtr<AActor>(FoundClass);

	while (FoundClass != nullptr && !ClassPathToActorGroup.Contains(ClassPtr))
	{
		FoundClass = FoundClass->GetSuperClass();
		ClassPtr = TSoftClassPtr<AActor>(FoundClass);
	}

	if (FoundClass != nullptr)
	{
		if (ClassPathToActorGroup.Contains(ClassPtr))
		{
			FName ActorGroup = ClassPathToActorGroup.FindRef(ClassPtr);
			ClassPathToActorGroup.Add(TSoftClassPtr<AActor>(Class), ActorGroup);
			return ActorGroup;
		}
	}
	return NAME_None;
}

#if WITH_EDITOR

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

void UActorGroupManager::ValidateOffloadingSettings(TMap<FName, FActorClassSet> OldActorGroups, TMap<FName, FActorClassSet>* ActorGroups,
	TSet<FName> OldWorkerTypes, TSet<FName>* WorkerTypes, FWorkerAssociation& WorkerAssociation)
{
	if (ActorGroups->Num() == 0)
	{
		ActorGroups->Append(UActorGroupManager::DefaultActorGroups());
	}

	// Check for empty Worker Types
	if (WorkerTypes->Num() == 0)
	{
		WorkerTypes->Append(UActorGroupManager::DefaultWorkerTypes());
	}

	TArray<FName> ActorGroupKeys;
	ActorGroups->GetKeys(ActorGroupKeys);

	TArray<FName> OldActorGroupKeys;
	OldActorGroups.GetKeys(OldActorGroupKeys);

	// Check for renamed Actor Group
	if (ActorGroups->Num() == OldActorGroups.Num())
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

	// Check for invalid WorkerType (Currently just UnrealClient)
	if (WorkerTypes->Contains(FName(*SpatialConstants::ClientWorkerType)))
	{
		WorkerTypes->Remove(FName(*SpatialConstants::ClientWorkerType));
	}

	// Check for renamed WorkerType
	if (WorkerTypes->Num() == OldWorkerTypes.Num())
	{
		FName FromWorkerType, ToWorkerType;
		if (GetFirstDifferentValue(OldWorkerTypes.Array(), WorkerTypes->Array(), FromWorkerType) &&
			GetFirstDifferentValue(WorkerTypes->Array(), OldWorkerTypes.Array(), ToWorkerType))
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

	FName FirstWorkerType = WorkerTypes->Array()[0];

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
		if (!WorkerTypes->Contains(Entry->Value))
		{
			WorkerAssociation.ActorGroupToWorker.Add(Entry->Key, FirstWorkerType);
		}
	}
}

#endif
