// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Improbable.WorkerCoordinator
{
	public class Logger
	{
		private string LogPath;

		public Logger(string logPath)
		{
			LogPath = logPath;
		}

		public void ClearLog()
		{
			File.WriteAllText(LogPath, string.Empty);
		}

		public void WriteLog(string logMessage = "", bool logToConsole = false)
		{
			using (StreamWriter file = new StreamWriter(LogPath, true))
			{
				file.WriteLine(logMessage);
			}
			if (logToConsole)
			{
				Console.WriteLine(logMessage);
			}
		}

		public void WriteWarning(string logMessage, bool logToConsole = false)
		{
			WriteLog("Warning: " + logMessage, logToConsole);
		}

		public void WriteError(string logMessage, bool logToConsole = false)
		{
			if (logToConsole)
			{
				Console.Error.WriteLine(logMessage);
			}
			WriteLog("Error: " + logMessage);
		}
	}
}
