#include "Utils/ActorGroupManager.h"
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

FString UActorGroupManager::GetWorkerTypeForClass(UClass* Class)
{
	FName ActorGroup = GetActorGroupForClass(Class);

	if (ActorGroupToWorkerType.Contains(ActorGroup))
	{
		return ActorGroupToWorkerType.FindRef(ActorGroup);
	}

	return TEXT("");
}
