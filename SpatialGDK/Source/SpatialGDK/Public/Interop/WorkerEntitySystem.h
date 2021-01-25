// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

DECLARE_LOG_CATEGORY_EXTERN(LogWorkerEntitySystem, Log, All)

namespace SpatialGDK
{
class FSubView;

class WorkerEntitySystem
{
public:
	WorkerEntitySystem(const FSubView& SubView);
	void Advance();

private:
	const FSubView* SubView;
};

} // namespace SpatialGDK
