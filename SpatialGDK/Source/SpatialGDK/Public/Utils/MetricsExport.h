#pragma once

#include "CoreMinimal.h"
#include "HttpModule.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MetricsExport.generated.h"

UCLASS(ClassGroup = (Custom))
class UMetricsExport : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	UMetricsExport();

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	bool Tick(float DeltaSeconds);

	UFUNCTION()
	void SpatialConnected();

	UPROPERTY(EditAnywhere, Category = "Influx Metrics")
	FString InfluxDbName = "mydb";

	UPROPERTY(EditAnywhere, Category = "Influx Metrics")
	FString InfluxHost = "localhost";

	UPROPERTY(EditAnywhere, Category = "Influx Metrics")
	FString InfluxPort = "8086";

	UPROPERTY(EditAnywhere, Category = "Influx Metrics")
	int64 InfluxMetricsSendGapSecs = 2;

	UPROPERTY(EditAnywhere, Category = "Influx Metrics", BlueprintReadWrite)
	bool bPrintInfluxMetricsToLog = false;

	void WriteMetricsToProtocolBuffer(const FString& Worker, FString Field, float Value);

private:
	UFUNCTION(BlueprintCallable, Category = "Influx Metrics")
	void PostLineProtocolToInfluxDBServer(const FString& Lines);

	void FlushProtocolBufferToDatabase();
	void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	// Map of worker name to map of metric name mapped to value
	TMap<FString, TMap<FString, float>> MetricsMap;

	int64 LastUpdateSendTime = 0;
	uint32 FramesSinceLastFpsWrite = 0;
	FString InfluxUrl;
	FString InfluxToken;
	FString ProjectName;
	FString DeploymentName;
	FString WorkerType;
	FString WorkerId;

	FDelegateHandle TickerHandle;
};
