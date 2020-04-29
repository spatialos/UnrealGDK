using System;
using System.IO;
using System.Text;
using System.Text.RegularExpressions;
using CommandLine;
using Octokit;

namespace ReleaseTool
{
    /// <summary>
    ///     Runs the commands required for releasing a candidate.
    ///     * Merges the candidate branch into the release branch.
    ///     * Pushes the release branch.
    ///     * Creates a GitHub release draft.
    ///     * Creates a PR from the release branch into the master branch.
    /// </summary>
    internal class ReleaseCommand
    {
        private static readonly NLog.Logger Logger = NLog.LogManager.GetCurrentClassLogger();

        private const string PullRequestNameTemplate = "Release {0} - Merge release into master";
        private const string pullRequestBody = "Merging the release branch into master.";

        private const string releaseAnnotationTemplate = "* Successfully created a [draft release]({0}) " +
           "in the repo `{1}`. Your human labour is now required to publish it.\n";
        private const string prAnnotationTemplate = "* Successfully created a [pull request]({0}) " +
            "in the repo `{1}` from `{2}` into `{3}`. " +
            "Your human labour is now required to merge these PRs.\n";

        private const string ChangeLogFilename = "CHANGELOG.md";

        [Verb("release", HelpText = "Merge a release branch and create a github release draft.")]
        public class Options : GitHubClient.IGitHubOptions
        {
            [Value(0, MetaName = "version", HelpText = "The version that is being released.")]
            public string Version { get; set; }

            [Option('u', "pull-request-url", HelpText = "The link to the release candidate branch to merge.",
                Required = true)]
            public string PullRequestUrl { get; set; }

            [Option("source-branch", HelpText = "The source branch name from which we are cutting the candidate.", Required = true)]
            public string SourceBranch { get; set; }

            [Option("candidate-branch", HelpText = "The candidate branch name.", Required = true)]
            public string CandidateBranch { get; set; }

            [Option("release-branch", HelpText = "The name of the branch into which we are merging the candidate.", Required = true)]
            public string ReleaseBranch { get; set; }

            [Option("github-organization", HelpText = "The Github Organization that contains the targeted repository.", Required = true)]
            public string GithubOrgName { get; set; }

            public string GitHubTokenFile { get; set; }

            public string GitHubToken { get; set; }

            public string MetadataFilePath { get; set; }
        }

        private readonly Options options;

        public ReleaseCommand(Options options)
        {
            this.options = options;
        }

        /*
         *     This tool is designed to execute most of the git operations required when releasing:
         *         1. Merge the RC PR into the release branch.
         *         2. Draft a GitHub release using the changelog notes.
         *         3. Open a PR from the release branch into master
         */
        public int Run()
        {
            Common.VerifySemanticVersioningFormat(options.Version);

            try
            {
                var gitHubClient = new GitHubClient(options);

                var (repoName, pullRequestId) = ExtractPullRequestInfo(options.PullRequestUrl);

                var repoUrl = string.Format(Common.RepoUrlTemplate, options.GithubOrgName, repoName);
                var gitHubRepo = gitHubClient.GetRepositoryFromUrl(repoUrl);

                // Merge into release
                var mergeResult = gitHubClient.MergePullRequest(gitHubRepo, pullRequestId);

                if (!mergeResult.Merged)
                {
                    throw new InvalidOperationException(
                        $"Was unable to merge pull request at: {options.PullRequestUrl}. Received error: {mergeResult.Message}");
                }

                // Delete candidate branch.
                gitHubClient.DeleteBranch(gitHubClient.GetRepositoryFromUrl(repoUrl), options.CandidateBranch);

                using (var gitClient = GitClient.FromRemote(repoUrl))
                {
                    // Create GitHub release in the repo
                    gitClient.Fetch();
                    gitClient.CheckoutLocalBranch(options.ReleaseBranch);
                    var release = CreateRelease(gitHubClient, gitHubRepo, gitClient, repoName);

                    BuildkiteAgent.Annotate(AnnotationLevel.Info, "draft-releases",
                        string.Format(releaseAnnotationTemplate, release.HtmlUrl, repoName), true);

                    Logger.Info("Release Successful!");
                    Logger.Info("Release hash: {0}", gitClient.GetHeadCommit().Sha);
                    Logger.Info("Draft release: {0}", release.HtmlUrl);
                }

                // Open a PR for merging the release branch into master.
                var branchFrom = options.ReleaseBranch;
                var branchTo = options.SourceBranch;

                // Only open a PR if one does not exist yet.
                if (!gitHubClient.TryGetPullRequest(gitHubRepo, branchFrom, branchTo, out var pullRequest))
                {
                    pullRequest = gitHubClient.CreatePullRequest(gitHubRepo,
                        branchFrom,
                        branchTo,
                        string.Format(PullRequestNameTemplate, options.Version),
                        pullRequestBody);
                }

                var prAnnotation = string.Format(prAnnotationTemplate,
                    pullRequest.HtmlUrl, repoName, options.ReleaseBranch, options.SourceBranch);
                BuildkiteAgent.Annotate(AnnotationLevel.Info, "release-into-source-prs", prAnnotation, true);

                Logger.Info("Pull request available: {0}", pullRequest.HtmlUrl);
                Logger.Info("Successfully created PR for merging the release into master!");
                Logger.Info("Merge hash: {0}", pullRequest.MergeCommitSha);
            }
            catch (Exception e)
            {
                Logger.Error(e, "ERROR: Unable to release candidate branch or merge the release branch back into master. Error: {0}", e);
                return 1;
            }

            return 0;
        }

