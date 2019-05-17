## Step 5: Configure the Deployment Manager

There are two parts to configuring the Deployment Manager:

* Edit the `spatialos.json` file.
* Build the Deployment Manager.

### Edit the spatialos.json file

Make sure you are editing the `spatialos.json` in the Deployment Manager repository, and not the one in the Example Project repository. <br/>
These are the same steps you took as part of [Get started 3: Set up the Example Project ]({{urlRoot}}/content/get-started/exampleproject#Launchaclouddeployment), but for the Deployment Manager, rather than the Example Project. 

Every SpatialOS project has a spatialos.json file, so there may be several `spatialos.json` files on your machine. 

When you signed up for SpatialOS, your account was automatically given a SpatialOS cloud organization name and a SpatialOS cloud project name, both of which are the same generated name.

1. Find this name by going to the Console at <https://console.improbable.io>. The name should look something like beta_randomword_anotherword_randomnumber. In the example below, it’s beta_nuts_double_379. 
   ![img]({{assetRoot}}assets/tutorial/project-name.png)
   _Image: The SpatialOS Console with your SpaitalOS cloud project name highlighted._</br>


1. In File Explorer, navigate to `\deployment-manager\config\` and open the `spatialos.json` file in a text editor of your choice.
1. Replace the `projectName` field with the SpatialOS cloud project name shown in the Console. This tells SpatialOS which SpatialOS cloud project you intend to upload to.

Your `spatialos.json` file should look like this: 

```
{
    "configurationVersion": "0.1",
    "projectName": "beta_nuts_double_379",
    "schemaDescriptor": "../tmp/schema.descriptor",
    "clientWorkers": [
    ],
    "serverWorkers": [
      "deployment_manager.json"
    ]
  }
```

Where `beta_nuts_double_379` is your SpatialOS cloud project name. 

### Build the Deployment Manager

1. In File Explorer, navigate to your Deployment Manager repository
1. Select **File.**
1. Select **Open Windows Powershell**, then select **Open Windows Powershell as administrator.**
1. In Powershell, run the following SpatialOS CLI commands: 
	- `./build-nuget-packages.ps1`
	- `./generate-servicce-account-token.ps1 <project name> <token life time in days>`

Where `<project name>` is your SpatialOS project name and `<token life time in days>` is the the number of days you want your token to remain valid for. 

### Edit the config.json file

Next, you configure the Deployment Manager. 

In this example, you must tell the Deployment Manager:

*  the type of client-worker it can deploy - `UnrealClient` for this example, but this can be any client type you have set up for your project
* the number of deployments you want it to allow to run concurrently - 2 for this example
* which worker assembly to use for the deployments - the name for which you created in step 4, above.

Leave the other configuration file settings to the default, for this example.  For more information about the other Deployment Manager configuration options, see the Deployment Manager [documentation on GitHub](link to readme). 

To configure the Deployment Manager, you edit its configuration file. 
To do this, 

1. Navigate to the Deployment Manager repository you cloned in step 1.
1. Using a text editor of your choice, open `\deployment-manager\DeploymentManager\config.json`. 

In the `config.json`, you need to change the following lines: 

- `"ClientType": "YourClient"`, to `"ClientType": "UnrealClient"`,
- `"NumberOfDeployments": 1`, to `"NumberOfDeployments": 2`,
- `"AssemblyName": "YourAssembly"`, to `"AssemblyName": "<assembly_name>)"`, where `<assembly_name>` is the name you gave to your assembly in [Step 4](#Step5ConfiguretheDeploymentManager) 

Your `config.json` file should look like this: 

```
{}
  "TokenLifetimeDays": 7,
  "ClientType": "UnrealClient",
  "MaxNumberOfClients": 100,
  "DeploymentPrefix": "session",
  "NumberOfDeployments": 2,
  "AssemblyName": "<assembly_name>",
  "DeploymentTags": [ "dev_login" ],
  "RegionCode": "EU",
  "DeploymentIntervalSeconds": 5
}
```

When you are done, save and close your `config.json` file. 

**Next**: [Launch a session-based cloud deployment]({{urlRoot}}/content/tutorials/deployment-manager/tutorial-deploymentmgr-launch)