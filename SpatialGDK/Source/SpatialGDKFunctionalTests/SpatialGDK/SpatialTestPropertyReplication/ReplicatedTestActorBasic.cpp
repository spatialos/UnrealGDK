// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "ReplicatedTestActorBasic.h"
#include "Net/UnrealNetwork.h"

AReplicatedTestActorBasic::AReplicatedTestActorBasic()
{
	ReplicatedIntProperty = 0;
	ReplicatedFloatProperty = 0.0;
	bReplicatedBoolProperty = false;
	ReplicatedStringProperty = "";

	IntProperty = 0;
	FloatProperty = 0.0;
	bBoolProperty = false;
	StringProperty = "";
}

void AReplicatedTestActorBasic::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AReplicatedTestActorBasic, ReplicatedIntProperty);
	DOREPLIFETIME(AReplicatedTestActorBasic, ReplicatedFloatProperty);
	DOREPLIFETIME(AReplicatedTestActorBasic, bReplicatedBoolProperty);
	DOREPLIFETIME(AReplicatedTestActorBasic, ReplicatedStringProperty);
}
