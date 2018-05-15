using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.IO;

namespace Improbable
{
    public static class Common
    {
        static Common()
        {
            AppDomain.CurrentDomain.UnhandledException += (sender, eventArgs) =>
            {
                Console.WriteLine(((Exception)eventArgs.ExceptionObject).Message);
                Environment.Exit(1);
            };
        }

        public static void EnsureDirectoryEmpty(string dir)
        {
            if(Directory.Exists(dir))
            {
                Directory.Delete(dir, true);
            }

            Directory.CreateDirectory(dir);
        }

        public static void RunRedirected(string command, IEnumerable<string> arguments)
        {
            var startInfo = new ProcessStartInfo(command, string.Join(" ", arguments.ToArray()))
            {
                CreateNoWindow = true,
                RedirectStandardError = true,
                RedirectStandardOutput = true,
                RedirectStandardInput = true,
                UseShellExecute = false
            };

            using(var process = Process.Start(startInfo))
            {
                process.EnableRaisingEvents = true;
                process.OutputDataReceived += (sender, e) => Console.WriteLine("{0}", e.Data);
                process.ErrorDataReceived += (sender, e) => Console.Error.WriteLine("{0}", e.Data);
                process.BeginOutputReadLine();
                process.BeginErrorReadLine();

                process.WaitForExit();

                if(process.ExitCode != 0)
                {
                    throw new Exception(string.Format("Exit code {0} while running:\n{1}\n\t{2}", process.ExitCode, command, string.Join("\n\t", arguments)));
                }
            }
        }
    }
}
