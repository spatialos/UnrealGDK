// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKServicesModule.h"

#include "Editor/WorkspaceMenuStructure/Public/WorkspaceMenuStructure.h"
#include "Editor/WorkspaceMenuStructure/Public/WorkspaceMenuStructureModule.h"
#include "EditorStyleSet.h"
#include "Features/IModularFeatures.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Docking/TabManager.h"
#include "Modules/ModuleManager.h"
#include "SSpatialOutputLog.h"
#include "Textures/SlateIcon.h"
#include "Widgets/Docking/SDockTab.h"

#include "SpatialGDKServicesPrivate.h"

#define LOCTEXT_NAMESPACE "FSpatialGDKServicesModule"

DEFINE_LOG_CATEGORY(LogSpatialGDKServices);

IMPLEMENT_MODULE(FSpatialGDKServicesModule, SpatialGDKServices);

static const FName SpatialOutputLogTabName = FName(TEXT("SpatialOutputLog"));

/** This class is to capture all log output even if the log window is closed */
class FSpatialOutputLogHistory : public FOutputDevice
{
public:

	FSpatialOutputLogHistory()
	{
	}

	~FSpatialOutputLogHistory()
	{
	}

	/** Gets all captured messages */
	const TArray<TSharedPtr<FSpatialLogMessage>>& GetMessages() const
	{
		return Messages;
	}

protected:

	virtual void Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, const class FName& Category) override
	{
		// Capture all incoming messages and store them in history
		SSpatialOutputLog::CreateLogMessages(V, Verbosity, Category, Messages);
	}

private:

	/** All log messages since this module has been started */
	TArray<TSharedPtr<FSpatialLogMessage>> Messages;
};

/** Spatial output log app spawner */
static TSharedPtr<FSpatialOutputLogHistory> SpatialOutputLogHistory;

TSharedRef<SDockTab> SpawnSpatialOutputLog(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		.Icon(FEditorStyle::GetBrush("Log.TabIcon"))
		.TabRole(ETabRole::NomadTab)
		.Label(NSLOCTEXT("OutputLog", "SpatialTabTitle", "Spatial Output Log"))
		[
			SNew(SSpatialOutputLog).Messages(SpatialOutputLogHistory->GetMessages())
		];
}

void FSpatialGDKServicesModule::StartupModule()
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(SpatialOutputLogTabName, FOnSpawnTab::CreateStatic(&SpawnSpatialOutputLog))
		.SetDisplayName(NSLOCTEXT("UnrealEditor", "SpatialOutputLogTab", "Spatial Output Log"))
		.SetTooltipText(NSLOCTEXT("UnrealEditor", "SpatialOutputLogTooltipText", "Open the Spatial Output Log tab."))
		.SetGroup(WorkspaceMenu::GetMenuStructure().GetDeveloperToolsLogCategory())
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "Log.TabIcon"));

	SpatialOutputLogHistory = MakeShareable(new FSpatialOutputLogHistory());
}

void FSpatialGDKServicesModule::ShutdownModule()
{
	if (FSlateApplication::IsInitialized())
	{
		FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(SpatialOutputLogTabName);
	}
}

FLocalDeploymentManager* FSpatialGDKServicesModule::GetLocalDeploymentManager()
{
	return &LocalDeploymentManager;
}

FString FSpatialGDKServicesModule::GetSpatialOSDirectory(const FString& AppendPath)
{
	return FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectDir(), TEXT("/../spatial/"), AppendPath));
}

FString FSpatialGDKServicesModule::GetSpatialGDKPluginDirectory(const FString& AppendPath)
{
	FString PluginDir = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectPluginsDir(), TEXT("UnrealGDK")));

	if (!FPaths::DirectoryExists(PluginDir))
	{
		// If the Project Plugin doesn't exist then use the Engine Plugin.
		PluginDir = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::EnginePluginsDir(), TEXT("UnrealGDK")));
		ensure(FPaths::DirectoryExists(PluginDir));
	}

	return FPaths::ConvertRelativePathToFull(FPaths::Combine(PluginDir, AppendPath));
}

#undef LOCTEXT_NAMESPACE
