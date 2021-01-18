// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

using Google.LongRunning;
using Google.Protobuf.WellKnownTypes;
using Improbable.SpatialOS.Deployment.V1Alpha1;
using Improbable.SpatialOS.Platform.Common;
using Improbable.SpatialOS.PlayerAuth.V2Alpha1;
using Improbable.SpatialOS.Snapshot.V1Alpha1;
using Newtonsoft.Json.Linq;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net;
using System.Security.Cryptography;
using System;
using System.Threading.Tasks;

namespace Improbable
{
    internal class DeploymentLauncher
    {
        private const string SIM_PLAYER_DEPLOYMENT_TAG = "simulated_players";
        private const string DEPLOYMENT_LAUNCHED_BY_LAUNCHER_TAG = "unreal_deployment_launcher";

        private const string CoordinatorWorkerName = "SimulatedPlayerCoordinator";

        private const string CHINA_ENDPOINT_URL = "platform.api.spatialoschina.com";
        private const int CHINA_ENDPOINT_PORT = 443;

        private static readonly string ChinaProductionTokenPath = Path.Combine(Environment.ExpandEnvironmentVariables("%LOCALAPPDATA%"), ".improbable/oauth2/oauth2_refresh_token_cn-production");

        // Populated in the Main method if the Chinese platform is to be used
        private static string ChinaRefreshToken = String.Empty;
        private static PlatformRefreshTokenCredential ChinaCredentials;

        private static string GetConsoleHost(bool useChinaPlatform)
        {
            return useChinaPlatform ? "console.spatialoschina.com" : "console.improbable.io";
        }

        private static string UploadSnapshot(SnapshotServiceClient client, string snapshotPath, string projectName,
            string deploymentName, bool useChinaPlatform)
        {
            Console.WriteLine($"Uploading {snapshotPath} to project {projectName}");

            // Read snapshot.
            var bytes = File.ReadAllBytes(snapshotPath);

            if (bytes.Length == 0)
            {
                Console.Error.WriteLine($"Unable to load {snapshotPath}. Does the file exist?");
                return string.Empty;
            }

            // Create HTTP endpoint to upload to.
            var snapshotToUpload = new Snapshot
            {
                ProjectName = projectName,
                DeploymentName = deploymentName
            };

            using (var md5 = MD5.Create())
            {
                snapshotToUpload.Checksum = Convert.ToBase64String(md5.ComputeHash(bytes));
                snapshotToUpload.Size = bytes.Length;
            }

            var uploadSnapshotResponse =
                client.UploadSnapshot(new UploadSnapshotRequest { Snapshot = snapshotToUpload });
            snapshotToUpload = uploadSnapshotResponse.Snapshot;

            // Upload content.
            var httpRequest = WebRequest.Create(uploadSnapshotResponse.UploadUrl) as HttpWebRequest;
            httpRequest.Method = "PUT";
            httpRequest.ContentLength = snapshotToUpload.Size;
            httpRequest.Headers.Add("Content-MD5", snapshotToUpload.Checksum);

            if (useChinaPlatform)
            {
                httpRequest.Headers.Add("x-amz-server-side-encryption", "AES256");
            }

            using (var dataStream = httpRequest.GetRequestStream())
            {
                dataStream.Write(bytes, 0, bytes.Length);
            }

            // Block until we have a response.
            httpRequest.GetResponse();

            // Confirm that the snapshot was uploaded successfully.
            var confirmUploadResponse = client.ConfirmUpload(new ConfirmUploadRequest
            {
                DeploymentName = snapshotToUpload.DeploymentName,
                Id = snapshotToUpload.Id,
                ProjectName = snapshotToUpload.ProjectName
            });

            return confirmUploadResponse.Snapshot.Id;
        }

        private static PlatformApiEndpoint GetApiEndpoint(bool useChinaPlatform)
        {
            if (useChinaPlatform)
            {
                return new PlatformApiEndpoint(CHINA_ENDPOINT_URL, CHINA_ENDPOINT_PORT);
            }
            return null; // Use default
        }

        private static PlatformRefreshTokenCredential GetPlatformRefreshTokenCredential(bool useChinaPlatform)
        {
            return useChinaPlatform ? ChinaCredentials : null;
        }

