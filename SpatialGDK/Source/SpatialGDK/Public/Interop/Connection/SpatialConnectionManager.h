// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Interop/Connection/SpatialOSWorkerInterface.h"
#include "Interop/Connection/ConnectionConfig.h"
#include "SpatialCommonTypes.h"
#include "SpatialGDKSettings.h"

#include "SpatialConnectionManager.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialConnectionManager, Log, All);

class USpatialWorkerConnection;

enum class ESpatialConnectionType
{
	Receptionist,
	LegacyLocator,
	Locator,
	DevAuthFlow
};

UCLASS()
class SPATIALGDK_API USpatialConnectionManager : public UObject
{
	GENERATED_BODY()

public:
	virtual void FinishDestroy() override;
	void DestroyConnection();
	
	using LoginTokenResponseCallback = TFunction<bool(const Worker_Alpha_LoginTokensResponse*)>;
	using LogCallback = TFunction<void(const Worker_LogData*)>;
    
    /// Register a callback using this function.
    /// It will be triggered when receiving login tokens using the development authentication flow inside SpatialWorkerConnection.
    /// @param Callback - callback function.
	void RegisterOnLoginTokensCallback(const LoginTokenResponseCallback& Callback) {LoginTokenResCallback = Callback;}

	void Connect(bool bConnectAsClient, uint32 PlayInEditorID);

	FORCEINLINE bool IsConnected() { return bIsConnected; }

	void SetConnectionType(ESpatialConnectionType InConnectionType);

	// TODO: UNR-2753
	FReceptionistConfig ReceptionistConfig;
	FLocatorConfig LocatorConfig;
	FDevAuthConfig DevAuthConfig;

	DECLARE_DELEGATE(OnConnectionToSpatialOSSucceededDelegate)
	OnConnectionToSpatialOSSucceededDelegate OnConnectedCallback;

	DECLARE_DELEGATE_TwoParams(OnConnectionToSpatialOSFailedDelegate, uint8_t, const FString&);
	OnConnectionToSpatialOSFailedDelegate OnFailedToConnectCallback;

	bool TrySetupConnectionConfigFromCommandLine(const FString& SpatialWorkerType);
	void SetupConnectionConfigFromURL(const FURL& URL, const FString& SpatialWorkerType);

	USpatialWorkerConnection* GetWorkerConnection() { return WorkerConnection; }

	void RequestDeploymentLoginTokens();

	static void OnLogCallback(void* UserData, const Worker_LogData* Message);

private:
	void ConnectToReceptionist(uint32 PlayInEditorID);
	void ConnectToLocator(FLocatorConfig* InLocatorConfig);
	void FinishConnecting(Worker_ConnectionFuture* ConnectionFuture);

	void OnConnectionSuccess();
	void OnConnectionFailure(uint8_t ConnectionStatusCode, const FString& ErrorMessage);

	ESpatialConnectionType GetConnectionType() const;

	void StartDevelopmentAuth(const FString& DevAuthToken);
	static void OnPlayerIdentityToken(void* UserData, const Worker_Alpha_PlayerIdentityTokenResponse* PIToken);
	static void OnLoginTokens(void* UserData, const Worker_Alpha_LoginTokensResponse* LoginTokens);
	void ProcessLoginTokensResponse(const Worker_Alpha_LoginTokensResponse* LoginTokens);

private:
	UPROPERTY()
	USpatialWorkerConnection* WorkerConnection;

	Worker_Locator* WorkerLocator;

	bool bIsConnected;
	bool bConnectAsClient = false;

	ESpatialConnectionType ConnectionType = ESpatialConnectionType::Receptionist;
	LoginTokenResponseCallback LoginTokenResCallback;
	LogCallback SpatialLogCallback;
};
