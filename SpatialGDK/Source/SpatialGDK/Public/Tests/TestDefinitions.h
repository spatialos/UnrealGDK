// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Misc/AutomationTest.h"

#define GDK_TEST(ModuleName, ComponentName, TestName)                                                                                      \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST(TestName, "SpatialGDK." #ModuleName "." #ComponentName "." #TestName,                                 \
									 EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::EngineFilter)                    \
	bool TestName::RunTest(const FString& Parameters)

#define GDK_TEST_NEW(ModuleName, ComponentName, TestName)                                                                                  \
	IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(TestName, GDKTestBase, "SpatialGDK." #ModuleName "." #ComponentName "." #TestName,             \
											EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::EngineFilter)             \
	bool TestName::RunGDKTest(const FString& Parameters)

#define GDK_COMPLEX_TEST(ModuleName, ComponentName, TestName)                                                                              \
	IMPLEMENT_COMPLEX_AUTOMATION_TEST(TestName, "SpatialGDK." #ModuleName "." #ComponentName "." #TestName,                                \
									  EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::EngineFilter)

#define GDK_SLOW_TEST(ModuleName, ComponentName, TestName)                                                                                 \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST(TestName, "SpatialGDKSlow." #ModuleName "." #ComponentName "." #TestName,                             \
									 EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::EngineFilter)                    \
	bool TestName::RunTest(const FString& Parameters)

#define GDK_SLOW_COMPLEX_TEST(ModuleName, ComponentName, TestName)                                                                         \
	IMPLEMENT_COMPLEX_AUTOMATION_TEST(TestName, "SpatialGDKSlow." #ModuleName "." #ComponentName "." #TestName,                            \
									  EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::EngineFilter)