        private static int CreateDeployment(string[] args, bool useChinaPlatform)
        {
            // Argument count can vary because of optional arguments
            bool launchSimPlayerDeployment = args.Length == 16 || args.Length == 15;

            var projectName = args[1];
            var assemblyName = args[2];
            var runtimeVersion = args[3];
            var mainDeploymentName = args[4];
            var mainDeploymentJsonPath = args[5];
            var mainDeploymentSnapshotPath = args[6];
            var mainDeploymentRegion = args[7];
            var mainDeploymentCluster = args[8];
            var mainDeploymentTags = args[9];

            var simDeploymentBaseName = string.Empty;
            var simDeploymentJson = string.Empty;
            var simDeploymentRegion = string.Empty;
            var simDeploymentCluster = string.Empty;
            var numSimPlayers = 0;
            var maxPlayersPerDeployment = -1; // Will be initialized to numSimPlayers

            if (launchSimPlayerDeployment)
            {
                simDeploymentBaseName = args[10];
                simDeploymentJson = args[11];
                simDeploymentRegion = args[12];
                simDeploymentCluster = args[13];

                if (!Int32.TryParse(args[14], out numSimPlayers))
                {
                    Console.WriteLine("Cannot parse the number of simulated players to connect.");
                    return 1;
                }
                else if (numSimPlayers <= 0)
                {
                    Console.WriteLine("The number of players must be positive.");
                    return 1;
                }

                // Start a single deployment by default
                maxPlayersPerDeployment = numSimPlayers;
                if (args.Length >= 16)
                {
                    if (!Int32.TryParse(args[15], out maxPlayersPerDeployment))
                    {
                        Console.WriteLine("Cannot parse the maximum number of simulated players per deployment.");
                        return 1;
                    }
                    else if (maxPlayersPerDeployment <= 0)
                    {
                        Console.WriteLine("The maximum number of simulated players per deployment must be positive.");
                        return 1;
                    }
                }
            }

            try
            {
                var deploymentServiceClient = DeploymentServiceClient.Create(GetApiEndpoint(useChinaPlatform), GetPlatformRefreshTokenCredential(useChinaPlatform));

                if (DeploymentExists(deploymentServiceClient, projectName, mainDeploymentName))
                {
                    StopDeploymentByName(deploymentServiceClient, projectName, mainDeploymentName);
                }

                var createMainDeploymentOp = CreateMainDeploymentAsync(deploymentServiceClient,
                    launchSimPlayerDeployment, projectName, assemblyName, runtimeVersion,
                    mainDeploymentName, mainDeploymentJsonPath, mainDeploymentSnapshotPath,
                    mainDeploymentRegion, mainDeploymentCluster, mainDeploymentTags, useChinaPlatform);

                if (!launchSimPlayerDeployment)
                {
                    // Don't launch a simulated player deployment. Wait for main deployment to be created and then return.
                    Console.WriteLine("Waiting for the main deployment to be ready...");
                    var result = createMainDeploymentOp.PollUntilCompleted().GetResultOrNull();
                    if (result == null)
                    {
                        Console.WriteLine("Failed to create the main deployment");
                        return 1;
                    }

                    Console.WriteLine("Successfully created the main deployment");
                    return 0;
                }

                // We are using the main deployment snapshot also for the sim player deployment(s), because we only need to specify a snapshot
                // to be able to start the deployment. The sim players don't care about the actual snapshot.
                var simDeploymentCreationOps = CreateSimPlayerDeploymentsAsync(deploymentServiceClient,
                    projectName, assemblyName, runtimeVersion, mainDeploymentName, simDeploymentBaseName,
                    simDeploymentJson, mainDeploymentSnapshotPath, simDeploymentRegion, simDeploymentCluster,
                    numSimPlayers, maxPlayersPerDeployment, useChinaPlatform);

                if (simDeploymentCreationOps == null || simDeploymentCreationOps.Count == 0)
                {
                    Console.WriteLine("Failed to start any simulated player deployments.");
                    return 1;
                }

                // Wait for the main deployment to be ready
                Console.WriteLine("Waiting for the main deployment to be ready...");
                var mainDeploymentResult = createMainDeploymentOp.PollUntilCompleted().GetResultOrNull();
                if (mainDeploymentResult == null)
                {
                    Console.WriteLine("Failed to create the main deployment");
                    return 1;
                }
                Console.WriteLine("Successfully created the main deployment");

                // Waiting for the simulated player deployment(s) to be ready
                var numSuccessfullyStartedSimDeployments = 0;
                for (var simDeploymentIndex = 0; simDeploymentIndex < simDeploymentCreationOps.Count; simDeploymentIndex++)
                {
                    var deploymentDescription = $"(deployment {simDeploymentIndex + 1}/{simDeploymentCreationOps.Count})";
                    Console.WriteLine($"Waiting for the simulated player deployment to be ready... {deploymentDescription}");

                    var simPlayerDeployment = simDeploymentCreationOps[simDeploymentIndex].PollUntilCompleted().GetResultOrNull();
                    if (simPlayerDeployment == null)
                    {
                        Console.WriteLine($"Failed to create the simulated player deployment {deploymentDescription}");
                        continue;
                    }

                    Console.WriteLine($"Deployment startup complete!");

                    numSuccessfullyStartedSimDeployments++;
                }

                Console.WriteLine($"Successfully started {numSuccessfullyStartedSimDeployments} out of {simDeploymentCreationOps.Count} simulated player deployments.");
            }
            catch (Grpc.Core.RpcException e)
            {
                if (e.Status.StatusCode == Grpc.Core.StatusCode.NotFound)
                {
                    Console.WriteLine($"Unable to launch the deployment(s). This is likely because the project '{projectName}' or assembly '{assemblyName}' doesn't exist.");
                    Console.WriteLine($"Detail: '{e.Status.Detail}'");
                }
                else if (e.Status.StatusCode == Grpc.Core.StatusCode.ResourceExhausted)
                {
                    Console.WriteLine($"Unable to launch the deployment(s). Cloud cluster resources exhausted, Detail: '{e.Status.Detail}'" );
                }
                else
                {
                    Console.WriteLine($"Unable to launch the deployment(s). Detail: '{e.Status.Detail}'");
                }

                return 1;
            }

            return 0;
        }

