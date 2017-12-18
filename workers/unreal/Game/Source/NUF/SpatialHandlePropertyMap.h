// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

struct RepHandleData
{
	UProperty* Parent;
	UProperty* Property;
	int32 Offset;
};

using RepHandlePropertyMap = TMap<int32, RepHandleData>;
