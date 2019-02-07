# Get started: 1 - Get the dependencies

To start using the GDK for Unreal, you need to ensure you have the correct software installed and that your machine is capable of running Unreal Engine. 

## Sign up for a SpatialOS account, or make sure you are logged in

If you have already signed up, make sure you are logged into [Improbable.io](https://improbable.io). If you are logged in, you should see your picture in the top right of this page. If you are not logged in, select __Sign in__ at the top of this page and follow the instructions.

If you have not signed up before, you can sign up [here](<https://improbable.io/get-spatialos>).

## Set up your machine

### Step 1: Hardware

- Ensure your machine meets the minimum hardware requirements for Unreal Engine. 

Refer to the <a href="https://docs.unrealengine.com/en-US/GettingStarted/RecommendedSpecifications" data-track-link="Clicked UE4 Recommendations|product=Docs|platform=Win|label=Win" target="_blank">Unreal Engine hardware recommendations</a> for further information about the minimum hardware requirements.

- Recommended storage: 60GB+ available space

### Step 2: Network settings

To configure your network to work with SpatialOS, refer to the [SpatialOS network settings](https://docs.improbable.io/reference/latest/shared/get-started/requirements#network-settings). 

### Step 3: Software

To build the GDK for Unreal you need the following software installed on your machine:

- **Windows 10,** with Command Prompt or PowerShell as your terminal.

  - The GDK for Unreal is only supported on Windows 10. 

- <a href="https://gitforwindows.org" data-track-link="Clicked GIT for Windows|product=Docs|platform=Win|label=Win" target="_blank">**Git for Windows**</a>

  - You need to download and install Git for windows to clone the GDK and Unreal Engine GitHub repositories. 

- <a href="https://console.improbable.io/installer/download/stable/latest/win" data-track-link="Clicked Download SpatialOS|product=Docs|platform=Win|label=Win" target="_blank">**SpatialOS version 13.2**</a>
    - This installs the [SpatialOS CLI]({{urlRoot}}/content/glossary#spatial-command-line-tool-cli), the [SpatialOS Launcher]({{urlRoot}}/content/glossary#launcher), and 32-bit and 64-bit Visual C++ Redistributables.

- The <a href="https://developer.microsoft.com/en-us/windows/downloads/sdk-archive" data-track-link="Clicked Windows SDK 8.1|product=Docs|platform=Win|label=Win" target="_blank">**Windows SDK 8.1**</a>

  - The Windows SDK 8.1 provides libraries, headers, metadata and tools required for building Windows 10 applications. 

- **Visual Studio** <a href="https://visualstudio.microsoft.com/vs/older-downloads/" data-track-link="Clicked VS 2015|product=Docs|platform=Win|label=Win" target="_blank">2015</a> or <a href="https://visualstudio.microsoft.com/downloads/2017" data-track-link="Clicked VS 2017|product=Docs|platform=Win|label=Win">2017</a> (we recommend 2017). During the installation, select the following items in the Workloads tab:
    - Universal Windows Platform development<br>
    - Desktop development with C++<br>
    - Game development with C++

#### Next: [Get and build the GDK’s Unreal Engine Fork]({{urlRoot}}/content/get-started/build-unreal-fork.md)
