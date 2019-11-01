// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Misc/AutomationTest.h"

#define TEST(ModuleName, ComponentName, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST(TestName, "SpatialGDK."#ModuleName"."#ComponentName"."#TestName, EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::EngineFilter) \
	bool TestName::RunTest(const FString& Parameters)

#define COMPLEX_TEST(ModuleName, ComponentName, TestName) \
	IMPLEMENT_COMPLEX_AUTOMATION_TEST(TestName, "SpatialGDK."#ModuleName"."#ComponentName"."#TestName, EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::EngineFilter) \
	bool TestName::RunTest(const FString& Parameters)

#define DEFINE_LATENT_COMMAND(CommandName) \
	DEFINE_LATENT_AUTOMATION_COMMAND(CommandName); \
	bool CommandName::Update()

#define DEFINE_LATENT_COMMAND_ONE_PARAMETER(CommandName, ParamType1, Param1) \
	DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(CommandName, ParamType1, Param1); \
	bool CommandName::Update()

#define DEFINE_LATENT_COMMAND_TWO_PARAMETERS(CommandName, ParamType1, Param1, ParamType2, Param2) \
	DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(CommandName, ParamType1, Param1, ParamType2, Param2); \
	bool CommandName::Update()
