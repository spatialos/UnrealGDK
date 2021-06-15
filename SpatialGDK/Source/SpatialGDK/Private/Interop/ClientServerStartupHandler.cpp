#include "ClientServerStartupHandler.h"


#include "ActorSubviews.h"
#include "SpatialConstants.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialNetDriverDebugContext.h"
#include "Interop/GlobalStateManager.h"
#include "EngineClasses/SpatialVirtualWorkerTranslator.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "EngineClasses/SpatialVirtualWorkerTranslationManager.h"
#include "EngineClasses/SpatialWorldSettings.h"
#include "Interop/Connection/SpatialWorkerConnection.h"

#include "Templates/Function.h"

namespace SpatialGDK
{
	
	
	// class FReceiveGlobalStateManager : public FState
	// {
	// public:
	// 	FReceiveGlobalStateManager(ViewCoordinator& InCoordinator) : AuthSubview(&InCoordinator.CreateSubView(
 //            SpatialConstants::GDK_KNOWN_ENTITY_TAG_COMPONENT_ID,
 //            [](const Worker_EntityId, const EntityViewElement& Element){ return Element.Authority.Contains(SpatialConstants::GDK_KNOWN_ENTITY_AUTH_COMPONENT_SET_ID); },
 //            {
 //            	InCoordinator.CreateAuthorityChangeRefreshCallback(SpatialConstants::GDK_KNOWN_ENTITY_AUTH_COMPONENT_SET_ID)
	// 		}
	// 		)), NonAuthSubview(&InCoordinator.CreateSubView(
 //                SpatialConstants::GDK_KNOWN_ENTITY_TAG_COMPONENT_ID,
 //                [](const Worker_EntityId, const EntityViewElement& Element){ return !Element.Authority.Contains(SpatialConstants::GDK_KNOWN_ENTITY_AUTH_COMPONENT_SET_ID); },
 //                {
 //                    InCoordinator.CreateAuthorityChangeRefreshCallback(SpatialConstants::GDK_KNOWN_ENTITY_AUTH_COMPONENT_SET_ID)
 //                }
 //            )){}
	// 	virtual void OnEnter() override;
	// 	virtual void OnExit() override;
	// 	virtual bool Update(EStage& OutNextStage) override;
	// private:
	// 	const FSubView* AuthSubview;
	// 	const FSubView* NonAuthSubview;
	// };
	//
	// bool FReceiveGlobalStateManager::Update(EStage& OutNextStage)
	// {
	// 	for (const EntityDelta& Delta : AuthSubview->GetViewDelta())
	// 	{
	// 		if (ensure(Delta.Type == EntityDelta::ADD))
	// 		{
	// 			// This worker has authority over the GSM.
	// 			const Worker_EntityId EntityId = Delta.EntityId;
	// 			
	// 		}
	// 	}
	// 	for (const EntityDelta& Delta : NonAuthSubview->GetViewDelta())
	// 	{
	// 		if (ensure(Delta.Type == EntityDelta::ADD))
	// 		{
	// 			// This worker doesn't have authority over the GSM.
	// 			const Worker_EntityId EntityId = Delta.EntityId;
 //
	// 		}
	// 	}
	// }

	// class FSetGlobalStateManagerValues : public FState
	// {
	// public:
	// 	virtual void OnEnter() override;
	// 	virtual void OnExit() override;
	// 	virtual bool Update(EStage& OutNextStage) override;
	// };

	struct FGlobalStateManagerData
	{
		bool bIsAuth;
		Worker_EntityId GSMEntityId;
	};
	