        private Release CreateRelease(GitHubClient gitHubClient, Repository gitHubRepo, GitClient gitClient, string repoName)
        {
            var headCommit = gitClient.GetHeadCommit().Sha;

            string name;
            string releaseBody;
            
            switch (repoName)
            {
                case "UnrealGDK":
                    string changelog;
                    using (new WorkingDirectoryScope(gitClient.RepositoryPath))
                    {
                        changelog = GetReleaseNotesFromChangeLog();
                    }
                    name = $"GDK for Unreal Alpha Release {options.Version}";
                    releaseBody =
$@"Unreal GDK version {options.Version} is go!

* You can find the corresponding UnrealEngine version(s) [here](https://github.com/improbableio/UnrealEngine/releases).
* You can find the corresponding UnrealGDKExampleProject version [here](https://github.com/spatialos/UnrealGDKExampleProject/releases).

Follow [these steps](https://documentation.improbable.io/gdk-for-unreal/docs/keep-your-gdk-up-to-date) to upgrade your GDK, Unreal Engine fork and your Project to the latest release.

You can read the full release notes [here](https://github.com/spatialos/UnrealGDK/blob/release/CHANGELOG.md).

Join the community on our [forums](https://forums.improbable.io/), or on [Discord](https://discordapp.com/invite/vAT7RSU).

Happy developing!<br>
GDK team

---

{changelog}";
                    break;
                case "UnrealEngine":
                    name = $"{options.Version}";
                    releaseBody =
$@"Unreal GDK version {options.Version} is go!

* This Engine version corresponds to GDK version: [{options.Version}](https://github.com/spatialos/UnrealGDK/releases).
* You can find the corresponding UnrealGDKExampleProject version [here](https://github.com/spatialos/UnrealGDKExampleProject/releases).

Follow [these steps](https://documentation.improbable.io/gdk-for-unreal/docs/keep-your-gdk-up-to-date) to upgrade your GDK, Unreal Engine fork and your Project to the latest release.

You can read the full release notes [here](https://github.com/spatialos/UnrealGDK/blob/release/CHANGELOG.md).

Join the community on our [forums](https://forums.improbable.io/), or on [Discord](https://discordapp.com/invite/vAT7RSU).

Happy developing!<br>
GDK team";
                    break;
                case "UnrealGDKTestGyms":
                    name = $"{options.Version}";
                    releaseBody =
$@"Unreal GDK version {options.Version} is go!

* This UnrealGDKTestGyms version corresponds to GDK version: [{options.Version}](https://github.com/spatialos/UnrealGDK/releases).
* You can find the corresponding UnrealGDKExampleProject version [here](https://github.com/spatialos/UnrealGDKExampleProject/releases).
* You can find the corresponding UnrealEngine version(s) [here](https://github.com/improbableio/UnrealEngine/releases).

Follow [these steps](https://documentation.improbable.io/gdk-for-unreal/docs/keep-your-gdk-up-to-date) to upgrade your GDK, Unreal Engine fork and your Project to the latest release.

You can read the full release notes [here](https://github.com/spatialos/UnrealGDK/blob/release/CHANGELOG.md).

Join the community on our [forums](https://forums.improbable.io/), or on [Discord](https://discordapp.com/invite/vAT7RSU).

Happy developing!<br>
GDK team";
                    break;
                case "UnrealGDKEngineNetTest":
                    name = $"{options.Version}";
                    releaseBody =
$@"Unreal GDK version {options.Version} is go!

* This UnrealGDKEngineNetTest version corresponds to GDK version: [{options.Version}](https://github.com/spatialos/UnrealGDK/releases).
* You can find the corresponding UnrealGDKTestGyms version [here](https://github.com/improbable/UnrealGDKTestGyms/releases).
* You can find the corresponding UnrealGDKExampleProject version [here](https://github.com/spatialos/UnrealGDKExampleProject/releases).
* You can find the corresponding UnrealEngine version(s) [here](https://github.com/improbableio/UnrealEngine/releases).

Follow [these steps](https://documentation.improbable.io/gdk-for-unreal/docs/keep-your-gdk-up-to-date) to upgrade your GDK, Unreal Engine fork and your Project to the latest release.

You can read the full release notes [here](https://github.com/spatialos/UnrealGDK/blob/release/CHANGELOG.md).

Join the community on our [forums](https://forums.improbable.io/), or on [Discord](https://discordapp.com/invite/vAT7RSU).

Happy developing!<br>
GDK team";
                    break;
                case "UnrealGDKExampleProject":
                    name = $"{options.Version}";
                    releaseBody =
$@"Unreal GDK version {options.Version} is go!

* This UnrealGDKExampleProject version corresponds to GDK version: [{options.Version}](https://github.com/spatialos/UnrealGDK/releases).
* You can find the corresponding UnrealEngine version(s) [here](https://github.com/improbableio/UnrealEngine/releases).

Follow [these steps](https://documentation.improbable.io/gdk-for-unreal/docs/keep-your-gdk-up-to-date) to upgrade your GDK, Unreal Engine fork and your Project to the latest release.

You can read the full release notes [here](https://github.com/spatialos/UnrealGDK/blob/release/CHANGELOG.md).

Join the community on our [forums](https://forums.improbable.io/), or on [Discord](https://discordapp.com/invite/vAT7RSU).

Happy developing!<br>
GDK team";
                    break;
                default:
                    throw new ArgumentException("Unsupported repository.", nameof(repoName));
            }

            return gitHubClient.CreateDraftRelease(gitHubRepo, options.Version, releaseBody, name, headCommit);
        }

        private static (string, int) ExtractPullRequestInfo(string pullRequestUrl)
        {
            const string regexString = "github\\.com\\/spatialos\\/(.*)\\/pull\\/([0-9]*)";

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

        private static string GetReleaseNotesFromChangeLog()
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
    }
}
