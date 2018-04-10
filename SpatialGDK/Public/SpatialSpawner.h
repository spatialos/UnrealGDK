// #pragma once

// #include "CoreMinimal.h"
// #include "GameFramework/Actor.h"
// #include "SpatialOSCommandResult.h"
// #include "SpatialOsComponent.h"
// #include "SpatialSpawner.generated.h"

// class UPlayerSpawnerComponent;
// class USpawnPlayerCommandResponder;
// class SpawnPlayerResponse;

// UCLASS()
// class SPATIALGDK_API ASpatialSpawner : public AActor
// {
// 	GENERATED_BODY()
	
// public:	
// 	ASpatialSpawner();

// protected:
// 	virtual void BeginPlay() override;
// 	virtual void PostInitializeComponents() override;
// 	virtual void BeginDestroy() override;
	
// 	UFUNCTION()
// 	virtual void HandleSpawnRequest(USpawnPlayerCommandResponder* Responder);

// 	UPROPERTY()
// 	UPlayerSpawnerComponent* PlayerSpawnerComponent;
// };
