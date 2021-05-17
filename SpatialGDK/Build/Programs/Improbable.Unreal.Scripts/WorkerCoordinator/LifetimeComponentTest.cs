using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Improbable.WorkerCoordinator
{
    /// <summary>
    /// Lifetime component test code.
    /// Call `LifetimeComponentTest.Run()` in `Program.Main` to start local test.
    /// </summary>
    internal class LifetimeComponentTest
    {
        public static void Run()
        {
            // Set test arguments
            // Set `maxLifetime=0` means use non-lifetime mode.
            var testArgs = new[]
            {
                $"{LifetimeComponent.MaxLifetimeArg}=0",
                $"{LifetimeComponent.MinLifetimeArg}=1",
                $"{LifetimeComponent.RestartAfterSecondsArg}=10",
                $"{LifetimeComponent.UseNewSimulatedPlayerArg}=0"
            };

            var logger = new LoggerTest();

            var component = LifetimeComponent.Create(logger, testArgs, out var num);

            component.SetHost(new LifetimeComponentHostTest());

            for (var i = 0; i < 10; ++i)
            {
                component.AddSimulatedPlayer(new ClientInfo()
                {
                    ClientName = $"simplayer-{Guid.NewGuid()}",
                    StartTime = DateTime.Now.AddSeconds(2 + 2 * i)
                });
            }

            component.Start();

            logger.WriteLog("Finish test.");
        }
    }

    internal class LifetimeComponentHostTest : ILifetimeComponentHost
    {
        private readonly List<string> ActiveProcessList = new List<string>();

        public void StartClient(ClientInfo clientInfo)
        {
            ActiveProcessList.Add(clientInfo.ClientName);
        }

        public void StopClient(ClientInfo clientInfo)
        {
            ActiveProcessList.Remove(clientInfo.ClientName);
        }

        public bool CheckPlayerStatus()
        {
            return ActiveProcessList.Count == 0;
        }
    }

    internal class LoggerTest : Logger
    {
        public LoggerTest(string logPath="", string loggerName="") : base(logPath, loggerName)
        {
        }

        public override void WriteLog(string logMessage, bool logToConnectionIfExists = true)
        {
            Console.WriteLine($"[info]-{logMessage}");
        }

        public override void WriteWarning(string logMessage, bool logToConnectionIfExists = true)
        {
            Console.WriteLine($"[warning]-{logMessage}");
        }

        public override void WriteError(string logMessage, bool logToConnectionIfExists = true)
        {
            Console.WriteLine($"[error]-{logMessage}");
        }
    }
}
