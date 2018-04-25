// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#pragma once

#include "TestSchema.h"

#include "improbable/standard_library.h"
#include "SpatialOSConversionFunctionLibrary.h"

#include "CoreMinimal.h"
#include "SpatialOSWorkerTypes.h"
#include "UObject/NoExportTypes.h"
#include "TestCommandCoordinatesCommandResponder.generated.h"

/**
 *
 */
UCLASS(BlueprintType)
class SPATIALOS_API UTestCommandCoordinatesCommandResponder : public UObject
{
    GENERATED_BODY()

  public:
    UTestCommandCoordinatesCommandResponder();
	UTestCommandCoordinatesCommandResponder* Init(
        const TWeakPtr<SpatialOSConnection>& InConnection,
        worker::RequestId<
            worker::IncomingCommandRequest<test::TestCommandResponseTypes::Commands::TestCommandCoordinates>
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

    worker::RequestId<worker::IncomingCommandRequest<test::TestCommandResponseTypes::Commands::TestCommandCoordinates>>
        RequestId;

	UPROPERTY()
    FVector Request;

	FString CallerWorkerId;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTestCommandCoordinatesCommand, UTestCommandCoordinatesCommandResponder*, responder);
