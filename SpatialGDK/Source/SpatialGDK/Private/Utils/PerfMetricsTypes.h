// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#pragma once

/******
******* Note this is an internal only feature
*******/

enum class GDKMetric
{
	NumActorsReplicated,
	ActorReplicationTime,
	NetTick,
	TickFlush,
	//WorkerOps, // This is hard
	Count
};
