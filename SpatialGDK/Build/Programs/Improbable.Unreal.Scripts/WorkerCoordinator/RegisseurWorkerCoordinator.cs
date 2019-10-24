// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading;

namespace Improbable.WorkerCoordinator
{
    /// <summary>
    /// For Improbable internal use only.
    /// Defines a worker coordinator to be run as part of a Regisseur scenario.
    /// Runs as a standalone binary without creating a connection to a deployment, by connecting a configurable
    /// number of simulated players over configurable delays.
    /// </summary>
    internal class RegisseurWorkerCoordinator : AbstractWorkerCoordinator
    {
        private const string SimulatedPlayerSpawnCountArg = "simulated_player_spawn_count";
        private const string InitialStartDelayArg = "coordinator_start_delay_millis";
        private const string SpawnIntervalArg = "coordinator_spawn_interval_millis";
        private const string SimulatedPlayerWorkerNamePlaceholderArg = "<IMPROBABLE_WORKER_ID>";

        private const string SimulatedPlayerFilename = "StartSimulatedClient.sh";

        // Coordinator options.
        private int NumSimulatedPlayersToStart;
        private int InitialStartDelayMillis;
        private int StartIntervalMillis;
        private string[] SimulatedPlayerArgs;

        /// <summary>
        /// The arguments to start the coordinator must begin with:
        ///
        ///     "simulated_player_spawn_count={integer}",
        ///     "coordinator_spawn_interval_millis={integer}",
        ///     "coordinator_start_delay_millis={integer}",
        ///     "<IMPROBABLE_WORKER_ID>"
        ///
        /// All following arguments are passed to the simulated player process, where
        /// the "<IMPROBABLE_WORKER_ID>" placeholder will be replaced by the
        /// worker id of the simulated player.
        /// </summary>
        public static RegisseurWorkerCoordinator FromArgs(Logger logger, string[] args)
        {
            return new RegisseurWorkerCoordinator(logger)
            {
                NumSimulatedPlayersToStart = Util.GetIntegerArgumentOrDefault(args, SimulatedPlayerSpawnCountArg, 1),
                InitialStartDelayMillis = Util.GetIntegerArgumentOrDefault(args, InitialStartDelayArg, 10000),
                StartIntervalMillis = Util.GetIntegerArgumentOrDefault(args, SpawnIntervalArg, 1000),

                // First 3 arguments are for the coordinator worker only.
                // The 4th argument (worker id) is consumed by the simulated player startup script,
                // and not passed to the simulated player.
                SimulatedPlayerArgs = args.Skip(3).ToArray()
            };
        }

        private RegisseurWorkerCoordinator(Logger logger) : base(logger)
        {
        }

        public override void Run()
        {
            Thread.Sleep(InitialStartDelayMillis);
            for (int i = 0; i < NumSimulatedPlayersToStart; i++)
            {
                StartSimulatedPlayer($"SimulatedPlayer{Guid.NewGuid()}");
                Thread.Sleep(StartIntervalMillis);
            }

            WaitForPlayersToExit();
        }

        public void StartSimulatedPlayer(string name)
        {
            var args = Util.ReplacePlaceholderArgs(SimulatedPlayerArgs, new Dictionary<string, string>()
            {
                { SimulatedPlayerWorkerNamePlaceholderArg, name }
            });

            string simulatedPlayerArgs = string.Join(" ", args);
            Logger.WriteLog("Starting worker " + name + " with args: " + simulatedPlayerArgs);
            CreateSimulatedPlayerProcess(SimulatedPlayerFilename, simulatedPlayerArgs);
        }
    }
}
