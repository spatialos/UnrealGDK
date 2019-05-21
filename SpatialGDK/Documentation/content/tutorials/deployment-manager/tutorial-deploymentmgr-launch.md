## Step 6: Launch multiple session-based cloud deployments

Now you use the Deployment Manager to launch multiple session-based cloud deployments. 

To do this: 

1. In File Explorer, navigate to your Deployment Manager repository.
1. Select **File.**
1. Select **Open Windows Powershell**, then select **Open Windows Powershell as administrator.**
1. In Powershell, run the following SpatialOS CLI commands: 
	* `.\publish-linux-workers.ps1 <launch config path> <snapshot path>`
	* `.\cloud-launch.ps1 <assembly name> <deployment name>`
Where:

* `<launch config path>` is the file path to the` one_worker_test.json` file in the Example project
* `<snapshot path>` is the path to the snapshot file you generated in the [Example Project set up guide]({{urlRoot}}/content/get-started/example-project/exampleproject-local-deployment)
*  `<assembly name>` is a name that you choose for your Deployment Manager assembly. This must be a different name to the assembly you created in [Step 4]({{urlRoot}}/content/tutorials/deployment-manager/tutorial-deploymentmgr-workers#step-4-upload-your-worker-assemblies).
* `<deployment name>` is a name that you choose for your deployment. 

For example: 

* `.\publish-linux-workers.ps1 ..\UnrealGDKExampleProject\spatial\one_worker_test.json ..\UnrealGDKExampleProject\spatial\snapshots\default.snapshot`
* `.\cloud-launch.ps1 deploymentassembly deploymentmanager`

After running these commands, the SpatialOS CLI automatically deploys your project. The Deployment Overview Console page opens automatically after your project has successfully deployed.

With the Console open, on the Deployment Overview page, select **Projects** to see a list of your deployments. You should see three deployments in this list: 

* **deploymentmanager**
* **session_0**
* **session_1**

![img]({{assetRoot}}assets/deployment-manager/deploymentmgr-consoledeployments.png)<br/>
Image: *The SpatialOS Console Projects page,  with running deployments highlighted.*

If you can see these three deployments, then you have successfully launched multiple deployments of the Example project using the Deployment Manager. By default, these deployment sessions run for 60 minutes before restarting automatically. 

### Troubleshooting

<%(#Expandable title="The PowerShell Cannot find path '\UnrealGDKExampleProject\spatial\one_worker_test.json' because it does not exist.")%>

Make sure your Deployment Manager repository is in the same parent directory as your Example Project repository. If it is not, you have to edit the file paths in these Powershell commands to point to wherever your Example Project `one_worker_test.json` and `default.snapshot` files are located. 

<%(/Expandable)%>

**Next**: [Play your game]({{urlRoot}}/content/tutorials/deployment-manager/tutorial-deploymentmgr-play)

--------<br/>

_2019-05-21 Page added with full review_