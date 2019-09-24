// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Threading;
using Google.Api.Gax.Grpc;
using Google.LongRunning;
using Grpc.Core;
using Improbable.OnlineServices.Proto.Gateway;
using Improbable.OnlineServices.Proto.Party;
using Improbable.Worker;

namespace Improbable.WorkerCoordinator
{
    /// <summary>

    ///     receptionist
    ///     {hostname}                              Receptionist hostname
    ///     {port}                                  Receptionist port
    ///     {worker_id}                             Worker id of the coordinator
    ///     coordinator_start_delay_millis={value}  Minimum delay before the coordinator starts a simulated client, to prevent clients from connecting too soon to the target deployment
    ///
    /// All following arguments will be passed to the simulated player instance.
    /// These arguments can contain the following placeholders, which will be replaced by the coordinator:
    ///     `<IMPROBABLE_SIM_PLAYER_WORKER_ID>`     will be replaced with the worker id of the simulated player
    ///     `<LOGIN_TOKEN>`                         will be replaced by the generated development auth login token
    ///     `<PLAYER_IDENTITY_TOKEN>`               will be replaced by the generated development auth player identity token
    ///
    /// WORKER FLAGS
    /// Additionally, the following worker flags are required for the coordinator:
    ///     simulated_players_dev_auth_token     The development auth token used to generate login tokens and player identity tokens
    ///     simulated_players_target_deployment     Name of the target deployment which the simulated players will connect to
    ///     target_num_simulated_players            The total number of simulated players that will connect to the target deployment. This value is used to determine the delay in connecting simulated players.
    ///
    /// </summary>
    internal static class WorkerCoordinator
    {
        private const string PitRequestHeaderName = "x-player-identity-token";

        // Arguments for the coordinator.
        private const string START_DELAY_ARG = "coordinator_start_delay_millis";

        // Argument placeholders for simulated players - these will be replaced by the coordinator by their actual values.
        private const string WORKER_NAME_ARG = "<IMPROBABLE_SIM_PLAYER_WORKER_ID>";
        private const string LOGIN_TOKEN_ARG = "<LOGIN_TOKEN>";
        private const string PLAYER_IDENTITY_TOKEN_ARG = "<PLAYER_IDENTITY_TOKEN>";

        // Worker flags.
        private const string DEV_AUTH_TOKEN_FLAG = "simulated_players_dev_auth_token";
        private const string TARGET_DEPLOYMENT_FLAG = "simulated_players_target_deployment";
        private const string NUM_SIM_PLAYERS_FLAG = "target_num_simulated_players";
        private const string TARGET_DEPLOYMENT_READY_FLAG = "target_deployment_ready";
        private const string PARTY_SERVER_TARGER_FLAG = "target_party_server";
        private const string GATEWAY_SERVER_TARGER_FLAG = "target_gateway_server";
        private const string DEPLOYMENT_TAG = "deployment_tag";

        private const string CoordinatorWorkerType = "SimulatedPlayerCoordinator";
        private const string SimulatedPlayerWorkerType = "UnrealClient";
        private const string SimulatedPlayerFilename = "StartSimulatedClient.sh";

        private static Stack<Process> SimulatedPlayerList = new Stack<Process>();

        private static Logger Logger = new Logger("/improbable/logs/WorkerCoordinator.log");
        private const string LoggerName = "WorkerCoordinator.cs";
        private const int ErrorExitStatus = 1;

        private static Connection Connection;

        private static Random Random;

        // Average amount of delay between connecting each client to the target deployment.
        private const int AVG_MS_BETWEEN_CLIENTS_CONNECTING = 1500;

