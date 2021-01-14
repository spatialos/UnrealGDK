using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text.RegularExpressions;

namespace ReleaseTool
{
    internal static class Common
    {
        public const string RepoUrlTemplate = "git@github.com:{0}/{1}.git";

        // Names of the version files that live in the UnrealEngine repository.
        public const string UnrealGDKVersionFile = "UnrealGDKVersion.txt";
        public const string UnrealGDKExampleProjectVersionFile = "UnrealGDKExampleProjectVersion.txt";

        // Plugin file configuration.
        public const string PluginFileName = "SpatialGDK.uplugin";
        public const string VersionKey = "Version";
        public const string VersionNameKey = "VersionName";

        // Changelog file configuration
        public const string ChangeLogFilename = "CHANGELOG.md";
        public const string ChangeLogReleaseHeadingTemplate = "## [`{0}`] - {1:yyyy-MM-dd}";

        // Unreal version configuration
        public const string UnrealEngineVersionFile = "ci/unreal-engine.version";

        public static bool UpdateVersionFilesWithEngine(GitClient gitClient, string gitRepoName, string versionRaw, string versionSuffix, string engineVersions, NLog.Logger logger)
        {
            return UpdateVersionFiles_Internal(gitClient, gitRepoName, versionRaw, logger, versionSuffix, engineVersions);
        }

        public static bool UpdateVersionFilesButNotEngine(GitClient gitClient, string gitRepoName, string versionRaw, NLog.Logger logger)
        {
            return UpdateVersionFiles_Internal(gitClient, gitRepoName, versionRaw, logger);
        }

        private static bool UpdateVersionFiles_Internal(GitClient gitClient, string gitRepoName,  string versionRaw, NLog.Logger logger, string versionSuffix = "", string engineVersions = "")
        {
            string versionDecorated = versionRaw + versionSuffix;
            switch (gitRepoName)
            {
                case "UnrealEngine":
                {
                    bool madeChanges = false;
                    madeChanges |= UpdateVersionFile(gitClient, versionDecorated, UnrealGDKVersionFile, logger);
                    madeChanges |= UpdateVersionFile(gitClient, versionDecorated, UnrealGDKExampleProjectVersionFile, logger);
                    return madeChanges;
                }
                case "UnrealGDK":
                {
                    bool madeChanges = false;
                    madeChanges |= UpdateChangeLog(gitClient, versionRaw, ChangeLogReleaseHeadingTemplate, ChangeLogFilename);
                    if (!madeChanges) logger.Info("{0} was already up-to-date.", ChangeLogFilename);
                    if (engineVersions != "")
                    {
                        madeChanges |= UpdatePluginFile(gitClient, versionRaw, PluginFileName, logger);

                        var engineCandidateBranches = engineVersions.Split(" ")
                            .Select(engineVersion => $"HEAD {engineVersion.Trim()}-{versionDecorated}")
                            .ToList();
                        madeChanges |= UpdateUnrealEngineVersionFile(gitClient, engineCandidateBranches, UnrealEngineVersionFile);
                    }
                    return madeChanges;
                }
                case "UnrealGDKExampleProject":
                case "UnrealGDKTestGyms":
                case "UnrealGDKEngineNetTest":
                case "TestGymBuildKite":
                    return UpdateVersionFile(gitClient, versionDecorated, UnrealGDKVersionFile, logger);
                default:
                    throw new ArgumentException($"Invalid gitRepoName: '{gitRepoName}'");
            }
        }

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

        public static bool UpdateChangeLog(GitClient gitClient, string version, string changeLogReleaseHeadingTemplate, string changeLogFilePath)
        {
            using (new WorkingDirectoryScope(gitClient.RepositoryPath))
            {
                if (File.Exists(changeLogFilePath))
                {
                    var originalContents = File.ReadAllText(changeLogFilePath);
                    var changelog = File.ReadAllLines(changeLogFilePath).ToList();
                    var releaseHeading = string.Format(changeLogReleaseHeadingTemplate, version,
                        DateTime.Now);
                    var releaseIndex = changelog.FindIndex(line => IsMarkdownHeading(line, 2, $"[`{version}`] - "));
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
                    File.WriteAllLines(changeLogFilePath, changelog);

                    // If nothing has changed, return false, so we can react to it from the caller.
                    if (File.ReadAllText(changeLogFilePath) == originalContents)
                    {
                        return false;
                    }

                    gitClient.StageFile(changeLogFilePath);
                    return true;
                }
            }

            throw new Exception($"Failed to update the changelog. Arguments: " +
                $"ChangeLogFilePath: {changeLogFilePath}, Version: {version}, Heading template: {changeLogReleaseHeadingTemplate}.");
        }

