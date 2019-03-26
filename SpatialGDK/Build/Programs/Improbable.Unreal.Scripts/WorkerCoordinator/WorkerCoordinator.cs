// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Threading;
using Improbable.Worker;

namespace Improbable.WorkerCoordinator
{
    /// <summary>
    /// Executable that connects and runs a simulated player.
    /// The class defines the coordinator worker executable that will run simulated players.
    ///
    /// ARGUMENTS
    /// The first 5 arguments are consumed by the coordinator, and used for connecting the coordinator to
    /// the simulated player deployment using the Receptionist and defining the initial delay before
    /// connecting players to the target deployment:
    ///     receptionist
    ///     {hostname}                              Receptionist hostname
    ///     {port}                                  Receptionist port
    ///     {worker_id}                             Worker id of the coordinator
    ///     coordinator_start_delay_millis={value}  Minimum delay before the coordinator starts a simulated client
    ///
    /// All following arguments will be passed to the simulated player instance.
    /// These arguments can contain the following placeholders, which will be replaced by the coordinator:
    ///     `<IMPROBABLE_SIM_PLAYER_WORKER_ID>`     will be replaced with the worker id of the simulated player
    ///     `<LOGIN_TOKEN>`                         will be replaced by the generated development auth login token
    ///     `<PLAYER_IDENTITY_TOKEN>`               will be replaced by the generated development auth player identity token
    ///
    /// WORKER FLAGS
    /// Additionally, the following worker flags are required for the coordinator:
    ///     simulated_players_dev_auth_token_id     The development auth token id used to generate login tokens and player identity tokens
    ///     simulated_players_target_deployment     Name of the target deployment which the simulated players will connect to
    ///     target_num_simulated_players            The total number of simulated players that will connect to the target deployment. This value is used to determine the delay in connecting simulated players.
    /// 
    /// </summary>
    internal static class WorkerCoordinator
    {
        // Arguments for the coordinator.
        private const string START_DELAY_ARG = "coordinator_start_delay_millis";

        // Argument placeholders for simulated players - these will be replaced by the coordinator by their actual values.
        private const string WORKER_NAME_ARG = "<IMPROBABLE_SIM_PLAYER_WORKER_ID>";
        private const string LOGIN_TOKEN_ARG = "<LOGIN_TOKEN>";
        private const string PLAYER_IDENTITY_TOKEN_ARG = "<PLAYER_IDENTITY_TOKEN>";

        // Worker flags.
        private const string DEV_AUTH_TOKEN_ID_FLAG = "simulated_players_dev_auth_token_id";
        private const string TARGET_DEPLOYMENT_FLAG = "simulated_players_target_deployment";
        private const string NUM_SIM_PLAYERS_FLAG = "target_num_simulated_players";

        private const string CoordinatorWorkerType = "SimulatedPlayerCoordinator";
        private const string SimulatedPlayerWorkerType = "UnrealClient";
        private const string SimulatedPlayerFilename = "StartSimulatedClient.sh";

        private static Stack<Process> SimulatedPlayerList = new Stack<Process>();

        private static Logger Logger = new Logger("/improbable/logs/WorkerCoordinator.log");
        private const string LoggerName = "WorkerCoordinator.cs";
        private const int ErrorExitStatus = 1;

        private const uint GetOpListTimeoutInMilliseconds = 100;
        private static Connection connection;

