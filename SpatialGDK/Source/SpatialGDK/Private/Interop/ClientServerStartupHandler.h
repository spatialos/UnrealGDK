#pragma once

#include "PacketHandler.h"
#include "Templates/UniquePtr.h"

class USpatialNetDriver;

namespace SpatialGDK
{
	
enum class EStage
{
	Initial,
    WaitForEntityPool,
    WaitForGSM,
    WaitForVirtualWorkerMapping,
    WaitForLocalPartitionAuthority,
    Finished,
};

class FState
{
public:
	virtual ~FState() = default;
	virtual void OnEnter() = 0;
	virtual void OnExit() = 0;
	virtual bool Update(EStage& OutNextStage) = 0;
};
	struct FRuntimeStateBase;

class FStateBase
{public:
	virtual ~FStateBase() = default;
	virtual void OnEnter() {};
	virtual void OnExit() {};
	virtual bool Update(const void*  Input, void* Output) = 0;

	virtual TSharedRef<FRuntimeStateBase> CreateState() = 0;
};
	
template <typename TInput, typename TOutput>
	class TState : public FStateBase, public TSharedFromThis<TState<TInput, TOutput>>
{
public:
	virtual ~TState() = default;
	virtual bool Update(const TInput& Input, TOutput& Output) = 0;
	virtual bool Update(const void* Input, void* Output) override final
	{
		const TInput* TypedInput = static_cast<const TInput*>(Input); 
		TOutput* TypedOutput = static_cast<TOutput*>(Output);
		return Update(*TypedInput, *TypedOutput);
	}

	virtual TSharedRef<FRuntimeStateBase> CreateState() override final;
};


	struct FRuntimeStateBase
	{
		static TFunction<void(FRuntimeStateBase&)> OnStateExecuted;
		explicit FRuntimeStateBase(TSharedRef<FStateBase> InState):State(InState){}
		virtual ~FRuntimeStateBase() = default;
		virtual void Execute(const void* Input) = 0;
		void TryAddState(TSharedRef<FStateBase> Origin, TSharedRef<FStateBase> Destination);
		TSharedRef<FStateBase> State;
		TArray<TSharedRef<FRuntimeStateBase>> LinkedStates;
		bool bWasEntered = false;
	};
	
	template <typename TInput, typename TOutput>
    struct TRuntimeState : public FRuntimeStateBase
	{
		explicit TRuntimeState(TSharedRef<TState<TInput, TOutput>> InState) : FRuntimeStateBase(InState){}
		virtual void Execute(const void* Input) override
		{
			const TInput* TypedInput = static_cast<const TInput*>(Input);
			const bool bDidSucceed = GetState().Update(*TypedInput, Output);
			
			if (bDidSucceed)
			{
				if (OnStateExecuted)
				{
					OnStateExecuted(*this);
				}
			}
			
			// Propagate to linked states.
			for (const TSharedRef<FRuntimeStateBase>& LinkedState : LinkedStates)
			{
				if (bDidSucceed)
				{
					// Issue OnEnter() for newly activated states.
					if (!LinkedState->bWasEntered)
					{
						LinkedState->State->OnEnter();
						LinkedState->bWasEntered = true;
					}

					// Propagate execution to children.
					LinkedState->Execute(&Output);
				}
				else
				{
					// Issue OnExit() for previously active states.
					if (LinkedState->bWasEntered)
					{
						LinkedState->State->OnExit();
						LinkedState->bWasEntered = false;
					}
				}
			}
		}
		TOutput Output;
		TState<TInput, TOutput>& GetState(){return (TState<TInput, TOutput>&)State.Get();};
	};

	

	template <typename TInput, typename TOutput>
    TSharedRef<FRuntimeStateBase> TState<TInput, TOutput>::CreateState()
	{
		
		auto Ptr = MakeShared<TRuntimeState<TInput, TOutput>>(AsShared());
		return Ptr;
	}
	
class FStartupGraphDefinition
{
public:
	template <typename TRootStateOutput>
	static FStartupGraphDefinition Create(TSharedRef<TState<void*, TRootStateOutput>> RootState)
	{
		FStartupGraphDefinition Definition;
		Definition.Root = RootState->CreateState();
		return Definition;
	}
private:
	FStartupGraphDefinition() = default;
public:
	template<typename TState1, typename TState2>
	void AddNext(TSharedRef<TState1> Origin, TSharedRef<TState2> NextState)
	{
		static_assert(TIsDerivedFrom<TState1, FStateBase>::Value, "States must be FStateBase!");
		static_assert(TIsDerivedFrom<TState2, FStateBase>::Value, "States must be FStateBase!");
		Root->TryAddState(Origin, NextState);
	}
	
	bool ExecuteGraph()
	{
		TSet<TSharedPtr<FStateBase>> ExecutedStates;
		FRuntimeStateBase::OnStateExecuted = [&ExecutedStates](FRuntimeStateBase& ExecutedState){ExecutedStates.Emplace(ExecutedState.State);};

		Root->Execute(nullptr);

		FRuntimeStateBase::OnStateExecuted = {};
		const auto ExecutedFinalStates = ExecutedStates.Intersect(FinishStates);
		ensureMsgf(ExecutedFinalStates.Num() <= 1, TEXT("No more than one final state may be executed, instead executed %d"), ExecutedFinalStates.Num());
		return ExecutedStates.Intersect(FinishStates).Num() != 0;
	}

	TSharedPtr<FRuntimeStateBase> Root;
	TSet<TSharedPtr<FStateBase>> FinishStates;
};

	class FClientServerStartupHandlerV2
	{
	public:
	FClientServerStartupHandlerV2(USpatialNetDriver& NetDriver) : Graph(CreateGraph(NetDriver))
	{
	}
	bool TryFinishStartup();
	private:
		static FStartupGraphDefinition CreateGraph(USpatialNetDriver& NetDriver);
		
		FStartupGraphDefinition Graph;
	};
	
class FClientServerStartupHandler
{
public:
	explicit FClientServerStartupHandler(USpatialNetDriver& InNetDriver);
	bool TryFinishStartup();
private:
	USpatialNetDriver* NetDriver;

#define STARTUP_VERSION 2
#if STARTUP_VERSION == 1
	TUniquePtr<FState> CreateState(EStage Stage) const;
	EStage Stage = EStage::Initial;
	TUniquePtr<FState> CurrentState;
#elif STARTUP_VERSION == 2
	FClientServerStartupHandlerV2 StartupHandler;
#endif
};

}
