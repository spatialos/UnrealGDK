using System;
using System.Threading;
using Improbable.Worker;

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

            KeepConnectionAlive(connection, logger);

            return connection;
        }

        // We do not use the connection to the simulated player deployment,
        // but we must ensure it is kept open to prevent the coordinator worker
        // from being killed.
        // This keeps the connection open in a separate thread by repeatedly calling
        // GetOpList on the connection.
        private static void KeepConnectionAlive(Connection connection, Logger logger)
        {
            var thread = new Thread(() =>
            {
                using (var dispatcher = new Dispatcher())
                {
                    var isConnected = true;

                    dispatcher.OnDisconnect(op =>
                    {
                        logger.WriteError("[disconnect] " + op.Reason);
                        isConnected = false;
                    });

                    dispatcher.OnLogMessage(op =>
                    {
                        connection.SendLogMessage(op.Level, LoggerName, op.Message);
                        if (op.Level == LogLevel.Fatal)
                        {
                            logger.WriteError("Fatal error: " + op.Message);
                            Environment.Exit(1);
                        }
                    });

                    while (isConnected)
                    {
                        using (var opList = connection.GetOpList(GetOpListTimeoutInMilliseconds))
                        {
                            dispatcher.Process(opList);
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
