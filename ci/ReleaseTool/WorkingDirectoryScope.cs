using System;

namespace ReleaseTool
{
    public class WorkingDirectoryScope : IDisposable
    {
        private readonly string oldWorkingDirectory;

        public WorkingDirectoryScope(string newWorkingDirectory)
        {
            oldWorkingDirectory = Environment.CurrentDirectory;
            Environment.CurrentDirectory = newWorkingDirectory;
        }

        public void Dispose()
        {
            Environment.CurrentDirectory = oldWorkingDirectory;
        }
    }
}