	class FWaitForGlobalStateManagerStep : public TState<void*, FGlobalStateManagerData>
	{
	public:
		FWaitForGlobalStateManagerStep(ViewCoordinator& InCoordinator) : AuthSubview(&InCoordinator.CreateSubView(
            SpatialConstants::DEPLOYMENT_MAP_COMPONENT_ID,
            [](const Worker_EntityId, const EntityViewElement& Element){ return Element.Authority.Contains(SpatialConstants::GDK_KNOWN_ENTITY_AUTH_COMPONENT_SET_ID); },
            {
                InCoordinator.CreateAuthorityChangeRefreshCallback(SpatialConstants::GDK_KNOWN_ENTITY_AUTH_COMPONENT_SET_ID)
            }
            )), NonAuthSubview(&InCoordinator.CreateSubView(
                SpatialConstants::DEPLOYMENT_MAP_COMPONENT_ID,
                [](const Worker_EntityId, const EntityViewElement& Element){ return !Element.Authority.Contains(SpatialConstants::GDK_KNOWN_ENTITY_AUTH_COMPONENT_SET_ID); },
                {
                    InCoordinator.CreateAuthorityChangeRefreshCallback(SpatialConstants::GDK_KNOWN_ENTITY_AUTH_COMPONENT_SET_ID)
                }
            )){}
		virtual bool Update(void*const& Input, FGlobalStateManagerData& Output) override
		{
			bool bHasFoundEntity = false;
			const auto MarkFoundEntity = [&bHasFoundEntity]
			{
				if (ensure(!bHasFoundEntity))
				{
					bHasFoundEntity = true;
				}
				else
				{
					bHasFoundEntity = false;
				}
			}; 
			for (const EntityDelta& Delta : AuthSubview->GetViewDelta().EntityDeltas)
			{
				if (ensure(Delta.Type == EntityDelta::ADD))
				{
					// This worker has authority over the GSM.
					const Worker_EntityId EntityId = Delta.EntityId;
					MarkFoundEntity();
					Output.bIsAuth = true;
					Output.GSMEntityId = EntityId;
				}
			}
			for (const EntityDelta& Delta : NonAuthSubview->GetViewDelta().EntityDeltas)
			{
				if (ensure(Delta.Type == EntityDelta::ADD))
				{
					// This worker doesn't have authority over the GSM.
					const Worker_EntityId EntityId = Delta.EntityId;
					MarkFoundEntity();
					Output.bIsAuth = false;
					Output.GSMEntityId = EntityId;
				}
			}
			return bHasFoundEntity;
		}
	private:
		const FSubView* AuthSubview;
		const FSubView* NonAuthSubview;
	};

	class FFinishServerStartupStep : public TState<FGlobalStateManagerData, bool>
	{
	public:
		explicit FFinishServerStartupStep(USpatialNetDriver& InNetDriver) : NetDriver(&InNetDriver){}
		
		virtual bool Update(const FGlobalStateManagerData& Input, bool& Output) override
		{
			return true;
		}

		virtual void OnEnter() override
		{
			NetDriver->OnStartupComplete();
		};
	private:
		USpatialNetDriver* NetDriver;
	};

	class FUpdateDeploymentMapDataStep : public TState<FGlobalStateManagerData, FGlobalStateManagerData>
	{
	public:
		explicit FUpdateDeploymentMapDataStep(USpatialNetDriver& InNetDriver) : NetDriver(&InNetDriver){}

		virtual bool Update(const FGlobalStateManagerData& Input, FGlobalStateManagerData& Output) override
		{
			Output = Input;
			return true;
		}

		virtual void OnEnter() override
		{
			NetDriver->GlobalStateManager->SetDeploymentState();
		}

		USpatialNetDriver* NetDriver;
	};

	struct FVirtualWorkerStartupState
	{
		bool bIsAuth;
		Worker_EntityId EntityId;
	};
	
	class FApplyVirtualWorkersInitialState : public TState<FGlobalStateManagerData, FVirtualWorkerStartupState>
	{
	public:
		explicit FApplyVirtualWorkersInitialState(ViewCoordinator& Coordinator, SpatialVirtualWorkerTranslator& InTranslator) : Subview(&Coordinator.CreateSubView(
            SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID,
            [](const Worker_EntityId, const EntityViewElement& Entity){return Entity.Authority.Contains(SpatialConstants::GDK_KNOWN_ENTITY_AUTH_COMPONENT_SET_ID);},
            {Coordinator.CreateAuthorityChangeRefreshCallback(SpatialConstants::GDK_KNOWN_ENTITY_AUTH_COMPONENT_SET_ID)})
            ),NonauthSubview(&Coordinator.CreateSubView(
                SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID,
                [](const Worker_EntityId, const EntityViewElement& Entity){return !Entity.Authority.Contains(SpatialConstants::GDK_KNOWN_ENTITY_AUTH_COMPONENT_SET_ID);},
                {Coordinator.CreateAuthorityChangeRefreshCallback(SpatialConstants::GDK_KNOWN_ENTITY_AUTH_COMPONENT_SET_ID)})
            ), Translator(&InTranslator)
		{}
		virtual bool Update(const FGlobalStateManagerData& Input, FVirtualWorkerStartupState& Output) override
		{
			bool bHasFoundEntity = false;
			const auto MarkFoundEntity = [&bHasFoundEntity]
			{
				if (ensure(!bHasFoundEntity))
				{
					bHasFoundEntity = true;
				}
				else
				{
					bHasFoundEntity = false;
				}
			};
			
			for (const EntityDelta& Delta : Subview->GetViewDelta().EntityDeltas)
			{
				ensure(Delta.Type != EntityDelta::REMOVE);
				// Find the worker translator and apply its initial state.
				if (Delta.Type == EntityDelta::ADD)
				{
					MarkFoundEntity();
					Output.bIsAuth = true;
					Output.EntityId = Delta.EntityId;
				}
			}
			
			for (const EntityDelta& Delta : NonauthSubview->GetViewDelta().EntityDeltas)
			{
				// Find the worker translator and apply its initial state.
				if (Delta.Type == EntityDelta::ADD)
				{
					MarkFoundEntity();
					Output.bIsAuth = false;
					Output.EntityId = Delta.EntityId;
				}
			}

			return bHasFoundEntity;
		}
	private:
		FSubView* Subview;
		FSubView* NonauthSubview;
		SpatialVirtualWorkerTranslator* Translator;
	};

