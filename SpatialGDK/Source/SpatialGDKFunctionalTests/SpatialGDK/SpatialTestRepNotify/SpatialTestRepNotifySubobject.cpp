// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTestRepNotifySubobject.h"

#include "Net/UnrealNetwork.h"
#include "SpatialTestRepNotify.h"

USpatialTestRepNotifySubobject::USpatialTestRepNotifySubobject()
	: Super()
{
	SetIsReplicatedByDefault(true);
}

void USpatialTestRepNotifySubobject::OnRep_OnChangedRepNotifyInt1(int32 OldOnChangedRepNotifyInt1)
{
	bParentPropertyWasExpectedProperty = false;
	if (ASpatialTestRepNotify* Parent = Cast<ASpatialTestRepNotify>(GetOwner()))
	{
		if (Parent->OnChangedRepNotifyInt1 == ExpectedParentInt1Property)
		{
			bParentPropertyWasExpectedProperty = true;
		}
	}
}

void USpatialTestRepNotifySubobject::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USpatialTestRepNotifySubobject, OnChangedRepNotifyInt1);
}
