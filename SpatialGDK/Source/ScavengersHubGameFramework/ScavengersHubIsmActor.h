#pragma once
#include "Containers/Queue.h"
#include "GameFramework/Actor.h"

#include "SpatialCommonTypes.h"

#include "ScavengersHubIsmActor.generated.h"

UCLASS(Blueprintable)
class SCAVENGERSHUBGAMEFRAMEWORK_API AScavengersHubIsmActor : public AActor
{
	GENERATED_BODY()
public:
	AScavengersHubIsmActor();

	// AActor interface
	virtual void BeginPlay() override;

	// These functions may only be called after BeginPlay.
	void SetStaticMesh(UStaticMesh* Mesh);
	UStaticMesh* GetStaticMesh();

	// If CanUpdateEntityTransformsInParallel returns true, this function may be called concurrently from different threads.
	void SetEntityTransform(Worker_EntityId EntityId, TOptional<FTransform> Transform, const TArray<float>& CustomInstanceData,
							bool IsParallel);

	bool CanUpdateEntityTransformsInParallel() const { return bUseCustomInstancedStaticMeshComponent; }

	// If this is set to true, CustomInstancedStaticMeshComponent will be used, otherwise HierarchicalInstancedStaticMeshComponent.
	UPROPERTY(EditDefaultsOnly)
	bool bUseCustomInstancedStaticMeshComponent;

protected:
	UPROPERTY(EditDefaultsOnly)
	class UScavengersHubIsmComponent* CustomInstancedStaticMeshComponent;

	UPROPERTY(EditDefaultsOnly)
	class UHierarchicalInstancedStaticMeshComponent* HierarchicalInstancedStaticMeshComponent;

private:
	UPROPERTY()
	class UInstancedStaticMeshComponent* UsedIsmComponent;

	TMap<int64, int32> EntityIdToEntityIndex;
	TQueue<int32> FreeEntityIndices;
	int32 NextEntityIndex = 0;
};
