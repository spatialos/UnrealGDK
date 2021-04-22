// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKTests/Public/GDKAutomationTestBase.h"
#include "Tests/TestDefinitions.h"

#include "Interop/Connection/SpatialTraceEventDataBuilder.h"

#include "CoreMinimal.h"
#include "Engine/Engine.h"

#include <string>

#define EVENTTRACING_TEST(TestName) GDK_AUTOMATION_TEST(Core, EventTracingTests, TestName)

using namespace SpatialGDK;

namespace
{
} // anonymous namespace

EVENTTRACING_TEST(GIVEN_a_string_cache_WHEN_combing_two_string_THEN_the_strings_are_combined_and_stored_correctly)
{
	FSpatialTraceEventDataBuilder::FStringCache StringCache;
	int32 Handle = StringCache.CombineStrings("string1", "string2");
	bool bSuccess = FCStringAnsi::Strcmp("string1string2", StringCache.Get(Handle)) == 0;
	TestTrue("String succesfully stored and retreived", bSuccess);
	return true;
}

EVENTTRACING_TEST(
	GIVEN_a_string_cache_WHEN_combing_two_string_that_will_overflow_but_cant_combine_THEN_the_strings_are_combined_and_stored_correctly)
{
	FSpatialTraceEventDataBuilder::FStringCache StringCache;

	FString InputString;
	for (int32 i = 0; i < StringCache.GetBufferSize() - 1; ++i)
	{
		InputString += "a";
	}

	std::string InputSrc = (const char*)TCHAR_TO_ANSI(*InputString);
	int32 Handle = StringCache.CombineStrings(InputSrc.c_str(), "string2");
	bool bSuccess = FCStringAnsi::Strcmp(InputSrc.c_str(), StringCache.Get(Handle)) == 0;

	TestTrue("String succesfully stored and retreived", bSuccess);
	return true;
}

EVENTTRACING_TEST(
	GIVEN_a_string_cache_WHEN_combing_two_string_that_will_overflow_but_can_partially_combine_THEN_the_strings_are_combined_and_stored_correctly)
{
	FSpatialTraceEventDataBuilder::FStringCache StringCache;

	FString InputString;
	for (int32 i = 0; i < StringCache.GetBufferSize() - 2; ++i)
	{
		InputString += "a";
	}

	FString OutputString = InputString + "s";
	std::string InputSrc = (const char*)TCHAR_TO_ANSI(*InputString);
	std::string OuputSrc = (const char*)TCHAR_TO_ANSI(*OutputString);

	int32 Handle = StringCache.CombineStrings(InputSrc.c_str(), "string2");
	bool bSuccess = FCStringAnsi::Strcmp(OuputSrc.c_str(), StringCache.Get(Handle)) == 0;

	TestTrue("String succesfully stored and retreived", bSuccess);
	return true;
}

EVENTTRACING_TEST(GIVEN_a_string_cache_WHEN_adding_a_string_THEN_the_string_is_stored_correctly)
{
	FSpatialTraceEventDataBuilder::FStringCache StringCache;
	int32 Handle = StringCache.AddString("string1");
	bool bSuccess = FCStringAnsi::Strcmp("string1", StringCache.Get(Handle)) == 0;
	TestTrue("String succesfully stored and retreived", bSuccess);
	return true;
}

EVENTTRACING_TEST(GIVEN_a_string_cache_WHEN_adding_an_fstring_THEN_the_string_is_stored_correctly)
{
	FSpatialTraceEventDataBuilder::FStringCache StringCache;
	int32 Handle = StringCache.AddFString(TEXT("string1"));
	bool bSuccess = FCStringAnsi::Strcmp("string1", StringCache.Get(Handle)) == 0;
	TestTrue("String succesfully stored and retreived", bSuccess);
	return true;
}

EVENTTRACING_TEST(GIVEN_a_string_cache_WHEN_adding_a_uint32_THEN_the_string_is_stored_correctly)
{
	FSpatialTraceEventDataBuilder::FStringCache StringCache;
	int32 Handle = StringCache.AddUInt32(101);
	bool bSuccess = FCStringAnsi::Strcmp("101", StringCache.Get(Handle)) == 0;
	TestTrue("String succesfully stored and retreived", bSuccess);
	return true;
}

