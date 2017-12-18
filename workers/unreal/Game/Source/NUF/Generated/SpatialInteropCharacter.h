#include "GameFramework/Actor.h"
#include "UnrealCharacterReplicatedDataComponent.h"

TMap<int, TPair<UProperty*, UProperty*>> CreateCmdIndexToPropertyMap_Character();

void ApplyUpdateToSpatial_Character(AActor* Actor, int CmdIndex, UProperty* ParentProperty, UProperty* Property, UUnrealCharacterReplicatedDataComponent* ReplicatedData, UPackageMapClient* PackageMap);

void ReceiveUpdateFromSpatial_Character(AActor* Actor, TMap<int, TPair<UProperty*, UProperty*>>& CmdIndexToPropertyMap, UUnrealCharacterReplicatedDataComponentUpdate* Update);
