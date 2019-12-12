

# 3. 设置 MacOS 系统的开发环境

> **注意**：目前仅用于演示早期 MacOS 对 Unreal GDK的支持。本节中涉及的操作可视为临时**破解**方案。

在某些情况下，例如在 iOS 设备上进行调试和性能分析，您可能希望在 MacOS 上开发游戏。

**本页使用的术语**

- `<GameRoot>` - 包含项目的 .uproject 文件和 Source 文件夹的目录。
- `<ProjectRoot>` - 包含 `<GameRoot>` 目录的目录。
- `<YourProject>` - 项目名称和 `.uproject` 文件（例如，`\<GameRoot>\YourProject.uproject`)。

## 系统要求

在开始之前，请确保下载并安装以下软件和工具：

* [Xcode](https://developer.apple.com/xcode/)：最新版本的 Xcode
* Java: [Gradle](https://gradle.org/install) v2.10
* [Mono](https://www.mono-project.com/download/stable/): v5.x
* [适用于 MacOS 的SpatialOS](https://docs.improbable.io/reference/latest/shared/setup/mac)

## 获取 Unreal GDK 的 MacOS 分支

1. 打开您选择的终端并导航到 `<GameRoot>/Plugins/UnrealGDK`。
2. 运行以下命令以获取 MacOS 分支：
   `git checkout bugfix/UNR-1450-macos-toolbar`
3. 构建 GDK：

    1. 打开你选择的终端，导航到 `<Game Root>/Plugins/UnrealGDK/`，并运行 `Setup.sh`。
    2. 如果使用 Unreal GDK v0.4.0, v0.4.1, 或 v0.4.2，则必须应用此修复：
        1. 打开您选择的终端，导航到 `<Game Root>/Plugins/UnrealGDK/SpatialGDK/Binaries/ThirdParty/Improbable/MacOS`。
        2. 运行以下命令行：
        `install_name_tool -id @rpath/libworker.dylib libworker.dylib`
        3. 打开 **Finder**，导航到 `<YourProject>` 目录，右键单击 `.uproject` 文件并点击 **Services** > **Generate Xcode Project** 创建 `.xcworkspace` 项目工作区。
        4. 打开您创建的工作区。
        5. 选择您的游戏项目，然后点击 **Run** 按钮。

## 在本地运行游戏

1. 打开 Unreal 编辑器并删除 `Content / Spatial / SchemaDatabase.uasset`；否则，在您生成完整的模式语言时，可能会出错。
2. 单击 **[GDK toolbar](https://docs.improbable.io/unreal/latest/content/unreal-editor-interface/toolbars#spatialos-gdk-for-unreal-toolbar)** 的 **Schema** 生成模式语言。
3. 单击 **[GDK toolbar](https://docs.improbable.io/unreal/latest/content/unreal-editor-interface/toolbars#spatialos-gdk-for-unreal-toolbar)** 的 **Snapshot** 生成快照。
4. 启动 SpatialOS 运行时。由于 Unreal 编辑器中的 **Start** 按钮当前不起作用，因此您必须完成以下步骤：
    1. 打开终端，导航到 `<ProjectRoot>/spatial`。
    2. 运行以下命令。如果您尚未登录 SpatialOS，可能会提示您登录 SpatialOS 帐户。

   [block:code]
{
  "codes": [
  {
      "code": "   spatial worker build build-config \n spatial local launch --runtime_ip=<Your Mac's LAN IP>\n",
      "language": "text"
    }
  ]
}
[/block]
5. 在 **[Unreal toolbar](https://docs.improbable.io/unreal/latest/content/unreal-editor-interface/toolbars#unreal-toolbar)** 中，单击 **Play** 在本地运行您的游戏。

<br/>
<br/>------------<br/>
_2019-08-06 Topic added with editorial review._
