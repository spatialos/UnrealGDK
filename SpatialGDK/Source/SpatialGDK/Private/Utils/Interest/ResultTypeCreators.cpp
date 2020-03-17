// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/Interest/ResultTypeCreators.h"

#include "UObject/UObjectIterator.h"

#include "SpatialGDKSettings.h"

namespace
{

SpatialGDK::ResultType ResultTypeCreators::CreateClientNonAuthInterestResultType(USpatialClassInfoManager* ClassInfoManager)
{
	SpatialGDK::ResultType ClientNonAuthResultType;

	// Add the required unreal components
	ClientNonAuthResultType.Append(SpatialConstants::REQUIRED_COMPONENTS_FOR_NON_AUTH_CLIENT_INTEREST);

	// Add all data components- clients shouldn't need to see handover or owner only components on other entities.
	ClientNonAuthResultType.Append(ClassInfoManager->GetComponentIdsForComponentType(ESchemaComponentType::SCHEMA_Data));

	// TODO(jacques): The above comment is correct in that they shouldn't need to, but right now they still do.
	// This is because GDK workers currently make assumptions about information being available at the point of possession.
	// (ticket to fix: unr-2865 - would give a small bandwidth improvement for clients)
	ClientNonAuthResultType.Append(ClassInfoManager->GetComponentIdsForComponentType(ESchemaComponentType::SCHEMA_OwnerOnly));

	return ClientNonAuthResultType;
}

SpatialGDK::ResultType ResultTypeCreators::CreateClientAuthInterestResultType(USpatialClassInfoManager* ClassInfoManager)
{
	SpatialGDK::ResultType ClientAuthResultType;

	// Add the required known components
	ClientAuthResultType.Append(SpatialConstants::REQUIRED_COMPONENTS_FOR_AUTH_CLIENT_INTEREST);
	ClientAuthResultType.Append(SpatialConstants::REQUIRED_COMPONENTS_FOR_NON_AUTH_CLIENT_INTEREST);

	// Add all the generated unreal components
	ClientAuthResultType.Append(ClassInfoManager->GetComponentIdsForComponentType(ESchemaComponentType::SCHEMA_Data));
	ClientAuthResultType.Append(ClassInfoManager->GetComponentIdsForComponentType(ESchemaComponentType::SCHEMA_OwnerOnly));

	return ClientAuthResultType;
}

SpatialGDK::ResultType ResultTypeCreators::CreateServerNonAuthInterestResultType(USpatialClassInfoManager* ClassInfoManager)
{
	SpatialGDK::ResultType ServerNonAuthResultType;

	// Add the required unreal components
	ServerNonAuthResultType.Append(SpatialConstants::REQUIRED_COMPONENTS_FOR_NON_AUTH_SERVER_INTEREST);

	// Add all data, owner only, and handover components
	ServerNonAuthResultType.Append(ClassInfoManager->GetComponentIdsForComponentType(ESchemaComponentType::SCHEMA_Data));
	ServerNonAuthResultType.Append(ClassInfoManager->GetComponentIdsForComponentType(ESchemaComponentType::SCHEMA_OwnerOnly));
	ServerNonAuthResultType.Append(ClassInfoManager->GetComponentIdsForComponentType(ESchemaComponentType::SCHEMA_Handover));

	return ServerNonAuthResultType;
}

SpatialGDK::ResultType ResultTypeCreators::CreateServerAuthInterestResultType()
{
	// Just the components that we won't have already checked out through authority
	return SpatialConstants::REQUIRED_COMPONENTS_FOR_AUTH_SERVER_INTEREST;
}

}
