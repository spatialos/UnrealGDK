using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading;
using CommandLine;
using Octokit;

namespace ReleaseTool
{
    /// <summary>
    ///     Runs the commands required for releasing a candidate.
    ///     * Merges the candidate branch into the release branch.
    ///     * Pushes the release branch.
    ///     * Creates a GitHub release draft.
    ///     * Creates a PR from the release-branch (defaults to release) branch into the source-branch (defaults to master).
    /// </summary>
    internal class PrepFullReleaseCommand
    {
        private static readonly NLog.Logger Logger = NLog.LogManager.GetCurrentClassLogger();

        // Changelog file configuration
        private const string CandidateCommitMessageTemplate = "Prepare GDK for Unreal release {0}.";

        [Verb("prepfullrelease", HelpText = "Prepare a release candidate branch for the full release.")]
        public class Options : GitHubClient.IGitHubOptions
        {
            [Value(0, MetaName = "version", HelpText = "The version that is being released.")]
            public string Version { get; set; }

            [Option("candidate-branch", HelpText = "The candidate branch name.", Required = true)]
            public string CandidateBranch { get; set; }

            [Option("git-repository-name", HelpText = "The Git repository that we are targeting.", Required = true)]
            public string GitRepoName { get; set; }

            [Option("github-organization", HelpText = "The Github Organization that contains the targeted repository.", Required = true)]
            public string GithubOrgName { get; set; }

            public string GitHubTokenFile { get; set; }

            public string GitHubToken { get; set; }

            public string MetadataFilePath { get; set; }
        }

        private readonly Options options;

        public PrepFullReleaseCommand(Options options)
        {
            this.options = options;
        }

        /*
         *     This command does the necessary preparations for releasing an rc-branch:
         *         1. Re-pointing all <RepoName>Version.txt files
         */
        public int Run()
        {
            Common.VerifySemanticVersioningFormat(options.Version);
            var gitRepoName = options.GitRepoName;
            var remoteUrl = string.Format(Common.RepoUrlTemplate, options.GithubOrgName, gitRepoName);
            try
            {
                // 1. Clones the source repo.
                using (var gitClient = GitClient.FromRemote(remoteUrl))
                {
                    // 2. Checks out the candidate branch, which defaults to 4.xx-SpatialOSUnrealGDK-x.y.z-rc in UnrealEngine and x.y.z-rc in all other repos.
                    gitClient.CheckoutRemoteBranch(options.CandidateBranch);

                    // 3. Makes repo-specific changes for prepping the release (e.g. updating version files, formatting the CHANGELOG).
                    if (Common.UpdateVersionFilesButNotEngine(gitClient, gitRepoName, options.Version, Logger))
                    {
                        // 4. Commit changes and push them to a remote candidate branch.
                        gitClient.Commit(string.Format(CandidateCommitMessageTemplate, options.Version));
                        gitClient.ForcePush(options.CandidateBranch);
                        Logger.Info($"Updated branch '{options.CandidateBranch}' in preparation for the full release.");
                    }
                    else
                    {
                        Logger.Info($"Tried to update branch '{options.CandidateBranch}' in preparation for the full release, but it was already up-to-date.");
                    }
                }
            }
            catch (Exception e)
            {
                Logger.Error(e, $"ERROR: Unable to update '{options.CandidateBranch}'. Error: {0}", e.Message);
                return 1;
            }

            return 0;
        }
    }
}
