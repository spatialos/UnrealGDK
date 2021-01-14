using System;
using CommandLine;

namespace ReleaseTool
{
    /// <summary>
    ///     Runs the steps required to cut release candidate branches in all repos:
    ///     UnrealGDK, UnrealGDKExampleProject, UnrealEngine, UnrealGDKEngineNetTest, UnrealGDKTestGyms and TestGymBuildKite.
    ///
    ///     * Checks out the source branch, which defaults to 4.xx-SpatialOSUnrealGDK in UnrealEngine and master in all other repos.
    ///     * IF the release branch does not already exits, creates it from the source branch.
    ///     * Makes repo-specific changes for prepping the release (e.g. updating version files, formatting the CHANGELOG).
    ///     * Commits these changes to release candaidate branches.
    ///     * Pushes the release candaidate branches to origin.
    ///     * Opens PRs to merge the release candaidate branches into the release branches.
    /// </summary>

    internal class PrepCommand
    {
        private static readonly NLog.Logger Logger = NLog.LogManager.GetCurrentClassLogger();

        private const string CandidateCommitMessageTemplate = "Release candidate for version {0}.";
        private const string PullRequestTemplate = "Release {0}";
        private const string prAnnotationTemplate = "* Successfully created a [pull request]({0}) " +
            "in the repo `{1}` from `{2}` into `{3}`. " +
            "Your human labour is now required to complete the tasks listed in the PR descriptions and unblock the pipeline and resume the release.\n";
        private const string branchAnnotationTemplate = "* Successfully created a [release candidate branch]({0}) " +
            "in the repo `{1}`, and it will evantually become `{2}` (no pull request as the specified release branch did not exist for this repository).\n";

        [Verb("prep", HelpText = "Prep a release candidate branch.")]
        public class Options : GitHubClient.IGitHubOptions
        {
            [Value(0, MetaName = "version", HelpText = "The release version that is being cut.", Required = true)]
            public string Version { get; set; }

            [Option("source-branch", HelpText = "The source branch name from which we are cutting the candidate.", Required = true)]
            public string SourceBranch { get; set; }

            [Option("candidate-branch", HelpText = "The candidate branch name.", Required = true)]
            public string CandidateBranch { get; set; }

            [Option("release-branch", HelpText = "The name of the branch into which we are merging the candidate.", Required = true)]
            public string ReleaseBranch { get; set; }

            [Option("git-repository-name", HelpText = "The Git repository that we are targeting.", Required = true)]
            public string GitRepoName { get; set; }

            [Option("github-organization", HelpText = "The Github Organization that contains the targeted repository.", Required = true)]
            public string GithubOrgName { get; set; }

            [Option("engine-versions", HelpText = "An array containing every engine version source branch.", Required = false)]
            public string EngineVersions {get;set;}

            #region IBuildkiteOptions implementation

            public string MetadataFilePath { get; set; }

            #endregion

            #region IGithubOptions implementation

            public string GitHubTokenFile { get; set; }

            public string GitHubToken { get; set; }

            #endregion
        }

        private readonly Options options;

        public PrepCommand(Options options)
        {
            this.options = options;
        }

        /*
         *     This tool is designed to be used with a robot Github account. When we prep a release:
         *         1. Clones the source repo.
         *         2. Checks out the source branch, which defaults to 4.xx-SpatialOSUnrealGDK in UnrealEngine and master in all other repos.
         *         3. Makes repo-specific changes for prepping the release (e.g. updating version files, formatting the CHANGELOG).
         *         4. Commit changes and push them to a remote candidate branch.
         *         5. IF the release branch does not exist, creates it from the source branch and pushes it to the remote.
         *         6. Opens a PR for merging the RC branch into the release branch.
         */
        public int Run()
        {
            Common.VerifySemanticVersioningFormat(options.Version);
            var gitRepoName = options.GitRepoName;
            var remoteUrl = Common.makeRepoUrl(options.GithubOrgName, gitRepoName);
            try
            {
                // 1. Clones the source repo.
                using (var gitClient = GitClient.FromRemote(remoteUrl))
                {
                    if (!gitClient.LocalBranchExists($"origin/{options.CandidateBranch}"))
                    {
                        // 2. Checks out the source branch, which defaults to 4.xx-SpatialOSUnrealGDK in UnrealEngine and master in all other repos.
                        gitClient.CheckoutRemoteBranch(options.SourceBranch);

                        // 3. Makes repo-specific changes for prepping the release (e.g. updating version files, formatting the CHANGELOG).
                        // UpdateVersionFilesWithEngine returns a bool to indicate if anything has changed, we could use this to only push when
                        // version files etc have changed which may be reasonable but might have side-effects as our github ci interactions are fragile
                        Common.UpdateVersionFilesWithEngine(gitClient, gitRepoName, options.Version, options.EngineVersions, Logger, "-rc");
                        // 4. Commit changes and push them to a remote candidate branch.
                        gitClient.Commit(string.Format(CandidateCommitMessageTemplate, options.Version));
                        gitClient.ForcePush(options.CandidateBranch);
                        Logger.Info($"Updated branch '{options.CandidateBranch}' for the release candidate release.");
    
                    }

                    // 5. IF the release branch does not exist, creates it from the source branch and pushes it to the remote.
                    if (!gitClient.LocalBranchExists($"origin/{options.ReleaseBranch}"))
                    {
                        Logger.Info("The release branch {0} does not exist! Going ahead with the PR-less release process.", options.ReleaseBranch);
                        Logger.Info("Release candidate head hash: {0}", gitClient.GetHeadCommit().Sha);
                        var branchAnnotation = string.Format(branchAnnotationTemplate,
                            $"https://github.com/{options.GithubOrgName}/{options.GitRepoName}/tree/{options.CandidateBranch}", options.GitRepoName, options.ReleaseBranch);
                        BuildkiteAgent.Annotate(AnnotationLevel.Info, "candidate-into-release-prs", branchAnnotation, true);
                        return 0;
                    }

                    // 6. Opens a PR for merging the RC branch into the release branch.
                    var gitHubClient = new GitHubClient(options);
                    var gitHubRepo = gitHubClient.GetRepositoryFromUrl(remoteUrl);
                    var githubOrg = options.GithubOrgName;
                    var branchFrom = options.CandidateBranch;
                    var branchTo = options.ReleaseBranch;

                    // Only open a PR if one does not exist yet.
                    if (!gitHubClient.TryGetPullRequest(gitHubRepo, githubOrg, branchFrom, branchTo, out var pullRequest))
                    {
                        Logger.Info("No PR exists. Attempting to open a new PR");
                        pullRequest = gitHubClient.CreatePullRequest(gitHubRepo,
                            branchFrom,
                            branchTo,
                            string.Format(PullRequestTemplate, options.Version),
                            GetPullRequestBody(options.GitRepoName, options.CandidateBranch, options.ReleaseBranch));
                    }

                    BuildkiteAgent.SetMetaData($"{options.GitRepoName}-{options.SourceBranch}-pr-url", pullRequest.HtmlUrl);

                    var prAnnotation = string.Format(prAnnotationTemplate,
                        pullRequest.HtmlUrl, options.GitRepoName, options.CandidateBranch, options.ReleaseBranch);
                    BuildkiteAgent.Annotate(AnnotationLevel.Info, "candidate-into-release-prs", prAnnotation, true);

                    Logger.Info("Pull request available: {0}", pullRequest.HtmlUrl);
                    Logger.Info("Successfully created pull request for the release!");
                    Logger.Info("PR head hash: {0}", gitClient.GetHeadCommit().Sha);
                }
            }
            catch (Exception e)
            {
                Logger.Error(e, "ERROR: Unable to prep release candidate branch. Error: {0}", e);
                return 1;
            }

            return 0;
        }

