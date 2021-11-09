// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;

namespace Improbable.WorkerCoordinator
{
    /// <summary>
    /// Defines the base class of a worker coordinator.
    /// Keeps track of spawned processes for simulated players,
    /// and provides functionality to wait for these processes to exit.
    /// </summary>
    internal abstract class AbstractWorkerCoordinator
    {
        private const string AdditionalProcessArguments = "-FailOnNetworkFailure";

        protected Logger Logger;
        private List<Process> ActiveProcesses = new List<Process>();

        public AbstractWorkerCoordinator(Logger logger)
        {
            Logger = logger;
        }

        /// <summary>
        /// Contains the logic for running this coordinator, including starting a number of simulated players,
        /// generating required tokens, and waiting for the players to exit.
        /// When this method returns the coordinator process will exit.
        /// </summary>
        public abstract void Run();

        /// <summary>
        /// Creates a new process that runs a simulated player, providing it with the specified arguments.
        /// </summary>
        /// <param name="simulatedPlayerName">Simulated player instance name.</param>
        /// <param name="fileName">File name of the simulated player executable to start.</param>
        /// <param name="args">Arguments to pass to the started process.</param>
        /// <returns>The started simulated player process, or null if something went wrong.</returns>
        protected Process CreateSimulatedPlayerProcess(string simulatedPlayerName, string fileName, string args)
        {
            try
            {
                var argsToStartWith = args + " " + AdditionalProcessArguments;

                Logger.WriteLog("Starting worker " + simulatedPlayerName + " with args: " + argsToStartWith);

                var process = Process.Start(fileName, argsToStartWith);

                if (process != null)
                {
                    ActiveProcesses.Add(process);
                }

                return process;
            }
            catch (Exception e)
            {
                Logger.WriteError($"Error starting simulated player: {e.Message}");
                return null;
            }
        }

        /// <summary>
        /// Check simulated player clients' status.
        /// If it stopped early by accident, restart it.
        /// If it stopped with code 137, that means we stop it with StopSimulatedClient.sh.
        /// </summary>
        /// <returns>return true if active processes list is empty.</returns>
        public bool CheckPlayerStatus()
        {
            bool haveProcessFinishedWithoutError = false;
            var finishedProcesses = ActiveProcesses.Where(process => process.HasExited).ToList();

            foreach (var process in finishedProcesses)
            {
                // StopSimulatedClient.sh will cause 137 code.
                if (process.ExitCode != 0 && process.ExitCode != 137)
                {
                    Logger.WriteLog($"Restarting simulated player after it failed with exit code {process.ExitCode}");
                    process.Start();
                }
                else
                {
                    ActiveProcesses.Remove(process);
                    haveProcessFinishedWithoutError = true;
                }
            }

            // Need haveProcessFinishedWithoutError to ignore the initial case.
            return haveProcessFinishedWithoutError && ActiveProcesses.Count == 0;
        }
    }
}