        public static bool UpdateVersionFile(GitClient gitClient, string fileContents, string versionFileRelativePath, NLog.Logger logger)
        {
            using (new WorkingDirectoryScope(gitClient.RepositoryPath))
            {
                logger.Info("Updating contents of version file '{0}' to '{1}'...", versionFileRelativePath, fileContents);

                if (!File.Exists(versionFileRelativePath))
                {
                    throw new InvalidOperationException("Could not update the version file as the file " +
                        $"'{versionFileRelativePath}' does not exist.");
                }

                if (File.ReadAllText(versionFileRelativePath) == fileContents)
                {
                    logger.Info("Contents of '{0}' are already up-to-date ('{1}').", versionFileRelativePath, fileContents);
                    return false;
                }

                File.WriteAllText(versionFileRelativePath, $"{fileContents}");

                gitClient.StageFile(versionFileRelativePath);
            }

            return true;
        }

        public static bool UpdateUnrealEngineVersionFile(GitClient client, List<string> versions, string unrealEngineVersionFile)
        {
            using (new WorkingDirectoryScope(client.RepositoryPath))
            {
                if (!File.Exists(unrealEngineVersionFile))
                {
                    throw new InvalidOperationException("Could not update the unreal engine file as the file " +
                        $"'{unrealEngineVersionFile}' does not exist.");
                }
                var originalContents = File.ReadAllText(unrealEngineVersionFile);
                File.WriteAllLines(unrealEngineVersionFile, versions);

                // If nothing has changed, return false, so we can react to it from the caller.
                if (File.ReadAllText(unrealEngineVersionFile) == originalContents)
                {
                    return false;
                }

                client.StageFile(unrealEngineVersionFile);
                return true;
            }
        }

        public static bool UpdatePluginFile(GitClient gitClient, string version, string pluginFileName, NLog.Logger Logger)
        {
            using (new WorkingDirectoryScope(gitClient.RepositoryPath))
            {
                var pluginFilePath = Directory.GetFiles(".", pluginFileName, SearchOption.AllDirectories).First();
                if (File.Exists(pluginFilePath))
                {
                    Logger.Info("Updating {0}...", pluginFilePath);
                    var originalContents = File.ReadAllText(pluginFilePath);

                    JObject jsonObject;
                    using (var streamReader = new StreamReader(pluginFilePath))
                    {
                        jsonObject = JObject.Parse(streamReader.ReadToEnd());

                        if (!jsonObject.ContainsKey(Common.VersionKey) || !jsonObject.ContainsKey(VersionNameKey))
                        {
                            throw new InvalidOperationException($"Could not update the plugin file at '{pluginFilePath}', " + $"because at least one of the two expected keys '{Common.VersionKey}' and '{Common.VersionNameKey}' " + $"could not be found.");
                        }

                        var oldVersion = (string)jsonObject[VersionNameKey];
                        if (ShouldIncrementPluginVersion(oldVersion, version))
                        {
                            jsonObject[VersionKey] = ((int)jsonObject[VersionKey] + 1);
                        }

                        // Update the version name to the new one
                        jsonObject[VersionNameKey] = version;
                    }

                    File.WriteAllText(pluginFilePath, jsonObject.ToString());

                    // If nothing has changed, return false, so we can react to it from the caller.
                    if (File.ReadAllText(pluginFilePath) == originalContents)
                    {
                        return false;
                    }

                    gitClient.StageFile(pluginFilePath);
                    return true;
                }

                throw new Exception($"Failed to update the plugin file. Argument: " + $"pluginFilePath: {pluginFilePath}.");
            }
        }
        public static bool ShouldIncrementPluginVersion(string oldVersionName, string newVersionName)
        {
            var oldMajorMinorVersions = oldVersionName.Split('.').Take(2).Select(s => int.Parse(s));
            var newMajorMinorVersions = newVersionName.Split('.').Take(2).Select(s => int.Parse(s));
            return Enumerable.Any(Enumerable.Zip(oldMajorMinorVersions, newMajorMinorVersions, (o, n) => o < n));
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
