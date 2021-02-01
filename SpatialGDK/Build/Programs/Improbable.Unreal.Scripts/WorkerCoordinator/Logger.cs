// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

using System.IO;
using Improbable.Worker.CInterop;

namespace Improbable.WorkerCoordinator
{
    public class Logger
    {
        private readonly string LogPath;
        private readonly string LoggerName;
        private Connection Connection = null;

        public Logger(string logPath, string loggerName)
        {
            LogPath = logPath;
            LoggerName = loggerName;
        }

        public void EnableSpatialOSLogging(Connection connection)
        {
            Connection = connection;
        }

        public void ClearLog()
        {
            File.WriteAllText(LogPath, string.Empty);
        }

        public void WriteLog(string logMessage, bool logToConnectionIfExists = true)
        {
            WriteLogToFile(logMessage);
            if (logToConnectionIfExists && Connection != null)
            {
                Connection.SendLogMessage(LogLevel.Info, LoggerName, logMessage);
            }
        }

        public void WriteWarning(string logMessage, bool logToConnectionIfExists = true)
        {
            WriteLogToFile("Warning: " + logMessage);
            if (logToConnectionIfExists && Connection != null)
            {
                Connection.SendLogMessage(LogLevel.Warn, LoggerName, logMessage);
            }
        }

        public void WriteError(string logMessage, bool logToConnectionIfExists = true)
        {
            WriteLogToFile("Error: " + logMessage);
            if (logToConnectionIfExists && Connection != null)
            {
                Connection.SendLogMessage(LogLevel.Error, LoggerName, logMessage);
            }
        }

        private void WriteLogToFile(string logMessage)
        {
            using (StreamWriter file = new StreamWriter(LogPath, true))
            {
                file.WriteLine(logMessage);
            }
        }
    }
}