        private static int CreateSimDeployments(string[] args, bool useChinaPlatform)
        {
            var projectName = args[1];
            var assemblyName = args[2];
            var runtimeVersion = args[3];
            var targetDeploymentName = args[4];
            var simDeploymentBaseName = args[5];
            var simDeploymentJson = args[6];
            var simDeploymentRegion = args[7];
            var simDeploymentCluster = args[8];
            var simDeploymentSnapshotPath = args[9];
            var numSimplayers = 0;
            if (!Int32.TryParse(args[10], out numSimplayers))
            {
                Console.WriteLine("Cannot parse the number of simulated players to connect.");
                return 1;
            }
            else if (numSimplayers <= 0)
            {
                Console.WriteLine("The number of players must be positive.");
                return 1;
            }

            var autoConnect = false;
            if (!Boolean.TryParse(args[12], out autoConnect))
            {
                Console.WriteLine("Cannot parse the auto-connect flag.");
                return 1;
            }

            var maxPlayersPerDeployment = numSimplayers;
            if (args.Length >= 12)
            {
                if (!Int32.TryParse(args[11], out maxPlayersPerDeployment))
                {
                    Console.WriteLine("Cannot parse the maximum number of simulated players per deployments.");
                    return 1;
                }
                else if (maxPlayersPerDeployment <= 0)
                {
                    Console.WriteLine("The maximum number of simulated players per deployment must be positive.");
                    return 1;
                }
            }

            var deploymentServiceClient = DeploymentServiceClient.Create(GetApiEndpoint(useChinaPlatform));

            var simDeploymentCreationOps = CreateSimPlayerDeploymentsAsync(deploymentServiceClient,
                projectName, assemblyName, runtimeVersion, targetDeploymentName, simDeploymentBaseName,
                simDeploymentJson, simDeploymentSnapshotPath, simDeploymentRegion, simDeploymentCluster,
                numSimplayers, maxPlayersPerDeployment, useChinaPlatform);

            if (simDeploymentCreationOps == null || simDeploymentCreationOps.Count == 0)
            {
                Console.WriteLine("Failed to start any simulated player deployments.");
                return 1;
            }

            var numSuccessfullyStartedDeployments = 0;
            for (var simDeploymentIndex = 0; simDeploymentIndex < simDeploymentCreationOps.Count; simDeploymentIndex++)
            {
                var deploymentDescription = $"(deployment {simDeploymentIndex + 1}/{simDeploymentCreationOps.Count})";
                Console.WriteLine($"Waiting for the simulated player deployment to be ready... {deploymentDescription}");

                var simPlayerDeployment = simDeploymentCreationOps[simDeploymentIndex].PollUntilCompleted().GetResultOrNull();
                if (simPlayerDeployment == null)
                {
                    Console.WriteLine($"Failed to create the simulated player deployment {deploymentDescription}");
                    continue;
                }

                Console.WriteLine($"Deployment startup complete!");

                numSuccessfullyStartedDeployments++;
            }

            Console.WriteLine($"Successfully started {numSuccessfullyStartedDeployments} out of {simDeploymentCreationOps.Count} simulated player deployments.");

            return 0;
        }

