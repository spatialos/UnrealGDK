<%(TOC)%>
# Get started
## 1 - Get the dependencies

To start using the GDK for Unreal, you need to ensure you have the correct software installed and that your machine is capable of running Unreal Engine. 

### Step 1: Login

Before you start, make sure you are logged in to your SpatialOS account.

* If you have already signed up to SpatialOS, make sure you are logged into [Improbable.io](https://improbable.io). If you are logged in, you should see your picture in the top right of this page. If you are not logged in, select __Sign in__ at the top of this page and follow the instructions.

* If you have not signed up before, you can sign up [here](<https://improbable.io/get-spatialos>).

### Step 2: Hardware

- Ensure your machine meets the minimum hardware requirements for Unreal Engine. 
</br>See the <a href="https://docs.unrealengine.com/en-US/GettingStarted/RecommendedSpecifications" data-track-link="Clicked UE4 Recommendations|product=Docs|platform=Win|label=Win" target="_blank">Unreal Engine hardware recommendations</a> for further information about the minimum hardware requirements.

- Recommended storage: 60GB+ available space

### Step 3: Software

To build the GDK for Unreal you need the following software installed on your machine:

- **Windows 10** with Command Prompt or PowerShell.
  - **Note:** The GDK for Unreal is currently only supported on Windows 10.
- **A Git client**
  - In order to clone the GDK and Unreal Engine repos, we recommend using a Git client such as <a href="https://gitforwindows.org" data-track-link="Clicked GIT for Windows|product=Docs|platform=Win|label=Win" target="_blank">**Git for Windows**</a>
- <a href="https://console.improbable.io/installer/download/stable/latest/win" data-track-link="Clicked Download SpatialOS|product=Docs|platform=Win|label=Win" target="_blank">**SpatialOS**</a>
    - This installs the [SpatialOS CLI]({{urlRoot}}/content/glossary#spatialos-command-line-tool-cli), the [SpatialOS Launcher]({{urlRoot}}/content/glossary#launcher), and 32-bit and 64-bit Visual C++ Redistributables.
- The [**DirectX End-User Runtimes (June 2010)**](https://www.microsoft.com/en-us/download/details.aspx?id=8109)

  - You need the DirectX End-User Runtime to run Unreal Engine 4 clients.
- <a href="https://visualstudio.microsoft.com/vs/" data-track-link="Clicked VS 2019|product=Docs|platform=Win|label=Win">**Visual Studio 2019**</a><a href="https://visualstudio.microsoft.com/vs/older-downloads/"" data-track-link="Clicked VS 2017|product=Docs|platform=Win|label=Win"> (or 2017)</a>.
    
    If you're using **Visual Studio 2019**, during the installation of Visual Studio, select the following items in the Workloads tab:

    - **Universal Windows Platform development**
    - **.NET desktop development**
      - You must also select the **.NET Framework 4.6.2 development tools** component in the Installation details section.
    - **Desktop development with C++**
      - You must then deselect **MSVC v142 - VS 2019 C++ x64/x86 build tools (v14.23)**, which was added as part of the **Desktop development with C++** Workload. You will be notified that: "If you continue, we'll remove the componenet and any items liseted above that depend on it." Select remove to confirm your decision.
    - Lastly, add **MSVC v142 - VS 2019 C++ x64/x86 build tools (v14.22)** from the Individual components tab.

    If you're using **Visual Studio 2017**, during the installation of Visual Studio, select the following items in the Workloads tab:

    - **Universal Windows Platform development**<br>
    - **.NET desktop development** <br>
      - You must also select the **.NET Framework 4.6.2 development tools**
    - **Desktop development with C++**<br>
    - **Game development with C++**, including the optional **Unreal Engine installer** component.

- [**Linux Cross-Compilation toolchain**](https://docs.unrealengine.com/en-US/Platforms/Linux/GettingStarted/index.html)
    - You need to download and install Unreal's Linux Cross-Compilation toolchain in order to build Linux server-workers using your Windows machine. Use the Unreal documentation link above to install `-v15 clang-8.0.1-based`, the appropriate toolchain for Unreal Engine 4.23.

</br>
#### **> Next:** [2 - Set up the fork and plugin]({{urlRoot}}/content/get-started/build-unreal-fork.md)

<br/>

------</br>
_2020-01-03 Page updated with editorial review: Git recommendation clarified._<br/>
_2019-11-08 Updated version of Linux Cross-Compilation toolchain._<br/>
_2019-08-08 Page updated with editorial review: text clarification only._<br/>
_2019-07-22 Page updated with limited editorial review_
