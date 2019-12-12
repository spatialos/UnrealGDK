

# 4. 连接到本地部署

使用本地开发工作流程要求您为 iOS 客户端设置本地连接，启动本地部署并构建 iOS 包。在iOS 设备上启动 iOS 客户端时，iOS 客户端会自动连接到本地部署。

在 Unreal 编辑器中打开您的项目并完成以下步骤：

1. 从 **Edit** 菜单中，选择 **Project Settings** 显示 Project Settings 编辑器。
2. 向下滚动到 **SpatialOS GDK for Unreal** 并选择 **Runtime Settings** 显示 SpatialOS Runtime Settings 面板。
3. 在 **Local Connection** 区域，在 **Default Receptionist Host** 字段中，输入 Windows PC 在局域网中的 IP 地址，例如，`192.168.0.123`。
    **注意事项**：
    - 确保输入正确的 IP 地址，以便在开发 PC 上启动服务器后，iOS 客户端会自动连接到部署。
    - 确保您的开发 PC 和 Mac 位于同一局域网中。
    <%(Lightbox image="{{assetRoot}}assets/ios/ip-address.png")%>
4. 在 **Cloud Connection** 区域，确保已去除 **Use Development Authentication Flow** 复选框项。否则，会使用云开发工作流程。
5. 切换到 **SpatialOS GDK for Unreal** > **Editor Settings** > **Launch** 操作界面，在 **Command line flags for local launch** 栏，添加以下新命令行 flag 将 SpatialOS 运行时的位置暴露给外部网络，其中 `Your LAN IP` 是您在步骤 3 中输入的 IP 地址：
    `--runtime_ip=<Your LAN IP>`
6. 在 **[GDK toolbar](https://docs.improbable.io/unreal/latest/content/unreal-editor-interface/toolbars#spatialos-gdk-for-unreal-toolbar)** 中，单击 **Start** 在开发 PC 上启动本地部署。
7. 下拉 **File** 菜单，点击 **Package Project** > **iOS** 构建 iOS 包。**注意**：iOS 包构建可能需要很长时间。
8. 在 iOS 设备上，安装 iOS 包。
9. 在 iOS 设备上，启动 iOS 客户端，然后游戏应用程序自动连接到本地部署。

<br/>
<br/>------------<br/>
_2019-08-06 Topic added with editorial review._
