// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
using System;
using System.Threading;
using System.Collections.Generic;

namespace Improbable.WorkerCoordinator
{
    /// <summary>
    /// Simulated player client's information.
    /// The coordinator use this to start & restart simulated player clients.
    /// </summary>
    struct ClientInfo
    {
        public string ClientName;
        public string DevAuthToken;
        public string TargetDeployment;
        public DateTime StartTime;
        public DateTime EndTime;
    }

    internal interface ILifetimeComponentHost
    {
        void StartClient(ClientInfo clientInfo);
        void StopClient(ClientInfo clientInfo);
        bool CheckPlayerStatus();
    }

    internal class LifetimeComponent
    {
        // Arguments for lifetime management.
        public const string MaxLifetimeArg = "max_lifetime";
        public const string MinLifetimeArg = "min_lifetime";
        public const string RestartAfterSecondsArg = "restart_after_seconds";
        public const string UseNewSimulatedPlayerArg = "use_new_simulated_player";

        // Lifetime management parameters.
        private bool IsLifetimeMode;
        private ILifetimeComponentHost Host;
        private bool UseNewSimulatedPlayer;
        private int MaxLifetime;
        private int MinLifetime;
        private int RestartAfterSeconds;
        private List<ClientInfo> WaitingList;
        private List<ClientInfo> RunningList;

        /// <summary>
        /// Tick interval in milliseconds
        /// </summary>
        private int TickInterval = 1000;
        private Logger Logger;
        private Random Random;
        private Timer TickTimer;
        private AutoResetEvent ResetEvent;

        /// <summary>
        /// Create lifetime component, will return null if max lifetime argument not in args.
        /// </summary>
        /// <param name="logger"></param>
        /// <param name="args"></param>
        /// <param name="numArgs"></param>
        /// <returns></returns>
        public static LifetimeComponent Create(Logger logger, string[] args, out int numArgs)
        {
            numArgs = 0;

            // Max lifetime argument.
            int maxLifetime = 0;
            if (Util.HasIntegerArgument(args, MaxLifetimeArg))
            {
                maxLifetime = Util.GetIntegerArgument(args, MaxLifetimeArg);
                numArgs++;
            }

            // Use max lifetime as default.
            int minLifetime = maxLifetime;
            if (Util.HasIntegerArgument(args, MinLifetimeArg))
            {
                minLifetime = Util.GetIntegerArgument(args, MinLifetimeArg);
                numArgs++;
            }

            // Use 60 seconds as default.
            int restartAfterSeconds = 60;
            if (Util.HasIntegerArgument(args, RestartAfterSecondsArg))
            {
                restartAfterSeconds = Util.GetIntegerArgument(args, RestartAfterSecondsArg);
                numArgs++;
            }

            // Default do not use new simulated player to restart.
            int useNewSimulatedPlayer = 0;
            if (Util.HasIntegerArgument(args, UseNewSimulatedPlayerArg))
            {
                useNewSimulatedPlayer = Util.GetIntegerArgument(args, UseNewSimulatedPlayerArg);
                numArgs++;
            }

            // Disable function by do not define max lifetime.
            // With this we do not need to change the old configuration files.
            bool isLifetimeMode = maxLifetime > 0;

            return new LifetimeComponent(isLifetimeMode, maxLifetime, minLifetime, restartAfterSeconds, useNewSimulatedPlayer > 0, logger);
        }

        private LifetimeComponent(bool isLifetimeMode, int maxLifetime, int minLifetime, int restartAfterSeconds, bool useNewSimulatedPlayer, Logger logger)
        {
            IsLifetimeMode = isLifetimeMode;
            UseNewSimulatedPlayer = useNewSimulatedPlayer;
            MaxLifetime = maxLifetime;
            MinLifetime = minLifetime;
            RestartAfterSeconds = restartAfterSeconds;
            Logger = logger;

            WaitingList = new List<ClientInfo>();
            RunningList = new List<ClientInfo>();

            Random = new Random(Guid.NewGuid().GetHashCode());

            ResetEvent = new AutoResetEvent(false);
        }

        public void SetHost(ILifetimeComponentHost host)
        {
            Host = host;
        }

        public void AddSimulatedPlayer(ClientInfo clientInfo)
        {
            WaitingList.Add(clientInfo);

            Logger.WriteLog($"[LifetimeComponent][{DateTime.Now:HH:mm:ss}] Add client ClientName={clientInfo.ClientName}, IsLifetimeMode={IsLifetimeMode}.");
        }
        
        /// <summary>
        /// Start will keep blocking until it is finished.
        /// </summary>
        public void Start()
        {
            // Start timer
            TickTimer = new Timer(Tick, ResetEvent, 0, TickInterval);

            // Wait until Tick finish.
            ResetEvent.WaitOne();
            TickTimer.Dispose();

            Logger.WriteLog($"[LifetimeComponent][{DateTime.Now:HH:mm:ss}] All simulated clients are finished, IsLifetimeMode={IsLifetimeMode}.");
        }

        private void Tick(object state)
        {
            // Check player status, restart player client if it exit early.
            bool isActiveProcessListEmpty = Host.CheckPlayerStatus();

            // Non-lifetime mode, do not stop any clients.
            if (!IsLifetimeMode)
            {
                // All clients are finished.
                if (isActiveProcessListEmpty)
                {
                    ResetEvent.Set();
                }

                for (int i = WaitingList.Count - 1; i >= 0; --i)
                {
                    var clientInfo = WaitingList[i];

                    Host?.StartClient(clientInfo);
                    Logger.WriteLog($"[LifetimeComponent][{DateTime.Now:HH:mm:ss}] Start client ClientName ={clientInfo.ClientName}.");

                    // Move to running list.
                    WaitingList.RemoveAt(i);
                    RunningList.Add(clientInfo);
                }

                return;
            }

            // Lifetime mode.
            DateTime curTime = DateTime.Now;

            // Data flow is waiting list -> running list -> waiting list.
            // Checking sequence is running list -> waiting list.

            // Running list.
            for (int i = RunningList.Count - 1; i >= 0; --i)
            {
                var clientInfo = RunningList[i];
                if (curTime >= clientInfo.EndTime)
                {
                    // End client.
                    Host?.StopClient(clientInfo);

                    Logger.WriteLog($"[LifetimeComponent][{DateTime.Now:HH:mm:ss}] Stop client ClientName={clientInfo.ClientName}.");

                    // Delay a few seconds to restart. Make sure the client has timed out and gone offline.
                    clientInfo.StartTime = curTime.AddSeconds(RestartAfterSeconds);

                    // Restart with new simulated player.
                    if (UseNewSimulatedPlayer)
                    {
                        clientInfo.ClientName = "SimulatedPlayer" + Guid.NewGuid();
                    }

                    // Move to wait list.
                    RunningList.RemoveAt(i);
                    WaitingList.Add(clientInfo);
                }
            }

            // Waiting list.
            for (int i = WaitingList.Count - 1; i >= 0; --i)
            {
                var clientInfo = WaitingList[i];
                if (curTime >= clientInfo.StartTime)
                {
                    // Start client.
                    Host?.StartClient(clientInfo);

                    Logger.WriteLog($"[LifetimeComponent][{DateTime.Now:HH:mm:ss}] Start client ClientName ={clientInfo.ClientName}.");

                    // Update lifetime.
                    clientInfo.EndTime = curTime.AddMinutes(Random.Next(MinLifetime, MaxLifetime));

                    // Move to running list.
                    WaitingList.RemoveAt(i);
                    RunningList.Add(clientInfo);
                }
            }
        }
    }
}
