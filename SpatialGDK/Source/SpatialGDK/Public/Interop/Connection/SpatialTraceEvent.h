// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

namespace SpatialGDK
{
struct FSpatialTraceEvent
{
	explicit FSpatialTraceEvent(FString InType, FString InMessage)
		: Type(MoveTemp(InType))
		, Message(MoveTemp(InMessage))
	{
	}

	void AddData(FString Key, FString Value) { Data.Add(MakeTuple(MoveTemp(Key), MoveTemp(Value))); }

	FString Type;
	FString Message;
	TArray<TTuple<FString, FString>> Data;
};
} // namespace SpatialGDK
