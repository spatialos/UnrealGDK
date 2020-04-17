using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using CommandLine;
using Newtonsoft.Json.Linq;
using NLog;

namespace ReleaseTool
{
    /// <summary>
    ///     Runs the steps required to cut a release candidate branch.

    ///     * TODO: Remove this and modify our equivalent version files instead.
    ///     * Alters all package.json.

    ///     * TODO: Modify this to work with our CHANGELOG.
    ///     * Bumps the version in the CHANGELOG.md.

    ///     * TODO: Remove this and modify our equivalent version files instead.
    ///     * Updates the hash in the gdk.pinned file.

    ///     * TODO: Modify this to work with our repos.
    ///     * Pushes the candidate and creates a Pull Request against master.
    /// </summary>

    ///     * TODO: Most of the strings in this class need modification and/or deletion
    internal class PrepCommand
    {
        internal const string ChangeLogUpdateGdkTemplate = "- Upgraded to GDK for Unity version `{0}`";

        private const string PackerConfigFile = "packer.config.json";

        private const string PackageJsonFilename = "package.json";
        private const string ManifestJsonFilename = "manifest.json";
        private const string PackageJsonNameKey = "name";
        private const string PackageJsonVersionString = "version";
        private const string PackageJsonDependenciesString = "dependencies";
        private const string PackageJsonDependenciesPrefix = "io.improbable";

        private const string CommitMessageTemplate = "Release candidate for version {0}.";

        private const string ChangeLogFilename = "CHANGELOG.md";
        private const string ChangeLogReleaseHeadingTemplate = "## `{0}` - {1:yyyy-MM-dd}";

        private const string PullRequestTemplate = "Release {0} - Pre-Validation";
        private static readonly Logger Logger = LogManager.GetCurrentClassLogger();

        [Verb("prep", HelpText = "Prep a release candidate branch.")]
        public class Options : GitHubClient.IGitHubOptions, BuildkiteMetadataSink.IBuildkiteOptions
        {
            [Value(0, MetaName = "version", HelpText = "The release version that is being cut.", Required = true)]
            public string Version { get; set; }

            [Option('g', "update-pinned-gdk", HelpText =
                "The git hash of the version of the gdk to upgrade to. (Only if this is a project).")]
            public string PinnedGdkVersion { get; set; }

            [Option("git-repository-name", HelpText = "The Git repository that we are targeting.", Required = true)]
            public string GitRepoName { get; set; }

            #region IBuildkiteOptions implementation

            public string MetadataFilePath { get; set; }

            #endregion

            #region IGithubOptions implementation

            public string GitHubTokenFile { get; set; }

            public string GitHubToken { get; set; }

            #endregion

            public bool ShouldUpdateGdkVersion => !string.IsNullOrEmpty(PinnedGdkVersion);
        }

        private readonly Options options;

        public PrepCommand(Options options)
        {
            this.options = options;
        }

