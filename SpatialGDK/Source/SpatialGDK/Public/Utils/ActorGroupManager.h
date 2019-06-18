#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpatialConstants.h"

#include "ActorGroupManager.generated.h"

USTRUCT()
struct FActorClassSet
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TSet<TSoftClassPtr<AActor>> ActorClasses;

	FActorClassSet()
	{
		ActorClasses = {};
	}

	FActorClassSet(TSet<TSoftClassPtr<AActor>> Classes)
	{
		ActorClasses = Classes;
	}
};

USTRUCT()
struct FWorkerAssociation
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TMap<FName, FName> ActorGroupToWorker;

	FWorkerAssociation()
	{
		ActorGroupToWorker = {};
	}

	FWorkerAssociation(TMap<FName, FName> Values)
	{
		ActorGroupToWorker = TMap<FName, FName>(Values);
	}
};

UCLASS(Config=SpatialGDKSettings)
class SPATIALGDK_API UActorGroupManager : public UObject
{
	GENERATED_BODY()

private:
	TMap<TSoftClassPtr<AActor>, FName> ClassPathToActorGroup;

	TMap<FName, FName> ActorGroupToWorkerType;

	UActorGroupManager();

	void InitFromSettings();

public:

	static UActorGroupManager* GetInstance();

	FName GetActorGroupForClass(UClass* Class);

	FName GetWorkerTypeForClass(UClass* Class);

	static TMap<FName, FActorClassSet> DefaultActorGroups() {
		return { TPairInitializer<const FName&, const FActorClassSet&>(SpatialConstants::DefaultActorGroup, FActorClassSet({ AActor::StaticClass() })) };
	}

	static TSet<FName> DefaultWorkerTypes() {
		return { FName(*SpatialConstants::ServerWorkerType) };
	}

	static TMap<FName, FName> DefaultWorkerAssociation() {
		return { TPairInitializer<const FName&, const FName&>(SpatialConstants::DefaultActorGroup, FName(*SpatialConstants::ServerWorkerType)) };
	}

#if WITH_EDITOR
	static void ValidateOffloadingSettings(TMap<FName, FActorClassSet> OldActorGroups, TMap<FName, FActorClassSet>* ActorGroups,
		TSet<FName> OldWorkerTypes, TSet<FName>* WorkerTypes, FWorkerAssociation& WorkerAssociation);
#endif
};
