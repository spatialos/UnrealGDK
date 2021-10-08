#include "Utils/MetricsExport.h"
#include <chrono>
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
    MetricsPrefix = FString::Printf(TEXT("worker_data,worker=%s,project=%s,deployment=%s,type=%s"), *WorkerId, *ProjectName,
                                    *DeploymentName, *WorkerType);
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
        PushMetric(TEXT("fps"), Fps, {});
        FlushMetrics();
        LastUpdateSendTime = Now;
        FramesSinceLastFpsWrite = 0;
    }
}
void UMetricsExport::PushMetric(const FString& Field, float Value, const TMap<FString, FString>& Tags)
{
    MetricsToFlush.Emplace(MetricData{ Field, Value, Tags });
}
void UMetricsExport::FlushMetrics()
{
    const auto EpochDiff = std::chrono::system_clock::now().time_since_epoch();
    FString Timestamp = FString::Printf(TEXT("%lld"), std::chrono::duration_cast<std::chrono::nanoseconds>(EpochDiff).count());
    for (const auto& Metric : MetricsToFlush)
    {
        FString Lines = MetricsPrefix;
        for (const auto& Tag : Metric.Tags)
        {
            Lines.Append(FString::Printf(TEXT(",%s=%s"), *Tag.Key, *Tag.Value));
        }
        Lines.Append(TEXT(" "));
        Lines.Append(FString::Printf(TEXT("%s=%f"), *Metric.MetricName, Metric.MetricValue));
        Lines.Append(TEXT(" "));
        Lines.Append(Timestamp);
        ExportMetricOverHttp(Lines);
    }
    MetricsToFlush.Reset();
}
void UMetricsExport::ExportMetricOverHttp(const FString& Lines)
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
