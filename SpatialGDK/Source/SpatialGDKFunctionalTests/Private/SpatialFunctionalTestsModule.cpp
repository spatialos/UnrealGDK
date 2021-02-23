// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialFunctionalTestsModule.h"
#include "IAutomationControllerModule.h"
#include "Modules/ModuleManager.h"
#include "SpatialFunctionalTest.h"
#include "SpatialGDKEditor/Public/SpatialGDKEditorModule.h"
#include "SpatialGDKEditor/Public/SpatialGDKEditorSettings.h"

FSpatialFunctionalTestsModule::FSpatialFunctionalTestsModule() {}

void FSpatialFunctionalTestsModule::StartupModule()
{
	// Allow Spatial Plugin to stop PIE after Automation Manager completes the tests
	IAutomationControllerModule& AutomationControllerModule =
		FModuleManager::LoadModuleChecked<IAutomationControllerModule>(TEXT("AutomationController"));
	IAutomationControllerManagerPtr AutomationController = AutomationControllerModule.GetAutomationController();
	AutomationController->OnTestsComplete().AddLambda([]() {
		// Make sure to clear the snapshot in case something happened with Tests (or they weren't ran properly).
		ASpatialFunctionalTest::ClearAllTakenSnapshots();

#if ENGINE_MINOR_VERSION < 25
		if (GetDefault<USpatialGDKEditorSettings>()->bStopPIEOnTestingCompleted && GEditor->EditorWorld != nullptr)
#else
		if (GetDefault<USpatialGDKEditorSettings>()->bStopPIEOnTestingCompleted && GEditor->IsPlayingSessionInEditor())
#endif
		{
			GEditor->EndPlayMap();
		}
	});

	FSpatialGDKEditorModule& GDKEditorModule = FModuleManager::LoadModuleChecked<FSpatialGDKEditorModule>(TEXT("SpatialGDKEditor"));
	GDKEditorModule.OverrideSettingsForTestingDelegate.AddLambda([](UWorld* World, const FString& MapName) {
		FSpatialFunctionalTestsModule::OverrideSettingsForTesting(World, MapName);
	});
}

void FSpatialFunctionalTestsModule::OverrideSettingsForTesting(UWorld* World, const FString& MapName)
{
	// By default, clear that the runtime/test was loaded from a snapshot taken for a given world.
	ASpatialFunctionalTest::ClearLoadedFromTakenSnapshot();

	if (GetDefault<UGeneralProjectSettings>()->UsesSpatialNetworking())
	{
		FString SnapshotForMap = ASpatialFunctionalTest::GetTakenSnapshotPath(World);

		if (!SnapshotForMap.IsEmpty())
		{
			GetMutableDefault<ULevelEditorPlaySettings>()->SetSnapshotOverride(SnapshotForMap);
			// Set that we're loading from taken snapshot.
			ASpatialFunctionalTest::SetLoadedFromTakenSnapshot();
		}
	}
}
