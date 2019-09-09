// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "SpatialPersistenceConfig.generated.h"

UENUM()
enum class EPersistenceSelectionMode : uint8
{
	PERSISTENCE_SELECTION_MODE_Include,
	PERSISTENCE_SELECTION_MODE_Exclude
};

UCLASS()
class USpatialPersistenceConfig : public UDataAsset
{
	GENERATED_UCLASS_BODY()

public:
	EPersistenceSelectionMode GetPersistenceSelectionMode() const;
	const TArray<UClass*>& GetClassList() const;

protected:
	UPROPERTY(EditAnywhere, Category = Spatial, meta = (DisplayName = "Persistence Selection Mode", Tooltip = "If 'include' is selected, you must specify all classes you'd like to persist to the snapshot (descendants are included automatically).  If 'exclude' is selected, everything but the specified classes (and their descendants) will be written to the snapshot."))
	EPersistenceSelectionMode PersistenceSelectionMode;

	UPROPERTY(EditAnywhere, Category = Spatial, meta = (DisplayName = "Class List", Tooltip = "List of classes to include or exclude"))
	TArray<UClass*> ClassList;
};

