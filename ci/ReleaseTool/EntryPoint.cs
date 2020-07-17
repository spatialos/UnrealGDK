using CommandLine;
using NLog;

namespace ReleaseTool
{
    internal static class EntryPoint
    {
        private static int Main(string[] args)
        {
            ConfigureLogger();

            return Parser.Default.ParseArguments<PrepCommand.Options, ReleaseCommand.Options>(args)
                .MapResult(
                    (PrepCommand.Options options) => new PrepCommand(options).Run(),
                    (ReleaseCommand.Options options) => new ReleaseCommand(options).Run(),
                    errors => 1);
        }

        private static void ConfigureLogger()
        {
            var config = new NLog.Config.LoggingConfiguration();

            var logfile = new NLog.Targets.FileTarget("logfile")
            {
                FileName = "release-tool.log"
            };
            var logconsole = new NLog.Targets.ConsoleTarget("logconsole");

            config.AddRule(LogLevel.Info, LogLevel.Fatal, logconsole);
            config.AddRule(LogLevel.Debug, LogLevel.Fatal, logfile);

            LogManager.Configuration = config;
        }
    }
}