        private string GetPullRequestBody(string repoName, string candidateBranch, string releaseBranch)
        {
            // If repoName is UnrealGDK do nothing, otherwise get the UnrealGDK-pr-url
            var unrealGdkSourceBranch = repoName == "UnrealGDK" ? "" : BuildkiteAgent.GetMetadata("gdk-source-branch");
            var unrealGdkPrUrl = repoName == "UnrealGDK" ? "" : BuildkiteAgent.GetMetadata($"UnrealGDK-{unrealGdkSourceBranch}-pr-url");
            switch (repoName)
            {
                case "UnrealGDK":
                    return $@"#### Description
- This PR merges `{candidateBranch}` into `{releaseBranch}`.
- It was created by the [unrealgdk-release](https://buildkite.com/improbable/unrealgdk-release) Buildkite pipeline.
- Your human labour is now required to unblock the pipeline to resume the release.

#### Next Steps
- [ ] **Release Sheriff** - Delegate the tasks below.
- [ ] **Release Sheriff** - Once the Build & upload all UnrealEngine release candidates step completes, click the [run all tests](https://buildkite.com/improbable/unrealgdk-release) button.
- [ ] **Tech writers** - Review and translate [CHANGELOG.md](https://github.com/spatialos/UnrealGDK/blob/{candidateBranch}/CHANGELOG.md). Merge the translation and any edits into `{candidateBranch}`. 
- [ ] **QA** - Create and complete one [component release](https://improbabletest.testrail.io/index.php?/suites/view/72) test run per Unreal Engine version you're releasing.
- [ ] **Release Sheriff** - If any blocking defects are discovered, merge fixes into release candidate branches.
- [ ] **Release Sheriff** - Get approving reviews on *all* release candidate PRs.
- [ ] **Release Sheriff** - When the above tasks are complete, unblock the [pipeline](https://buildkite.com/improbable/unrealgdk-release). This action will merge all release candidates into their respective release branches and create draft GitHub releases that you must then publish.
";
                case "UnrealGDKExampleProject":
                    return $@"#### Description
- This PR merges `{candidateBranch}` into `{releaseBranch}`.
- It corresponds to {unrealGdkPrUrl}, where you can find more information about this release.";
                case "UnrealGDKTestGyms":
                    return $@"#### Description
- This PR merges `{candidateBranch}` into `{releaseBranch}`.
- It corresponds to {unrealGdkPrUrl}, where you can find more information about this release.";
                case "UnrealGDKEngineNetTest":
                    return $@"#### Description
- This PR merges `{candidateBranch}` into `{releaseBranch}`.
- It corresponds to {unrealGdkPrUrl}, where you can find more information about this release.";
                case "TestGymBuildKite":
                    return $@"#### Description
- This PR merges `{candidateBranch}` into `{releaseBranch}`.
- It corresponds to {unrealGdkPrUrl}, where you can find more information about this release.";
                case "UnrealEngine":
                    return $@"#### Description
- This PR merges `{candidateBranch}` into `{releaseBranch}`.
- It corresponds to {unrealGdkPrUrl}, where you can find more information about this release.";
                default:
                    throw new ArgumentException($"No PR body template found for repo {repoName}");
            }
        }
    }
}
