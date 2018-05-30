using System.IO;

namespace Improbable
{
    public static class DiffCopy
    {
        public static void Main(string[] args)
        {
            var diffOnly = args.Count(arg => arg.ToLowerInvariant() == "--diff-only") == 0;
            var verbose = args.Count(arg => arg.ToLowerInvariant() == "--verbose") == 0;
            var help = args.Count(arg => arg == "/?" || arg.ToLowerInvariant() == "--help") > 0;

            var exitCode = 0;
            if (args.Length < 2 && !help)
            {
                help = true;
                exitCode = 1;
                Console.Error.WriteLine("Input and outpu path must be specified.");
            }

            if (help)
            {
                Console.WriteLine("Usage: <InputFolder> <OutputFolder> [flags]");
                Console.WriteLine("Flags:");
                Console.WriteLine("\t--diff-only : only perform the diffing.");
                Console.WriteLine("\t--verbose : print the log to stdout.");

                Environment.Exit(exitCode);
            }

            //Get files for intermediate folder

            //for each file in output folder,

            //check if there is a file in the input folder with the same name

                //if yes then

                    //hash files

                    //if hashes dont match then
                        //delete output file
                        //copy input file

                //else

                    // delete file

        }
    }
}