EVENTTRACING_TEST(GIVEN_a_string_cache_WHEN_adding_a_uint64_THEN_the_string_is_stored_correctly)
{
	FSpatialTraceEventDataBuilder::FStringCache StringCache;
	int32 Handle = StringCache.AddUInt64(101);
	bool bSuccess = FCStringAnsi::Strcmp("101", StringCache.Get(Handle)) == 0;
	TestTrue("String succesfully stored and retreived", bSuccess);
	return true;
}

EVENTTRACING_TEST(GIVEN_a_string_cache_WHEN_adding_a_int32_THEN_the_string_is_stored_correctly)
{
	FSpatialTraceEventDataBuilder::FStringCache StringCache;
	int32 Handle = StringCache.AddInt32(101);
	bool bSuccess = FCStringAnsi::Strcmp("101", StringCache.Get(Handle)) == 0;
	TestTrue("String succesfully stored and retreived", bSuccess);
	return true;
}

EVENTTRACING_TEST(GIVEN_a_string_cache_WHEN_adding_a_int64_THEN_the_string_is_stored_correctly)
{
	FSpatialTraceEventDataBuilder::FStringCache StringCache;
	int32 Handle = StringCache.AddInt64(101);
	bool bSuccess = FCStringAnsi::Strcmp("101", StringCache.Get(Handle)) == 0;
	TestTrue("String succesfully stored and retreived", bSuccess);
	return true;
}

EVENTTRACING_TEST(GIVEN_a_string_cache_WHEN_adding_multiple_strings_THEN_the_strings_is_stored_correctly)
{
	FSpatialTraceEventDataBuilder::FStringCache StringCache;
	int32 Handle1 = StringCache.CombineStrings("string1", "string2");
	int32 Handle2 = StringCache.AddString("string1");
	int32 Handle3 = StringCache.AddFString(TEXT("string1"));
	int32 Handle4 = StringCache.AddUInt32(101);
	int32 Handle5 = StringCache.AddUInt64(101);
	int32 Handle6 = StringCache.AddInt32(101);
	int32 Handle7 = StringCache.AddInt64(101);
	bool bSuccess = FCStringAnsi::Strcmp("string1string2", StringCache.Get(Handle1)) == 0;
	bSuccess &= FCStringAnsi::Strcmp("string1", StringCache.Get(Handle2)) == 0;
	bSuccess &= FCStringAnsi::Strcmp("string1", StringCache.Get(Handle3)) == 0;
	bSuccess &= FCStringAnsi::Strcmp("101", StringCache.Get(Handle4)) == 0;
	bSuccess &= FCStringAnsi::Strcmp("101", StringCache.Get(Handle5)) == 0;
	bSuccess &= FCStringAnsi::Strcmp("101", StringCache.Get(Handle6)) == 0;
	bSuccess &= FCStringAnsi::Strcmp("101", StringCache.Get(Handle7)) == 0;
	TestTrue("Strings succesfully stored and retreived", bSuccess);
	return true;
}

EVENTTRACING_TEST(GIVEN_a_string_cache_WHEN_adding_string_that_overflows_THEN_the_output_string_is_truncated_to_buffer_size)
{
	FSpatialTraceEventDataBuilder::FStringCache StringCache;
	int32 CacheBufferSize = StringCache.GetBufferSize();

	FString InputString;
	for (int32 i = 0; i < 2 * CacheBufferSize; ++i)
	{
		InputString += "a";
	}

	int32 Handle = StringCache.AddFString(InputString);
	const char* OutputString = StringCache.Get(Handle);

	int32 StringLength = strlen(OutputString);
	bool bSuccess = StringLength == CacheBufferSize - 1;

	TestTrue("Strings succesfully stored and retreived", bSuccess);
	return true;
}

EVENTTRACING_TEST(GIVEN_a_string_cache_that_is_full_WHEN_adding_string_THEN_the_output_string_empty)
{
	FSpatialTraceEventDataBuilder::FStringCache StringCache;
	int32 CacheBufferSize = StringCache.GetBufferSize();

	FString InputString;
	for (int32 i = 0; i < 2 * CacheBufferSize; ++i)
	{
		InputString += "a";
	}

	StringCache.AddFString(InputString);
	int32 Handle = StringCache.AddString("string1");

	const char* OutputString = StringCache.Get(Handle);
	bool bSuccess = FCStringAnsi::Strcmp("", OutputString) == 0;

	TestTrue("Strings succesfully stored and retreived", bSuccess);
	return true;
}
