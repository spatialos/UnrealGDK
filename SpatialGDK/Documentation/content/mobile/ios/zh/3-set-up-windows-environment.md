

# 3. 设置 Windows 系统的开发环境

在 Windows 机器上实现在 Mac 上远程编译以创建 iOS 构建。有关详细信息，请参阅 [Building for iOS on Windows](https://docs.unrealengine.com/en-US/Platforms/Mobile/iOS/Windows/index.html)。

## 故障排除：远程构建

**注意**：设置 Unreal 引擎的远程构建时可能会遇到以下问题。要解决这些问题，请单击以下某个问题以查看说明和解决方案：

- [Permissions are too open](#permissions-are-too-open) (权限过于开放)
- [Identity file not accessible](#identity-file-not-accessible) （身份文件无法访问）
- [Invalid path of ssh](#invalid-path-of-ssh) (ssh 的路径无效)

### Permissions are too open

**描述**：使用生成的私钥设置 iOS 构建时，您可能会在 Unreal 编辑器中收到以下错误消息：

```
@ WARNING: UNPROTECTED PRIVATE KEY FILE! @

Permissions 0660 for '/cygdrive/C/Users/<username>/AppData/Roaming/Unreal Engine/UnrealBuildTool/SSHKeys/<ip>/<username>/RemoteToolChainPrivate.key' are too open.

It is recommended that your private key files are NOT accessible by others. This private key will be ignored.
```

**解决方案**：要解决此问题，请完成以下步骤以使用 Cygwin 更改私钥的权限：

1. 下载 Cygwin installer 并运行 `setup.exe`：

    1. 在 Cygwin 设置窗口中，单击 **Next**。
    2. 选择 **Install from Internet** 并单击 **Next**。
    3. 选择要安装 Cygwin 的根目录，单击 **All Users**，然后单击 **Next**。您可以使用默认的根目录。
    4. 选择存储 Cygwin 安装程序下载的安装文件的目录，然后单击 **Next**。您可以使用默认目录。
    5. 单击 **Use System Proxy Settings**，然后单击 **Next**。
    6. 选择下载站点。您可以默认使用选定的下载站点。如果无法从该站点下载，请尝试其他下载站点。然后，单击 **Next**。
    7. 在 Select Packages 窗口中，在 **Search** 栏 输入 `cygwin`。
    8. 选择 **base-cygwin** 和 **cygwin**，然后单击 **Next**。
    9. 查看并确认更改。
2. 打开 Cygwin64 终端。
3. 运行以下命令导航到包含私钥的文件夹，其中 `ip` 是用于设置远程构建的 Mac 的 IP 地址：
   `cd /cygdrive/C/Users/<Windows username>/AppData/Roaming/Unreal Engine/UnrealBuildTool/SSHKeys/<ip>/<Mac username>/`
4. 运行以下命令降级私钥的权限：
   ```
   chgrp Users RemoteToolChainPrivate.key
   chmod 600 RemoteToolChainPrivate.key
   ```

### Identity file not accessible

**描述**：在您解决 **权限过于开放** 的问题后，您可能会收到以下错误消息：

```
Packaging (iOS):   Warning: Identity file /cygdrive/C/Users/<username>/AppData/Roaming/Unreal Engine/UnrealBuildTool/SSHKeys/<ip>/<username>/RemoteToolChainPrivate.key not accessible: No such file or directory.
```

**说明**：发生错误是因为您安装了 Windows 本机版本的 SSH（通常是Git），并且 SSH 无法识别Cygwin 路径。要检查 Cygwin 的 `ssh.exe` 是否作为第一项列出，运行  `where ssh`。

**解决方案**：要使用 Cygwin 的 `OpenSSH`，请完成以下步骤：

1. 安装 Cygwin 的 `OpenSSH` 包。有关更多信息，参阅 [Install Cygwin on WIndow](https://pantheon.io/docs/cygwin-windows/)。
2. 修改系统路径以确保优先加载 Cygwin 的 `ssh`：
    1. 打开 File Explorer 并导航到 **Control Panel** > **System and Security** > **System** > **Advanced system settings** > **Advanced** > **Environment variables**，打开 Environment Variables 对话框。
    2. 在 **System variables** 中，向下滚动并显示 **Path** 变量。单击 **Edit**。
    3. 单击 **New** 并将路径添加 Cygwin 的 Bin 目录。以下示例将 Cygwin 路径显示为 `C:\tools\cygwin64\bin`
    4. 如果在 Edit environment variable 窗口中看到 `OpenSSH`，将 Cygwin 路径移到 `OpenSSH`上方。
    5. 单击 **OK** 关闭窗口。
    <%(Lightbox image="{{assetRoot}}assets/ios/identify-file-not-accessible.png")%>
3. 重新打开终端并再次运行 `where ssh`，确认 Cygwin 的 `ssh.exe` 被列为第一项。

### Invalid path of ssh

**描述**：构建 iOS 包时，您可能会收到以下错误消息：

`rsync error : error in rsync protocol data stream (code 12) at /home/lapo/packaging/rsync-3.0.4-1/src/rsync-3.0.4/io.c(632) [sender=3.0.4]`

**解决方案**：要解决此问题，请应用此 [commit](https://github.com/EpicGames/UnrealEngine/commit/3ab2ab6cebd701e051a70a4f92d278494701e737)。

<br/>
<br/>------------<br/>
_2019-08-06 Topic added with editorial review._
