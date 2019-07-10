// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Misc/AutomationTest.h"

#define TEST(ComponentName, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST(TestName, "SpatialGDK.EngineClasses."#ComponentName"."#TestName, EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::EngineFilter) \
	bool TestName::RunTest(const FString& Parameters)