	class FVirtualWorkerTranslatorFork : public TState<FVirtualWorkerStartupState,FVirtualWorkerStartupState>
	{
	public:
		explicit FVirtualWorkerTranslatorFork(const bool bInShouldBeAuth) : bShouldBeAuth(bInShouldBeAuth)
		{}
		virtual bool Update(const FVirtualWorkerStartupState& Input, FVirtualWorkerStartupState& Output) override
		{
			Output = Input;
			return Input.bIsAuth == bShouldBeAuth;
		}
	private:
		bool bShouldBeAuth;
	};
	
	class FOptionallyCreateInitialState : public TState<FVirtualWorkerStartupState, FVirtualWorkerStartupState>
	{
	public:
		FOptionallyCreateInitialState(const SpatialVirtualWorkerTranslator& InTranslator, SpatialVirtualWorkerTranslationManager& InTranslationManager) : Translator(&InTranslator), TranslationManager(&InTranslationManager){}
		virtual bool Update(const FVirtualWorkerStartupState& Input, FVirtualWorkerStartupState& Output) override;
		virtual void OnEnter() override
		{
			if (Translator->GetMappingCount() == 0)
			{
				TranslationManager->SpawnPartitionEntitiesForVirtualWorkerIds();
				return true;
			}
			return false;
		}
	private:
		const SpatialVirtualWorkerTranslator* Translator; 
		SpatialVirtualWorkerTranslationManager* TranslationManager; 
	};
	
	class FRoleSwitch : public TState<FGlobalStateManagerData, FGlobalStateManagerData>
	{
	public:
		explicit FRoleSwitch(bool bInShouldBeAuth) : bShouldBeAuth(bInShouldBeAuth){}
		virtual bool Update(const FGlobalStateManagerData& Input, FGlobalStateManagerData& Output) override
		{
			Output = Input;
			return bShouldBeAuth == Input.bIsAuth;
		}
	private:
		bool bShouldBeAuth;
	};
	
	class FSetAcceptingPlayersStep : public TState<FGlobalStateManagerData, bool>
	{
	public:
		explicit FSetAcceptingPlayersStep(ViewCoordinator& InCoordinator): Coordinator(&InCoordinator){}
		
		virtual bool Update(const FGlobalStateManagerData& GSMData, bool& Output) override
		{
			if (!GSMData.bIsAuth)
			{
				return false;
			}

			ComponentUpdate C(SpatialConstants::DEPLOYMENT_MAP_COMPONENT_ID);
			
			Schema_AddBool(C.GetFields(), SpatialConstants::DEPLOYMENT_MAP_ACCEPTING_PLAYERS_ID, static_cast<uint8_t>(true));

			Coordinator->SendComponentUpdate(GSMData.GSMEntityId, MoveTemp(C), {});

			return true;
		}

		ViewCoordinator* Coordinator;
	};

	void FRuntimeStateBase::TryAddState(TSharedRef<FStateBase> Origin, TSharedRef<FStateBase> Destination)
	{
		if (State == Origin)
		{
			TSharedRef<FRuntimeStateBase> AddedState = Destination->CreateState();
			LinkedStates.Emplace(MoveTemp(AddedState));
		}
		else
		{
			for (auto LinkedState : LinkedStates)
			{
				LinkedState->TryAddState(Origin, Destination);
			}
		}
	}

	bool FClientServerStartupHandlerV2::TryFinishStartup()
	{
		return Graph.ExecuteGraph();
	}
	
