#pragma once

#include "Containers/Array.h"
#include "Templates/Function.h"

namespace SpatialGDK
{
using CallbackId = int32;

/**
 * Container holding a set of callbacks.
 * Callbacks are called in the order in which they were registered.
 * Callbacks added or removed during callback invocation will be reconciled once all callbacks have been invoked.
 * Nested calls to Invoke are not allowed.
 */
template <typename T>
class TCallbacks
{
public:
	using CallbackType = TFunction<void(const T&)>;

	bool IsEmpty() const { return Callbacks.Num() == 0; }

	void Register(CallbackId CallbackId, CallbackType Callback)
	{
		if (bCurrentlyInvokingCallbacks)
		{
			CallbacksToAdd.Push({ MoveTemp(Callback), CallbackId });
		}
		else
		{
			Callbacks.Push({ MoveTemp(Callback), CallbackId });
		}
	}

	void Remove(CallbackId Id)
	{
		if (bCurrentlyInvokingCallbacks)
		{
			CallbacksToRemove.Emplace(Id);
		}
		else
		{
			CallbackAndId* Element = Callbacks.FindByPredicate([Id](const CallbackAndId& E) {
				return E.Id == Id;
			});
			if (Element != nullptr)
			{
				Callbacks.RemoveAt(Element - Callbacks.GetData());
			}
		}
	}

	void Invoke(const T& Value)
	{
		check(!bCurrentlyInvokingCallbacks);

		bCurrentlyInvokingCallbacks = true;
		for (const CallbackAndId& Callback : Callbacks)
		{
			Callback.Callback(Value);
		}
		bCurrentlyInvokingCallbacks = false;

		// Sort out pending adds and removes.
		if (CallbacksToAdd.Num() > 0)
		{
			Callbacks.Append(MoveTemp(CallbacksToAdd));
			CallbacksToAdd.Empty();
		}
		if (CallbacksToRemove.Num() > 0)
		{
			Callbacks.RemoveAll([this](const CallbackAndId& E) {
				return CallbacksToRemove.Contains(E.Id);
			});
			CallbacksToRemove.Empty();
		}
	}

private:
	struct CallbackAndId
	{
		CallbackType Callback;
		CallbackId Id;
	};

	TArray<CallbackAndId> Callbacks;
	bool bCurrentlyInvokingCallbacks = false;
	TArray<CallbackAndId> CallbacksToAdd;
	TArray<CallbackId> CallbacksToRemove;
};

} // namespace SpatialGDK
