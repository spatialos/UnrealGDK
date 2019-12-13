
# [Deprecated] Multiple deployments for session-based games
## 5: Launch multiple session-based deployments

### Step 1: Launch deployments using the Deployment Manager

Now you use the Deployment Manager to launch multiple session-based cloud deployments. 

To do this: 

1. In File Explorer, navigate to your Deployment Manager repository.
1. Select **File** > **Open Windows Powershell** > **Open Windows Powershell as administrator**.
2. In Powershell, run the following two commands, making sure to replace the text in `<....>` according to your project. See the list below the commands: 
	* `.\publish-linux-workers.ps1 <launch config path> <snapshot path>`
	* `.\cloud-launch.ps1 <assembly name> <deployment name>`

* `<launch config path>` is `UnrealGDKExampleProject/spatial/one_worker_test.json`.
* `<snapshot path>` is the path to the snapshot file you generated in the [Example Project set up guide]({{urlRoot}}/content/get-started/example-project/exampleproject-local-deployment)
*  `<deployment manager assembly name>` is a name that you choose for your Deployment Manager assembly. This **must be a different name** to the `<assembly name>` you created when you uploaded your assembly in [Step 3: Build and upload your workers]({{urlRoot}}/content/tutorials/deployment-manager/tutorial-deploymentmgr-workers#step-3-upload-your-assembly).
* `<deployment name>` is a name that you choose for your deployment.

For example, your commands might look like:

* `.\publish-linux-workers.ps1 ..\UnrealGDKExampleProject\spatial\one_worker_test.json ..\UnrealGDKExampleProject\spatial\snapshots\default.snapshot`
* `.\cloud-launch.ps1 deploymentassembly deploymentmanager`

</br>

### Step 2: Review the deployments in the SpatialOS Console

After running these commands, the SpatialOS CLI automatically deploys your project. The Deployment Overview Console page opens automatically after your project has successfully deployed.

With the Console open, on the Deployment Overview page, select **Projects** to see a list of your deployments. You should see three deployments in this list:

* **deploymentmanager**
* **session_0**
* **session_1**

<%(Lightbox image="{{assetRoot}}assets/deployment-manager/deploymentmgr-consoledeployments.png")%><br/>

_Image: The SpatialOS Console Projects page,  with running deployments highlighted._

If you can see these three deployments, then you have successfully launched multiple deployments of the Example project using the Deployment Manager. By default, these deployment sessions run for 60 minutes before restarting automatically. 
</br>
</br>

**Troubleshooting**</br>

[block:html]
{
  "html": "<button class=\"collapsible\">The PowerShell Cannot find path '\UnrealGDKExampleProject\spatial\one_worker_test.json' because it does not exist.</button><div>\n\n\nMake sure your Deployment Manager repository is in the same parent directory as your Example Project repository. If it is not, you have to edit the file paths in these Powershell commands to point to wherever your Example Project `one_worker_test.json` and `default.snapshot` files are located. \n\n\n</div>"
}
[/block]


</br>
</br>
### **> Next**: [6. Play your game]({{urlRoot}}/content/tutorials/deployment-manager/tutorial-deploymentmgr-play)



<br/>------<br/>
_2019-10-16 Tutorial deprecated_<br/>
_2019-05-21 Page added with editorial review_
