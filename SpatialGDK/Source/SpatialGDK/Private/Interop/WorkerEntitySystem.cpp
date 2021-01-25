// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/WorkerEntitySystem.h"

#include "SpatialView/EntityDelta.h"
#include "SpatialView/SubView.h"

DEFINE_LOG_CATEGORY(LogWorkerEntitySystem);

namespace SpatialGDK
{
struct FSubViewDelta;

WorkerEntitySystem::WorkerEntitySystem(const FSubView& SubView)
	: SubView(&SubView)
{
}

void WorkerEntitySystem::Advance()
{
	const FSubViewDelta& SubViewDelta = SubView->GetViewDelta();
	for (const EntityDelta& Delta : SubViewDelta.EntityDeltas)
	{
		switch (Delta.Type)
		{
		default:
			break;
		}
	}
}

} // Namespace SpatialGDK
