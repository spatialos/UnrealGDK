    // Copyright (c) Improbable Worlds Ltd, All Rights Reserved

using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading;
using Google.Api.Gax.Grpc;
using Google.LongRunning;
using Grpc.Core;
using Improbable.Collections;
using Improbable.OnlineServices.Proto.Gateway;
using Improbable.OnlineServices.Proto.Party;
using Improbable.SpatialOS.Platform.Common;
using Improbable.SpatialOS.PlayerAuth.V2Alpha1;
using Improbable.Worker;

namespace Improbable.WorkerCoordinator
{
    /// <summary>
    /// Worker coordinator that connects and runs simulated players.
    /// The coordinator runs as a managed worker inside a hosting deployment (i.e. the simulated player deployment).
    /// </summary>
    internal class ManagedWorkerCoordinator : AbstractWorkerCoordinator
    {
        private const string PitRequestHeaderName = "player-identity-token";

        // Arguments for the coordinator.
        private const string SimulatedPlayerSpawnCountArg = "simulated_player_spawn_count";
        private const string InitialStartDelayArg = "coordinator_start_delay_millis";

        // Worker flags.
        private const string DevAuthTokenWorkerFlag = "simulated_players_dev_auth_token";
        private const string TargetDeploymentWorkerFlag = "simulated_players_target_deployment";
        private const string DeploymentTotalNumSimulatedPlayersWorkerFlag = "total_num_simulated_players";
        private const string TargetDeploymentReadyWorkerFlag = "target_deployment_ready";

        // Matchmaking
        private const string SPATIAL_REFRESH_TOKEN_FLAG = "spatial_refresh_token";
        private const string PARTY_SERVER_TARGER_FLAG = "target_party_server";
        private const string GATEWAY_SERVER_TARGER_FLAG = "target_gateway_server";
        private const string DEPLOYMENT_TAG = "deployment_tag";

        private const int AverageDelayMillisBetweenConnections = 1500;
        private const int PollTargetDeploymentReadyIntervalMillis = 5000;

        // Argument placeholders for simulated players - these will be replaced by the coordinator by their actual values.
        private const string SimulatedPlayerWorkerNamePlaceholderArg = "<IMPROBABLE_SIM_PLAYER_WORKER_ID>";
        private const string PlayerIdentityTokenPlaceholderArg = "<PLAYER_IDENTITY_TOKEN>";
        private const string LoginTokenPlaceholderArg = "<LOGIN_TOKEN>";

        private const string CoordinatorWorkerType = "SimulatedPlayerCoordinator";
        private const string SimulatedPlayerWorkerType = "UnrealClient";
        private const string SimulatedPlayerFilename = "StartSimulatedClient.sh";

        private static Random Random;

        // Coordinator options.
        private string ReceptionistHost;
        private ushort ReceptionistPort;
        private string CoordinatorWorkerId;
        private int NumSimulatedPlayersToStart;
        private int InitialStartDelayMillis;
        private string[] SimulatedPlayerArgs;