	FStartupGraphDefinition FClientServerStartupHandlerV2::CreateGraph(USpatialNetDriver& NetDriver)
	{
		ViewCoordinator& Coordinator = NetDriver.Connection->GetCoordinator();
		auto Root = MakeShared<FWaitForGlobalStateManagerStep>(Coordinator);
		FStartupGraphDefinition Definition = FStartupGraphDefinition::Create<FGlobalStateManagerData>(Root);

		auto GSMAuthSwitch = MakeShared<FRoleSwitch>(/*bShouldBeAuth =*/true);
		Definition.AddNext(Root, GSMAuthSwitch);
		
		auto UpdateDeploymentInfo = MakeShared<FUpdateDeploymentMapDataStep>(NetDriver);
		Definition.AddNext(GSMAuthSwitch, UpdateDeploymentInfo);
		
		auto FinishServerStartupStep = MakeShared<FFinishServerStartupStep>(NetDriver);

		Definition.AddNext(UpdateDeploymentInfo, FinishServerStartupStep);
		

		auto GSMNonAuthSwitch = MakeShared<FRoleSwitch>(/*bShouldBeAuth =*/false);
		Definition.AddNext(Root, GSMNonAuthSwitch);

		Definition.AddNext(GSMNonAuthSwitch, FinishServerStartupStep);
		
		const auto FinalStep = FinishServerStartupStep;
		
		Definition.FinishStates = { FinalStep };
		return Definition;
	}

	TFunction<void(FRuntimeStateBase&)> FRuntimeStateBase::OnStateExecuted;

	class FStartupExecutorV3
	{
	public:
		bool TryFinishStartup()
		{
			// Get virtual worker initial state
			// if auth
			//     if partitions don't exist in the initial state
			//         create partition entities
			//         wait until all CreateEntity requests are received
			//     get all worker entities, corresponding to partitions
			//     wait until all workers mark themselves ready
		}
	
	private:
		
	};
	
	FClientServerStartupHandler::FClientServerStartupHandler(USpatialNetDriver& InNetDriver) : NetDriver(&InNetDriver), StartupHandler(InNetDriver)
	{
#if STARTUP_VERSION == 1
		CurrentState = CreateState(Stage);
		CurrentState->OnEnter();
#endif
	}
	PRAGMA_DISABLE_OPTIMIZATION
    bool FClientServerStartupHandler::TryFinishStartup()
	{
#if STARTUP_VERSION == 1
		check(Stage != EStage::Finished);
		EStage NextStage;
		while (CurrentState->Update(NextStage))
		{
			TUniquePtr<FState> NewState = CreateState(NextStage);
			CurrentState->OnExit();
			NewState->OnEnter();
			CurrentState = MoveTemp(NewState);
			Stage = NextStage;
		}
		return Stage == EStage::Finished;
#elif STARTUP_VERSION == 2
		return StartupHandler.TryFinishStartup();
#endif
	}
	PRAGMA_ENABLE_OPTIMIZATION
    class FFunctionDefinedState : public FState
	{
	public:
		using FEventFunc = TFunction<void(void)>;
		using FUpdateFunc = TFunction<bool(EStage&)>;

		explicit FFunctionDefinedState(FUpdateFunc InOnUpdateFunc, FEventFunc InOnEnterFunc, FEventFunc InOnExitFunc)
            : UpdateFunc(MoveTemp(InOnUpdateFunc)),
            EnterFunc(MoveTemp(InOnEnterFunc)),
            ExitFunc(MoveTemp(InOnExitFunc))
		{
			check(UpdateFunc);
		}
	
		explicit FFunctionDefinedState(FUpdateFunc InOnUpdateFunc)
            : UpdateFunc(MoveTemp(InOnUpdateFunc))
		{
			check(UpdateFunc);
		}
	
		virtual void OnEnter() override
		{
			if (EnterFunc)
			{
				EnterFunc();
			}
		}
	
		virtual void OnExit() override
		{
			if (ExitFunc)
			{
				ExitFunc();
			}
		}
	
