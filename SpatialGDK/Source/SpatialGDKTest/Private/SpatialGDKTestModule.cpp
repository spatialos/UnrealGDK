// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKTestModule.h"
#include "Misc/AutomationTest.h"

#define LOCTEXT_NAMESPACE "FSpatialGDKModuleTest"

DEFINE_LOG_CATEGORY(LogSpatialGDKTestModule);

IMPLEMENT_MODULE(FSpatialGDKTestModule, SpatialGDKTest);

void FSpatialGDKTestModule::StartupModule()
{
}

void FSpatialGDKTestModule::ShutdownModule()
{
	FAutomationTestFramework::Get().UnregisterAutomationTest("FSpatialReceiverOnEntityQueryResponseTestStatusCodeSuccess");
	FAutomationTestFramework::Get().UnregisterAutomationTest("FSpatialReceiverOnEntityQueryResponseTestStatusCodeFail");
}

#undef LOCTEXT_NAMESPACE