        private static int Main(string[] args)
        {
            Logger.WriteLog("Starting coordinator with args: " + ArgsToString(args));

            // Create connection between worker coordinator and simulated player deployment
            // and keep it alive to prevent the coordinator from being killed.
            string receptionistHost = args[1];
            ushort receptionistPort = Convert.ToUInt16(args[2]);
            string workerId = args[3];
            Connection = CoordinatorConnection.ConnectAndKeepAlive(Logger, receptionistHost, receptionistPort, workerId, CoordinatorWorkerType);

            Random = new Random(Guid.NewGuid().GetHashCode());

            // Read worker flags.
            string devAuthToken, targetDeployment, partyServerTarget, gatewayServerTarget, deploymentTag;
            int numSimulatedPlayers;
            try
            {
                devAuthToken = GetRequiredWorkerFlag(DEV_AUTH_TOKEN_FLAG);
                targetDeployment = GetRequiredWorkerFlag(TARGET_DEPLOYMENT_FLAG);
                numSimulatedPlayers = Convert.ToInt32(GetRequiredWorkerFlag(NUM_SIM_PLAYERS_FLAG));
                partyServerTarget = GetRequiredWorkerFlag(PARTY_SERVER_TARGER_FLAG);
                gatewayServerTarget = GetRequiredWorkerFlag(GATEWAY_SERVER_TARGER_FLAG);
                deploymentTag = GetRequiredWorkerFlag(DEPLOYMENT_TAG);
            }
            catch (Exception e)
            {
                Connection.SendLogMessage(LogLevel.Error, LoggerName, $"Failed to start simulated player deployment: {e.Message}");
                return ErrorExitStatus;
            }

            // Wait for target deployment to be ready
            WaitForTargetDeploymentReady();

            // Start the simulated player.
            // First 4 args are for coordinator - all following args are for simulated players.
            var simulatedPlayerArgs = args.Skip(4).ToArray();

            // Add a random delay between 0 and maxDelaySec to spread out the connecting of simulated clients (plus a fixed start delay).
            var maxDelayMillis = numSimulatedPlayers * AVG_MS_BETWEEN_CLIENTS_CONNECTING;
            maxDelayMillis = 1000 * 60 * 10;
            var ourRandomDelayMillis = Random.Next(maxDelayMillis);
            var fixedStartDelayMillis = GetIntegerArgument(args, START_DELAY_ARG, 0);
            var startDelayMillis = fixedStartDelayMillis + ourRandomDelayMillis;
            Connection.SendLogMessage(LogLevel.Info, LoggerName, $"Sleeping for {startDelayMillis} ms.");
            Thread.Sleep(startDelayMillis);

            string clientName = "SimulatedPlayer" + Guid.NewGuid();

            // Create player identity token and login token
            Metadata pitMetadata;
            string pit, loginToken;

            try
            {
                ChannelCredentials partyCredentials = ChannelCredentials.Insecure;
                ChannelCredentials gatewayCredentials = ChannelCredentials.Insecure;

                var partyClient = new PartyService.PartyServiceClient(new Channel(partyServerTarget, partyCredentials));
                var gatewayClient = new GatewayService.GatewayServiceClient(
                    new Channel(gatewayServerTarget, gatewayCredentials));
                /* We use the ops client to check the state of our request. */

                var gatewayOpsClient = OperationsClient.Create(new Channel(gatewayServerTarget, gatewayCredentials));

                pit = Authentication.GetDevelopmentPlayerIdentityToken(devAuthToken, clientName);
                Connection.SendLogMessage(LogLevel.Info, LoggerName, $"PIT created: {pit}");
                pitMetadata = new Metadata {{PitRequestHeaderName, pit}};

                Connection.SendLogMessage(LogLevel.Info, LoggerName, "Creating and joining a solo party");
                partyClient.CreateParty(new CreatePartyRequest
                {
                    MinMembers = 1,
                    MaxMembers = 1
                }, pitMetadata);

                Connection.SendLogMessage(LogLevel.Info, LoggerName, "Successfully created and joined a solo party");

                Connection.SendLogMessage(LogLevel.Info, LoggerName, $"Sending a join request for a deployment with tag {deploymentTag}");
                var op = gatewayClient.Join(new JoinRequest
                {
                    /* A client must provide a matchmaking type when send a join request and matchers retrieve waiting parties by a specified matchmaking type.
                     * You can think of this as a namespace - this allows you to use the gateway service for multiple match types.
                     * E.g. you could have types "Free For All" and "Team Deathmatch" and different matchers to service them. */
                    MatchmakingType = "match",
                    Metadata = {{"deployment_tag", deploymentTag}}
                }, pitMetadata);

                DateTime timeBeforeQueue = DateTime.Now;
                int millisecondBetweenPrints = 60000;
                int currentMillisecondBetweenPrints = millisecondBetweenPrints;
                while (!op.Done)
                {
                    Thread.Sleep(1000);
                    currentMillisecondBetweenPrints += 1000;
                    if (currentMillisecondBetweenPrints > millisecondBetweenPrints)
                    {
                        currentMillisecondBetweenPrints = 0;
                        Connection.SendLogMessage(LogLevel.Info, LoggerName, "Waiting...");
                    }
                    op = gatewayOpsClient.GetOperation(op.Name, CallSettings.FromHeader(PitRequestHeaderName, pit));
                }
                Connection.SendLogMessage(LogLevel.Info, LoggerName, $"{clientName} spent {(DateTime.Now - timeBeforeQueue).Milliseconds} ms waiting for op.");

                if (op.Error != null)
                {
                    Connection.SendLogMessage(LogLevel.Info, LoggerName, $"Failed to find a match: {op.Error}");
                    Connection.SendLogMessage(LogLevel.Info, LoggerName, $"This is likely because there is no running deployment with tag {deploymentTag}");
                    return ErrorExitStatus;
                }
                else
                {
                    Connection.SendLogMessage(LogLevel.Info, LoggerName, $"Found a match!");
                    var assignment = op.Response.Unpack<JoinResponse>();
                    Connection.SendLogMessage(LogLevel.Info, LoggerName, $"Deployment name: {assignment.DeploymentName}");
                    Connection.SendLogMessage(LogLevel.Info, LoggerName, $"Deployment Login Token: {assignment.LoginToken}");
                    loginToken = assignment.LoginToken;
                }
                // OLD
//                var loginTokens = Authentication.GetDevelopmentLoginTokens(SimulatedPlayerWorkerType, pit);
//                loginToken = Authentication.SelectLoginToken(loginTokens, targetDeployment);
            }
            catch (Exception e)
            {
                Connection.SendLogMessage(LogLevel.Error, LoggerName, $"Failed to launch simulated player: {e.Message}");
                return ErrorExitStatus;
            }

            ReplacePlaceholderArgs(simulatedPlayerArgs, clientName, loginToken, pit);

            // Prepend the simulated player id as an argument to the start client script.
            // This argument is consumed by the start client script and will not be passed to the client worker.
            simulatedPlayerArgs = new string[] { clientName }.Concat(simulatedPlayerArgs).ToArray();

            // Start the client
            Connection.SendLogMessage(LogLevel.Info, LoggerName, "Starting worker " + clientName + " with args: " + ArgsToString(simulatedPlayerArgs));
            StartClient(string.Join(" ", simulatedPlayerArgs));

            // Wait for client to exit
            WaitForClientsToExit();

            return 1;
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

        private static void WaitForTargetDeploymentReady()
        {
            Connection.SendLogMessage(LogLevel.Info, LoggerName, "Waiting for target deployment to become ready");
            while (true)
            {
                var readyFlagOpt = Connection.GetWorkerFlag(TARGET_DEPLOYMENT_READY_FLAG);
                if (readyFlagOpt == "true")
                {
                    // Ready.
                    Connection.SendLogMessage(LogLevel.Info, LoggerName, "Target deployment is ready");
                    break;
                }

                // Poll every 5 seconds.
                Thread.Sleep(5_000);
            }
        }

        private static void StartClient(string args)
        {
            try
            {
                SimulatedPlayerList.Push(Process.Start(SimulatedPlayerFilename, args));
            }
            catch (Exception e)
            {
                Connection.SendLogMessage(LogLevel.Error, LoggerName, "Exception from starting simulated player: " + e.Message);
            }
        }

        private static void WaitForClientsToExit()
        {
            while (SimulatedPlayerList.Count > 0)
            {
                try
                {
                    SimulatedPlayerList.Pop().WaitForExit();
                    Connection.SendLogMessage(LogLevel.Info, LoggerName, "Simulated Player process has ended - harakiri.");
                }
                catch (Exception e)
                {
                    Logger.WriteError("Error while waiting for simulated player to exit: " + e.Message);
                }
            }
        }

        private static string ArgsToString(string[] args)
        {
            return string.Join(" ", args);
        }

        private static string GetRequiredWorkerFlag(string flagName)
        {
            var flagValue = Connection.GetWorkerFlag(flagName);
            if (!flagValue.HasValue)
            {
                throw new Exception($"Missing worker flag `{flagName}`.");
            }

            return flagValue.Value;
        }

        private static void ReplacePlaceholderArgs(string[] args, string clientName, string loginToken, string pit)
        {
            for (int j = 0; j < args.Length; j++)
            {
                if (args[j] == WORKER_NAME_ARG)
                {
                    args[j] = clientName;
                }
                else if (args[j] == LOGIN_TOKEN_ARG)
                {
                    args[j] = loginToken;
                }
                else if (args[j] == PLAYER_IDENTITY_TOKEN_ARG)
                {
                    args[j] = pit;
                }
            }
        }
    }
}
