// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#pragma once

#include "CoreMinimal.h"
#include "ModuleManager.h"
#include "TickableEditorObject.h"

class FToolBarBuilder;
class FMenuBuilder;
class FUICommandList;

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialOSEditor, Log, All);

class FSpatialOSEditorToolbarModule : public IModuleInterface, public FTickableEditorObject
{
public:
  FSpatialOSEditorToolbarModule();

  void StartupModule() override;
  void ShutdownModule() override;
  void PreUnloadCallback() override;

  /** FTickableEditorObject interface */
  void Tick(float DeltaTime) override;
  bool IsTickable() const override
  {
	return true;
  }

  TStatId GetStatId() const override
  {
	RETURN_QUICK_DECLARE_CYCLE_STAT(FSpatialOSEditorToolbarModule, STATGROUP_Tickables);
  }

private:
  void RegisterSettings();
  void UnregisterSettings();
  bool HandleSettingsSaved();

  void MapActions(TSharedPtr<FUICommandList> PluginCommands);
  void SetupToolbar(TSharedPtr<FUICommandList> PluginCommands);
  void AddToolbarExtension(FToolBarBuilder& Builder);
  void AddMenuExtension(FMenuBuilder& Builder);

  void StartSpatialOSButtonClicked();
  void StopSpatialOSButtonClicked();

  void StopRunningStack();

  void LaunchInspectorWebpageButtonClicked();

  bool StartSpatialOSStackCanExecute() const;
  bool StopSpatialOSStackCanExecute() const;

  void CheckForRunningStack();
  void OnPropertyChanged(UObject* ObjectBeingModified, FPropertyChangedEvent& PropertyChangedEvent);

private:
  TSharedPtr<FUICommandList> PluginCommands;
  FDelegateHandle OnPropertyChangedDelegateHandle;

  uint32 SpatialOSStackProcessID;
  FProcHandle SpatialOSStackProcHandle;
  bool bStopSpatialOnExit;
};