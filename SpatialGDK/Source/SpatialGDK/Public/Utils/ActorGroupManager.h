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
	TMap<FName, FString> ActorGroupToWorker;

	FWorkerAssociation()
	{
		ActorGroupToWorker = {};
	}

	FWorkerAssociation(TMap<FName, FString> Values)
	{
		ActorGroupToWorker = TMap<FName, FString>(Values);
	}
};

UCLASS(Config=SpatialGDKSettings)
class SPATIALGDK_API UActorGroupManager : public UObject
{
	GENERATED_BODY()

private:
	TMap<TSoftClassPtr<AActor>, FName> ClassPathToActorGroup;

	TMap<FName, FString> ActorGroupToWorkerType;

	UActorGroupManager();

	void InitFromSettings();

public:

	static UActorGroupManager* GetInstance();

	FName GetActorGroupForClass(UClass* Class);

	FString GetWorkerTypeForClass(UClass* Class);

	static TMap<FName, FActorClassSet> DefaultActorGroups() {
		return { TPairInitializer<const FName&, const FActorClassSet&>(SpatialConstants::DefaultActorGroup, FActorClassSet({ AActor::StaticClass() })) };
	}

	static TSet<FString> DefaultWorkerTypes() {
		return { SpatialConstants::ServerWorkerType };
	}

	static TMap<FName, FString> DefaultWorkerAssociation() {
		return { TPairInitializer<const FName&, const FString&>(SpatialConstants::DefaultActorGroup, SpatialConstants::ServerWorkerType) };
	}

#if WITH_EDITOR
	static void ValidateOffloadingSettings(TMap<FName, FActorClassSet> OldActorGroups, TMap<FName, FActorClassSet>* ActorGroups,
		TSet<FString> OldWorkerTypes, TSet<FString>* WorkerTypes, FWorkerAssociation& WorkerAssociation);
#endif
};