        // Determines the name for a simulated player deployment. The first index is assumed to be 1.
        private static string GetSimDeploymentName(string baseName, int index)
        {
            if (index == 1)
            {
                return baseName;
            }

            return baseName + ("_" + index);
        }

        private static bool DeploymentExists(DeploymentServiceClient deploymentServiceClient, string projectName,
            string deploymentName)
        {
            var activeDeployments = ListLaunchedActiveDeployments(deploymentServiceClient, projectName);

            return activeDeployments.FirstOrDefault(d => d.Name == deploymentName) != null;
        }

        private static void StopDeploymentByName(DeploymentServiceClient deploymentServiceClient, string projectName,
            string deploymentName)
        {
            var activeDeployments = ListLaunchedActiveDeployments(deploymentServiceClient, projectName);

            var deployment = activeDeployments.FirstOrDefault(d => d.Name == deploymentName);

            if (deployment == null)
            {
                Console.WriteLine($"Unable to stop the deployment {deployment.Name} because it can't be found or isn't running.");
                return;
            }

            Console.WriteLine($"Stopping active deployment by name: {deployment.Name}");

            deploymentServiceClient.StopDeployment(new StopDeploymentRequest
            {
                Id = deployment.Id,
                ProjectName = projectName
            });
        }

        private static Operation<Deployment, CreateDeploymentMetadata> CreateMainDeploymentAsync(DeploymentServiceClient deploymentServiceClient,
            bool launchSimPlayerDeployment, string projectName, string assemblyName, string runtimeVersion,
            string mainDeploymentName, string mainDeploymentJsonPath, string mainDeploymentSnapshotPath,
            string regionCode, string clusterCode, string deploymentTags, bool useChinaPlatform)
        {
            var snapshotServiceClient = SnapshotServiceClient.Create(GetApiEndpoint(useChinaPlatform), GetPlatformRefreshTokenCredential(useChinaPlatform));

            // Upload snapshots.
            var mainSnapshotId = UploadSnapshot(snapshotServiceClient, mainDeploymentSnapshotPath, projectName,
                mainDeploymentName, useChinaPlatform);

            if (mainSnapshotId.Length == 0)
            {
                throw new Exception("Error while uploading snapshot.");
            }

            // Create main deployment.
            var mainDeploymentConfig = new Deployment
            {
                AssemblyId = assemblyName,
                LaunchConfig = new LaunchConfig
                {
                    ConfigJson = File.ReadAllText(mainDeploymentJsonPath)
                },
                Name = mainDeploymentName,
                ProjectName = projectName,
                StartingSnapshotId = mainSnapshotId,
                RuntimeVersion = runtimeVersion
            };

            if (!String.IsNullOrEmpty(clusterCode))
            {
                mainDeploymentConfig.ClusterCode = clusterCode;
            }
            else
            {
                mainDeploymentConfig.RegionCode = regionCode;
            }

            mainDeploymentConfig.Tag.Add(DEPLOYMENT_LAUNCHED_BY_LAUNCHER_TAG);
            foreach (String tag in deploymentTags.Split(' '))
            {
                if (tag.Length > 0)
                {
                    mainDeploymentConfig.Tag.Add(tag);
                }
            }

            if (launchSimPlayerDeployment)
            {
                // This tag needs to be added to allow simulated players to connect using login
                // tokens generated with anonymous auth.
                mainDeploymentConfig.Tag.Add("dev_login");
            }

            Console.WriteLine(
                $"Creating the main deployment {mainDeploymentName} in project {projectName} with snapshot ID {mainSnapshotId}. Link: https://{GetConsoleHost(useChinaPlatform)}/projects/{projectName}/deployments/{mainDeploymentName}/overview");

            var mainDeploymentCreateOp = deploymentServiceClient.CreateDeployment(new CreateDeploymentRequest
            {
                Deployment = mainDeploymentConfig
            });

            return mainDeploymentCreateOp;
        }

