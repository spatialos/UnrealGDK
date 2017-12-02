#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpatialSpawner.generated.h"

class USpawnerComponent;
class USpawnPlayerCommandResponder;

UCLASS()
class NUF_API ASpatialSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	ASpatialSpawner();

protected:
	virtual void BeginPlay() override;
	virtual void BeginDestroy() override;
	
	UFUNCTION()
	virtual void HandleSpawnRequest(USpawnPlayerCommandResponder* Responder);

	UPROPERTY()
	USpawnerComponent* SpawnerComponent;
};
