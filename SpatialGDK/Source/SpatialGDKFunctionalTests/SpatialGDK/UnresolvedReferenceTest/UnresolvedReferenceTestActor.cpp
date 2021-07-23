#include "UnresolvedReferenceTestActor.h"
#include "Net/UnrealNetwork.h"

void AUnresolvedReferenceTestActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AUnresolvedReferenceTestActor, ActorRefs);
}

void AUnresolvedReferenceTestActor::OnRep_ActorRefs()
{
	++RepNotifyCount;
}