        /*
         *     This tool is designed to be used with a robot Github account which has its own fork of the GDK
         *     repositories. This means that when we prep a release:
         *         1. Checkout our fork of the repo.
         *         2. Add the spatialos org remote to our local copy and fetch this remote.
         *         3. Checkout the spatialos/master branch (the non-forked master branch).
         *         4. Make the changes for prepping the release.
         *         5. Push this to an RC branch on the forked repository.
         *         6. Open a PR from the fork into the source repository.
         */
        public int Run()
        {
            var remoteUrl = string.Format(Common.RemoteUrlTemplate, Common.GithubBotUser, options.GitRepoName);

            try
            {
                var gitHubClient = new GitHubClient(options);

                using (var gitClient = GitClient.FromRemote(remoteUrl))
                {
                    // This does step 2 from above.
                    var spatialOsRemote =
                        string.Format(Common.RemoteUrlTemplate, Common.SpatialOsOrg, options.GitRepoName);
                    gitClient.AddRemote(Common.SpatialOsOrg, spatialOsRemote);
                    gitClient.Fetch(Common.SpatialOsOrg);

                    // This does step 3 from above.
                    gitClient.CheckoutRemoteBranch(Common.MasterBranch, Common.SpatialOsOrg);

                    // This does step 4 from above.
                    using (new WorkingDirectoryScope(gitClient.RepositoryPath))
                    {
                        UpdateManifestJson(gitClient);
                        UpdateAllPackageJsons(gitClient);

                        if (options.ShouldUpdateGdkVersion)
                        {
                            UpdateGdkVersion(gitClient, options.PinnedGdkVersion);
                        }

                        if (!File.Exists(ChangeLogFilename))
                        {
                            throw new InvalidOperationException("Could not update the change log as the file," +
                                $" {ChangeLogFilename}, does not exist");
                        }

                        Logger.Info("Updating {0}...", ChangeLogFilename);

                        var changelog = File.ReadAllLines(ChangeLogFilename).ToList();
                        UpdateChangeLog(changelog, options);
                        File.WriteAllLines(ChangeLogFilename, changelog);
                        gitClient.StageFile(ChangeLogFilename);
                    }

                    // This does step 5 from above.
                    var branchName = string.Format(Common.ReleaseBranchNameTemplate, options.Version);
                    gitClient.Commit(string.Format(CommitMessageTemplate, options.Version));
                    gitClient.ForcePush(branchName);

                    // This does step 6 from above.
                    var gitHubRepo = gitHubClient.GetRepositoryFromRemote(spatialOsRemote);

                    var branchFrom = $"{Common.GithubBotUser}:{branchName}";
                    var branchTo = Common.MasterBranch;

                    // Only open a PR if one does not exist yet.
                    if (!gitHubClient.TryGetPullRequest(gitHubRepo, branchFrom, branchTo, out var pullRequest))
                    {
                        pullRequest = gitHubClient.CreatePullRequest(gitHubRepo,
                            branchFrom,
                            branchTo,
                            string.Format(PullRequestTemplate, options.Version),
                            GetPullRequestBody(options.GitRepoName));
                    }

                    if (BuildkiteMetadataSink.CanWrite(options))
                    {
                        using (var sink = new BuildkiteMetadataSink(options))
                        {
                            sink.WriteMetadata($"{options.GitRepoName}-release-branch",
                                $"pull/{pullRequest.Number}/head:{branchName}");
                            sink.WriteMetadata($"{options.GitRepoName}-pr-url", pullRequest.HtmlUrl);
                        }
                    }

                    Logger.Info("Pull request available: {0}", pullRequest.HtmlUrl);
                    Logger.Info("Successfully created release!");
                    Logger.Info("Release hash: {0}", gitClient.GetHeadCommit().Sha);
                }
            }
            catch (Exception e)
            {
                Logger.Error(e, "ERROR: Unable to prep release candidate branch. Error: {0}", e);
                return 1;
            }

            return 0;
        }

        private void UpdateManifestJson(GitClient gitClient)
        {
            var manifestFile = Path.Combine(Directory.GetCurrentDirectory(), "workers", "unity", "Packages",
                ManifestJsonFilename);

            UpdatePackageJson(manifestFile, gitClient);
        }

        private void UpdateAllPackageJsons(GitClient gitClient)
        {
            Logger.Info("Updating all {0}...", PackageJsonFilename);

            var packageFiles = Directory.GetFiles(Directory.GetCurrentDirectory(), PackageJsonFilename,
                SearchOption.AllDirectories);

            foreach (var packageFile in packageFiles)
            {
                UpdatePackageJson(packageFile, gitClient);
            }
        }

        private void UpdatePackageJson(string packageFile, GitClient gitClient)
        {
            Logger.Info("Updating {0}...", packageFile);

            JObject jsonObject;
            using (var streamReader = new StreamReader(packageFile))
            {
                jsonObject = JObject.Parse(streamReader.ReadToEnd());

                if (jsonObject.ContainsKey(PackageJsonNameKey))
                {
                    var name = (string) jsonObject[PackageJsonNameKey];

                    if (name.StartsWith(PackageJsonDependenciesPrefix) &&
                        jsonObject.ContainsKey(PackageJsonVersionString))
                    {
                        jsonObject[PackageJsonVersionString] = options.Version;
                    }
                }

                if (jsonObject.ContainsKey(PackageJsonDependenciesString))
                {
                    var dependencies = (JObject) jsonObject[PackageJsonDependenciesString];

                    foreach (var property in dependencies.Properties())
                    // If it's an Improbable package and it's not a "file:" reference.
                    {
                        if (property.Name.StartsWith(PackageJsonDependenciesPrefix) &&
                            !((string) property.Value).StartsWith("file:"))
                        {
                            dependencies[property.Name] = options.Version;
                        }
                    }
                }
            }

            File.WriteAllText(packageFile, jsonObject.ToString());

            gitClient.StageFile(packageFile);
        }