		virtual bool Update(EStage& OutNextStage) override
		{
			return UpdateFunc(OutNextStage);
		}
	private:
		FUpdateFunc UpdateFunc;
		FEventFunc EnterFunc;
		FEventFunc ExitFunc;
	};
#if STARTUP_VERSION == 1
	TUniquePtr<FState> FClientServerStartupHandler::CreateState(EStage Stage1) const
	{
		switch (Stage1)
		{
		case EStage::Initial:
			return MakeUnique<FFunctionDefinedState>(
                [](EStage& OutStage){OutStage = EStage::WaitForEntityPool;return true;}
            );
		case EStage::WaitForEntityPool:
			{
				FFunctionDefinedState::FUpdateFunc WaitForEntityPoolUpdate = [NetDriver = NetDriver](EStage& OutStage)
				{
					OutStage = EStage::WaitForGSM;
					return NetDriver->PackageMap->IsEntityPoolReady();
				};
			
				FFunctionDefinedState::FEventFunc WaitForEntityPoolInit = []{};
			
				return MakeUnique<FFunctionDefinedState>(MoveTemp(WaitForEntityPoolUpdate), MoveTemp(WaitForEntityPoolInit), []{});
			}
		case EStage::WaitForGSM:
			{
				FFunctionDefinedState::FUpdateFunc WaitForGSMUpdate = [NetDriver = NetDriver](EStage& OutStage)->bool
				{
					OutStage = EStage::WaitForVirtualWorkerMapping;
					return NetDriver->GlobalStateManager->IsReady();
				};

				return MakeUnique<FFunctionDefinedState>(MoveTemp(WaitForGSMUpdate));
			}
		case EStage::WaitForVirtualWorkerMapping:
			{
				FFunctionDefinedState::FUpdateFunc WaitForVirtualWorkerMapping = [NetDriver = NetDriver](EStage& OutStage)->bool
				{
					OutStage = EStage::WaitForLocalPartitionAuthority;
					return NetDriver->VirtualWorkerTranslator->IsReady();
				};

				return MakeUnique<FFunctionDefinedState>(MoveTemp(WaitForVirtualWorkerMapping));
			}
		case EStage::WaitForLocalPartitionAuthority:
			{
				FFunctionDefinedState::FUpdateFunc WaitForLocalPartitionAuthority = [NetDriver = NetDriver](EStage& OutStage)->bool
				{
					OutStage = EStage::Finished;
					return NetDriver->Connection->GetCoordinator().HasEntity(NetDriver->VirtualWorkerTranslator->GetClaimedPartitionId());
				};

				return MakeUnique<FFunctionDefinedState>(MoveTemp(WaitForLocalPartitionAuthority));
			}
		case EStage::Finished:
			{
				FFunctionDefinedState::FUpdateFunc DummyFinishIterationUpdateFunc = [](EStage&)->bool
				{
					return false;
				};

				FFunctionDefinedState::FEventFunc OnEnter = [NetDriver = NetDriver]()
				{
					NetDriver->bIsReadyToStart = true;
					NetDriver->Connection->SetStartupComplete();

					NetDriver->CreateAndInitializeCoreClassesAfterStartup();

#if WITH_EDITORONLY_DATA
					ASpatialWorldSettings* WorldSettings = Cast<ASpatialWorldSettings>(NetDriver->GetWorld()->GetWorldSettings());
					if (WorldSettings && WorldSettings->bEnableDebugInterface)
					{
						const FFilterPredicate DebugCompFilter = [](const Worker_EntityId EntityId,
                                                                        const SpatialGDK::EntityViewElement& Element) {
							return Element.Components.ContainsByPredicate(
                                SpatialGDK::ComponentIdEquality{ SpatialConstants::GDK_DEBUG_COMPONENT_ID });
						};

						const TArray<FDispatcherRefreshCallback> DebugCompRefresh = {
							NetDriver->Connection->GetCoordinator().CreateComponentExistenceRefreshCallback(SpatialConstants::GDK_DEBUG_COMPONENT_ID)
                        };

						// Create the subview here rather than with the others as we only know if we need it or not at
						// this point.
						const SpatialGDK::FSubView& DebugActorSubView = SpatialGDK::ActorSubviews::CreateCustomActorSubView(
                            SpatialConstants::GDK_DEBUG_TAG_COMPONENT_ID, DebugCompFilter, DebugCompRefresh, *NetDriver);
						USpatialNetDriverDebugContext::EnableDebugSpatialGDK(DebugActorSubView, NetDriver);
					}
#endif
					// We've found and dispatched all ops we need for startup,
					// trigger BeginPlay() on the GSM and process the queued ops.
					// Note that FindAndDispatchStartupOps() will have notified the Dispatcher
					// to skip the startup ops that we've processed already.
					NetDriver->GlobalStateManager->TriggerBeginPlay();	
				};
			
				return MakeUnique<FFunctionDefinedState>(MoveTemp(DummyFinishIterationUpdateFunc), MoveTemp(OnEnter), []{});
			}
		default:
			checkNoEntry();
			return nullptr;
		}
	}

#endif
}
