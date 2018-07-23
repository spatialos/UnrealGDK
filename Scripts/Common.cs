using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;

namespace Improbable
{
    public static class Common
    {
        static Common()
        {
            AppDomain.CurrentDomain.UnhandledException += (sender, eventArgs) =>
            {
                Console.WriteLine(((Exception) eventArgs.ExceptionObject).Message);
                Environment.Exit(1);
            };
        }

        public static void EnsureDirectoryEmpty(string dir)
        {
            if (Directory.Exists(dir))
            {
                Directory.Delete(dir, true);
            }

            Directory.CreateDirectory(dir);
        }

        public static int RunRedirected(string command, IEnumerable<string> arguments, bool throwOnNonZeroExitCode = true)
        {
            command = Environment.ExpandEnvironmentVariables(command);
            arguments = arguments.Select(Environment.ExpandEnvironmentVariables).ToArray();

            var startInfo = new ProcessStartInfo(command, string.Join(" ", arguments.ToArray()))
            {
                CreateNoWindow = true,
                RedirectStandardError = true,
                RedirectStandardOutput = true,
                RedirectStandardInput = true,
                UseShellExecute = false
            };

            try
            {
                using (var process = Process.Start(startInfo))
                {
                    if (process != null)
                    {
                        process.EnableRaisingEvents = true;
                        process.OutputDataReceived += (sender, e) =>
                        {
                            if (!string.IsNullOrEmpty(e.Data))
                            {
                                Console.WriteLine("{0}", e.Data);
                            }
                        };
                        process.ErrorDataReceived += (sender, e) =>
                        {
                            if (!string.IsNullOrEmpty(e.Data))
                            {
                                Console.Error.WriteLine("{0}", e.Data);
                            }
                        };

                        // Async print lines of output as they come in.
                        process.BeginOutputReadLine();
                        process.BeginErrorReadLine();

                        process.WaitForExit();

                        if (process.ExitCode != 0 && throwOnNonZeroExitCode)
                        {
                            throw new Exception($"Exit code {process.ExitCode}");
                        }

                        return process.ExitCode;
                    }
                    else
                    {
                        throw new Exception("Process is null");
                    }
                }
            }
            catch (System.Exception ex)
            {
                throw new Exception($"{ex.Message} while running:\n{command}\n\t{string.Join("\n\t", arguments)}");
            }
        }

        public static void WriteHeading(string format, params object[] args)
        {
            Console.ForegroundColor = ConsoleColor.Cyan;
            Console.WriteLine(format, args);
            Console.ForegroundColor = ConsoleColor.Gray;
        }
    }
}
