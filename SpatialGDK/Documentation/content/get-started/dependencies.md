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
- <a href="https://visualstudio.microsoft.com/vs/older-downloads/" data-track-link="Clicked VS 2017|product=Docs|platform=Win|label=Win">**Visual Studio 2017**</a>.
    
    During the installation of Visual Studio, select the following items in the Workloads tab:
    - **Universal Windows Platform development**<br>
    - **.NET desktop development** <br>
      - You must also select the **.NET Framework 4.6.2 development tools**
    - **Desktop development with C++**<br>
    - **Game development with C++**, including the optional **Unreal Engine installer** component.

[block:callout]
{
  "type": "warn",
  "body": "Make sure you install Visual Studio 2017. The GDK does not currently support any other version."
}
[/block]

- [**Linux Cross-Compilation toolchain**](https://docs.unrealengine.com/en-US/Platforms/Linux/GettingStarted/index.html)
    - You need to download and install Unreal's Linux Cross-Compilation toolchain in order to build Linux server-workers using your Windows machine. Use the Unreal documentation link above to install `-v13 clang-7.0.1-based`, the appropriate toolchain for Unreal Engine 4.22.

</br>
#### **> Next:** [2 - Set up the fork and plugin]({{urlRoot}}/content/get-started/build-unreal-fork.md)

<br/>

------</br>
_2019-08-08 Page updated with editorial review: text clarification only._
_2019-07-22 Page updated with limited editorial review_
