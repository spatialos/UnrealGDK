// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#include "WorkerConnection.h"

#include "Async.h"
#include "SpatialOS.h"
#include "SpatialGDKCommon.h"
#include "SpatialOSViewTypes.h"
#include "SpatialGDKWorkerTypes.h"

#define LOCTEXT_NAMESPACE "FWorkerConnection"

namespace
{
void LogMessage(const worker::LogMessageOp& Message)
{
  switch (Message.Level)
  {
    case worker::LogLevel::kDebug:
      UE_LOG(LogSpatialOS, Log, TEXT("[debug] %s"), UTF8_TO_TCHAR(Message.Message.c_str()));
      break;
    case worker::LogLevel::kInfo:
      UE_LOG(LogSpatialOS, Log, TEXT("[info] %s"), UTF8_TO_TCHAR(Message.Message.c_str()));
      break;
    case worker::LogLevel::kWarn:
      UE_LOG(LogSpatialOS, Warning, TEXT("%s"), UTF8_TO_TCHAR(Message.Message.c_str()));
      break;
    case worker::LogLevel::kError:
      UE_LOG(LogSpatialOS, Error, TEXT("%s"), UTF8_TO_TCHAR(Message.Message.c_str()));
      break;
    case worker::LogLevel::kFatal:
      UE_LOG(LogSpatialOS, Error, TEXT("[fatal] %s"), UTF8_TO_TCHAR(Message.Message.c_str()));
    default:
      break;
  }
}
const std::string kLoggerName = "WorkerConnection.cpp";
}

