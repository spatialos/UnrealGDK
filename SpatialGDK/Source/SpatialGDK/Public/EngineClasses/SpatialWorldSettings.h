// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "LoadBalancing/SpatialMultiWorkerSettings.h"

#include "GameFramework/WorldSettings.h"
#include "Templates/SubclassOf.h"

#include "SpatialWorldSettings.generated.h"

UCLASS()
class SPATIALGDK_API ASpatialWorldSettings : public AWorldSettings
{
	GENERATED_UCLASS_BODY()
	friend class USpatialStatics;

public:
	/** If command line override -OverrideMultiWorkerSettingsClass is set then return the specified class from the command line.
	 * Else if bForceNonEditorSettings is set, return the MultiWorkerSettingsClass.
	 * Else if the EditorMultiWorkerSettingsOverride is set and we are in the Editor, return the EditorMultiWorkerSettings.
	 * Else if multi-worker is disabled in the editor, return the single worker settings class
	 * Else if the MultiWorkerSettingsClass is set return it.
	 * Otherwise return the single worker settings class.  */
	TSubclassOf<USpatialMultiWorkerSettings> GetMultiWorkerSettingsClass(bool bForceNonEditorSettings = false);

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = "Testing")
	bool bEnableDebugInterface = false;
#endif

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	static void EditorRefreshSpatialDebugger();

	// This function was specifically designed to be used with the GenerateTestMapsCommandlet.
	// Other uses are untested, and probably produce undefined behavior.
	void SetMultiWorkerSettingsClass(TSubclassOf<USpatialMultiWorkerSettings> MultiWorkerSettingsClass);
#endif // WITH_EDITOR

private:
	/** Specify the load balancing strategy to be used for multiple workers */
	UPROPERTY(EditAnywhere, Category = "Multi-Worker")
	TSubclassOf<USpatialMultiWorkerSettings> MultiWorkerSettingsClass;

	/** Editor override to specify a different load balancing strategy to run in-editor */
	UPROPERTY(EditAnywhere, Category = "Multi-Worker")
	TSubclassOf<USpatialMultiWorkerSettings> EditorMultiWorkerSettingsOverride;

	/** Gets MultiWorkerSettingsClass if set, otherwise returns a single worker behaviour. */
	TSubclassOf<USpatialMultiWorkerSettings> GetValidWorkerSettings() const;
};
