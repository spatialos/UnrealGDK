using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;

namespace ReleaseTool
{
    internal static class Common
    {
        private const string RepoUrlTemplate = "git@github.com:{0}/{1}.git";

        // Names of the version files that live in the UnrealEngine repository.
        private const string UnrealGDKVersionFile = "UnrealGDKVersion.txt";
        private const string UnrealGDKExampleProjectVersionFile = "UnrealGDKExampleProjectVersion.txt";

        // Plugin file configuration.
        private const string PluginFileName = "SpatialGDK.uplugin";
        private const string VersionKey = "Version";
        private const string VersionNameKey = "VersionName";

        // Changelog file configuration
        private const string ChangeLogFilename = "CHANGELOG.md";
        private const string ChangeLogReleaseHeadingTemplate = "## [`{0}`] - {1:yyyy-MM-dd}";

        // Unreal version configuration
        private const string UnrealEngineVersionFile = "ci/unreal-engine.version";

        public static bool UpdateVersionFilesWithEngine(GitClient gitClient, string gitRepoName, string versionRaw, string engineVersions, NLog.Logger logger, string versionSuffix = "")
        {
            return UpdateVersionFiles_Internal(gitClient, gitRepoName, versionRaw, logger, versionSuffix, engineVersions);
        }

        public static bool UpdateVersionFilesButNotEngine(GitClient gitClient, string gitRepoName, string versionRaw, NLog.Logger logger, string versionSuffix = "")
        {
            return UpdateVersionFiles_Internal(gitClient, gitRepoName, versionRaw, logger, versionSuffix);
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
                    madeChanges |= UpdateChangeLog(gitClient, versionRaw);
                    if (!madeChanges) logger.Info("{0} was already up-to-date.", ChangeLogFilename);
                    if (engineVersions != "")
                    {
                        madeChanges |= UpdatePluginFile(gitClient, versionRaw, PluginFileName, logger);

                        var engineCandidateBranches = engineVersions.Split(" ")
                            .Select(engineVersion => $"HEAD {engineVersion.Trim()}-{versionDecorated}")
                            .ToList();
                        madeChanges |= UpdateUnrealEngineVersionFile(gitClient, engineCandidateBranches);
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

        public static bool UpdateChangeLog(GitClient gitClient, string version)
        {
            using (new WorkingDirectoryScope(gitClient.RepositoryPath))
            {
                if (File.Exists(ChangeLogFilename))
                {
                    var originalContents = File.ReadAllText(ChangeLogFilename);
                    var changelog = File.ReadAllLines(ChangeLogFilename).ToList();
                    var releaseHeading = string.Format(ChangeLogReleaseHeadingTemplate, version,
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
                    File.WriteAllLines(ChangeLogFilename, changelog);

                    // If nothing has changed, return false, so we can react to it from the caller.
                    if (File.ReadAllText(ChangeLogFilename) == originalContents)
                    {
                        return false;
                    }

                    gitClient.StageFile(ChangeLogFilename);
                    return true;
                }
            }

            throw new Exception($"Failed to update the changelog. Arguments: " +
                $"ChangeLogFilePath: {ChangeLogFilename}, Version: {version}, Heading template: {ChangeLogReleaseHeadingTemplate}.");
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

        public static bool UpdateUnrealEngineVersionFile(GitClient client, List<string> versions)
        {
            using (new WorkingDirectoryScope(client.RepositoryPath))
            {
                if (!File.Exists(UnrealEngineVersionFile))
                {
                    throw new InvalidOperationException("Could not update the unreal engine file as the file " +
                        $"'{UnrealEngineVersionFile}' does not exist.");
                }
                var originalContents = File.ReadAllText(UnrealEngineVersionFile);
                File.WriteAllLines(UnrealEngineVersionFile, versions);

                // If nothing has changed, return false, so we can react to it from the caller.
                if (File.ReadAllText(UnrealEngineVersionFile) == originalContents)
                {
                    return false;
                }

                client.StageFile(UnrealEngineVersionFile);
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

        public static string GetReleaseNotesFromChangeLog(NLog.Logger Logger)
        {
            if (!File.Exists(ChangeLogFilename))
            {
                throw new InvalidOperationException("Could not get draft release notes, as the change log file, " +
                    $"{ChangeLogFilename}, does not exist.");
            }

            Logger.Info("Reading {0}...", ChangeLogFilename);

            var releaseBody = new StringBuilder();
            var changedSection = 0;

            using (var reader = new StreamReader(ChangeLogFilename))
            {
                while (!reader.EndOfStream)
                {
                    // Here we target the second Heading2 ("##") section.
                    // The first section will be the "Unreleased" section. The second will be the correct release notes.
                    var line = reader.ReadLine();
                    if (line.StartsWith("## "))
                    {
                        changedSection += 1;

                        if (changedSection == 3)
                        {
                            break;
                        }

                        continue;
                    }

                    if (changedSection == 2)
                    {
                        releaseBody.AppendLine(line);
                    }
                }
            }

            return releaseBody.ToString();
        }

        public static string makeRepoUrl(string githubOrgName, string gitRepoName)
        {
            return string.Format(RepoUrlTemplate, githubOrgName, gitRepoName);
        }
    }
}
