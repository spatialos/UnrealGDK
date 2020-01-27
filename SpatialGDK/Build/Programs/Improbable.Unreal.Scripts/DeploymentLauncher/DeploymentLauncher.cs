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

namespace Improbable
{
    internal class DeploymentLauncher
    {
        private const string SIM_PLAYER_DEPLOYMENT_TAG = "simulated_players";
        private const string DEPLOYMENT_LAUNCHED_BY_LAUNCHER_TAG = "unreal_deployment_launcher";

        private const string CoordinatorWorkerName = "SimulatedPlayerCoordinator";

        private const string CHINA_ENDPOINT_URL = "platform.api.spatialoschina.com";
        private const int CHINA_ENDPOINT_PORT = 443;

        private static string UploadSnapshot(SnapshotServiceClient client, string snapshotPath, string projectName,
            string deploymentName)
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
            httpRequest.Headers.Set("Content-MD5", snapshotToUpload.Checksum);

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
        
        private static PlatformApiEndpoint GetApiEndpoint(string region)
        {
            if (region == "CN") 
            {
                return new PlatformApiEndpoint(CHINA_ENDPOINT_URL, CHINA_ENDPOINT_PORT);
            }
            return null; // Use default
        }

        private static int CreateDeployment(string[] args)
        {
            bool launchSimPlayerDeployment = args.Length == 11;

            var projectName = args[1];
            var assemblyName = args[2];
            var mainDeploymentName = args[3];
            var mainDeploymentJsonPath = args[4];
            var mainDeploymentSnapshotPath = args[5];
            var mainDeploymentRegion = args[6];

            var simDeploymentName = string.Empty;
            var simDeploymentJson = string.Empty;
            var simDeploymentRegion = string.Empty;
            var simNumPlayers = 0;

            if (launchSimPlayerDeployment)
            {
                simDeploymentName = args[7];
                simDeploymentJson = args[8];
                simDeploymentRegion = args[9];

                if (!Int32.TryParse(args[10], out simNumPlayers))
                {
                    Console.WriteLine("Cannot parse the number of simulated players to connect.");
                    return 1;
                }
            }

            try
            {
                var deploymentServiceClient = DeploymentServiceClient.Create(GetApiEndpoint(mainDeploymentRegion));

                if (DeploymentExists(deploymentServiceClient, projectName, mainDeploymentName))
                {
                    StopDeploymentByName(deploymentServiceClient, projectName, mainDeploymentName);
                }

                var createMainDeploymentOp = CreateMainDeploymentAsync(deploymentServiceClient, launchSimPlayerDeployment, projectName, assemblyName, mainDeploymentName, mainDeploymentJsonPath, mainDeploymentSnapshotPath, mainDeploymentRegion);

                if (!launchSimPlayerDeployment)
                {
                    // Don't launch a simulated player deployment. Wait for main deployment to be created and then return.
                    Console.WriteLine("Waiting for deployment to be ready...");
                    var result = createMainDeploymentOp.PollUntilCompleted().GetResultOrNull();
                    if (result == null)
                    {
                        Console.WriteLine("Failed to create the main deployment");
                        return 1;
                    }

                    Console.WriteLine("Successfully created the main deployment");
                    return 0;
                }

                if (DeploymentExists(deploymentServiceClient, projectName, simDeploymentName))
                {
                    StopDeploymentByName(deploymentServiceClient, projectName, simDeploymentName);
                }

                var createSimDeploymentOp = CreateSimPlayerDeploymentAsync(deploymentServiceClient, projectName, assemblyName, mainDeploymentName, simDeploymentName, simDeploymentJson, simDeploymentRegion, simNumPlayers);

                // Wait for both deployments to be created.
                Console.WriteLine("Waiting for deployments to be ready...");
                var mainDeploymentResult = createMainDeploymentOp.PollUntilCompleted().GetResultOrNull();
                if (mainDeploymentResult == null)
                {
                    Console.WriteLine("Failed to create the main deployment");
                    return 1;
                }

                Console.WriteLine("Successfully created the main deployment");
                var simPlayerDeployment = createSimDeploymentOp.PollUntilCompleted().GetResultOrNull();
                if (simPlayerDeployment == null)
                {
                    Console.WriteLine("Failed to create the simulated player deployment");
                    return 1;
                }

                Console.WriteLine("Successfully created the simulated player deployment");

                // Update coordinator worker flag for simulated player deployment to notify target deployment is ready.
                simPlayerDeployment.WorkerFlags.Add(new WorkerFlag
                {
                    Key = "target_deployment_ready",
                    Value = "true",
                    WorkerType = CoordinatorWorkerName
                });
                deploymentServiceClient.UpdateDeployment(new UpdateDeploymentRequest { Deployment = simPlayerDeployment });

                Console.WriteLine("Done! Simulated players will start to connect to your deployment");
            }
            catch (Grpc.Core.RpcException e)
            {
                if (e.Status.StatusCode == Grpc.Core.StatusCode.NotFound)
                {
                    Console.WriteLine(
                        $"Unable to launch the deployment(s). This is likely because the project '{projectName}' or assembly '{assemblyName}' doesn't exist.");
                }
                else
                {
                    throw;
                }
            }

            return 0;
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
            bool launchSimPlayerDeployment, string projectName, string assemblyName, string mainDeploymentName, string mainDeploymentJsonPath, string mainDeploymentSnapshotPath, string regionCode)
        {
            var snapshotServiceClient = SnapshotServiceClient.Create(GetApiEndpoint(regionCode));

            // Upload snapshots.
            var mainSnapshotId = UploadSnapshot(snapshotServiceClient, mainDeploymentSnapshotPath, projectName,
                mainDeploymentName);

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
                RegionCode = regionCode
            };

