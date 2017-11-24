#pragma once

#include "SpatialShadowActor.h"
#include "SpatialShadowActor_Character.generated.h"

class UUnrealACharacterReplicatedDataComponent;
class UUnrealACharacterReplicatedDataComponentUpdate;
class UUnrealACharacterCompleteDataComponent;

struct RepHandleData
{
	UProperty* Parent;
	UProperty* Property;
	int32 Offset;
};

UCLASS()
class ASpatialShadowActor_Character : public ASpatialShadowActor
{
	GENERATED_BODY()
public:
	ASpatialShadowActor_Character();

	void ApplyUpdateToSpatial(FArchive& Reader, int32 Handle, UProperty* Property);
	void ReceiveUpdateFromSpatial(AActor* Actor, UUnrealACharacterReplicatedDataComponentUpdate* Update);

	void ReplicateChanges(float DeltaTime) override;
	const TMap<int32, RepHandleData>& GetHandlePropertyMap() const;

	UPROPERTY()
	UUnrealACharacterReplicatedDataComponent* ReplicatedData;
	UPROPERTY()
	UUnrealACharacterCompleteDataComponent* CompleteData; 

private:
	TMap<int32, RepHandleData> HandleToPropertyMap;

	UFUNCTION()
	void OnReplicatedMovement();
};
