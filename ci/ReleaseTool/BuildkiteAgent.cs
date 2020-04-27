using System;
using System.Collections.Generic;
using Medallion.Shell;
using NLog;

namespace ReleaseTool
{
    public static class BuildkiteAgentImpl
    {
        private static readonly Logger Logger = LogManager.GetCurrentClassLogger();

        public static string GetMetadata(string key)
        {
            var commandResult = Command
                .Run("buildkite-agent", "meta-data", "get", key)
                .Result;

            if (commandResult.Success)
            {
                return commandResult.StandardOutput;
            }

            throw new MetadataNotFoundException(key, commandResult.StandardError);
        }

        public static void SetMetaData(string key, string value)
        {
            var commandResult = Command
                .Run("buildkite-agent", "meta-data", "set", key, value)
                .Result;

            if (!commandResult.Success)
            {
                throw new MetadataNotFoundException(key, commandResult.StandardError);
            }
        }

        public static void Annotate(AnnotationLevel level, string context, string message, bool append = false)
        {
            string style;
            switch (level) {
                case AnnotationLevel.Info:
                    style = "info";
                    break;
                case AnnotationLevel.Warning:
                    style = "warning";
                    break;
                case AnnotationLevel.Error:
                    style = "error";
                    break;
                case AnnotationLevel.Success:
                    style = "success";
                    break;
                default:
                    throw new ArgumentOutOfRangeException(nameof(level));
            }

            var args = new List<string>
                {
                    "annotate", message,
                    "--style", style,
                    "--context", context
                };

            if (append)
            {
                args.Add("--append");
            }

            Logger.Debug($"Annotating build with style '{style}', context '{context}':\n{message}");

            var commandResult = Command
                .Run("buildkite-agent", args)
                .Result;

            if (!commandResult.Success)
            {
                throw new Exception($"Failed to annotate build\nStdout: {commandResult.StandardOutput}\nStderr: {commandResult.StandardError}");
            }
        }
    }

    public class MetadataNotFoundException : Exception
    {
        public MetadataNotFoundException(string key, string stderr)
            : base($"Could not find meta-data associated with {key}.\nRaw stderr: {stderr}")
        {
        }
    }

    public class CouldNotSetMetadataException : Exception
    {
        public CouldNotSetMetadataException(string key, string value, string stderr)
            : base($"Could not set meta-data with {key} and value {value}.\nRaw stderr: {stderr}")
        {
        }
    }

    public enum AnnotationLevel
    {
        Success = 0,
        Info = 1,
        Warning = 2,
        Error = 3,
    }
}
