using System;
using System.Linq;

namespace Improbable
{
    public static class Build
    {
        public static void Main(string[] args)
        {
            var runCodegen = args.Count(arg => arg == "--skip-codegen") == 0;
            var localOnly = args.Count(arg => arg == "--local") > 0;
            var cloudOnly = args.Count(arg => arg == "--cloud") > 0;
            var help = args.Count(arg => arg == "/?" || arg == "--help") > 0;

            if(help)
            {
                Console.WriteLine("Flags:");
                Console.WriteLine("\t--skip-codegen  : Don't run the code generator.");
                Console.WriteLine("\t--local         : Build only enough for local testing.");
                Console.WriteLine("\t--cloud         : Build only enough for cloud uploads.");
            }

            if( localOnly && cloudOnly)
            {
                throw new Exception("Specific only one of --local and --cloud");
            }

            if(runCodegen)
            {
                Codegen.Main(args);
            }

            // Always build the editor - it's required for everything else.
            Common.RunRedirected(@"Game\Binaries\ThirdParty\Improbable\Programs\unreal_packager", new [] {
                "build",
                "--target=Editor",
                "--configuration=Development",
                "--platform=Win64"
            });

            if(localOnly)
            {
                Common.RunRedirected(@"Game\Binaries\ThirdParty\Improbable\Programs\unreal_packager", new [] {
                "build",
                "--target=",
                "--configuration=Development",
                "--platform=Win64"
                });
            }

            if(cloudOnly)
            {
                    Common.RunRedirected(@"Game\Binaries\ThirdParty\Improbable\Programs\unreal_packager", new [] {
                    "build",
                    "--target=Server",
                    "--configuration=Development",
                    "--platform=Linux"
                });
            }
        }
    }
}
