#include "GameFramework/Actor.h"
#include "UnrealACharacterReplicatedDataComponent.h"

void ApplyUpdateToSpatial_Character(AActor* Actor, int CmdIndex, UProperty* ParentProperty, UProperty* Property, UUnrealACharacterReplicatedDataComponent* ReplicatedData);
void ReceiveUpdateFromSpatial_Character(AActor* Actor) {}
