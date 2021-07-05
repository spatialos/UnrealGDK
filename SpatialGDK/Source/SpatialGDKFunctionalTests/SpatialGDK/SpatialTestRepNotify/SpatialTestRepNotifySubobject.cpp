// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTestRepNotifySubobject.h"

#include "Net/UnrealNetwork.h"
#include "SpatialTestRepNotify.h"
#include "SpatialTestRepNotifyActor.h"

USpatialTestRepNotifySubobject::USpatialTestRepNotifySubobject()
	: Super()
{
	SetIsReplicatedByDefault(true);
}

void USpatialTestRepNotifySubobject::OnRep_OnChangedRepNotifyInt(int32 OldOnChangedRepNotifyInt)
{
	bParentPropertyWasExpectedProperty = false;
	if (ASpatialTestRepNotifyActor* Parent = Cast<ASpatialTestRepNotifyActor>(GetOwner()))
	{
		bParentPropertyWasExpectedProperty = Parent->OnChangedRepNotifyInt1 == ExpectedParentInt1Property;
	}
}

void USpatialTestRepNotifySubobject::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USpatialTestRepNotifySubobject, OnChangedRepNotifyInt);
}
