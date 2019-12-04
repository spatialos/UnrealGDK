// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SpatialCommonTypes.h"
#include "WorkerSDK/improbable/c_worker.h"

#include "ExternalSchemaActor.generated.h"

/*
	Provides a way to add user-defined component types to SpatialOS entities during creation.
	Any Actor class that wants to add schema components to it's entity should implement this interface.

	For more information about how this is used, see Interop/SpatialSender.cpp
*/

UINTERFACE(MinimalAPI)
class UExternalSchemaActor : public UInterface
{
	GENERATED_BODY()
};

class SPATIALGDK_API IExternalSchemaActor
{
	GENERATED_BODY()

public:

	// Gather data for all custom components that should be added to the actor's entity
	virtual TArray<Worker_ComponentData> GetInitialComponentData() const PURE_VIRTUAL(IExternalSchemaActor::GetInitialComponentData, return TArray<Worker_ComponentData>(); );

	// Gather writeAcls for each schema component that has been manually added to this actor's entity
	virtual WriteAclMap GetWriteAclMap() const PURE_VIRTUAL(IExternalSchemaActor::GetWriteAclMap, return WriteAclMap(); );
};
