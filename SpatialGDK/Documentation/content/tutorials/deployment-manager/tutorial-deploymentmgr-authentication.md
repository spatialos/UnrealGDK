


# [Deprecated] Multiple deployments for session-based games
## 2: Generate an authentication token

>**Note:** This section describes generating and using development authentication tokens. These are for in-development testing only. They are for early-stage development so you don’t have to create your own game authentication service before testing your game clients. Do not include development authentication tokens in production projects. 

### Step 1: Generate a token
To ensure that game clients are able to see all available deployments and can connect to them, you must generate a development authentication token and update your project’s code with the generated token. 

[block:html]
{
  "html": "<button class="collapsible">What is development authentication?</button>\n<div>\n\nWhile developing your game you may want to test game clients using connection authentication before you set up your own authentication and login servers. To facilitate this, Improbable hosts a development authentication service and a development login service for early-stage game development. This development authentication workflow provides you with a long-lived `DevelopmentAuthenticationToken` that you can hardcode into your game client.\n\nYou should only use development authentication and login for testing during development of your game.You can find out more about the development authentication workflow in the [development authentication documentation](https://docs.improbable.io/reference/latest/shared/auth/development-authentication). \n\n</div>"
}
[/block]


To generate a development authentication token: 

1. Open a terminal window and navigate to `\UnrealGDKExampleProject\spatial`
2. Run the following SpatialOS CLI command: 

`spatial project auth dev-auth-token create --description exampleProjectDeploymentToken`. 

This creates a temporary authentication token that lasts for 30 days. For information about updating and refreshing development authentication tokens, refer to the [development authentication token](https://docs.improbable.io/reference/Latest/shared/auth/development-authentication) documentation. 

In the terminal window, copy the string displayed after `tokenSecret` and make a note of it, you will use this token secret in the next step. This token is always 100 characters long and ends in an equals (=) sign.

### Step 2: Add the token to your project

Next, you must add your development authentication token to the Example Project code.

1. In File Explorer, navigate to `UnrealGDKExampleProject\Game\Source\GDKShooter\Private\Deployments` and double click **DeploymentsPlayerController.cpp** to open it.
1. With **DeploymentsPlayerController.cpp** open, search for `PITParams->development_authentication_token` and replace `“REPLACE ME"` with the `tokenSecret` string you copied in step 3. <br/>
<%(Lightbox image="{{assetRoot}}assets/deployment-manager/deploymentmgr-token.png")%><br/>
_Image: The development authentication token id code as shown in Visual Studio_ <br/>
1. Save your changes.
1. In the directory `UnrealEngine\Samples\UnrealGDKExampleProject\Game`, double-click **GDKShooter.sln** to open it with Visual Studio.
1. Press F5 on your computer’s keyboard or, from the Visual Studio toolbar, select **Local Windows Debugger** to launch a debug build of your project.

If your project opens without errors you have successfully added the developer authentication token to your project.

#### **> Next**: [3: Build and upload workers]({{urlRoot}}/content/tutorials/deployment-manager/tutorial-deploymentmgr-workers)


<br/>------<br/>
_2019-05-21 Page added with editorial review_