        /// <summary>
        /// The arguments to start the coordinator must begin with:
        ///     receptionist
        ///     {hostname}                              Receptionist hostname
        ///     {port}                                  Receptionist port
        ///     {worker_id}                             Worker id of the coordinator
        ///
        /// Next, two optional arguments can be specified:
        ///     simulated_player_spawn_count={value}    Number of simulated player clients to start per coordinator (defaults to 1)
        ///     coordinator_start_delay_millis={value}  Minimum delay before the coordinator starts a simulated client, to prevent clients
        ///                                             from connecting too soon to the target deployment (defaults to 10000)
        ///
        /// All following arguments will be passed to the simulated player instance.
        /// These arguments can contain the following placeholders, which will be replaced by the coordinator:
        ///     `<IMPROBABLE_SIM_PLAYER_WORKER_ID>`     will be replaced with the worker id of the simulated player
        ///     `<LOGIN_TOKEN>`                         if a development auth token is specified as a worker flag, this will be replaced by the generated development auth login token
        ///     `<PLAYER_IDENTITY_TOKEN>`               if both a development auth token and a target deployment are specified through the worker flags, this will be replaced by the generated development auth player identity token
        ///
        /// WORKER FLAGS
        /// Additionally, the following worker flags are expected by the coordinator:
        ///     simulated_players_dev_auth_token        The development auth token used to generate login tokens and player identity tokens
        ///     simulated_players_target_deployment     Name of the target deployment which the simulated players will connect to
        ///     target_num_simulated_players            The total number of simulated players that will connect to the target deployment. This value is used to determine the delay in connecting simulated players.
        /// </summary>
        public static ManagedWorkerCoordinator FromArgs(Logger logger, string[] args)
        {
            // Keeps track of the number of arguments used for the coordinator.
            // The first 4 arguments are for connecting to the Receptionist.
            // 2 optional arguments can be provided after the Receptionist args to
            // modify the default options of the coordinator.
            var numArgsToSkip = 4;

            // Optional args with default values.
            var numSimulatedPlayersToStart = 1;
            var initialStartDelayMillis = 10000;
            if (Util.HasIntegerArgument(args, SimulatedPlayerSpawnCountArg))
            {
                numSimulatedPlayersToStart = Util.GetIntegerArgument(args, SimulatedPlayerSpawnCountArg);
                numArgsToSkip++;
            }
            if (Util.HasIntegerArgument(args, InitialStartDelayArg))
            {
                initialStartDelayMillis = Util.GetIntegerArgument(args, InitialStartDelayArg);
                numArgsToSkip++;
            }

            return new ManagedWorkerCoordinator(logger)
            {
                // Receptionist args.
                ReceptionistHost = args[1],
                ReceptionistPort = Convert.ToUInt16(args[2]),
                CoordinatorWorkerId = args[3],

                // Coordinator options.
                NumSimulatedPlayersToStart = numSimulatedPlayersToStart,
                InitialStartDelayMillis = initialStartDelayMillis,

                // Remove arguments that are only for the coordinator.
                SimulatedPlayerArgs = args.Skip(numArgsToSkip).ToArray()
            };
        }

        private ManagedWorkerCoordinator(Logger logger) : base(logger)
        {
            Random = new Random(Guid.NewGuid().GetHashCode());
        }

        public override void Run()
        {
            string partyServerTarget, gatewayServerTarget, deploymentTag, refreshToken;

            var connection = CoordinatorConnection.ConnectAndKeepAlive(Logger, ReceptionistHost, ReceptionistPort, CoordinatorWorkerId, CoordinatorWorkerType);

            // Read worker flags.
            partyServerTarget = connection.GetWorkerFlag(PARTY_SERVER_TARGER_FLAG).Value;
            gatewayServerTarget = connection.GetWorkerFlag(GATEWAY_SERVER_TARGER_FLAG).Value;
            deploymentTag = connection.GetWorkerFlag(DEPLOYMENT_TAG).Value;

            Option<string> devAuthTokenOpt = connection.GetWorkerFlag(DevAuthTokenWorkerFlag);
            Option<string> targetDeploymentOpt = connection.GetWorkerFlag(TargetDeploymentWorkerFlag);
            int deploymentTotalNumSimulatedPlayers = int.Parse(GetWorkerFlagOrDefault(connection, DeploymentTotalNumSimulatedPlayersWorkerFlag, "100"));

            Logger.WriteLog("Waiting for target deployment to become ready.");
            WaitForTargetDeploymentReady(connection);

            Logger.WriteLog($"Target deployment is ready. Starting {NumSimulatedPlayersToStart} simulated players.");
            Thread.Sleep(InitialStartDelayMillis);

            var maxDelayMillis = deploymentTotalNumSimulatedPlayers * AverageDelayMillisBetweenConnections;

            // Distribute player connections uniformly by generating a random time to connect between now and maxDelayMillis,
            // such that, on average, a player connects every AverageDelayMillisBetweenConnections milliseconds deployment-wide.
            // There can be multiple coordinator workers per deployment, to ensure that the combined connections created by all
            // coordinators are spread out uniformly, generate a start delay for each player independently of other players' start delays.
            var startDelaysMillis = new int[NumSimulatedPlayersToStart];
                for (int i = 0; i < NumSimulatedPlayersToStart; i++)
                {
                    startDelaysMillis[i] = Random.Next(maxDelayMillis);
                }

                Array.Sort(startDelaysMillis);
                for (int i = 0; i < NumSimulatedPlayersToStart; i++)
                {
                    string clientName = "SimulatedPlayer" + Guid.NewGuid();
                    var timeToSleep = startDelaysMillis[i];
                    if (i > 0)
                    {
                        timeToSleep -= startDelaysMillis[i - 1];
                    }

                    Thread.Sleep(timeToSleep);
                    StartSimulatedPlayer(clientName, devAuthTokenOpt, targetDeploymentOpt, partyServerTarget, gatewayServerTarget, deploymentTag);
                }

                // Wait for all clients to exit.
                WaitForPlayersToExit();
            }

