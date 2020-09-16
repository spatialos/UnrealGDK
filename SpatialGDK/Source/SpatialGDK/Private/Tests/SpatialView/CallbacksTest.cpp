// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Tests/TestDefinitions.h"

#include "SpatialView/Callbacks.h"

#define CALLBACKS_TEST(TestName) GDK_TEST(Core, Callbacks, TestName)

using FIntCallback = SpatialGDK::TCallbacks<int>::CallbackType;

CALLBACKS_TEST(GIVEN_Callbacks_With_Callback_WHEN_IsEmpty_Called_THEN_Returns_False)
{
	// GIVEN
	SpatialGDK::TCallbacks<int> Callbacks;
	TestTrue("Callbacks is initially empty", Callbacks.IsEmpty());

	// WHEN
	const FIntCallback Callback;
	Callbacks.Register(1, Callback);
	const bool IsEmpty = Callbacks.IsEmpty();

	// THEN
	TestFalse("Callbacks is no longer empty", IsEmpty);

	return true;
}

CALLBACKS_TEST(GIVEN_Callbacks_With_Callback_WHEN_Invoke_Called_THEN_Invokes_Callback)
{
	// GIVEN
	SpatialGDK::TCallbacks<int> Callbacks;
	bool Invoked = false;
	const FIntCallback Callback = [&Invoked](int) {
		Invoked = true;
	};
	Callbacks.Register(1, Callback);

	// WHEN
	Callbacks.Invoke(0);

	// THEN
	TestTrue("Callback was invoked", Invoked);

	return true;
}

CALLBACKS_TEST(GIVEN_Callbacks_With_Callback_WHEN_Invoke_Called_THEN_Invokes_Callback_With_Value)
{
	// GIVEN
	int CorrectValue = 1;
	SpatialGDK::TCallbacks<int> Callbacks;
	int InvokeCountWithCorrectValue = 0;
	const FIntCallback Callback = [&InvokeCountWithCorrectValue, CorrectValue](int Value) {
		if (Value == CorrectValue)
		{
			InvokeCountWithCorrectValue++;
		}
	};
	Callbacks.Register(1, Callback);

	// WHEN
	Callbacks.Invoke(1);
	Callbacks.Invoke(0);
	Callbacks.Invoke(1);

	// THEN
	TestEqual("Callback was invoked with the correct value", InvokeCountWithCorrectValue, 2);

	return true;
}

CALLBACKS_TEST(GIVEN_Callbacks_With_Two_Callback_WHEN_Invoke_Called_THEN_Invokes_Both_Callbacks)
{
	// GIVEN
	SpatialGDK::TCallbacks<int> Callbacks;
	int InvokeCount = 0;
	const FIntCallback Callback = [&InvokeCount](int) {
		InvokeCount++;
	};
	Callbacks.Register(1, Callback);
	Callbacks.Register(2, Callback);

	// WHEN
	Callbacks.Invoke(1);

	// THEN
	TestEqual("Callback was invoked twice", InvokeCount, 2);

	return true;
}

CALLBACKS_TEST(GIVEN_Callbacks_With_Callback_WHEN_Callback_Removed_THEN_No_Longer_Calls_Back)
{
	// GIVEN
	SpatialGDK::CallbackId Id = 1;
	SpatialGDK::TCallbacks<int> Callbacks;
	int InvokeCount = 0;
	const FIntCallback Callback = [&InvokeCount](int) {
		InvokeCount++;
	};
	Callbacks.Register(Id, Callback);

	// WHEN
	Callbacks.Invoke(1);
	Callbacks.Remove(Id);
	Callbacks.Invoke(1);

	// THEN
	TestEqual("Callback was invoked once", InvokeCount, 1);

	return true;
}

CALLBACKS_TEST(GIVEN_Callbacks_With_Callback_WHEN_Callback_Adds_Other_Callback_THEN_Only_Calls_First_Callback)
{
	// GIVEN
	SpatialGDK::TCallbacks<int> Callbacks;
	int InvokeCount = 0;
	const FIntCallback SecondCallback = [&InvokeCount](int) {
		InvokeCount++;
	};
	const FIntCallback FirstCallback = [&InvokeCount, &Callbacks, SecondCallback](int) {
		InvokeCount++;
		Callbacks.Register(2, SecondCallback);
	};
	Callbacks.Register(1, FirstCallback);

	// WHEN
	Callbacks.Invoke(1);

	// THEN
	TestEqual("Callback was invoked once", InvokeCount, 1);

	// sanity check: Both callbacks invoked on second invocation
	InvokeCount = 0;
	Callbacks.Invoke(1);
	TestEqual("Both callbacks invoked", InvokeCount, 2);

	return true;
}

CALLBACKS_TEST(GIVEN_Callbacks_With_Two_Callback_WHEN_Callback_Removes_Other_Callback_THEN_Calls_Both_Callbacks)
{
	// GIVEN
	SpatialGDK::CallbackId Id = 2;
	SpatialGDK::TCallbacks<int> Callbacks;
	int InvokeCount = 0;
	const FIntCallback SecondCallback = [&InvokeCount](int) {
		InvokeCount++;
	};
	const FIntCallback FirstCallback = [&InvokeCount, &Callbacks, Id](int) {
		InvokeCount++;
		Callbacks.Remove(Id);
	};
	Callbacks.Register(1, FirstCallback);
	Callbacks.Register(Id, SecondCallback);

	// WHEN
	Callbacks.Invoke(1);

	// THEN
	TestEqual("Both callbacks were invoked", InvokeCount, 2);

	// sanity check: Only one callback invoked on second invocation
	InvokeCount = 0;
	Callbacks.Invoke(1);
	TestEqual("Only one callback invoked", InvokeCount, 1);

	return true;
}

CALLBACKS_TEST(GIVEN_Callbacks_WHEN_Callbacks_Add_And_Remove_Other_Callback_THEN_Other_Callback_Not_Invoked)
{
	// GIVEN
	SpatialGDK::CallbackId Id = 1;
	SpatialGDK::CallbackId AddId = 2;
	SpatialGDK::CallbackId RemoveId = 3;
	SpatialGDK::TCallbacks<int> Callbacks;
	bool Invoked = false;
	const FIntCallback Callback = [&Invoked](int) {
		Invoked = true;
	};
	const FIntCallback AddCallback = [&Callbacks, Id, Callback](int) {
		Callbacks.Register(Id, Callback);
	};
	const FIntCallback RemoveCallback = [&Callbacks, Id](int) {
		Callbacks.Remove(Id);
	};
	Callbacks.Register(RemoveId, RemoveCallback);
	Callbacks.Register(AddId, AddCallback);

	// WHEN
	Callbacks.Invoke(1);

	// THEN
	TestFalse("Callback was not invoked", Invoked);

	// sanity check: Not invoked on a second call after the other callbacks have been removed
	Callbacks.Remove(AddId);
	Callbacks.Remove(RemoveId);
	Callbacks.Invoke(1);
	TestFalse("Callback was not invoked", Invoked);

	return true;
}
