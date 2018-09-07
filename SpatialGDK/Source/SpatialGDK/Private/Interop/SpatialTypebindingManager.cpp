// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTypebindingManager.h"

#include "AssetRegistryModule.h"
#include "Class.h"
#include "Core.h"
#include "Engine/Blueprint.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/SCS_Node.h"
#include "GameFramework/Actor.h"
#include "UObjectIterator.h"

void USpatialTypebindingManager::Init()
{
	TSoftObjectPtr<USchemaDatabase> SchemaDatabasePtr(FSoftObjectPath(TEXT("/Game/Spatial/SchemaDatabase.SchemaDatabase")));
	SchemaDatabasePtr.LoadSynchronous();
	SchemaDatabase = SchemaDatabasePtr.Get();

	if (SchemaDatabase == nullptr)
	{
		FMessageDialog::Debugf(FText::FromString(TEXT("SchemaDatabase not found! No classes will be supported for SpatialOS replication")));
		return;
	}

	FindSupportedClasses();
	CreateTypebindings();
}

void USpatialTypebindingManager::FindSupportedClasses()
{
	SchemaDatabase->ClassToSchema.GetKeys(SupportedClasses);
}

void USpatialTypebindingManager::CreateTypebindings()
{
	for (UClass* Class : SupportedClasses)
	{
		FClassInfo Info;

		for (TFieldIterator<UFunction> RemoteFunction(Class); RemoteFunction; ++RemoteFunction)
		{
			if (RemoteFunction->FunctionFlags & FUNC_NetClient ||
				RemoteFunction->FunctionFlags & FUNC_NetServer ||
				RemoteFunction->FunctionFlags & FUNC_NetCrossServer ||
				RemoteFunction->FunctionFlags & FUNC_NetMulticast)
			{
				ERPCType RPCType;
				if (RemoteFunction->FunctionFlags & FUNC_NetClient)
				{
					RPCType = RPC_Client;
				}
				else if (RemoteFunction->FunctionFlags & FUNC_NetServer)
				{
					RPCType = RPC_Server;
				}
				else if (RemoteFunction->FunctionFlags & FUNC_NetCrossServer)
				{
					RPCType = RPC_CrossServer;
				}
				else if (RemoteFunction->FunctionFlags & FUNC_NetMulticast)
				{
					RPCType = RPC_NetMulticast;
				}
				else
				{
					RPCType = RPC_Count;
					checkNoEntry();
				}

				TArray<UFunction*>& RPCArray = Info.RPCs.FindOrAdd(RPCType);

				FRPCInfo RPCInfo;
				RPCInfo.Type = RPCType;
				RPCInfo.Index = RPCArray.Num();

				RPCArray.Add(*RemoteFunction);
				Info.RPCInfoMap.Add(*RemoteFunction, RPCInfo);
			}
		}

		// TODO: Probably clean up duplication?
		Info.SingleClientComponent = SchemaDatabase->ClassToSchema[Class].SingleClientRepData;
		ComponentToClassMap.Add(Info.SingleClientComponent, Class);

		Info.MultiClientComponent = SchemaDatabase->ClassToSchema[Class].MultiClientRepData;
		ComponentToClassMap.Add(Info.MultiClientComponent, Class);

		Info.HandoverComponent = SchemaDatabase->ClassToSchema[Class].HandoverData;
		ComponentToClassMap.Add(Info.HandoverComponent, Class);

		Info.RPCComponents[RPC_Client] = SchemaDatabase->ClassToSchema[Class].ClientRPCs;
		ComponentToClassMap.Add(Info.RPCComponents[RPC_Client], Class);

		Info.RPCComponents[RPC_Server] = SchemaDatabase->ClassToSchema[Class].ServerRPCs;
		ComponentToClassMap.Add(Info.RPCComponents[RPC_Server], Class);

		Info.RPCComponents[RPC_NetMulticast] = SchemaDatabase->ClassToSchema[Class].NetMulticastRPCs;
		ComponentToClassMap.Add(Info.RPCComponents[RPC_NetMulticast], Class);

		Info.RPCComponents[RPC_CrossServer] = SchemaDatabase->ClassToSchema[Class].CrossServerRPCs;
		ComponentToClassMap.Add(Info.RPCComponents[RPC_CrossServer], Class);

		if (Class->IsChildOf<AActor>())
		{
			if (AActor* ContainerCDO = Cast<AActor>(Class->GetDefaultObject()))
			{
				TInlineComponentArray<UActorComponent*> NativeComponents;
				ContainerCDO->GetComponents(NativeComponents);

				for (UActorComponent* Component : NativeComponents)
				{
					if (IsSupportedClass(Component->GetClass()))
					{
						Info.ComponentClasses.Add(Component->GetClass());
					}
				}

				// Components that are added in a blueprint won't appear in the CDO.
				if (UBlueprintGeneratedClass* BGC = Cast<UBlueprintGeneratedClass>(Class))
				{
					if (USimpleConstructionScript* SCS = BGC->SimpleConstructionScript)
					{
						for (USCS_Node* Node : SCS->GetAllNodes())
						{
							if (Node->ComponentTemplate == nullptr)
							{
								continue;
							}

							if (IsSupportedClass(Node->ComponentTemplate->GetClass()))
							{
								Info.ComponentClasses.Add(Node->ComponentTemplate->GetClass());
							}
						}
					}
				}
			}
		}

		ClassInfoMap.Add(Class, Info);
	}
}

FClassInfo* USpatialTypebindingManager::FindClassInfoByClass(UClass* Class)
{
	for (UClass* CurrentClass = Class; CurrentClass; CurrentClass = CurrentClass->GetSuperClass())
	{
		FClassInfo* Info = ClassInfoMap.Find(CurrentClass);
		if (Info)
		{
			return Info;
		}
	}
	return nullptr;
}

FClassInfo* USpatialTypebindingManager::FindClassInfoByComponentId(Worker_ComponentId ComponentId)
{
	UClass* Class = FindClassByComponentId(ComponentId);
	return Class != nullptr ? FindClassInfoByClass(Class) : nullptr;
}

UClass* USpatialTypebindingManager::FindClassByComponentId(Worker_ComponentId ComponentId)
{
	UClass** Class = ComponentToClassMap.Find(ComponentId);
	return Class != nullptr ? *Class : nullptr;
}

bool USpatialTypebindingManager::IsSupportedClass(UClass* Class)
{
	for (UClass* SupportedClass : SupportedClasses)
	{
		if (Class->IsChildOf(SupportedClass))
		{
			return true;
		}
	}

	return false;
}
