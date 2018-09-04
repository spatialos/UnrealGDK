//// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
//
//#include "SpatialOutputDevice.h"
//
//FSpatialOutputDevice::FSpatialOutputDevice(USpatialOS* SpatialOSInstance, FString LoggerName)
//{
//	SpatialOS = SpatialOSInstance;
//	Name = LoggerName;
//	FilterLevel = ELogVerbosity::Warning;
//
//	FOutputDeviceRedirector::Get()->AddOutputDevice(this);
//}
//
//FSpatialOutputDevice::~FSpatialOutputDevice()
//{
//	FOutputDeviceRedirector::Get()->RemoveOutputDevice(this);
//}
//
//void FSpatialOutputDevice::Serialize(const TCHAR* InData, ELogVerbosity::Type Verbosity, const class FName& Category)
//{
//	if (Verbosity > FilterLevel || /*!CategoriesToRedirect.Contains(Category) ||*/ !IsValid(SpatialOS))
//	{
//		return;
//	}
//
//	TSharedPtr<worker::Connection> Connection = SpatialOS->GetConnection().Pin();
//	if (Connection.IsValid())
//	{
//		Connection->SendLogMessage(ConvertLogLevelToSpatial(Verbosity), TCHAR_TO_UTF8(*Name), TCHAR_TO_UTF8(InData));
//	}
//}
//
//void FSpatialOutputDevice::AddRedirectCategory(const FName& Category)
//{
//	CategoriesToRedirect.Add(Category);
//}
//
//void FSpatialOutputDevice::RemoveRedirectCategory(const FName& Category)
//{
//	CategoriesToRedirect.Remove(Category);
//}
//
//void FSpatialOutputDevice::SetVerbosityFilterLevel(ELogVerbosity::Type Verbosity)
//{
//	FilterLevel = Verbosity;
//}
//
//worker::LogLevel FSpatialOutputDevice::ConvertLogLevelToSpatial(ELogVerbosity::Type Verbosity)
//{
//	switch (Verbosity)
//	{
//	case ELogVerbosity::Fatal:
//		return worker::LogLevel::kFatal;
//	case ELogVerbosity::Error:
//		return worker::LogLevel::kError;
//	case ELogVerbosity::Warning:
//		return worker::LogLevel::kWarn;
//	default:
//		return worker::LogLevel::kInfo;
//	}
//}