            private void StartSimulatedPlayer(string simulatedPlayerName, Option<string> devAuthTokenOpt,
                Option<string> targetDeploymentOpt, string partyServerTarget, string gatewayServerTarget, string deploymentTag)
            {
                // Create player identity token and login token
                Metadata pitMetadata;
                string pit, loginToken;

                try
                {
                    string clientName = "SimulatedPlayer" + Guid.NewGuid();

                    ChannelCredentials partyCredentials = ChannelCredentials.Insecure;
                    ChannelCredentials gatewayCredentials = ChannelCredentials.Insecure;
                    var partyClient =
                        new PartyService.PartyServiceClient(new Channel(partyServerTarget, partyCredentials));
                    var gatewayClient = new GatewayService.GatewayServiceClient(
                        new Channel(gatewayServerTarget, gatewayCredentials));
                    /* We use the ops client to check the state of our request. */

                    var gatewayOpsClient =
                        OperationsClient.Create(new Channel(gatewayServerTarget, gatewayCredentials));

                    pit = Authentication.GetDevelopmentPlayerIdentityToken(devAuthTokenOpt.Value, clientName);

                    Logger.WriteLog($"PIT created: {pit}");

                    pitMetadata = new Metadata {{PitRequestHeaderName, pit}};
                    Logger.WriteLog("Creating and joining a solo party");
                    partyClient.CreateParty(new CreatePartyRequest
                    {
                        MinMembers = 1,
                        MaxMembers = 1
                    }, pitMetadata);

                    Logger.WriteLog("Successfully created and joined a solo party");

                    Logger.WriteLog($"Sending a join request for a deployment with tag {deploymentTag}");
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
                            Logger.WriteLog("Waiting...");
                        }

                        op = gatewayOpsClient.GetOperation(op.Name, CallSettings.FromHeader(PitRequestHeaderName, pit));
                    }

                    Logger.WriteLog(
                        $"{clientName} spent {(DateTime.Now - timeBeforeQueue).Milliseconds} ms waiting for op.");

                    if (op.Error != null)
                    {
                        Logger.WriteError($"Failed to find a match: {op.Error}");
                        Logger.WriteError(
                            $"This is likely because there is no running deployment with tag {deploymentTag}");
                        return;
                    }
                    else
                    {
                        Logger.WriteLog($"Found a match!");
                        var assignment = op.Response.Unpack<JoinResponse>();
                        Logger.WriteLog($"Deployment name: {assignment.DeploymentName}");
                        Logger.WriteLog($"Deployment Login Token: {assignment.LoginToken}");
                        loginToken = assignment.LoginToken;
                    }

                    string[] simulatedPlayerArgs = Util.ReplacePlaceholderArgs(SimulatedPlayerArgs,
                        new Dictionary<string, string>()
                        {
                            {SimulatedPlayerWorkerNamePlaceholderArg, simulatedPlayerName},
                            {PlayerIdentityTokenPlaceholderArg, pit},
                            {LoginTokenPlaceholderArg, loginToken}
                        });

                    // Prepend the simulated player id as an argument to the start client script.
                    // This argument is consumed by the start client script and will not be passed to the client worker.
                    simulatedPlayerArgs = new string[] {simulatedPlayerName}.Concat(simulatedPlayerArgs).ToArray();

                    // Start the client
                    string flattenedArgs = string.Join(" ", simulatedPlayerArgs);
                    Logger.WriteLog($"Starting simulated player {simulatedPlayerName} with args: {flattenedArgs}");
                    CreateSimulatedPlayerProcess(SimulatedPlayerFilename, flattenedArgs);
                    ;
                }
                catch (Exception e)
                {
                    Logger.WriteError($"Failed to start simulated player: {e.Message}");
                }
            }

        private static string GetWorkerFlagOrDefault(Connection connection, string flagName, string defaultValue)
        {
            Option<string> flagValue = connection.GetWorkerFlag(flagName);
            if (flagValue.HasValue)
            {
                return flagValue.Value;
            }

            return defaultValue;
        }

        private void WaitForTargetDeploymentReady(Connection connection)
        {
            while (true)
            {
                var readyFlagOpt = connection.GetWorkerFlag(TargetDeploymentReadyWorkerFlag);
                if (readyFlagOpt == "true")
                {
                    // Ready.
                    break;
                }

                Thread.Sleep(PollTargetDeploymentReadyIntervalMillis);
            }
        }
    }
}