        private static int Main(string[] args)
        {
            Logger.WriteLog("Starting coordinator with args: " + ArgsToString(args));

            // Create connection between worker coordinator and simulated player deployment
            // and keep it alive to prevent the coordinator from being killed.
            var connectionParameters = new ConnectionParameters
            {
                WorkerType = CoordinatorWorkerType,
                Network =
                {
                    ConnectionType = NetworkConnectionType.Tcp
                }
            };

            // Connect coordinator to simulated player deployment.
            string receptionistHost = args[1];
            ushort receptionistPort = Convert.ToUInt16(args[2]);
            string workerId = args[3];
            connection = ConnectWithReceptionist(receptionistHost, receptionistPort, workerId, connectionParameters);
            KeepConnectionOpen(connection);


            // Read worker flags.
            var devAuthTokenId = connection.GetWorkerFlag(DEV_AUTH_TOKEN_ID_FLAG);
            if (!devAuthTokenId.HasValue)
            {
                connection.SendLogMessage(LogLevel.Error, LoggerName, $"Missing worker flag with development authentication token id ({DEV_AUTH_TOKEN_ID_FLAG}). Unable to connect simulated players.");
                return ErrorExitStatus;
            }

            var targetDeployment = connection.GetWorkerFlag(TARGET_DEPLOYMENT_FLAG);
            if (!targetDeployment.HasValue)
            {
                connection.SendLogMessage(LogLevel.Error, LoggerName, $"Missing worker flag with target deployment ({TARGET_DEPLOYMENT_FLAG}). Unable to connect simulated players.");
                return ErrorExitStatus;
            }

            var numSimulatedPlayers = connection.GetWorkerFlag(NUM_SIM_PLAYERS_FLAG);
            int maxDelayMillis;
            if (!numSimulatedPlayers.HasValue)
            {
                maxDelayMillis = 30_000;
                connection.SendLogMessage(LogLevel.Warn, LoggerName, $"Missing worker flag with target number of simulated players ({NUM_SIM_PLAYERS_FLAG}). Unable to estimate good delay between connecting simulated clients, too many clients might start connecting at the same time.");
            }
            else
            {
                // Connect 1 player per 1.5 seconds (on average) across entire simulated player deployment.
                maxDelayMillis = Convert.ToInt32(numSimulatedPlayers.Value) * 1500;
            }

       
            // Start the simulated player.
            // First 4 args are for coordinator - all following args are for simulated players.
            var simulatedPlayerArgs = args.Skip(4).ToArray();

            // Add a random delay between 0 and maxDelaySec to spread out the connecting of simulated clients (plus a fixed start delay).
            var startDelayMillis = GetIntegerArgument(args, START_DELAY_ARG, 0) + new Random().Next(maxDelayMillis);
            connection.SendLogMessage(LogLevel.Debug, LoggerName, $"Waiting for {startDelayMillis} millis before starting client.");
            Thread.Sleep(startDelayMillis);

            string clientName = "SimulatedPlayer" + Guid.NewGuid();

            // Create player identity token and login token
            var pit = Authentication.GetDevelopmentPlayerIdentityToken(devAuthTokenId.Value, clientName);
            if (pit == null)
            {
                connection.SendLogMessage(LogLevel.Error, LoggerName, $"Did not receive player identity token for {clientName}, client will not connect.");
                return ErrorExitStatus;
            }

            var loginTokens = Authentication.GetDevelopmentLoginTokens(SimulatedPlayerWorkerType, pit);
            if (loginTokens == null)
            {
                connection.SendLogMessage(LogLevel.Error, LoggerName, $"Did not receive login tokens for {clientName}, client will not connect.");
                return ErrorExitStatus;
            }

            var loginToken = Authentication.SelectLoginToken(loginTokens, targetDeployment.Value);
            if (loginToken == null)
            {
                connection.SendLogMessage(LogLevel.Error, LoggerName, "Failed to launch simulated player. Login token for target deployment was not found in response. Does that deployment have the `dev_auth` tag?");
                return ErrorExitStatus;
            }

            // Replace client argument placeholders.
            for (int j = 0; j < simulatedPlayerArgs.Length; j++)
            {
                if (simulatedPlayerArgs[j] == WORKER_NAME_ARG)
                {
                    simulatedPlayerArgs[j] = clientName;
                }
                else if (simulatedPlayerArgs[j] == LOGIN_TOKEN_ARG)
                {
                    simulatedPlayerArgs[j] = loginToken;
                }
                else if (simulatedPlayerArgs[j] == PLAYER_IDENTITY_TOKEN_ARG)
                {
                    simulatedPlayerArgs[j] = pit;
                }
            }

            // Prepend the simulated player id as an argument to the start client script.
            // This argument is consumed by the start client script and will not be passed to the client worker.
            simulatedPlayerArgs = new string[] { clientName }.Concat(simulatedPlayerArgs).ToArray();

            // Start the client
            connection.SendLogMessage(LogLevel.Info, LoggerName, "Starting worker " + clientName + " with args: " + ArgsToString(simulatedPlayerArgs));
            StartClient(string.Join(" ", simulatedPlayerArgs));

            // Wait for client to have exited
            while (SimulatedPlayerList.Count > 0)
            {
                try
                {
                    SimulatedPlayerList.Pop().WaitForExit();
                }
                catch (Exception e)
                {
                    Logger.WriteError("Error while waiting for simulated player to exit: " + e.Message);
                }
            }

            return 0;
        }

        private static int GetIntegerArgument(IEnumerable<string> args, string argumentName, int defaultValue)
        {
            var argumentsWithName = args.Where(arg => arg.StartsWith(argumentName)).ToArray();
            if (!argumentsWithName.Any())
            {
                return defaultValue;
            }

            if (argumentsWithName.Length == 1)
            {
                var valueString = argumentsWithName.Single().Split(new[] {'='}, 2, StringSplitOptions.None)[1];
                int value;
                if (int.TryParse(valueString, out value))
                {
                    return value;
                }
                throw new ArgumentException($"Cannot parse value,\"{valueString}\", for argument \"{argumentName}\".");
            }
            throw new ArgumentException($"Multiple values for argument, \"{argumentName}\".");
        }

        private static void StartClient(string args)
        {
            try
            {
                SimulatedPlayerList.Push(Process.Start(SimulatedPlayerFilename, args));
            }
            catch (Exception e)
            {
                connection.SendLogMessage(LogLevel.Error, LoggerName, "Exception from starting simulated player: " + e.Message);
            }
        }
        
        private static string ArgsToString(string[] args)
        {
            return string.Join(" ", args);
        }

        private static Connection ConnectWithReceptionist(string hostname, ushort port,
            string workerId, ConnectionParameters connectionParameters)
        {
            Connection connection;

            connectionParameters.Network.UseExternalIp = false;

            using (var future = Connection.ConnectAsync(hostname, port, workerId, connectionParameters))
            {
                connection = future.Get();
            }

            connection.SendLogMessage(LogLevel.Info, LoggerName, "Successfully connected coordinator using the Receptionist");

            return connection;
        }

        // We do not use the connection to the simulated player deployment,
        // but we must ensure it is kept open to prevent the coordinator worker
        // from being killed.
        // This keeps the connection open in a separate thread by repeatedly calling
        // GetOpList on the connection.
        private static void KeepConnectionOpen(Connection connection)
        {
            var thread = new Thread(() =>
            {
                using (var dispatcher = new Dispatcher())
                {
                    var isConnected = true;

                    dispatcher.OnDisconnect(op =>
                    {
                        Logger.WriteError("[disconnect] " + op.Reason);
                        isConnected = false;
                    });

                    dispatcher.OnLogMessage(op =>
                    {
                        connection.SendLogMessage(op.Level, LoggerName, op.Message);
                        if (op.Level == LogLevel.Fatal)
                        {
                            Logger.WriteError("Fatal error: " + op.Message);
                            Environment.Exit(ErrorExitStatus);
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
