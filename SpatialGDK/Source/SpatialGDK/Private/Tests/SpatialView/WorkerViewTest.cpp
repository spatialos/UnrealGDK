// // Copyright (c) Improbable Worlds Ltd, All Rights Reserved
//
// #include "Tests/TestDefinitions.h"
//
// #include "SpatialView/OpList/ViewDeltaLegacyOpList.h"
// #include "SpatialView/WorkerView.h"
//
// #define WORKERVIEW_TEST(TestName) GDK_TEST(Core, WorkerView, TestName)
//
// using namespace SpatialGDK;
//
// WORKERVIEW_TEST(GIVEN_WorkerView_with_one_CreateEntityRequest_WHEN_FlushLocalChanges_called_THEN_one_CreateEntityRequest_returned)
// {
// 	// GIVEN
// 	WorkerView View;
// 	CreateEntityRequest Request = {};
// 	View.SendCreateEntityRequest(MoveTemp(Request));
//
// 	// WHEN
// 	const auto Messages = View.FlushLocalChanges();
//
// 	// THEN
// 	TestTrue("WorkerView has one CreateEntityRequest", Messages->CreateEntityRequests.Num() == 1);
//
// 	return true;
// }
//
// WORKERVIEW_TEST(
// 	GIVEN_WorkerView_with_multiple_CreateEntityRequest_WHEN_FlushLocalChanges_called_THEN_mutliple_CreateEntityRequests_returned)
// {
// 	// GIVEN
// 	WorkerView View;
// 	CreateEntityRequest Request = {};
// 	View.SendCreateEntityRequest(MoveTemp(Request));
// 	View.SendCreateEntityRequest(MoveTemp(Request));
//
// 	auto Messages = View.FlushLocalChanges();
//
// 	// THEN
// 	TestTrue("WorkerView has multiple CreateEntityRequest", Messages->CreateEntityRequests.Num() > 1);
//
// 	return true;
// }
