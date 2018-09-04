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
	FindSupportedClasses();
	CreateTypebindings();
}

void USpatialTypebindingManager::FindSupportedClasses()
{
	// Load the asset registry module
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(FName("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	// Before running, ensure all blueprint classes that have been tagged with 'spatial' are loaded
	TArray<FAssetData> AssetData;
	uint32 SpatialClassFlags = 0;
	AssetRegistry.GetAssetsByClass(UBlueprint::StaticClass()->GetFName(), AssetData, true);
	for (FAssetData& It : AssetData)
	{
		if (It.GetTagValue("SpatialClassFlags", SpatialClassFlags))
		{
			if (SpatialClassFlags & SPATIALCLASS_GenerateTypeBindings)
			{
				FString ObjectPath = It.ObjectPath.ToString() + TEXT("_C");
				UClass* LoadedClass = LoadObject<UClass>(nullptr, *ObjectPath, nullptr, LOAD_EditorOnly, nullptr);
				UE_LOG(LogTemp, Log, TEXT("Found spatial blueprint class `%s`."), *ObjectPath);
			}
		}
	}

	for (TObjectIterator<UClass> It; It; ++It)
	{
		if (It->HasAnySpatialClassFlags(SPATIALCLASS_GenerateTypeBindings) == false)
		{
			continue;
		}

		// Ensure we don't process skeleton or reinitialized classes
		if (It->GetName().StartsWith(TEXT("SKEL_"), ESearchCase::CaseSensitive)
			|| It->GetName().StartsWith(TEXT("REINST_"), ESearchCase::CaseSensitive))
		{
			continue;
		}

		SupportedClasses.Add(*It);
	}
}

void USpatialTypebindingManager::CreateTypebindings()
{
	int ComponentId = 100010;
	for (UClass* Class : SupportedClasses)
	{
		auto AddComponentId = [Class, &ComponentId, this]()
		{
			ComponentToClassMap.Add(ComponentId, Class);
			return ComponentId++;
		};

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

		Info.SingleClientComponent = AddComponentId();
		Info.MultiClientComponent = AddComponentId();
		Info.HandoverComponent = AddComponentId();
		for (int RPCType = RPC_Client; RPCType < RPC_Count; RPCType++)
		{
			Info.RPCComponents[RPCType] = AddComponentId();
		}

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
