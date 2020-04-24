using System;
using System.IO;
using System.Linq;

namespace ReleaseTool
{
    internal static class Common
    {
        public const string RepoUrlTemplate = "git@github.com:{0}/{1}.git";

        public static void VerifySemanticVersioningFormat(string version)
        {
            var majorMinorPatch = version.Split('.');
            var hasSemanticVersion = majorMinorPatch.Length == 3 && Enumerable.All(majorMinorPatch, s => int.TryParse(s, out _));

            if (!hasSemanticVersion)
            {
                throw new ArgumentException($"The provided version '{version}' should comply " +
                    $"with the Semantic Versioning Specification, but does not.");
            }
        }

        public static string ReplaceHomePath(string originalPath)
        {
            if (!originalPath.StartsWith("~"))
            {
                return originalPath;
            }

            var relativePath = new Uri(originalPath.Substring(2), UriKind.Relative);
            var homeDirectory = AppendDirectorySeparator(Environment.GetFolderPath(
                Environment.SpecialFolder.UserProfile));
            var homePath = new Uri(homeDirectory, UriKind.Absolute);

            return new Uri(homePath, relativePath).AbsolutePath;
        }

        /**
         * This is required as URIs treat paths that do not have a trailing slash as a file rather than a directory.
         */
        private static string AppendDirectorySeparator(string originalPath)
        {
            var directorySeparator = Path.DirectorySeparatorChar.ToString();
            var altDirectorySeparator = Path.AltDirectorySeparatorChar.ToString();

            if (originalPath.EndsWith(directorySeparator) || originalPath.EndsWith(altDirectorySeparator))
            {
                return originalPath;
            }

            if (originalPath.Contains(altDirectorySeparator))
            {
                return originalPath + altDirectorySeparator;
            }

            return originalPath + directorySeparator;
        }
    }
}
