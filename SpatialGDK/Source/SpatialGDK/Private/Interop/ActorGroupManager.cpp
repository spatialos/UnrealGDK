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