        private static Operation<Deployment, CreateDeploymentMetadata> CreateSimPlayerDeploymentAsync(DeploymentServiceClient deploymentServiceClient,
            string projectName, string assemblyName, string runtimeVersion, string mainDeploymentName, string simDeploymentName,
            string simDeploymentJsonPath, string simDeploymentSnapshotPath, string regionCode, string clusterCode, int numSimPlayers, bool useChinaPlatform)
        {
            var snapshotServiceClient = SnapshotServiceClient.Create(GetApiEndpoint(useChinaPlatform), GetPlatformRefreshTokenCredential(useChinaPlatform));

            // Upload snapshots.
            var simDeploymentSnapshotId = UploadSnapshot(snapshotServiceClient, simDeploymentSnapshotPath, projectName,
                simDeploymentName, useChinaPlatform);

            if (simDeploymentSnapshotId.Length == 0)
            {
                throw new Exception("Error while uploading sim player snapshot.");
            }

            var playerAuthServiceClient = PlayerAuthServiceClient.Create(GetApiEndpoint(useChinaPlatform), GetPlatformRefreshTokenCredential(useChinaPlatform));

            // Create development authentication token used by the simulated players.
            var dat = playerAuthServiceClient.CreateDevelopmentAuthenticationToken(
                new CreateDevelopmentAuthenticationTokenRequest
                {
                    Description = "DAT for simulated player deployment.",
                    Lifetime = Duration.FromTimeSpan(new TimeSpan(7, 0, 0, 0)),
                    ProjectName = projectName
                });

            // Add worker flags to sim deployment JSON.
            var devAuthTokenFlag = new JObject();
            devAuthTokenFlag.Add("name", "simulated_players_dev_auth_token");
            devAuthTokenFlag.Add("value", dat.TokenSecret);

            var targetDeploymentFlag = new JObject();
            targetDeploymentFlag.Add("name", "simulated_players_target_deployment");
            targetDeploymentFlag.Add("value", mainDeploymentName);

            var numSimulatedPlayersFlag = new JObject();
            numSimulatedPlayersFlag.Add("name", "total_num_simulated_players");
            numSimulatedPlayersFlag.Add("value", $"{numSimPlayers}");

            var simDeploymentConfigJson = File.ReadAllText(simDeploymentJsonPath);
            dynamic simDeploymentConfig = JObject.Parse(simDeploymentConfigJson);

            if (simDeploymentJsonPath.EndsWith(".pb.json"))
            {
                for (var i = 0; i < simDeploymentConfig.worker_flagz.Count; ++i)
                {
                    if (simDeploymentConfig.worker_flagz[i].worker_type == CoordinatorWorkerName)
                    {
                        simDeploymentConfig.worker_flagz[i].flagz.Add(devAuthTokenFlag);
                        simDeploymentConfig.worker_flagz[i].flagz.Add(targetDeploymentFlag);
                        simDeploymentConfig.worker_flagz[i].flagz.Add(numSimulatedPlayersFlag);
                        break;
                    }
                }

                for (var i = 0; i < simDeploymentConfig.flagz.Count; ++i)
                {
                    if (simDeploymentConfig.flagz[i].name == "loadbalancer_v2_config_json")
                    {
                        string layerConfigJson = simDeploymentConfig.flagz[i].value;
                        dynamic loadBalanceConfig = JObject.Parse(layerConfigJson);
                        var lbLayerConfigurations = loadBalanceConfig.layerConfigurations;
                        for (var j = 0; j < lbLayerConfigurations.Count; ++j)
                        {
                            if (lbLayerConfigurations[j].layer == CoordinatorWorkerName)
                            {
                                var rectangleGrid = lbLayerConfigurations[j].rectangleGrid;
                                rectangleGrid.cols = numSimPlayers;
                                rectangleGrid.rows = 1;
                                break;
                            }
                        }
                        simDeploymentConfig.flagz[i].value = Newtonsoft.Json.JsonConvert.SerializeObject(loadBalanceConfig);
                        break;
                    }
                }
            }
            else // regular non pb.json
            {
                for (var i = 0; i < simDeploymentConfig.workers.Count; ++i)
                {
                    if (simDeploymentConfig.workers[i].worker_type == CoordinatorWorkerName)
                    {
                        simDeploymentConfig.workers[i].flags.Add(devAuthTokenFlag);
                        simDeploymentConfig.workers[i].flags.Add(targetDeploymentFlag);
                        simDeploymentConfig.workers[i].flags.Add(numSimulatedPlayersFlag);
                    }
                }

                // Specify the number of managed coordinator workers to start by editing
                // the load balancing options in the launch config. It creates a rectangular
                // launch config of N cols X 1 row, N being the number of coordinators
                // to create.
                // This assumes the launch config contains a rectangular load balancing
                // layer configuration already for the coordinator worker.
                var lbLayerConfigurations = simDeploymentConfig.load_balancing.layer_configurations;
                for (var i = 0; i < lbLayerConfigurations.Count; ++i)
                {
                    if (lbLayerConfigurations[i].layer == CoordinatorWorkerName)
                    {
                        var rectangleGrid = lbLayerConfigurations[i].rectangle_grid;
                        rectangleGrid.cols = numSimPlayers;
                        rectangleGrid.rows = 1;
                    }
                }
            }

            // Create simulated player deployment.
            var simDeployment = new Deployment
            {
                AssemblyId = assemblyName,
                LaunchConfig = new LaunchConfig
                {
                    ConfigJson = simDeploymentConfig.ToString()
                },
                Name = simDeploymentName,
                ProjectName = projectName,
                RuntimeVersion = runtimeVersion,
                StartingSnapshotId = simDeploymentSnapshotId,
            };

            if (!String.IsNullOrEmpty(clusterCode))
            {
                simDeployment.ClusterCode = clusterCode;
            }
            else
            {
                simDeployment.RegionCode = regionCode;
            }

            simDeployment.Tag.Add(DEPLOYMENT_LAUNCHED_BY_LAUNCHER_TAG);
            simDeployment.Tag.Add(SIM_PLAYER_DEPLOYMENT_TAG);

            Console.WriteLine(
                $"Creating the simulated player deployment {simDeploymentName} in project {projectName} with {numSimPlayers} simulated players. Link: https://{GetConsoleHost(useChinaPlatform)}/projects/{projectName}/deployments/{simDeploymentName}/overview");

            var simDeploymentCreateOp = deploymentServiceClient.CreateDeployment(new CreateDeploymentRequest
            {
                Deployment = simDeployment
            });

            return simDeploymentCreateOp;
        }