            mainDeploymentConfig.Tag.Add(DEPLOYMENT_LAUNCHED_BY_LAUNCHER_TAG);

            if (launchSimPlayerDeployment)
            {
                // This tag needs to be added to allow simulated players to connect using login
                // tokens generated with anonymous auth.
                mainDeploymentConfig.Tag.Add("dev_login");
            }

            Console.WriteLine(
                $"Creating the main deployment {mainDeploymentName} in project {projectName} with snapshot ID {mainSnapshotId}. Link: https://console.improbable.io/projects/{projectName}/deployments/{mainDeploymentName}/overview");

            var mainDeploymentCreateOp = deploymentServiceClient.CreateDeployment(new CreateDeploymentRequest
            {
                Deployment = mainDeploymentConfig
            });

            return mainDeploymentCreateOp;
        }

        private static Operation<Deployment, CreateDeploymentMetadata> CreateSimPlayerDeploymentAsync(DeploymentServiceClient deploymentServiceClient,
            string projectName, string assemblyName, string mainDeploymentName, string simDeploymentName, string simDeploymentJsonPath, string regionCode, int simNumPlayers)
        {
            var playerAuthServiceClient = PlayerAuthServiceClient.Create(GetApiEndpoint(regionCode));

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
            numSimulatedPlayersFlag.Add("value", $"{simNumPlayers}");

            var simWorkerConfigJson = File.ReadAllText(simDeploymentJsonPath);
            dynamic simWorkerConfig = JObject.Parse(simWorkerConfigJson);

            for (var i = 0; i < simWorkerConfig.workers.Count; ++i)
            {
                if (simWorkerConfig.workers[i].worker_type == CoordinatorWorkerName)
                {
                    simWorkerConfig.workers[i].flags.Add(devAuthTokenFlag);
                    simWorkerConfig.workers[i].flags.Add(targetDeploymentFlag);
                    simWorkerConfig.workers[i].flags.Add(numSimulatedPlayersFlag);
                }
            }

            // Specify the number of managed coordinator workers to start by editing
            // the load balancing options in the launch config. It creates a rectangular
            // launch config of N cols X 1 row, N being the number of coordinators
            // to create.
            // This assumes the launch config contains a rectangular load balancing
            // layer configuration already for the coordinator worker.
            var lbLayerConfigurations = simWorkerConfig.load_balancing.layer_configurations;
            for (var i = 0; i < lbLayerConfigurations.Count; ++i)
            {
                if (lbLayerConfigurations[i].layer == CoordinatorWorkerName)
                {
                    var rectangleGrid = lbLayerConfigurations[i].rectangle_grid;
                    rectangleGrid.cols = simNumPlayers;
                    rectangleGrid.rows = 1;
                }
            }

            simWorkerConfigJson = simWorkerConfig.ToString();

            // Create simulated player deployment.
            var simDeploymentConfig = new Deployment
            {
                AssemblyId = assemblyName,
                LaunchConfig = new LaunchConfig
                {
                    ConfigJson = simWorkerConfigJson
                },
                Name = simDeploymentName,
                ProjectName = projectName,
                RegionCode = regionCode
                // No snapshot included for the simulated player deployment
            };

            simDeploymentConfig.Tag.Add(DEPLOYMENT_LAUNCHED_BY_LAUNCHER_TAG);
            simDeploymentConfig.Tag.Add(SIM_PLAYER_DEPLOYMENT_TAG);

            Console.WriteLine(
                $"Creating the simulated player deployment {simDeploymentName} in project {projectName} with {simNumPlayers} simulated players. Link: https://console.improbable.io/projects/{projectName}/deployments/{simDeploymentName}/overview");

            var simDeploymentCreateOp = deploymentServiceClient.CreateDeployment(new CreateDeploymentRequest
            {
                Deployment = simDeploymentConfig
            });

            return simDeploymentCreateOp;
        }


