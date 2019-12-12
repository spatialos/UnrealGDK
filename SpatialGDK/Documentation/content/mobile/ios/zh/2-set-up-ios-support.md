

# 2. 设置 iOS 支持

**本页使用的术语**

- `<GameRoot>` - 包含项目的 `.uproject` 文件和 Source 文件夹的目录。
- `<ProjectRoot>` - 包含 `<GameRoot>` 目录的目录。
- `<YourProject>` - 项目名称和 `.uproject `文件（例如，`<GameRoot>\YourProject.uproject`）。

## 获取 iOS 分支

完成以下步骤以获取 Unreal GDK的 iOS 分支，构建依赖项并重建项目：

1. 在您选择的终端中，运行``git checkout ios-eval``查看包含iOS开发功能的分支。
2. 打开 **File Explorer**，导航到 Unreal GDK 库 `<GameRoot>\Plugins\UnrealGDK\...`的根目录，然后双击 `Setup.bat` 下载在 Windows 上打包 iOS 所需的依赖项。如果您无法下载某些软件包，请完成以下步骤:
   
    1. 在终端中，导航到 `<GameRoot>\Plugins\UnrealGDK\SpatialGDK\Binaries\ThirdParty\Improbable Improbable`。
    2. 使用长时间超时手动下载这些软件包，例如，使用以下命令：

        `spatial package retrieve worker_sdk c-static-fullylinked-arm-clang_libcpp-ios 13.7.1 IOS --unzip --grpc_client_connection_timeout_s=10m`")%>
3. 在 **文件资源管理器** 中，导航到 `<GameRoot>`目录，右键单击 `<YourProject>`.uproject 并选择 **Generate Visual Studio Project files**。
4. 在同一目录中，双击 **`<YourProject>`.sln** 以使用 Visual Studio 打开它。
5. 在 Solution Explorer 窗口中，右键单击 **`<YourProject>`** 并选择 **Build**。
6. 当 Visual Studio 完成项目构建后，右键单击 **`<YourProject>`.uproject ** 以在 Unreal 编辑器中打开项目。

点击 **编辑**> **项目设置**> **SpatialOS GDK for Unreal**，您可以在 **[Runtime Settings](https://docs.improbable.io/unreal/latest/content/unreal-editor-interface/runtime-settings)** 面板中找到更多设置。使用本地和云开发流程时，需要进行配置。

## 设置 iOS 开发签名证书

获取 Apple Developer 证书设置以使用您的 UE4 项目。有关更多信息，请参阅 [iOS配置](https://docs.unrealengine.com/en-US/Platforms/Mobile/iOS/Provisioning/index.html)。

<br/>
<br/>------------<br/>
_2019-08-06 Topic added with editorial review._