        private static List<Operation<Deployment, CreateDeploymentMetadata>> CreateSimPlayerDeploymentsAsync(DeploymentServiceClient deploymentServiceClient,
            string projectName, string assemblyName, string runtimeVersion, string mainDeploymentName, string simDeploymentBaseName,
            string simDeploymentJsonPath, string simDeploymentSnapshotPath, string regionCode, string clusterCode, int numSimPlayers, int maxPlayersPerDeployment, bool useChinaPlatform)
        {
            var operations = new List<Operation<Deployment, CreateDeploymentMetadata>>();

            var numSimDeployments = (int)Math.Ceiling(numSimPlayers / (double)maxPlayersPerDeployment);

            var longestName = GetSimDeploymentName(simDeploymentBaseName, numSimDeployments);
            if (longestName.Length > 32)
            {
                Console.WriteLine($"The deployment name may not exceed 32 characters. '{longestName}' would have {longestName.Length}.");
                return operations;
            }

            for (var simPlayerDeploymentId = 1; simPlayerDeploymentId <= numSimDeployments; ++simPlayerDeploymentId)
            {
                var simDeploymentName = GetSimDeploymentName(simDeploymentBaseName, simPlayerDeploymentId);

                try
                {
                    if (DeploymentExists(deploymentServiceClient, projectName, simDeploymentName))
                    {
                        StopDeploymentByName(deploymentServiceClient, projectName, simDeploymentName);
                    }

                    // Determine the amount of simulated players in this deployment
                    var numSimPlayersPerDeployment = numSimPlayers / numSimDeployments;
                    // Spread leftover simulated players over deployments if the total isn't a multiple of the deployment count
                    if (simPlayerDeploymentId <= numSimPlayers % numSimDeployments)
                    {
                        ++numSimPlayersPerDeployment;
                    }

                    Console.WriteLine($"Kicking off startup of deployment {simPlayerDeploymentId} out of the target {numSimDeployments}");
                    var createSimDeploymentOp = CreateSimPlayerDeploymentAsync(deploymentServiceClient,
                        projectName, assemblyName, runtimeVersion, mainDeploymentName, simDeploymentName,
                        simDeploymentJsonPath, simDeploymentSnapshotPath, regionCode, clusterCode, numSimPlayersPerDeployment, useChinaPlatform);

                    operations.Add(createSimDeploymentOp);
                }
                catch (Grpc.Core.RpcException e)
                {
                    if (e.Status.StatusCode == Grpc.Core.StatusCode.NotFound)
                    {
                        Console.WriteLine($"Unable to launch the deployment(s). This is likely because the project '{projectName}' or assembly '{assemblyName}' doesn't exist.");
                        Console.WriteLine($"Detail: '{e.Status.Detail}'");
                    }
                    else if (e.Status.StatusCode == Grpc.Core.StatusCode.ResourceExhausted)
                    {
                        Console.WriteLine($"Unable to launch the deployment(s). Cloud cluster resources exhausted, Detail: '{e.Status.Detail}'");
                    }
                    else
                    {
                        Console.WriteLine($"Unable to launch the deployment(s). Detail: '{e.Status.Detail}'");
                    }

                    Console.WriteLine($"No further deployments will be started. Initiated startup for {simPlayerDeploymentId - 1} out of the target {numSimDeployments} deployments.");

                    return operations;
                }
            }

            return operations;
        }

