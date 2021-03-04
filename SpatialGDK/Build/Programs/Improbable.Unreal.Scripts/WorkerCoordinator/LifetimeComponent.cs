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
        public long StartTick;
        public long EndTick;
    }

    internal interface ILifetimeComponentHost
    {
        void StartClient(ClientInfo clientInfo);
        void StopClient(ClientInfo clientInfo);

        void CheckPlayerStatus();
    }

    internal class LifetimeComponent
    {
        // Arguments for lifetime management.
        public const string MaxLifetimeArg = "max_lifetime";
        public const string MinLifetimeArg = "min_lifetime";
        public const string RestartAfterSecondsArg = "restart_after_seconds";
        public const string UseNewSimulatedPlayerArg = "use_new_simulated_player";

        // Lifetime management parameters.
        private ILifetimeComponentHost Host;
        private bool UseNewSimulatedPlayer;
        private int MaxLifetime;
        private int MinLifetime;
        private int RestartAfterSeconds;
        private List<ClientInfo> WaitingList;
        private List<ClientInfo> RunningList;

        private int TickIntervalSeconds = 1;
        private Logger Logger;
        private Random Random;

        // Temp variables to avoid allocate variable in tick method.
        private long CurTicks;
        private int Length;
        private ClientInfo SimulatedClientInfo;

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
            if (maxLifetime > 0)
            {
                return new LifetimeComponent(maxLifetime, minLifetime, restartAfterSeconds, useNewSimulatedPlayer > 0, logger);
            }

            return null;
        }

        private LifetimeComponent(int maxLifetime, int minLifetime, int restartAfterSeconds, bool useNewSimulatedPlayer, Logger logger)
        {
            UseNewSimulatedPlayer = useNewSimulatedPlayer;
            MaxLifetime = maxLifetime;
            MinLifetime = minLifetime;
            RestartAfterSeconds = restartAfterSeconds;
            Logger = logger;

            WaitingList = new List<ClientInfo>();
            RunningList = new List<ClientInfo>();

            Random = new Random(Guid.NewGuid().GetHashCode());
        }

        public void SetHost(ILifetimeComponentHost host)
        {
            Host = host;
        }

        public void AddSimulatedPlayer(ClientInfo clientInfo)
        {
            WaitingList.Add(clientInfo);

            Logger.WriteLog($"[LifetimeComponent][{DateTime.Now.ToString("HH:mm:ss")}] Add client ClientName={clientInfo.ClientName}.");
        }

        private long NewLifetimeTicks()
        {
            return TimeSpan.FromMinutes(Random.Next(MinLifetime, MaxLifetime)).Ticks;
        }

        public void Start()
        {
            // Loop tick.
            while (true)
            {
                Tick();

                Thread.Sleep(TimeSpan.FromSeconds(TickIntervalSeconds));
            }
        }

        private void Tick()
        {
            CurTicks = DateTime.Now.Ticks;

            // Data flow is waiting list -> running list -> waiting list.
            // Checking sequence is running list -> waiting list.

            // Check player status, restart player client if it exit early.
            Host?.CheckPlayerStatus();

            // Running list.
            Length = RunningList.Count;
            for (int i = Length - 1; i >= 0; --i)
            {
                SimulatedClientInfo = RunningList[i];
                if (CurTicks >= SimulatedClientInfo.EndTick)
                {
                    // End client.
                    Host?.StopClient(SimulatedClientInfo);

                    Logger.WriteLog($"[LifetimeComponent][{DateTime.Now.ToString("HH:mm:ss")}] Stop client ClientName={SimulatedClientInfo.ClientName}.");

                    // Delay 10 seconds to restart.
                    SimulatedClientInfo.StartTick = TimeSpan.FromSeconds(RestartAfterSeconds).Ticks + CurTicks;

                    // Restart with new simulated player.
                    if (UseNewSimulatedPlayer)
                    {
                        SimulatedClientInfo.ClientName = "SimulatedPlayer" + Guid.NewGuid();
                    }

                    // Move to wait list.
                    RunningList.RemoveAt(i);
                    WaitingList.Add(SimulatedClientInfo);
                }
            }

            // Waiting list.
            Length = WaitingList.Count;
            for (int i = Length - 1; i >= 0; --i)
            {
                SimulatedClientInfo = WaitingList[i];
                if (CurTicks >= SimulatedClientInfo.StartTick)
                {
                    // Start client.
                    Host?.StartClient(SimulatedClientInfo);

                    Logger.WriteLog($"[LifetimeComponent][{DateTime.Now.ToString("HH:mm:ss")}] Start client ClientName ={SimulatedClientInfo.ClientName}.");

                    // Update lifetime.
                    SimulatedClientInfo.EndTick = CurTicks + NewLifetimeTicks();

                    // Move to running list.
                    WaitingList.RemoveAt(i);
                    RunningList.Add(SimulatedClientInfo);
                }
            }
        }
    }
}
