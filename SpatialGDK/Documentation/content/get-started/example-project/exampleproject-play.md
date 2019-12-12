

# The Example Project 
## 4: Play the game

To get playing, you use SpatialOS Launcher to set up clients for users. You then share the clients with users via a dedicated URL.

<%(#Expandable title="Reminder: What is the SpatialOS Launcher?")%>

The Launcher is a distribution tool which downloads and launches game clients for your deployment. You installed the Launcher when you [installed SpatialOS on your machine]({{urlRoot}}/content/get-started/dependencies#step-3-software). You access the Launcher from the Console; use it to create a URL to give end-users access to a game client for your game.
</br></br>
Find out more in the [glossary]({{urlRoot}}/content/glossary#launcher).
<%(/Expandable)%>

### Step 1: Set up a game client using the Launcher

1. Open the SpatialOS Console at [console.improbable.io](https://console.improbable.io/) to see a list of your SpatialOS cloud projects. </br></br>
The list looks something like this: </br></br>
![img]({{assetRoot}}assets/tutorial/console-projects-list.png)<br/>
_Image: The Console home screen project list_
</br></br>
1. Select your SpatialOS cloud project name to see a list of deployments associated with that cloud project. (In this example, the cloud project name is `gdc2019_demo`.)</br></br>
The list looks something like this: </br></br>
![img]({{assetRoot}}assets/tutorial/console-deployments-list.png)<br/>
_Image: The Console project screen deployment list_
</br></br>
1. Select your deployment name to see its overview screen.</br></br>
The deployment overview screen looks something like this (but won't have a **Restart** button): </br></br>
![img]({{assetRoot}}assets/tutorial/deployment-overview-screen.png)<br/>
_Image: The Console deployment overview screen_
</br></br>

1. Now start a deployment run. </br>
To do this: </br>
    * Locate the screen's **Start** button near the top (the example above has a **Restart** button - this is where you find the **Start** button). Select it to open the Start deployment dialogue box.
    * In the Start deployment dialog box, select **Start** - this starts a deployment run.</br></br>
    Now the deployment is running, you see three buttons near the top of the deployment overview screen:</br></br>
    ![img]({{assetRoot}}assets/tutorial/console-launcher-button.png)<br/>
    _Image: The Console deployment overview screen showing the blue **Launch** button to access the Launcher._
    </br></br>
1. Use the Launcher to get a game client. </br>
To do this: </br>
    * In the deployment overview screen, select the blue **Launch** button to open the Launcher dialog box.</br></br>
    It looks like this:</br></br>
    ![img]({{assetRoot}}assets/tutorial/launch.png)<br/>
    _Image: The Console's Launcher dialog box_
    </br></br>
    * Select **Launch** in the Launcher dialog box to download a game client.</br></br>
1. Once the client has launched, enter a name for your player character and select **Start** to start playing. <br/></br>
![img]({{assetRoot}}assets/example-project/example-project-lobby.png)<br/>
_Image: The Example project lobby screen_

### Step 2: Share your game
To share your cloud deployment: 

1. In the Console, go to the deployment overview screen.</br></br>
The screen has this near the top:</br></br>
![img]({{assetRoot}}assets/tutorial/console-launcher-button.png)<br/>
_Image: The Console deployment overview screen showing the white **Share** button_
<br/><br/>
1. Select **Share** to open the Share Application screen.<br/><br/>
1. In the Share Application screen, check the box to accept the terms and conditions and select **Get Share URL**.</br></br>
![img]({{assetRoot}}assets/example-project/example-project-share-tos.png)<br/>
_Image: The Console Share Application terms of service screen_
<br/><br/>
1. You can now see a URL, which is a token to download a game client. You can share this with game testers so they can try out your game. </br></br>
![img]({{assetRoot}}assets/example-project/example-project-share-screens.png)<br/>
_Image: The Console Share Application screen_
</br>
</br>

**Congratulations!**</br>
You've sucessfully set up and launched the Example Project using the Deployment Manager. You are now ready to start developing your own games with SpatialOS. 
</br>
</br>
**Next steps:**


* If you have an existing Unreal multiplayer project, you can follow the detailed [porting guide]({{urlRoot}}/content/tutorials/tutorial-porting-guide) to get it onto the GDK.

* If you want to start a project from scratch, follow the set up guide for the [Starter Template]({{urlRoot}}/content/get-started/starter-template/get-started-template-intro) to set up a blank project using the GDK. 


<br/>------<br/>
_2019-10-18 Page updated with editorial review: removed link to deprecated tutorial - multiple deployments for session-based games_</br>
_2019-08-14 Page updated with editorial review: new Console screens_</br>
_2019-05-23 Page added with editorial review_
