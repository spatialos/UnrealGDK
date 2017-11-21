#include "GameFramework/Actor.h"
#include "UnrealACharacterReplicatedDataComponent.h"

struct RepHandleData
{
UProperty* Parent;
UProperty* Property;
int32 Offset;
};

TMap<int, RepHandleData> CreateHandleToPropertyMap_Character();
void ApplyUpdateToSpatial_Character(FArchive& Reader, int32 Handle, UProperty* Property, UUnrealACharacterReplicatedDataComponent* ReplicatedData);
void ApplyUpdateToSpatial_Old_Character(AActor* Actor, int32 Handle, UProperty* ParentProperty, UProperty* Property, UUnrealACharacterReplicatedDataComponent* ReplicatedData);
void ReceiveUpdateFromSpatial_Character(AActor* Actor, TMap<int, RepHandleData>& HandleToPropertyMap, UUnrealACharacterReplicatedDataComponentUpdate* Update);
