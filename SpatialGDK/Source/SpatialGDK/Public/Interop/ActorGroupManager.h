#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

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

UCLASS(Config=SpatialGDKSettings, BlueprintType)
class SPATIALGDK_API UActorGroupManager : public UObject
{
	GENERATED_BODY()

private:
	TSet<FName> ActorGroupSet;

	TMap<TSoftClassPtr<AActor>, FName> ClassPathToActorGroup;

	UActorGroupManager();

	void InitFromSettings();

public:

	UFUNCTION(BlueprintCallable, Category = "Actor Groups")
	static UActorGroupManager* GetInstance();

	UFUNCTION(BlueprintCallable, Category = "Actor Groups")
	void DumpActorGroups();

	UFUNCTION(BlueprintCallable, Category = "Actor Groups")
	TSet<FName> GetActorGroups();

	UFUNCTION(BlueprintCallable, Category = "Actor Groups")
	FName GetActorGroupForClass(UClass* Class);

	static TMap<FName, FActorClassSet> DefaultActorGroups() {
		return { TPairInitializer<const FName&, const FActorClassSet&>(FName(TEXT("Default")), FActorClassSet({ AActor::StaticClass() })) };
	}

	static TSet<FName> DefaultWorkerTypes() {
		return { FName(TEXT("UnrealWorker")) };
	}

	static TMap<FName, FName> DefaultWorkerAssociation() {
		return { TPairInitializer<const FName&, const FName&>(FName(TEXT("Default")), FName(TEXT("UnrealWorker"))) };
	}

#if WITH_EDITOR
	static void ValidateOffloadingSettings(TMap<FName, FActorClassSet> OldActorGroups, TMap<FName, FActorClassSet>* ActorGroups,
		TSet<FName> OldWorkerTypes, TSet<FName>* WorkerTypes, FWorkerAssociation& WorkerAssociation);
#endif
};
