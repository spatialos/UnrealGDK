// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Threading;

namespace Improbable.WorkerCoordinator
{
    /// <summary>
    /// Defines the base class of a worker coordinator.
    /// Keeps track of spawned processes for simulated players,
    /// and provides functionality to wait for these processes to exit.
    /// </summary>
    internal abstract class AbstractWorkerCoordinator
    {
        private const int PollSimulatedPlayerProcessIntervalMillis = 5000;

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
        /// <param name="fileName">File name of the simulated player executable to start.</param>
        /// <param name="args">Arguments to pass to the started process.</param>
        /// <returns>The started simulated player process, or null if something went wrong.</returns>
        protected Process CreateSimulatedPlayerProcess(string fileName, string args)
        {
            try
            {
                var process = Process.Start(fileName, args);

                if (process != null)
                {
                    process.EnableRaisingEvents = true;
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
        /// Blocks until all active simulated player processes have exited.
        /// Restarts failed simulated player processes.
        /// Will only wait for processes started through CreateSimulatedPlayerProcess().
        /// </summary>
        protected void WaitForPlayersToExit()
        {
            foreach (var process in ActiveProcesses)
            {
                process.Exited += (sender, args) =>
                {
                    if (process.ExitCode != 0)
                    {
                        Logger.WriteLog($"Restarting client process after it exited with code {process.ExitCode}");

                        process.Start();
                    }
                    else
                    {
                        Logger.WriteLog($"Client process finished");
                    }
                };
            }

            while (!ActiveProcesses.All(process => process.HasExited))
            {
                Thread.Sleep(TimeSpan.FromMilliseconds(PollSimulatedPlayerProcessIntervalMillis));
            }
        }
    }
}
