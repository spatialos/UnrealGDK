// Copyright (c) Improbable Worlds Ltd, All Rights Reserved


#include "InitiallyDormantTestActor.h"

AInitiallyDormantTestActor::AInitiallyDormantTestActor()
{
	NetDormancy = ENetDormancy::DORM_Initial;
	//HardWareBp.Set(&Role, sizeof(Role), HardwareBreakpoint::Write);

	bNetLoadOnClient = false;
	UE_LOG(LogTemp, Warning, TEXT("I am being constructed again!"));
}

