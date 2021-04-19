// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
/*
#include "SpatialTraceString.generated.h"

class FSpatialTraceStrings

USTRUCT(Blueprintable)
struct FTraceStringHandle
{
	GENERATED_BODY()

public:

	FTraceStringHandle()
		: Handle(InvalidHandle)
	{
	}
	FTraceStringHandle(uint32 InHandle)
		: Handle(InHandle)
	{
	}

	const char* GetString() const
	{
		return FSpatialTraceStrings::Get().GetByIndex(Handle);
	};

	bool IsValid() const
	{
		return Handle != InvalidHandle;
	}

private:
	uint32 Handle;
	static const uint32 InvalidHandle = -1;
};

class FSpatialTraceStrings
{
public:

	static FSpatialTraceStrings& Get()
	{
		if (Instance == nullptr)
		{
			Instance = new FSpatialTraceStrings();
		}
		return *Instance;
	}

	FTraceStringHandle AddString(const char* InputString)
	{
		size_t InputStringLength = strlen(InputString);
		uint32 Hash = HashString(InputString, InputStringLength);
		uint32* Index = HashIndexs.Find(Hash);
		if (Index != nullptr)
		{
			return FTraceStringHandle(*Index);
		}

		uint32 InsertIndex = NextIndex;
		size_t Size = InputStringLength + sizeof(char);
		if (InsertIndex + Size >= BufferSize)
		{
			return FTraceStringHandle();
		}

		HashIndexs.Add(Hash, InsertIndex);
		strncpy_s(&Buffer[InsertIndex], Size, InputString, Size);
		NextIndex += Size;
		return FTraceStringHandle(InsertIndex);
	}

	const char* GetByIndex(int Index)
	{
		if (Index >= BufferSize || Index < 0)
		{
			return nullptr;
		}

		return &Buffer[Index];
	}

private:

	FSpatialTraceStrings();
	static FSpatialTraceStrings* Instance;

	static const uint32 BufferSize = 1000;
	char Buffer[BufferSize];
	uint32 NextIndex = 0;

	TMap<uint32, uint32> HashIndexs;

	static uint32 HashString(const char* Key, size_t Size)
	{
		uint32 Hash, i;
		for (Hash = i = 0; i < Size; ++i)
		{
			Hash += Key[i];
			Hash += (Hash << 10);
			Hash ^= (Hash >> 6);
		}
		Hash += (Hash << 3);
		Hash ^= (Hash >> 11);
		Hash += (Hash << 15);
		return Hash;
	}
};

FTraceStringHandle ReceivePropertyUpdateHandle = FSpatialTraceStrings::Get().AddString("unreal_gdk.receive_property_update");
*/
