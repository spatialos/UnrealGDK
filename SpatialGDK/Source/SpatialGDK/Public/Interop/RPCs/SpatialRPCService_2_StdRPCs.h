#include "Interop/RPCs/SpatialRPCService_2.h"

namespace SpatialGDK
{
struct ServerStdRPCs
{
	ServerStdRPCs(SpatialRPCService_2& Service);
	TSharedPtr<TRPCQueue<RPCPayload>> ClientReliableRPCQueue;
	TSharedPtr<TRPCQueue<RPCPayload>> ClientUnreliableRPCQueue;
	TSharedPtr<TRPCQueue<RPCPayload>> MulticastRPCQueue;

	TSharedPtr<RPCBufferReceiver> ServerReliableReceiver;
	TSharedPtr<RPCBufferReceiver> ServerUnreliableReceiver;
	// TSharedPtr<RPCBufferReceiver> MulticastReceiver;
};

struct ClientStdRPCs
{
	ClientStdRPCs(SpatialRPCService_2& Service);
	TSharedPtr<TRPCQueue<RPCPayload>> ServerReliableRPCQueue;
	TSharedPtr<TRPCQueue<RPCPayload>> ServerUnreliableRPCQueue;
	TSharedPtr<TRPCQueue<RPCPayload>> NetMovementRPCQueue;

	TSharedPtr<RPCBufferReceiver> ClientReliableReceiver;
	TSharedPtr<RPCBufferReceiver> ClientUnreliableReceiver;
	// TSharedPtr<RPCBufferReceiver> MulticastReceiver;
};

} // namespace SpatialGDK

void ExecuteIncomingRPC(USpatialNetDriver* NetDriver, SpatialGDK::RPCBufferReceiver& Receiver);
