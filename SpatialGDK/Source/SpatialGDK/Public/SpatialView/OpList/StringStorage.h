// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Containers/Array.h"
#include "Containers/StringConv.h"
#include "Templates/UniquePtr.h"
#include <cstddef>
#include <cstring>

namespace SpatialGDK
{
class StringStorage
{
public:
	StringStorage(const FString& InString)
	{
		const int32 SourceLength = TCString<TCHAR>::Strlen(*InString);
		// Add one to include the null terminator.
		const int32 BufferSize = FTCHARToUTF8_Convert::ConvertedLength(*InString, SourceLength) + 1;
		Storage = MakeUnique<char[]>(BufferSize);
		FTCHARToUTF8_Convert::Convert(Storage.Get(), BufferSize, *InString, SourceLength + 1);
	}

	StringStorage(const char* InString)
	{
		// Add one to include the null terminator.
		const size_t BufferSize = std::strlen(InString) + 1;
		Storage = MakeUnique<char[]>(BufferSize);
		std::memcpy(Storage.Get(), InString, BufferSize);
	}

	const char* Get() const { return Storage.Get(); }

private:
	TUniquePtr<char[]> Storage;
};
} // namespace SpatialGDK
