#pragma once

#include "Containers/Array.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "Interop/WorkingSetsCommon.h"
#include "SpatialCommonTypes.h"

#include "SpatialWorkingSetSubsystem.generated.h"

class UObject;
class AActor;

namespace SpatialGDK
{
class FSubView;
struct EntityDelta;
} // namespace SpatialGDK

USTRUCT(BlueprintType)
struct FActorSetHandle
{
	GENERATED_BODY()

	UPROPERTY()
	int64 ActorSetEntityHandle;
};

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnWorkingSetEventDynamic, bool, bWasSuccessful);

UCLASS()
class SPATIALGDK_API USpatialActorSetSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "SpatialGDK|Actor Set", meta = (WorldContext = Context))
	bool IsSetHandleValid(FActorSetHandle SetHandle, UObject* Context);

	UFUNCTION(BlueprintCallable, Category = "SpatialGDK|Actor Set", meta = (WorldContext = Context))
	FActorSetHandle CreateActorSet(AActor* Leader, const TArray<AActor*>& Actors, UObject* Context, FOnWorkingSetEventDynamic OnSetCreated);

	UFUNCTION(BlueprintCallable, Category = "SpatialGDK|Actor Set", meta = (WorldContext = Context))
	void ModifyActorSet(FActorSetHandle Handle, AActor* Leader, const TArray<AActor*>& Actors, UObject* Context,
						FOnWorkingSetEventDynamic OnSetModified);

	UFUNCTION(BlueprintCallable, Category = "SpatialGDK|Actor Set", meta = (WorldContext = Context))
	void DisbandActorSet(FActorSetHandle Handle, UObject* Context, FOnWorkingSetEventDynamic OnSetDisbanded);

	UFUNCTION(BlueprintCallable, Category = "SpatialGDK|Actor Set", meta = (WorldContext = Context))
	FActorSetHandle GetActorSetHandle(const AActor* Actor, UObject* Context);

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }

	void Advance(const SpatialGDK::FSubView& MarkerEntitiesSubview);
};