        private static int StopDeployments(string[] args)
        {
            var projectName = args[1];
            var regionCode = args[2];

            var deploymentServiceClient = DeploymentServiceClient.Create(GetApiEndpoint(regionCode));

            if (args.Length == 4)
            {
                // Stop only the specified deployment.
                var deploymentId = args[3];
                StopDeploymentById(deploymentServiceClient, projectName, deploymentId);

                return 0;
            }

            // Stop all active deployments launched by this launcher.
            var activeDeployments = ListLaunchedActiveDeployments(deploymentServiceClient, projectName);

            foreach (var deployment in activeDeployments)
            {
                var deploymentId = deployment.Id;
                StopDeploymentById(deploymentServiceClient, projectName, deploymentId);
            }

            return 0;
        }

        private static void StopDeploymentById(DeploymentServiceClient client, string projectName, string deploymentId)
        {
            try
            {
                Console.WriteLine($"Stopping deployment with id {deploymentId}");
                client.StopDeployment(new StopDeploymentRequest
                {
                    Id = deploymentId,
                    ProjectName = projectName
                });
            }
            catch (Grpc.Core.RpcException e)
            {
                if (e.Status.StatusCode == Grpc.Core.StatusCode.NotFound)
                {
                    Console.WriteLine("<error:unknown-deployment>");
                }
                else
                {
                    throw;
                }
            }
        }

        private static int ListDeployments(string[] args)
        {
            var projectName = args[1];
            var regionCode = args[2];

            var deploymentServiceClient = DeploymentServiceClient.Create(GetApiEndpoint(regionCode));
            var activeDeployments = ListLaunchedActiveDeployments(deploymentServiceClient, projectName);

            foreach (var deployment in activeDeployments)
            {
                var status = deployment.Status;
                var overviewPageUrl = $"https://console.improbable.io/projects/{projectName}/deployments/{deployment.Name}/overview/{deployment.Id}";

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
            Console.WriteLine("DeploymentLauncher create <project-name> <assembly-name> <main-deployment-name> <main-deployment-json> <main-deployment-snapshot> <main-deployment-region> [<sim-deployment-name> <sim-deployment-json> <sim-deployment-region> <num-sim-players>]");
            Console.WriteLine($"  Starts a cloud deployment, with optionally a simulated player deployment. The deployments can be started in different regions ('EU', 'US', 'AP' and 'CN').");
            Console.WriteLine("DeploymentLauncher stop <project-name> <main-deployment-region> [deployment-id]");
            Console.WriteLine("  Stops the specified deployment within the project.");
            Console.WriteLine("  If no deployment id argument is specified, all active deployments started by the deployment launcher in the project will be stopped.");
            Console.WriteLine("DeploymentLauncher list <project-name> <main-deployment-region>");
            Console.WriteLine("  Lists all active deployments within the specified project that are started by the deployment launcher.");
        }

        private static int Main(string[] args)
        {
            if (args.Length == 0 ||
                args[0] == "create" && (args.Length != 11 && args.Length != 7) ||
                args[0] == "stop" && (args.Length != 3 && args.Length != 4) ||
                args[0] == "list" && args.Length != 3)
            {
                ShowUsage();
                return 1;
            }

            try
            {
                if (args[0] == "create")
                {
                    return CreateDeployment(args);
                }

                if (args[0] == "stop")
                {
                    return StopDeployments(args);
                }

                if (args[0] == "list")
                {
                    return ListDeployments(args);
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
