// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

namespace Improbable.WorkerCoordinator
{
    /// <summary>
    /// Executable that connects and runs simulated players.
    ///
    /// There are two types of coordinator workers:
    ///
    /// ManagedWorkerCoordinator
    /// Starts as a managed worker and creates its own connection to the hosting deployment (simulated player deployment).
    /// <seealso cref="ManagedWorkerCoordinator"/>
    ///
    /// RegisseurWorkerCoordinator
    /// Starts as a standalone process, only for internal use at Improbable for running as part of a Regisseur scenario.
    /// <seealso cref="RegisseurWorkerCoordinator"/>
    ///
    /// To start a ManagedWorkerCoordinator, the first command line argument must be `receptionist`.
    /// Otherwise, a RegisseurWorkerCoordinator is started.
    /// </summary>
    internal static class Program
    {
        private static int Main(string[] args)
        {
            var logger = new Logger("/improbable/logs/WorkerCoordinator.log", "WorkerCoordinator");
            logger.WriteLog("Starting coordinator with args: " + string.Join(" ", args));

            AbstractWorkerCoordinator coordinator;
            if (args[0] == "receptionist")
            {
                // Start coordinator inside a deployment as a managed worker.
                coordinator = ManagedWorkerCoordinator.FromArgs(logger, args);
            }
            else
            {
                // Start coordinator as standalone process, not as a managed worker.
                coordinator = RegisseurWorkerCoordinator.FromArgs(logger, args);
            }

            // Run coordinator. Blocks until completion.
            coordinator.Run();

            return 0;
        }
    }
}
