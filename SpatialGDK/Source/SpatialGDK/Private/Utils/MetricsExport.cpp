#include "Utils/MetricsExport.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Runtime/Online/HTTP/Public/Http.h"

UMetricsExport::UMetricsExport(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	LastUpdateSendTime = FDateTime::Now().ToUnixTimestamp();
	FString InfluxUrlOverride;
	if (FParse::Value(FCommandLine::Get(), TEXT("influxUrl"), InfluxUrlOverride))
	{
		InfluxUrl = InfluxUrlOverride;
	}
	else
	{
		InfluxUrl = FString::Printf(TEXT("http://%s:%s/write?db=%s"), *InfluxHost, *InfluxPort, *InfluxDbName);
	}
	FString InfluxTokenFromCli;
	if (FParse::Value(FCommandLine::Get(), TEXT("influxToken"), InfluxTokenFromCli))
	{
		InfluxToken = InfluxTokenFromCli;
	}
	FString ProjectNameFromCli;
	if (FParse::Value(FCommandLine::Get(), TEXT("appName"), ProjectNameFromCli))
	{
		ProjectName = ProjectNameFromCli;
	}
	else
	{
		ProjectName = TEXT("local_project");
	}
	FString DeploymentNameFromCli;
	if (FParse::Value(FCommandLine::Get(), TEXT("deploymentName"), DeploymentNameFromCli))
	{
		DeploymentName = DeploymentNameFromCli;
	}
	else
	{
		DeploymentName = TEXT("local_dpl");
	}
}

void UMetricsExport::BeginPlay()
{
	Super::BeginPlay();

	USpatialNetDriver* NetDriver = Cast<USpatialNetDriver>(GetWorld()->GetNetDriver());
	if (NetDriver == nullptr)
	{
		return;
	}

	WorkerType = NetDriver->IsServer() ? TEXT("Server") : TEXT("Client");
	WorkerId = NetDriver->Connection->GetWorkerId();
}

void UMetricsExport::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	FramesSinceLastFpsWrite++;
	const int64 Now = FDateTime::Now().ToUnixTimestamp();
	const int64 TimeSinceLastUpdate = Now - LastUpdateSendTime;
	if (TimeSinceLastUpdate > InfluxMetricsSendGapSecs)
	{
		const uint32 Fps = FramesSinceLastFpsWrite / (Now - LastUpdateSendTime);
		WriteMetricsToProtocolBuffer(WorkerId, TEXT("fps"), Fps);
		FlushProtocolBufferToDatabase();
		LastUpdateSendTime = Now;
		FramesSinceLastFpsWrite = 0;
	}
}

void UMetricsExport::WriteMetricsToProtocolBuffer(const FString& Worker, FString Field, float Value)
{
	MetricsMap.FindOrAdd(Worker).FindOrAdd(Field) = Value;
}

void UMetricsExport::FlushProtocolBufferToDatabase()
{
	for (const auto& WorkerMetrics : MetricsMap)
	{
		FString Lines = "";

		const FString& WorkerIdentifier = WorkerMetrics.Key;
		Lines.Append(FString::Printf(TEXT("worker_data,worker=%s,project=%s,deployment=%s,type=%s "), *WorkerIdentifier, *ProjectName,
									 *DeploymentName, *WorkerType));
		const TMap<FString, float>& Metrics = WorkerMetrics.Value;
		for (const auto& MetricKeyValue : Metrics)
		{
			Lines.Append(FString::Printf(TEXT("%s=%f,"), *MetricKeyValue.Key, MetricKeyValue.Value));
		}
		Lines.RemoveFromEnd(TEXT(","));
		PostLineProtocolToInfluxDBServer(Lines);
	}
	MetricsMap.Reset();
}

void UMetricsExport::PostLineProtocolToInfluxDBServer(const FString& Lines)
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
	if (bPrintInfluxMetricsToLog)
	{
		UE_LOG(LogTemp, Log, TEXT("Posting %s to %s"), *Lines, *InfluxUrl);
		HttpRequest->OnProcessRequestComplete().BindUObject(this, &UMetricsExport::OnResponseReceived);
	}
	HttpRequest->SetURL(InfluxUrl);
	HttpRequest->SetVerb(TEXT("POST"));
	HttpRequest->SetHeader(TEXT("Authorization"), *FString::Printf(TEXT("Token %s"), *InfluxToken));
	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/x-www-form-urlencoded"));
	HttpRequest->SetContentAsString(Lines);
	HttpRequest->ProcessRequest();
}

void UMetricsExport::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!bWasSuccessful)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed sending metrics. Code: %d. Message: %s"), Response->GetResponseCode(),
			   *Response->GetContentAsString());
	}
	else
	{
		UE_LOG(LogTemp, Verbose, TEXT("Metrics sent successfully! Code: %d. Message: %s"), Response->GetResponseCode(),
			   *Response->GetContentAsString());
	}
}
