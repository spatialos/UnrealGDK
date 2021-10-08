#pragma once
#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "HttpModule.h"
#include "MetricsExport.generated.h"
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UMetricsExport : public UActorComponent
{
	GENERATED_UCLASS_BODY()
public:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void PushMetric(const FString& Field, float Value, const TMap<FString, FString>& Tags);
	UPROPERTY(EditAnywhere, config, Category = "Influx Metrics")
	FString InfluxDbName = "mydb";
	UPROPERTY(EditAnywhere, config, Category = "Influx Metrics")
	FString InfluxHost = "localhost";
	UPROPERTY(EditAnywhere, config, Category = "Influx Metrics")
	FString InfluxPort = "8086";
	UPROPERTY(EditAnywhere, config, Category = "Influx Metrics")
	int64 InfluxMetricsSendGapSecs = 2;
	UPROPERTY(EditAnywhere, config, Category = "Influx Metrics", BlueprintReadWrite)
	bool bPrintInfluxMetricsToLog = false;
private:
	struct MetricData
	{
		FString MetricName;
		float MetricValue;
		TMap<FString, FString> Tags;
	};
	UFUNCTION(BlueprintCallable, Category = "Influx Metrics")
	void ExportMetricOverHttp(const FString& Lines);
	void FlushMetrics();
	void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	// Map of worker name to map of metric name mapped to value
	TArray<MetricData> MetricsToFlush;
	int64 LastUpdateSendTime = 0;
	uint32 FramesSinceLastFpsWrite = 0;
	FString InfluxUrl;
	FString InfluxToken;
	FString ProjectName;
	FString DeploymentName;
	FString WorkerType;
	FString WorkerId;
	FString MetricsPrefix;
};
