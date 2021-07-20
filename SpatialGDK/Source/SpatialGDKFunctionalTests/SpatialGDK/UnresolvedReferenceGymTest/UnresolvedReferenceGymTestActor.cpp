#include "UnresolvedReferenceGymTestActor.h"
#include "Net/UnrealNetwork.h"

void AUnresolvedReferenceGymTestActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AUnresolvedReferenceGymTestActor, numRepNotify);
	DOREPLIFETIME(AUnresolvedReferenceGymTestActor, ActorRefs);
}

void AUnresolvedReferenceGymTestActor::OnRep_ActorRefs()
{
	numRepNotify++;
}