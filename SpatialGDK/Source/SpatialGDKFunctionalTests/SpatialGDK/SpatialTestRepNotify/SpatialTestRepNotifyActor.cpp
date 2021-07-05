// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTestRepNotifyActor.h"

#include "Net/UnrealNetwork.h"
#include "SpatialTestRepNotifySubobject.h"

ASpatialTestRepNotifyActor::ASpatialTestRepNotifyActor()
{
	bReplicates = true;

	TestSubobject = CreateDefaultSubobject<USpatialTestRepNotifySubobject>(TEXT("USpatialTestRepNotifySubobject"));
	TestSubobject->SetIsReplicated(true);

	AlwaysRepNotifyInt1 = 0;
	OnChangedRepNotifyInt2 = 0;
	AlwaysRepNotifyInt2 = 0;

	TestSubobject->ExpectedParentInt1Property = 350;
	ExpectedSubobjectIntProperty = 400;

	bOnRepOnChangedRepNotifyInt1Called = false;
	bOnRepAlwaysRepNotifyInt1Called = false;

	bOnRepOnChangedInt1CalledBeforeInt2 = false;

	bSubobjectIntPropertyWasExpectedProperty = false;
}


void ASpatialTestRepNotifyActor::OnRep_OnChangedRepNotifyInt1(int32 OldOnChangedRepNotifyInt1)
{
	bOnRepOnChangedRepNotifyInt1Called = true;

	ensureAlwaysMsgf(IsValid(TestSubobject), TEXT("TestSubobject should be valid"));
	bSubobjectIntPropertyWasExpectedProperty = TestSubobject->OnChangedRepNotifyInt == ExpectedSubobjectIntProperty;
}

void ASpatialTestRepNotifyActor::OnRep_AlwaysRepNotifyInt1(int32 OldAlwaysRepNotifyInt1)
{
	bOnRepAlwaysRepNotifyInt1Called = true;
}

void ASpatialTestRepNotifyActor::OnRep_OnChangedRepNotifyInt2(int32 InOldOnChangedRepNotifyInt2)
{
	OldOnChangedRepNotifyInt2 = InOldOnChangedRepNotifyInt2;
	bOnRepOnChangedInt1CalledBeforeInt2 = bOnRepOnChangedRepNotifyInt1Called;
}

void ASpatialTestRepNotifyActor::OnRep_AlwaysRepNotifyInt2(int32 InOldAlwaysRepNotifyInt2)
{
	OldAlwaysRepNotifyInt2 = InOldAlwaysRepNotifyInt2;
}

void ASpatialTestRepNotifyActor::OnRep_TestArray(TArray<int32> InOldTestArray)
{
	OldTestArray = InOldTestArray;
}

void ASpatialTestRepNotifyActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASpatialTestRepNotifyActor, OnChangedRepNotifyInt1);
	DOREPLIFETIME_CONDITION_NOTIFY(ASpatialTestRepNotifyActor, AlwaysRepNotifyInt1, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME(ASpatialTestRepNotifyActor, OnChangedRepNotifyInt2);
	DOREPLIFETIME_CONDITION_NOTIFY(ASpatialTestRepNotifyActor, AlwaysRepNotifyInt2, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME(ASpatialTestRepNotifyActor, TestArray);

	DOREPLIFETIME(ASpatialTestRepNotifyActor, TestSubobject);
}
