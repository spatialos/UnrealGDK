using System.Collections.Generic;
using System.Linq;
using Improbable.Worker;
using Improbable.Worker.Alpha;

namespace Improbable.WorkerCoordinator
{
    class Authentication
    {
        private static Logger Logger = new Logger("/improbable/logs/WorkerCoordinator.log");

        public static string GetDevelopmentPlayerIdentityToken(string devAuthTokenId, string clientName)
        {
            var pitResponse = DevelopmentAuthentication.CreateDevelopmentPlayerIdentityTokenAsync("locator.improbable.io", 444,
                    new PlayerIdentityTokenRequest
                    {
                        DevelopmentAuthenticationTokenId = devAuthTokenId,
                        PlayerId = clientName,
                        DisplayName = "SimulatedPlayer"
                    }).Get();

            if (pitResponse.Status != ConnectionStatusCode.Success)
            {
                Logger.WriteError($"Failed to retrieve player identity token.\n" +
                    $"error code: {pitResponse.Status}\n" +
                    $"error message: {pitResponse.Error}");
                return null;
            }

            return pitResponse.PlayerIdentityToken;
        }

        public static List<LoginTokenDetails> GetDevelopmentLoginTokens(string workerType, string pit)
        {
            var loginTokensResponse = DevelopmentAuthentication.CreateDevelopmentLoginTokensAsync("locator.improbable.io", 444,
                new LoginTokensRequest
                {
                    PlayerIdentityToken = pit,
                    WorkerType = workerType,
                    UseInsecureConnection = false,
                    DurationSeconds = 300
                }).Get();

            if (loginTokensResponse.Status != ConnectionStatusCode.Success)
            {
                Logger.WriteError($"Failed to retrieve any login tokens.\n" +
                    $"error code: {loginTokensResponse.Status}\n" +
                    $"error message: {loginTokensResponse.Error}");
                return null;
            }

            return loginTokensResponse.LoginTokens;
        }

        public static string SelectLoginToken(List<LoginTokenDetails> loginTokens, string targetDeployment)
        {
            var selectedLoginToken = loginTokens.FirstOrDefault(token => token.DeploymentName == targetDeployment).LoginToken;

            if (selectedLoginToken == null)
            {
                Logger.WriteError("Failed to launch simulated player. Login token for target deployment was not found in response. Does that deployment have the `dev_auth` tag?");
            }

            return selectedLoginToken;
        }
    }
}
