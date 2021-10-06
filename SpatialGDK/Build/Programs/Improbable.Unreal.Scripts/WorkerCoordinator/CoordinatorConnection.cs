// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

using System;
using System.Threading;
using Improbable.Worker.CInterop;

namespace Improbable.WorkerCoordinator
{
    class CoordinatorConnection
    {
        private const string LoggerName = "CoordinatorConnection.cs";
        private const uint GetOpListTimeoutInMilliseconds = 100;

        public static Connection ConnectAndKeepAlive(Logger logger, string receptionistHost, ushort receptionistPort, string workerId, string coordinatorWorkerType)
        {
            var connectionParameters = new ConnectionParameters
            {
                WorkerType = coordinatorWorkerType,
                Network =
                {
                    ConnectionType = NetworkConnectionType.Tcp,
                    UseExternalIp = false
                }
            };

            Connection connection;
            using (var future = Connection.ConnectAsync(receptionistHost, receptionistPort, workerId, connectionParameters))
            {
                connection = future.Get();
            }

            // Start sending logs to SpatialOS.
            logger.EnableSpatialOSLogging(connection);

            KeepConnectionAlive(connection, logger);

            return connection;
        }

        /// <summary>
        /// Starts a new background thread that will keep the connection to SpatialOS alive.
        /// We do not use the connection to the simulated player deployment,
        /// but we must ensure it is kept open to prevent the coordinator worker
        /// from being killed.
        /// </summary>
        private static void KeepConnectionAlive(Connection connection, Logger logger)
        {
            var thread = new Thread(() =>
            {
                var isConnected = true;

                while (isConnected)
                {
                    using (OpList opList = connection.GetOpList(GetOpListTimeoutInMilliseconds))
                    {
                        var OpCount = opList.GetOpCount();
                        for (var i = 0; i < OpCount; ++i)
                        {
                            switch (opList.GetOpType(i))
                            {
                                case OpType.Disconnect:
                                    {
                                        DisconnectOp op = opList.GetDisconnectOp(i);
                                        logger.WriteError("[disconnect] " + op.Reason, logToConnectionIfExists: false);
                                        isConnected = false;
                                    }
                                    break;
                                default:
                                    break;
                            }
                        }
                    }
                }
            });

            // Don't keep thread / connection alive when main process stops.
            thread.IsBackground = true;

            thread.Start();
        }
    }
}
