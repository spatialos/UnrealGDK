// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

struct FRepHandleData
{
	UProperty* Parent;
	UProperty* Property;
	ELifetimeCondition Condition;
	ELifetimeRepNotifyCondition RepNotifyCondition;
};

using FRepHandlePropertyMap = TMap<int32, FRepHandleData>;
