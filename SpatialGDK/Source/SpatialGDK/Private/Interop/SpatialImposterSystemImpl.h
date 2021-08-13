// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "EngineClasses/SpatialImposterSystem.h"
#include "LoadBalancing/LBDataStorage.h"

namespace SpatialGDK
{
class FSubView;

class FImposterSystemImpl
{
public:
	FImposterSystemImpl(const FSubView& SubView)
		: ImposterData(SubView)
	{
	}

	void Advance();

	FLBDataCollection ImposterData;
	TArray<SpatialGDK::FEntityEvent> Events;
};
} // namespace SpatialGDK