        internal static void UpdateChangeLog(List<string> changelog, Options options)
        {
            // If we already have a changelog entry for this release. Skip this step.
            if (changelog.Any(line => IsMarkdownHeading(line, 2, $"`{options.Version}` - ")))
            {
                Logger.Info($"Changelog already has release version {options.Version}. Skipping..", ChangeLogFilename);
                return;
            }

            // First add the new release heading under the "## Unreleased" one.
            // Assuming that this is the first heading.
            var unreleasedIndex = changelog.FindIndex(line => IsMarkdownHeading(line, 2));
            var releaseHeading = string.Format(ChangeLogReleaseHeadingTemplate, options.Version,
                DateTime.Now);

            changelog.InsertRange(unreleasedIndex + 1, new[]
            {
                string.Empty,
                releaseHeading
            });

            // If we don't need to update the Changed section. We are done!
            if (!options.ShouldUpdateGdkVersion)
            {
                return;
            }

            // Next find the changed section between the second and third ## heading.
            // If one doesn't exist, create it.

            // First find all ## sections (release markers).
            var headerTwoIndexes = changelog
                .Select((value, index) => (value, index))
                .Where(tuple => IsMarkdownHeading(tuple.Item1, 2))
                .Select(tuple => tuple.Item2)
                .ToList();

            // 2 headers = Unreleased, this release
            if (headerTwoIndexes.Count < 2)
            {
                throw new InvalidOperationException(
                    "Changelog is malformed. Expected at least two header two (##) entries.");
            }

            // Grab the valid range for the Changed section we want to target.
            // For the end, if is no previous release, use the entire changelog.
            var startCurrentReleaseIndex = headerTwoIndexes[1];
            var endCurrentReleaseIndex = headerTwoIndexes.Count == 2 ? changelog.Count - 1 : headerTwoIndexes[2];

            var changedIndexes = changelog.Select((value, index) => (value, index))
                .Where(tuple => IsMarkdownHeading(tuple.Item1, 3, "Changed"))
                .Select(tuple => tuple.Item2)
                .ToList();

            int changedHeaderIndex;

            // If there is no changed section or there isn't one in the current release, add one!
            if (changedIndexes.Count == 0 || changedIndexes[0] > endCurrentReleaseIndex)
            {
                // Insert a changed section.
                changelog.InsertRange(startCurrentReleaseIndex + 1, new[]
                {
                    string.Empty,
                    "### Changed"
                });

                changedHeaderIndex = startCurrentReleaseIndex + 2;
            }
            else
            {
                changedHeaderIndex = changedIndexes[0];
            }

            changelog.Insert(changedHeaderIndex + 2, string.Format(ChangeLogUpdateGdkTemplate, options.Version));
        }

        private static void UpdateGdkVersion(GitClient gitClient, string newPinnedVersion)
        {
            const string gdkPinnedFilename = "gdk.pinned";

            Logger.Info("Updating pinned gdk version to {0}...", newPinnedVersion);

            if (!File.Exists(gdkPinnedFilename))
            {
                throw new InvalidOperationException("Could not upgrade gdk version as the file, " +
                    $"{gdkPinnedFilename}, does not exist");
            }

            // Pin is always to master in this case.
            File.WriteAllText(gdkPinnedFilename, $"{Common.MasterBranch} {newPinnedVersion}");

            gitClient.StageFile(gdkPinnedFilename);
        }

        private static bool IsMarkdownHeading(string markdownLine, int level, string startTitle = null)
        {
            var heading = $"{new string('#', level)} {startTitle ?? string.Empty}";

            return markdownLine.StartsWith(heading);
        }

        private static string GetPullRequestBody(string repoName)
        {
            switch (repoName)
            {
                case "gdk-for-unity":
                    return @"#### Description
- Package versions
- Changelog
- Upgrade guide

#### Tests
- Windows
	- [ ] local deploy
	- [ ] cloud client (Release QA pipeline)
	- [ ] editor tooling
- Mac
	- [ ] local deploy
	- [ ] cloud client (Release QA pipeline)
	- [ ] editor tooling
- Android
	- [ ] local client
	- [ ] cloud client
- iOS
	- [ ] local client
	- [ ] cloud client";
                case "gdk-for-unity-fps-starter-project":
                    return @"#### Description
- Package versions
- Changelog
- Pinned gdk

#### Tests
- Windows
	- [ ] local deploy
	- [ ] cloud client (Release QA pipeline)
- Mac
	- [ ] local deploy
	- [ ] cloud client (Release QA pipeline)
- Android
	- [ ] local client
	- [ ] cloud client
- iOS
	- [ ] local client
	- [ ] cloud client";
                case "gdk-for-unity-blank-project":
                    return @"#### Description
- Package versions
- Changelog
- pinned gdk

#### Tests
- Windows
	- [ ] local deploy
	- [ ] cloud client (Release QA pipeline)
- Mac
	- [ ] local deploy
	- [ ] cloud client (Release QA pipeline)
- Android
	- [ ] local client
	- [ ] cloud client
- iOS
	- [ ] local client
	- [ ] cloud client";
                default:
                    throw new ArgumentException($"No PR body template found for repo {repoName}");
            }
        }
    }
}
