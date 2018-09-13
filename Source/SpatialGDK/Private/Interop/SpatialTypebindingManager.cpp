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
		FMessageDialog::Debugf(FText::FromString(TEXT("SchemaDatabase not found! No classes will be supported for SpatialOS replication.")));
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

		for (TFieldIterator<UProperty> PropertyIt(Class); PropertyIt; ++PropertyIt)
		{
			UProperty* Property = *PropertyIt;

			if (Property->PropertyFlags & CPF_Handover)
			{
				for (int32 ArrayIdx = 0; ArrayIdx < PropertyIt->ArrayDim; ++ArrayIdx)
				{
					FHandoverPropertyInfo HandoverInfo;
					HandoverInfo.Handle = Info.HandoverProperties.Num() + 1; // 1-based index
					HandoverInfo.Offset = Property->GetOffset_ForGC() + Property->ElementSize * ArrayIdx;
					HandoverInfo.ArrayIdx = ArrayIdx;
					HandoverInfo.Property = Property;

					Info.HandoverProperties.Add(HandoverInfo);
				}
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
						Info.SubobjectClasses.Add(Component->GetClass());
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
								Info.SubobjectClasses.Add(Node->ComponentTemplate->GetClass());
							}
						}
					}
				}

				// Iterate over all subobjects and add any non ActorComponents
				TArray<UObject*> DefaultSubobjects;
				ContainerCDO->GetDefaultSubobjects(DefaultSubobjects);
				for (auto Subobject : DefaultSubobjects)
				{
					if (!Subobject->IsA<UActorComponent>())
					{
						if(IsSupportedClass(Subobject->GetClass()))
						{
							Info.SubobjectClasses.Add(Subobject->GetClass());
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
	return ClassInfoMap.Find(Class);
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
	return SupportedClasses.Contains(Class);
}

TArray<UObject*> USpatialTypebindingManager::GetHandoverSubobjects(AActor* Actor)
{
	FClassInfo* Info = FindClassInfoByClass(Actor->GetClass());
	check(Info);

	TArray<UObject*> DefaultSubobjects;
	Actor->GetDefaultSubobjects(DefaultSubobjects);

	TArray<UObject*> FoundSubobjects;

	for (UClass* SubobjectClass : Info->SubobjectClasses)
	{
		FClassInfo* SubobjectInfo = FindClassInfoByClass(SubobjectClass);
		check(SubobjectInfo);

		if (SubobjectInfo->HandoverProperties.Num() == 0)
		{
			// Not interested in this component if it has no handover properties
			continue;
		}

		UObject** FoundSubobject = DefaultSubobjects.FindByPredicate([SubobjectClass](const UObject* Obj)
		{
			return Obj->IsA(SubobjectClass);
		});
		check(FoundSubobject);
		FoundSubobjects.Add(*FoundSubobject);
	}

	return FoundSubobjects;
}
