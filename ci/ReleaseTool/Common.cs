using NLog;
using System;
using System.IO;
using System.Linq;
using System.Text.RegularExpressions;

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

        public static bool IsMarkdownHeading(string markdownLine, int level, string startTitle = null)
        {
            var heading = $"{new string('#', level)} {startTitle ?? string.Empty}";

            return markdownLine.StartsWith(heading);
        }

        public static bool UpdateChangeLog(string ChangeLogFilePath, string Version, GitClient gitClient, string ChangeLogReleaseHeadingTemplate)
        {
            using (new WorkingDirectoryScope(gitClient.RepositoryPath))
            {
                if (File.Exists(ChangeLogFilePath))
                {
                    var originalContents = File.ReadAllText(ChangeLogFilePath);
                    var changelog = File.ReadAllLines(ChangeLogFilePath).ToList();
                    var releaseHeading = string.Format(ChangeLogReleaseHeadingTemplate, Version,
                        DateTime.Now);
                    var releaseIndex = changelog.FindIndex(line => IsMarkdownHeading(line, 2, $"[`{Version}`] - "));
                    // If we already have a changelog entry for this release, replace it.
                    if (releaseIndex != -1)
                    {
                        changelog[releaseIndex] = releaseHeading;
                    }
                    else
                    {
                        // Add the new release heading under the "## Unreleased" one.
                        // Assuming that this is the first heading.
                        var unreleasedIndex = changelog.FindIndex(line => IsMarkdownHeading(line, 2));
                        changelog.InsertRange(unreleasedIndex + 1, new[]
                        {
                            string.Empty,
                            releaseHeading
                        });
                    }
                    File.WriteAllLines(ChangeLogFilePath, changelog);

                    // If nothing has changed, return false, so we can react to it from the caller.
                    if (File.ReadAllText(ChangeLogFilePath) == originalContents)
                    {
                        return false;
                    }

                    gitClient.StageFile(ChangeLogFilePath);
                    return true;
                }
            }
            return false;
        }

        public static bool UpdateVersionFile(GitClient gitClient, string fileContents, string relativeFilePath, NLog.Logger Logger)
        {
            using (new WorkingDirectoryScope(gitClient.RepositoryPath))
            {
                Logger.Info("Updating contents of version file '{0}' to '{1}'...", relativeFilePath, fileContents);

                if (!File.Exists(relativeFilePath))
                {
                    throw new InvalidOperationException("Could not update the version file as the file " +
                        $"'{relativeFilePath}' does not exist.");
                }

                if (File.ReadAllText(relativeFilePath) == fileContents)
                {
                    Logger.Info("Contents of '{0}' are already up-to-date ('{1}').", relativeFilePath, fileContents);
                    return false;
                }

                File.WriteAllText(relativeFilePath, $"{fileContents}");

                gitClient.StageFile(relativeFilePath);
            }

            return true;
        }

        public static (string, int) ExtractPullRequestInfo(string pullRequestUrl)
        {
            const string regexString = "github\\.com\\/.*\\/(.*)\\/pull\\/([0-9]*)";

            var match = Regex.Match(pullRequestUrl, regexString);

            if (!match.Success)
            {
                throw new ArgumentException($"Malformed pull request url: {pullRequestUrl}");
            }

            if (match.Groups.Count < 3)
            {
                throw new ArgumentException($"Malformed pull request url: {pullRequestUrl}");
            }

            var repoName = match.Groups[1].Value;
            var pullRequestIdStr = match.Groups[2].Value;

            if (!int.TryParse(pullRequestIdStr, out int pullRequestId))
            {
                throw new Exception(
                    $"Parsing pull request URL failed. Expected number for pull request id, received: {pullRequestIdStr}");
            }

            return (repoName, pullRequestId);
        }
    }
}