        private static int StopDeployments(string[] args, bool useChinaPlatform)
        {
            var projectName = args[1];

            var deploymentServiceClient = DeploymentServiceClient.Create(GetApiEndpoint(useChinaPlatform), GetPlatformRefreshTokenCredential(useChinaPlatform));

            var deploymentIdsToStop = new List<string>();

            if (args.Length == 3)
            {
                // Stop only the specified deployment.
                var deploymentId = args[2];
                deploymentIdsToStop.Add(deploymentId);
            }
            else
            {
                // Stop all active deployments launched by this launcher.
                var activeDeployments = ListLaunchedActiveDeployments(deploymentServiceClient, projectName);
                foreach (var deployment in activeDeployments)
                {
                    deploymentIdsToStop.Add(deployment.Id);
                }
            }

            var deploymentIdsToTasks = new Dictionary<string, Task>();
            var erroredDeploymentIds = new List<string>();

            Console.WriteLine($"Will stop {deploymentIdsToStop.Count()} deployment(s)");
            foreach (var deploymentId in deploymentIdsToStop)
            {
                deploymentIdsToTasks.Add(deploymentId, StopDeploymentByIdAsync(deploymentServiceClient, projectName, deploymentId));
            };

            try
            {
                Task.WaitAll(deploymentIdsToTasks.Values.ToArray());
            }
            catch
            {
                // Retrieve individual exceptions from AggregateException thrown by Task.WaitAll
                var throwers = deploymentIdsToTasks.Where(task => task.Value.Exception != null);
                foreach (KeyValuePair<string, Task> erroredTask in throwers)
                {
                    Exception inner = erroredTask.Value.Exception.InnerException;

                    string erroredDeploymentId = erroredTask.Key;
                    erroredDeploymentIds.Add(erroredDeploymentId);
                    Console.WriteLine($"Error while stopping deployment {erroredDeploymentId}: {inner.Message}");
                }
            }

            Console.WriteLine($"Deployment(s) stopped with {erroredDeploymentIds.Count()} errors");
            return 0;
        }

        private static Task<StopDeploymentResponse> StopDeploymentByIdAsync(DeploymentServiceClient client, string projectName, string deploymentId)
        {
            Console.WriteLine($"Stopping deployment with id {deploymentId}");
            return client.StopDeploymentAsync(new StopDeploymentRequest
            {
                Id = deploymentId,
                ProjectName = projectName
            });
        }

        private static int ListDeployments(string[] args, bool useChinaPlatform)
        {
            var projectName = args[1];

            var deploymentServiceClient = DeploymentServiceClient.Create(GetApiEndpoint(useChinaPlatform), GetPlatformRefreshTokenCredential(useChinaPlatform));
            var activeDeployments = ListLaunchedActiveDeployments(deploymentServiceClient, projectName);

            foreach (var deployment in activeDeployments)
            {
                var status = deployment.Status;
                var overviewPageUrl = $"https://{GetConsoleHost(useChinaPlatform)}/projects/{projectName}/deployments/{deployment.Name}/overview/{deployment.Id}";

                if (deployment.Tag.Contains(SIM_PLAYER_DEPLOYMENT_TAG))
                {
                    Console.WriteLine($"<simulated-player-deployment> {deployment.Id} {deployment.Name} {deployment.RegionCode} {overviewPageUrl} {status}");
                }
                else
                {
                    Console.WriteLine($"<deployment> {deployment.Id} {deployment.Name} {deployment.RegionCode} {overviewPageUrl} {status}");
                }
            }

            return 0;
        }

        private static IEnumerable<Deployment> ListLaunchedActiveDeployments(DeploymentServiceClient client, string projectName)
        {
            var listDeploymentsResult = client.ListDeployments(new ListDeploymentsRequest
            {
                View = ViewType.Basic,
                ProjectName = projectName,
                DeploymentStoppedStatusFilter = ListDeploymentsRequest.Types.DeploymentStoppedStatusFilter.NotStoppedDeployments,
                PageSize = 50
            });

            return listDeploymentsResult.Where(deployment =>
            {
                var status = deployment.Status;
                if (status != Deployment.Types.Status.Starting && status != Deployment.Types.Status.Running)
                {
                    // Deployment not active - skip
                    return false;
                }

                if (!deployment.Tag.Contains(DEPLOYMENT_LAUNCHED_BY_LAUNCHER_TAG))
                {
                    // Deployment not launched by this launcher - skip
                    return false;
                }

                return true;
            });
        }

