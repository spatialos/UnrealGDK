using System;
using System.Diagnostics;
using System.IO;
using LibGit2Sharp;

namespace ReleaseTool
{
    /// <summary>
    ///     This class provides helper methods for git. It uses a hybrid approach using both LibGit2Sharp for most methods,
    ///     but uses Processes to invoke remote methods (pull, fetch, push). This is because LibGit2Sharp does not support
    ///     ssh authentication yet!
    /// </summary>
    internal class GitClient : IDisposable
    {
        private static readonly NLog.Logger Logger = NLog.LogManager.GetCurrentClassLogger();

        private const string DefaultRemote = "origin";

        private const string GitCommand = "git";
        private const string ForcePushArgumentsTemplate = "push -f {0} HEAD:refs/heads/{1}";
        private const string FetchArguments = "fetch {0}";
        private const string CloneArgumentsTemplate = "clone {0} {1}";
        private const string AddRemoteArgumentsTemplate = "remote add {0} {1}";

        private const string RemoteBranchRefTemplate = "{1}/{0}";

        public string RepositoryPath { get; }
        private readonly IRepository repo;

        public static GitClient FromRemote(string remoteUrl)
        {
            var repositoryPath = Path.Combine(Path.GetTempPath(), Path.GetRandomFileName());
            Directory.CreateDirectory(repositoryPath);
            Clone(remoteUrl, repositoryPath);

            return new GitClient(repositoryPath);
        }

        private GitClient(string repositoryPath)
        {
            RepositoryPath = repositoryPath;
            repo = new Repository($"{repositoryPath}/.git/");
        }

        public void Dispose()
        {
            repo?.Dispose();
        }

        public void CheckoutRemoteBranch(string branch, string remote = null)
        {
            var branchRef = string.Format(RemoteBranchRefTemplate, branch, remote ?? DefaultRemote);
            Logger.Info("Checking out branch... {0}", branchRef);
            Commands.Checkout(repo, branchRef);
        }


        public void StageFile(string filePath)
        {
            Logger.Info("Staging... {0}", filePath);

            if (!Path.IsPathRooted(filePath))
            {
                filePath = Path.Combine(RepositoryPath, filePath);
            }

            Commands.Stage(repo, filePath);
        }

        public void Commit(string commitMessage)
        {
            Logger.Info("Committing...");

            var signature = repo.Config.BuildSignature(DateTimeOffset.Now);
            repo.Commit(commitMessage, signature, signature, new CommitOptions { AllowEmptyCommit = true });
        }

        public void Fetch(string remote = null)
        {
            Logger.Info("Fetching from remote...");

            RunGitCommand("fetch", string.Format(FetchArguments, remote ?? DefaultRemote), RepositoryPath);
        }

        public void ForcePush(string remoteBranchName)
        {
            Logger.Info("Pushing to remote...");

            var pushArguments = string.Format(ForcePushArgumentsTemplate, DefaultRemote, remoteBranchName);

            RunGitCommand("push branch", pushArguments, RepositoryPath);
        }

        public void AddRemote(string name, string remoteUrl)
        {
            Logger.Info($"Adding remote {remoteUrl} as {name}...");
            RunGitCommand("add remote", string.Format(AddRemoteArgumentsTemplate, name, remoteUrl), RepositoryPath);
        }

        private static void Clone(string remoteUrl, string targetDirectory)
        {
            Logger.Info($"Cloning {remoteUrl} into {targetDirectory}...");
            RunGitCommand("clone repository",
                string.Format(CloneArgumentsTemplate, remoteUrl, $"\"{targetDirectory}\""));
        }

        private static void RunGitCommand(string description, string arguments, string workingDir = null)
        {
            Logger.Debug("Attempting to {0}. Running command [{1} {2}]", description,
                GitCommand, arguments);

            var procInfo = new ProcessStartInfo(GitCommand, arguments)
            {
                UseShellExecute = false
            };

            if (workingDir != null)
            {
                procInfo.WorkingDirectory = workingDir;
            }

            using (var process = Process.Start(procInfo))
            {
                if (process != null)
                {
                    process.WaitForExit();

                    if (process.ExitCode == 0)
                    {
                        return;
                    }
                }
            }

            throw new InvalidOperationException($"Failed to {description}.");
        }

        public Commit GetHeadCommit()
        {
            return repo.Head.Tip;
        }
    }
}
