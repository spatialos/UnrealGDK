// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#pragma once

#include "TestSchema.h"

#include "improbable/vector3.h"

#include "CoreMinimal.h"
#include "SpatialOSWorkerTypes.h"
#include "UObject/NoExportTypes.h"
#include "TestCommandVector3fCommandResponder.generated.h"

/**
 *
 */
UCLASS(BlueprintType)
class SPATIALOS_API UTestCommandVector3fCommandResponder : public UObject
{
    GENERATED_BODY()

  public:
    UTestCommandVector3fCommandResponder();
	UTestCommandVector3fCommandResponder* Init(
        const TWeakPtr<SpatialOSConnection>& InConnection,
        worker::RequestId<
            worker::IncomingCommandRequest<test::TestCommandResponseTypes::Commands::TestCommandVector3f>
		> InRequestId,
        FVector InRequest, 
		const std::string& InCallerWorkerId
	);

    UFUNCTION(BlueprintPure, Category = "TestCommandResponseTypesComponent")
    FVector GetRequest();

	UFUNCTION(BlueprintPure, Category = "TestCommandResponseTypesComponent")
    FString GetCallerWorkerId();

    UFUNCTION(BlueprintCallable, Category = "TestCommandResponseTypesComponent")
    void SendResponse(FVector response);

  private:

    TWeakPtr<SpatialOSConnection> Connection;

    worker::RequestId<worker::IncomingCommandRequest<test::TestCommandResponseTypes::Commands::TestCommandVector3f>>
        RequestId;

	UPROPERTY()
    FVector Request;

	FString CallerWorkerId;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTestCommandVector3fCommand, UTestCommandVector3fCommandResponder*, responder);