        private static void ShowUsage()
        {
            Console.WriteLine("Usage:");
            Console.WriteLine("DeploymentLauncher create <project-name> <assembly-name> <runtime-version> <main-deployment-name> <main-deployment-json> <main-deployment-snapshot> <main-deployment-region> <main-deployment-cluster> <main-deployment-tags> [<sim-deployment-base-name> <sim-deployment-json> <sim-deployment-region> <sim-deployment-cluster> <total-num-sim-players> [<max-sim-players-per-deployment>]]");
            Console.WriteLine($"  Starts a cloud deployment with optional simulated player deployments. The deployments can be started in different regions ('EU', 'US', 'AP' and 'CN'). If simulated player deployment details are provided but the maximum number of players per deployment is left unspecified, a single deployment is started for all simulated players.");
            Console.WriteLine("DeploymentLauncher createsim <project-name> <assembly-name> <runtime-version> <target-deployment-name> <sim-deployment-base-name> <sim-deployment-json> <sim-deployment-region> <sim-deployment-cluster> <sim-deployment-snapshot> <total-num-sim-players> <auto-connect> [<max-sim-players-per-deployment>]");
            Console.WriteLine($"  Starts simulated player deployment(s). Can be started in a different region from the target deployment ('EU', 'US', 'AP' and 'CN'). A single deployment for all simulated players is started by default.");
            Console.WriteLine("DeploymentLauncher stop <project-name> [deployment-id]");
            Console.WriteLine("  Stops the specified deployment within the project.");
            Console.WriteLine("  If no deployment id argument is specified, all active deployments started by the deployment launcher in the project will be stopped.");
            Console.WriteLine("DeploymentLauncher list <project-name>");
            Console.WriteLine("  Lists all active deployments within the specified project that are started by the deployment launcher.");
            Console.WriteLine();
            Console.WriteLine("Flags:");
            Console.WriteLine("  --china           Use China platform endpoints.");
        }

        private static int Main(string[] args)
        {
            // Filter flags from the rest of the arguments.
            string[] flags = args.Where(arg => arg.StartsWith("--")).Select(arg => arg.ToLowerInvariant()).ToArray();
            args = args.Where(arg => !arg.StartsWith("--")).ToArray();

            bool useChinaPlatform = flags.Contains("--china");

            if (useChinaPlatform)
            {
                if (!File.Exists(ChinaProductionTokenPath))
                {
                    Console.WriteLine("The 'china' flag was passed, but you are not authenticated for the 'cn-production' environment.");
                    return 1;
                }

                ChinaRefreshToken = File.ReadAllText(ChinaProductionTokenPath);
                ChinaCredentials = new PlatformRefreshTokenCredential(ChinaRefreshToken,
                    "https://auth.spatialoschina.com/auth/v1/authcode",
                    "https://auth.spatialoschina.com/auth/v1/token");
            }

            // Argument count for the same command can vary because of optional arguments
            if (args.Length == 0 ||
                (args[0] == "create" && (args.Length != 16 && args.Length != 15 && args.Length != 10)) ||
                (args[0] == "createsim" && (args.Length != 13 && args.Length != 12)) ||
                (args[0] == "stop" && (args.Length != 2 && args.Length != 3)) ||
                (args[0] == "list" && args.Length != 2))
            {
                ShowUsage();
                return 1;
            }

            try
            {
                if (args[0] == "create")
                {
                    return CreateDeployment(args, useChinaPlatform);
                }

                if (args[0] == "createsim")
                {
                    return CreateSimDeployments(args, useChinaPlatform);
                }

                if (args[0] == "stop")
                {
                    return StopDeployments(args, useChinaPlatform);
                }

                if (args[0] == "list")
                {
                    return ListDeployments(args, useChinaPlatform);
                }

                ShowUsage();
            }
            catch (Grpc.Core.RpcException e)
            {
                if (e.Status.StatusCode == Grpc.Core.StatusCode.Unauthenticated)
                {
                    Console.WriteLine("Error: unauthenticated. Please run `spatial auth login`");
                }
                else
                {
                    Console.Error.WriteLine($"Encountered an unknown gRPC error. Exception = {e.ToString()}");
                }
            }

            // Unknown command or error occurred.
            return 1;
        }
    }
}
