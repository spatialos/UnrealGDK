#pragma once
#include "Containers/Queue.h"
#include "Misc/Optional.h"

template <typename T>
class JitterBuffer
{
public:
	explicit JitterBuffer(float ArtificialLag, float MaxLocalDelay)
		: ArtificialLag(ArtificialLag)
		, MaxLocalDelay(MaxLocalDelay)
	{
	}

	// If TransmitterSentTime is empty, the message will just be dequeued as soon as the ordering allows.
	void ReceivedMessage(float CurrentTime, int64 SenderId, TOptional<float> TransmitterSentTime, T Message)
	{
		Sender& Sender = Senders.FindOrAdd(SenderId);

		if (TransmitterSentTime.IsSet())
		{
			const auto TimeDifference = CurrentTime - TransmitterSentTime.GetValue();

			if (Sender.AverageTimeDifference)
			{
				Sender.AverageTimeDifference = (0.2f * TimeDifference) + (0.8f * Sender.AverageTimeDifference.GetValue());
			}
			else
			{
				Sender.AverageTimeDifference = TimeDifference;
			}

			float LocalDelay = TransmitterSentTime.GetValue() + Sender.AverageTimeDifference.GetValue() + ArtificialLag - CurrentTime;
			LocalDelay = FMath::Min(LocalDelay, MaxLocalDelay);
			Sender.Messages->Enqueue(EnqueuedMessage{ MoveTemp(Message), CurrentTime + LocalDelay });
		}
		else
		{
			Sender.Messages->Enqueue(EnqueuedMessage{ MoveTemp(Message), 0 });
		}
	}

	void ClearSender(int64 SenderId) { Senders.Remove(SenderId); }

	void DequeueMessages(float CurrentTime, const TFunction<void(T)>& Callback)
	{
		for (auto& Kvp : Senders)
		{
			Sender& Sender = Kvp.Value;
			while (!Sender.Messages->IsEmpty())
			{
				EnqueuedMessage* Message = Sender.Messages->Peek();
				if (CurrentTime >= Message->LocalTimeToPlay)
				{
					check(Message);
					Callback(MoveTemp(Message->Message));
					Sender.Messages->Pop();
				}
				else
				{
					break;
				}
			}
		}
	}

private:
	struct EnqueuedMessage
	{
		T Message;
		float LocalTimeToPlay;
	};

	const float ArtificialLag;
	const float MaxLocalDelay;

	struct Sender
	{
		TUniquePtr<TQueue<EnqueuedMessage>> Messages;
		TOptional<float> AverageTimeDifference;

		Sender() { Messages = MakeUnique<TQueue<EnqueuedMessage>>(); }
	};
	TMap<int64, Sender> Senders;
};