namespace improbable
{
namespace unreal
{
namespace core
{
bool FWorkerConnection::IsConnected() const
{
  if (bIsConnecting)
  {
    return false;
  }
  return Connection.IsValid() && Connection->IsConnected();
}

FWorkerConnection::FWorkerConnection()
: bIsConnecting(false), View(nullptr), Connection(nullptr), Metrics(new worker::Metrics{})
{
  UE_LOG(LogSpatialOS, Log, TEXT("Initializing SpatialOS..."));

  View = TSharedPtr<SpatialOSView>(new SpatialOSView(improbable::unreal::Components{}));
  View->OnLogMessage(LogMessage);
  View->OnMetrics(std::bind(&FWorkerConnection::OnMetrics, this, std::placeholders::_1));
}

worker::LocatorParameters
FWorkerConnection::CreateLocatorParameters(const FSpatialOSWorkerConfiguration& WorkerConfiguration)
{
  worker::LocatorParameters LocatorParameters;
  LocatorParameters.CredentialsType = WorkerConfiguration.GetSteamToken().IsEmpty()
      ? worker::LocatorCredentialsType::kLoginToken
      : worker::LocatorCredentialsType::kSteam;
  LocatorParameters.LoginToken =
      worker::LoginTokenCredentials{TCHAR_TO_UTF8(*WorkerConfiguration.GetLoginToken())};
  LocatorParameters.ProjectName = TCHAR_TO_UTF8(*WorkerConfiguration.GetProjectName());
  LocatorParameters.Steam.DeploymentTag = TCHAR_TO_UTF8(*WorkerConfiguration.GetDeploymentTag());
  LocatorParameters.Steam.Ticket = TCHAR_TO_UTF8(*WorkerConfiguration.GetSteamToken());
  return LocatorParameters;
}

void FWorkerConnection::GetDeploymentListAsync(
    const FString& LocatorHost, const worker::LocatorParameters& LocatorParams,
    FOnDeploymentsFoundDelegate OnDeploymentsFoundCallback, std::uint32_t TimeoutMillis)
{
  AsyncTask(ENamedThreads::GameThread,
            [LocatorHost, LocatorParams, TimeoutMillis, OnDeploymentsFoundCallback, this]() {
              auto Locator = SpatialOSLocator{TCHAR_TO_UTF8(*LocatorHost), LocatorParams};
              WaitForDeploymentFuture(TimeoutMillis, Locator.GetDeploymentListAsync(),
                                      OnDeploymentsFoundCallback);
            });
}

void FWorkerConnection::ConnectToReceptionistAsync(const FString& Hostname, std::uint16_t Port,
                                                   const FString& WorkerId,
                                                   const worker::ConnectionParameters& Params,
                                                   FOnConnectedDelegate OnConnectedCallback,
                                                   std::uint32_t TimeoutMillis)
{
  if (!CanCreateNewConnection())
  {
    return;
  }

  UE_LOG(LogSpatialOS, Log, TEXT("Connecting to %s:%i..."), *Hostname, Port);
  bIsConnecting = true;
  AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [TimeoutMillis, Hostname, Port, WorkerId,
                                                           Params, OnConnectedCallback, this]() {
    WaitForConnectionFuture(TimeoutMillis, SpatialOSConnection::ConnectAsync(
                                               improbable::unreal::Components{},
                                               std::string{TCHAR_TO_UTF8(*Hostname)}, Port,
                                               std::string{TCHAR_TO_UTF8(*WorkerId)}, Params),
                            OnConnectedCallback);
  });
}

void FWorkerConnection::ConnectToLocatorAsync(const FString& LocatorHost,
                                              const worker::LocatorParameters& LocatorParams,
                                              const FString& DeploymentId,
                                              const worker::ConnectionParameters& Params,
                                              FQueueStatusDelegate QueueStatusCallback,
                                              FOnConnectedDelegate OnConnectedCallback,
                                              std::uint32_t TimeoutMillis)
{
  if (!CanCreateNewConnection())
  {
    UE_LOG(
        LogSpatialOS, Error,
        TEXT("Can not connect to SpatialOS; a connection already exists. Call Disconnect first."));
    return;
  }

  auto QueueStatusWrapper = [QueueStatusCallback](const worker::QueueStatus& status) {
    TPromise<bool> QueueStatusCallbackReturnValue;
    auto QueueStatusReturnValueFuture = QueueStatusCallbackReturnValue.GetFuture();
    AsyncTask(ENamedThreads::GameThread,
              [QueueStatusCallback, status, &QueueStatusCallbackReturnValue]() {
                QueueStatusCallbackReturnValue.SetValue(QueueStatusCallback.Execute(status));
              });
    return QueueStatusReturnValueFuture.Get();
  };

  bIsConnecting = true;
  AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask,
            [LocatorHost, LocatorParams, TimeoutMillis, DeploymentId, Params, QueueStatusWrapper,
             OnConnectedCallback, this]() {
              auto Locator = SpatialOSLocator{TCHAR_TO_UTF8(*LocatorHost), LocatorParams};
              WaitForConnectionFuture(TimeoutMillis,
                                      Locator.ConnectAsync(improbable::unreal::Components{},
                                                           TCHAR_TO_UTF8(*DeploymentId), Params,
                                                           QueueStatusWrapper),
                                      OnConnectedCallback);
            });
}

void FWorkerConnection::Disconnect()
{
  UE_LOG(LogSpatialOS, Log, TEXT("Disconnecting"));

  if (Connection.IsValid())
  {
    ProcessOps();
    Connection.Reset();
  }
}

void FWorkerConnection::ProcessOps()
{
  if (Connection.IsValid())
  {
    auto Ops = Connection->GetOpList(0);
    View->Process(Ops);
  }
}

void FWorkerConnection::SendLogMessage(ELogVerbosity::Type Level, const FString& Message)
{
  auto WorkerLogLevel = worker::LogLevel::kInfo;

  switch (Level)
  {
    case ELogVerbosity::Fatal:
      WorkerLogLevel = worker::LogLevel::kFatal;
      break;
    case ELogVerbosity::Error:
      WorkerLogLevel = worker::LogLevel::kError;
      break;
    case ELogVerbosity::Warning:
      WorkerLogLevel = worker::LogLevel::kWarn;
      break;
    case ELogVerbosity::Display:
      WorkerLogLevel = worker::LogLevel::kInfo;
      break;
    case ELogVerbosity::Log:
      WorkerLogLevel = worker::LogLevel::kInfo;
      break;
    case ELogVerbosity::Verbose:
      WorkerLogLevel = worker::LogLevel::kDebug;
      break;
    case ELogVerbosity::VeryVerbose:
      WorkerLogLevel = worker::LogLevel::kDebug;
      break;
    default:
      break;
  }

  Connection->SendLogMessage(WorkerLogLevel, kLoggerName, TCHAR_TO_UTF8(*Message));
}

TWeakPtr<SpatialOSView> FWorkerConnection::GetView()
{
  return TWeakPtr<SpatialOSView>(View);
}

const TWeakPtr<SpatialOSView> FWorkerConnection::GetView() const
{
  return TWeakPtr<SpatialOSView>(View);
}

TWeakPtr<SpatialOSConnection> FWorkerConnection::GetConnection()
{
  return TWeakPtr<SpatialOSConnection>(Connection);
}

const TWeakPtr<SpatialOSConnection> FWorkerConnection::GetConnection() const
{
  return TWeakPtr<SpatialOSConnection>(Connection);
}

worker::Metrics& FWorkerConnection::GetMetrics()
{
  return *Metrics;
}

void FWorkerConnection::WaitForDeploymentFuture(
    std::uint32_t TimeoutMillis, SpatialOSFuture<worker::DeploymentList> DeploymentListFuture,
    FOnDeploymentsFoundDelegate OnDeploymentsFoundCallback)
{
  if (DeploymentListFuture.Wait(TimeoutMillis))
  {
    auto DeploymentList = DeploymentListFuture.Get();
    AsyncTask(ENamedThreads::GameThread, [DeploymentList, OnDeploymentsFoundCallback]() {
      OnDeploymentsFoundCallback.Execute(DeploymentList);
    });
  }
  else
  {
    AsyncTask(ENamedThreads::GameThread, [OnDeploymentsFoundCallback]() {
      OnDeploymentsFoundCallback.Execute(
          worker::DeploymentList{worker::List<worker::Deployment>{},
                                 std::string{"Timed out waiting for deployment list."}});
    });
  }
}

void FWorkerConnection::WaitForConnectionFuture(
    std::uint32_t TimeoutMillis, SpatialOSFuture<SpatialOSConnection> ConnectionFuture,
    FOnConnectedDelegate OnConnectedCallback)
{
  if (ConnectionFuture.Wait(TimeoutMillis))
  {
    auto WorkerConnection =
        TSharedPtr<SpatialOSConnection>(new SpatialOSConnection{ConnectionFuture.Get()});
    if (WorkerConnection->IsConnected())
    {
      AsyncTask(ENamedThreads::GameThread, [OnConnectedCallback, WorkerConnection, this]() {
        Connection = WorkerConnection;
        OnConnectedCallback.Execute(true);
      });
      UE_LOG(LogSpatialOS, Log, TEXT("Connected."));
    }
    else
    {
      AsyncTask(ENamedThreads::GameThread,
                [OnConnectedCallback]() { OnConnectedCallback.Execute(false); });
      UE_LOG(LogSpatialOS, Error, TEXT("Connection failed."));
    }
  }
  else
  {
    AsyncTask(ENamedThreads::GameThread,
              [OnConnectedCallback]() { OnConnectedCallback.Execute(false); });
    UE_LOG(LogSpatialOS, Error, TEXT("Connection timed out."));
  }

  bIsConnecting = false;
}

void FWorkerConnection::OnMetrics(const worker::MetricsOp& Op)
{
  Metrics->Merge(Op.Metrics);
}

bool FWorkerConnection::CanCreateNewConnection() const
{
  return !(IsConnected() || bIsConnecting);
}

}  // ::core
}  // ::unreal
}  // ::improbable
#undef LOCTEXT_NAMESPACE
