# This script is based on https://github.com/spatialos/gdk-for-unity-shared-ci/blob/master/scripts/purge.ps1
#
# This script is used to kill all dangling procceses at the end of a Buildkite build.
#
# It does this by using [handle](https://docs.microsoft.com/en-us/sysinternals/downloads/handle)
# to find all processes of a particular name which have file locks on our build (project) directory.
# We can then attempt to kill each of these processes if any exist.
#
# The expected usage is as follows:
#   powershell -NoProfile -NonInteractive scripts/purge.ps1 -projectRoot <project root>

cd $projectRoot

function Kill-Dangling-Processes {
	param( [string]$ProcessName )

	echo "Looking for ${ProcessName} processes to kill..."
	$Dir=$(pwd).Path
	$Out=$(handle64 -accepteula -p "$($ProcessName).exe" $Dir)
	ForEach ($line in $($OUT -split "`r`n"))
	{
		$Result = $([regex]::Match("$line", "pid: (.*) type"))
		if ($Result.Success)
		{
			$ppid = $Result.Groups[1].Value
			taskkill /f /pid $ppid
		}
	}
}

$ProcessesToKill = @("AutomationToolLauncher")

ForEach ($Process in $ProcessesToKill)
{
	Kill-Dangling-Processes $Process
}