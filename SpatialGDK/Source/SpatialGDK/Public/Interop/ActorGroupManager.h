#pragma once

#include "CoreMinimal.h"

#include "ActorGroupManager.generated.h"

USTRUCT()
struct FActorGroupList
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TSet<TSoftClassPtr<AActor>> ActorClasses;

	FActorGroupList()
	{
		ActorClasses = {};
	}

	FActorGroupList(TSet<TSoftClassPtr<AActor>> Classes)
	{
		ActorClasses = Classes;
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
};
