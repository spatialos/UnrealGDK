// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Commandlets/CookCommandlet.h"
#include "CookAndGenerateSchemaCommandlet.generated.h"

/**
 * 
 */
UCLASS()
class SPATIALGDKEDITORCOMMANDLET_API UCookAndGenerateSchemaCommandlet : public UCookCommandlet
{
	GENERATED_BODY()
private:
	TSet<FString> ReferencedClasses;

public:
	UCookAndGenerateSchemaCommandlet();

	virtual int32 Main(const FString& CmdLineParams) override;

	void OnObjectsReplaced(const TMap<UObject*, UObject*>& ObjectsReplaced);

	virtual bool IsEditorOnly() const override;
};
