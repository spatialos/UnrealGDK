// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Utils/TransientUObjectEditor.h"

#include "PolyLBEditorTool.generated.h"

class UProceduralMeshComponent;
class UPolyLBEditorTool;

UCLASS(Transient)
class ALBPolyPreview : public AActor
{
	GENERATED_BODY()
public:
	ALBPolyPreview();

protected:
	friend UPolyLBEditorTool;
	UProceduralMeshComponent* Mesh;
};

// Utility class to create Editor tools exposing a UObject Field and automatically adding Exec UFUNCTION as buttons.
UCLASS()
class SPATIALGDKEDITOR_API UPolyLBEditorTool : public UTransientUObjectEditor
{
	GENERATED_BODY()
public:
	UPolyLBEditorTool();

	UPROPERTY(EditAnywhere)
	FString AssetPath;

	UPROPERTY(EditAnywhere)
	FString AssetName;

	UPROPERTY(EditAnywhere)
	FBox WorldExtents;

	UFUNCTION(Exec)
	void Preview();

	UFUNCTION(Exec)
	void Save();
};
