#pragma once

#include "Containers/Array.h"
#include "Templates/Function.h"
#include "Templates/UniqueObj.h"
#include "Templates/UniquePtr.h"

#include "WorkerSDK/improbable/c_schema.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialStartupHandler, Log, All);

namespace SpatialGDK
{
struct FDeploymentMapData
{
	FString DeploymentMapURL;

	bool bAcceptingPlayers;

	int32 DeploymentSessionId;

	uint32 SchemaHash;

	static bool TryRead(const Schema_Object& ComponentObject, FDeploymentMapData& InOutData);
};

struct FStartupStep
{
	virtual ~FStartupStep() = default;

	virtual void Start(){};
	virtual bool TryFinish() { return true; }
	virtual FString Describe() const { return StepName; }

	FString StepName;
};

struct FStartupExecutor
{
	explicit FStartupExecutor(TArray<TUniquePtr<FStartupStep>> InSteps)
	{
		class FBlankStartupStep : public FStartupStep
		{
		};

		// This lets us only execute actual startup logic
		// during TryFinish() calls and not when creating
		// the executor.
		Steps.Emplace(MakeUnique<FBlankStartupStep>());

		Steps.Append(MoveTemp(InSteps));
	}

	bool TryFinish()
	{
		while (Steps.Num() > 0)
		{
			if (!Steps[0]->TryFinish())
			{
				return false;
			}

			Steps.RemoveAt(0);

			if (Steps.Num() > 0)
			{
				Steps[0]->Start();
			}
		}
		return true;
	}

	FString Describe() const
	{
		if (Steps.Num() > 0)
		{
			return Steps[0]->Describe();
		}
		return TEXT("Startup finished");
	}

	TArray<TUniquePtr<FStartupStep>> Steps;
};
} // namespace SpatialGDK