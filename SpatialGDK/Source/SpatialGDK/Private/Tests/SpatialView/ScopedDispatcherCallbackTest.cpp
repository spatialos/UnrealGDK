// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialView/ScopedDispatcherCallback.h"

#include "SpatialView/Dispatcher.h"
#include "Tests/SpatialView/DispatcherSpy.h"
#include "Tests/TestDefinitions.h"

#define SCOPED_DISPATCHER_CALLBACK_TEST(TestName) GDK_TEST(Core, Dispatcher, TestName)

namespace SpatialGDK
{
SCOPED_DISPATCHER_CALLBACK_TEST(GIVEN_Constructed_ScopedDispatcherCallback_THEN_Is_Valid)
{
	FDispatcher Dispatcher;

	// GIVEN
	// WHEN
	const FScopedDispatcherCallback ScopedDispatcherCallback(Dispatcher, 1);

	// THEN
	TestTrue("Valid", ScopedDispatcherCallback.IsValid());

	return true;
}

SCOPED_DISPATCHER_CALLBACK_TEST(GIVEN_Constructed_ScopedDispatcherCallback_WHEN_Moved_THEN_New_Callback_Is_Valid)
{
	FDispatcher Dispatcher;

	// GIVEN
	FScopedDispatcherCallback ScopedDispatcherCallback(Dispatcher, 1);

	// WHEN
	const FScopedDispatcherCallback ScopedDispatcherCallbackMoved = MoveTemp(ScopedDispatcherCallback);

	// THEN
	TestTrue("Valid", ScopedDispatcherCallbackMoved.IsValid());

	return true;
}

SCOPED_DISPATCHER_CALLBACK_TEST(GIVEN_Constructed_ScopedDispatcherCallback_WHEN_Moved_THEN_Original_Callback_Is_Not_Valid)
{
	FDispatcher Dispatcher;

	// GIVEN
	FScopedDispatcherCallback ScopedDispatcherCallback(Dispatcher, 1);

	// WHEN
	const FScopedDispatcherCallback ScopedDispatcherCallbackMoved = MoveTemp(ScopedDispatcherCallback);

	// THEN
	TestFalse("Valid", ScopedDispatcherCallback.IsValid());

	return true;
}

SCOPED_DISPATCHER_CALLBACK_TEST(GIVEN_ScopedDispatcherCallback_With_Registered_Callback_WHEN_Destructed_THEN_Callback_Is_Unregistered)
{
	FDispatcherSpy Dispatcher;

	// GIVEN
	const FComponentValueCallback Callback = [](const FEntityComponentChange&) {};
	const CallbackId Id = Dispatcher.RegisterComponentAddedCallback(0, Callback);

	TUniquePtr<FScopedDispatcherCallback> CallbackPtr = MakeUnique<FScopedDispatcherCallback>(Dispatcher, 1);
	TestEqual("Callback initially registered", Dispatcher.GetNumCallbacks(), 1);

	// WHEN
	CallbackPtr.Reset();

	// THEN
	TestEqual("Callback not registered after scoped dispatcher callback is destroyed", Dispatcher.GetNumCallbacks(), 0);

	return true;
}

SCOPED_DISPATCHER_CALLBACK_TEST(
	GIVEN_ScopedDispatcherCallback_With_Registered_Callback_WHEN_Callback_Moved_And_Destructed_THEN_Callback_Is_Unregistered)
{
	FDispatcherSpy Dispatcher;

	// GIVEN
	const FComponentValueCallback Callback = [](const FEntityComponentChange&) {};
	const CallbackId Id = Dispatcher.RegisterComponentAddedCallback(0, Callback);

	TUniquePtr<FScopedDispatcherCallback> CallbackPtr = MakeUnique<FScopedDispatcherCallback>(Dispatcher, 1);
	TestEqual("Callback initially registered", Dispatcher.GetNumCallbacks(), 1);

	// WHEN
	TUniquePtr<FScopedDispatcherCallback> MovedCallbackPtr = MoveTemp(CallbackPtr);
	MovedCallbackPtr.Reset();

	// THEN
	TestEqual("Callback not registered after moved scoped dispatcher callback is destroyed", Dispatcher.GetNumCallbacks(), 0);

	return true;
}

} // namespace SpatialGDK
