// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKFunctionalTestsModule.h"

#include "IAutomationControllerModule.h"

#include "LogSpatialFunctionalTest.h"
#include "SpatialFunctionalTest.h"
#include "SpatialGDKEditor/Public/SpatialGDKEditorModule.h"
#include "SpatialGDKEditor/Public/SpatialGDKEditorSettings.h"
#include "SpatialGDKFunctionalTestsPrivate.h"

#define LOCTEXT_NAMESPACE "FSpatialGDKFunctionalTestsModule"

DEFINE_LOG_CATEGORY(LogSpatialGDKFunctionalTests);
DEFINE_LOG_CATEGORY(LogSpatialFunctionalTest);

IMPLEMENT_MODULE(FSpatialGDKFunctionalTestsModule, SpatialGDKFunctionalTests);

void FSpatialGDKFunctionalTestsModule::StartupModule()
{
	// Allow Spatial Plugin to stop PIE after Automation Manager completes the tests
	IAutomationControllerModule& AutomationControllerModule =
		FModuleManager::LoadModuleChecked<IAutomationControllerModule>(TEXT("AutomationController"));
	IAutomationControllerManagerPtr AutomationController = AutomationControllerModule.GetAutomationController();
	AutomationController->OnTestsComplete().AddLambda([]() {
		// Make sure to clear the snapshot in case something happened with Tests (or they weren't ran properly).
		ASpatialFunctionalTest::ClearAllTakenSnapshots();

		if (GetDefault<USpatialGDKEditorSettings>()->bStopPIEOnTestingCompleted && GEditor->IsPlayingSessionInEditor())
		{
			GEditor->EndPlayMap();
		}
	});

	FSpatialGDKEditorModule& GDKEditorModule = FModuleManager::LoadModuleChecked<FSpatialGDKEditorModule>(TEXT("SpatialGDKEditor"));
	GDKEditorModule.OverrideSettingsForTestingDelegate.AddLambda([](UWorld* World, const FString& MapName) {
		FSpatialGDKFunctionalTestsModule::ManageSnapshotsForTests(World, MapName);
	});
}

void FSpatialGDKFunctionalTestsModule::ShutdownModule() {}

void FSpatialGDKFunctionalTestsModule::ManageSnapshotsForTests(UWorld* World, const FString& MapName)
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

#undef LOCTEXT_NAMESPACE
