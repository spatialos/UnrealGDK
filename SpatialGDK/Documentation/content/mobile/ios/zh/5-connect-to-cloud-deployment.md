

# 5. 连接到云部署

使用云开发工作流程需要您完成以下步骤：

1. 将 UnrealServer worker 连接到云部署
2. 将 iOS 客户端连接到云部署

在开始之前，请确保已 [启动云部署](https://docs.improbable.io/unreal/latest/content/get-started/gdk-template#6-launch-a-cloud-deployment)。

## 将 UnrealServer worker 连接到云部署

1. 在终端中，将目录更改为 `<ProjectRoot>` 目录下的 spatial 文件夹，运行以下命令本地启动 [receptionist service](https://docs.improbable.io/reference/latest/cppsdk/using/connecting#connecting-a-managed-worker) ，其中 `deployment name` 是部署的名称：

    `spatial cloud connect external <deployment name>`
2. 在 Unreal 编辑器中打开您的项目，单击 **Play In Editor** 启动 UnrealServer worker 和游戏客户端。UnrealServer worker 通过 `local receptionist` 连接到云部署。

## 将 iOS 客户端连接到云部署

SpatialOS 为客户端提供了一种连接到云部署的方法，而无需开发自己的身份验证和登录服务器，这称为 **development authentication flow (开发身份验证流程)**。它集成在 `master` 分支中，因此您可以在创建 DevelopmentAuthenticationToken 后使用它。

有关更多信息，请参阅 [Using the development authentication flow](https://docs.improbable.io/reference/latest/shared/auth/development-authentication)。

### 创建 TokenSecret

在开始之前，请完成以下步骤以创建 TokenSecret：

1. 打开您选择的终端，然后导航到 `<ProjectRoot>\spatial` 文件夹：
2. 运行以下代码块：

    `spatial project auth dev-auth-token create --description="my description" --lifetime="24h10m20s"`

以下示例显示了返回结果，其中包含 ID，到期时间和 `TokenSecret`：
[block:code]
{
  "codes": [
    {
      "code": "{\"developmentAuthenticationToken\":{\"id\":\"0a51b04c-ab74-4276-88c5-4e2aa148509e\",\"projectName\":\"beta_glucose_purple_754\",\"description\":\"my description\",\"creationTime\":\"2019-07-11T10:40:36.469942565Z\",\"expirationTime\":\"2019-07-12T10:50:56.469942565Z\"},\"tokenSecret\":\"MGE1MWIwNGMtYWI3NC00Mjc2LTg4YzUtNGUyYWExNDg1MDllOjo0MGQyNzk0NC04YWMyLTRjMjQtOTc2MC03MGM3OTY3OTA0MDM=\"}",
      "language": "text"
    }
  ]
}
[/block]

### 启用云开发工作流程

创建 token 后，请完成以下步骤以启用云开发工作流：

1. 在 Unreal 编辑器中打开您的项目。从 **Edit** 菜单中，选择 **Project Settings** 显示 Project Settings 编辑器。
2. 向下滚动到 **SpatialOS GDK for Unreal** 并选择 **Runtime Settings** 显示 SpatialOS Runtime Settings 面板。
3. 从 **Cloud Connection** 部分，勾选 **Use Development Authentication Flow** 复选框。
4. 在 **Development Authentication Token** 字段中，输入您创建的 DevelopmentAuthenticationToken 中的 `TokenSecret`。
5. 将 `dev_login` flag 添加到您启动的云部署中。您可以选择以下方法之一：
    - 使用SpatialOS CLI。有关更多信息，请参阅[空间项目部署标记添加](https://docs.improbable.io/reference/latest/shared/spatial-cli/spatial-project-deployment-tags-add)。
    - 在 [Console](https://console.improbable.io/projects) 中，打开您的部署，并在 **Details** > **Tags** 中添加 `dev_login` 标记。

    > **注意**：如果有多个部署使用 `dev_login` flag 运行，请在 **Development Deployment to Connect** 字段中指定要使用的部署的名称。否则，当 **Development Deployment to Connect** 字段为空时，将使用部署列表中的第一个部署。
    
    <%(Lightbox image="{{assetRoot}}assets/ios/dev-login-tag.png")%>
6. 在 **[Unreal toolbar](https://docs.improbable.io/unreal/latest/content/unreal-editor-interface/toolbars#unreal-toolbar)** 中，单击 **Play** 以运行游戏。

<br/>
<br/>------------<br/>
_2019-08-06 Topic added with editorial review._
