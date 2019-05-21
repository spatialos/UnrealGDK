## Step 7: Play your game

1.  select **session_0** from the deployment list. This opens the deployment overview screen.
2. select **LAUNCH** on the left of the page.
![img]({{assetRoot}}assets/deployment-manager/deploymentmgr-consoleoverview.png)<br/>
Image: *The SpatialOS Console with the game client* ***LAUNCH*** *button* ***Launch*** *highlighted.*<br/>
3. select the **Launch** button that appears in the center of the page to open the [SpatialOS Launcher](https://docs.improbable.io/reference/13.7/shared/operate/launcher). 
![img]({{assetRoot}}assets/deployment-manager/deploymentmgr-launch.png)<br/>
_Image: *The SpatialOS Console* ***Launch a client*** *pop-up window._*

The Launcher automatically downloads a game client for this deployment and runs it on your local machine.

Once the game client has launched, you should see the Example Project start screen

Select **QUICK JOIN** to join one of your sessions or select **BROWSE...** to choose from a list of currently running sessions. 

The default game controls are listed below:

| **Key Binding**   | **Function**             |
| ----------------- | ------------------------ |
| W,A,S,D           | Standard movement        |
| Space             | Jump                     |
| Shift             | Sprint                   |
| Left ctrl         | Crouch                   |
| Left click        | Fire                     |
| Right click       | Weapon zoom/ iron sights |
| Number keys 1 - 0 | Select weapon            |

## Troubleshooting

<%(#Expandable title="I can see my deployments in my game client, but I can’t join any of them")%>

If you can see your deployments when you select **BROWSE**  but the **QUICK JOIN** button is greyed out, you might need to add the `status_lobby` tag to the deployments. 

To do this:

1. Open the Console.
2. Select the deployment you need to tag from the deployment list.
3. On the right-hand side of the screen, under **Details**, select **+ add tag.**
4. Enter `status_lobby` as the tag name. 
5. Repeat this for each running deployment. 

When you have done this, re-launch your game client and you should be able to join any of the deployments, provided the number of players has not exceeded the maximum.

<%(/Expandable)%>

![img]({{assetRoot}}assets/deployment-manager/deploymentmgr-startscreen.png)_Image: The Example Project start screen._

### Share your deployment

To share your deployment: 

1. Open the console and select your **session_0** deployment. 
1. Select **Share** on the right-hand side of the screen to see the Share Application screen
1. In the Share Application screen, accept the terms of service to activate the **Get Share URL** button<br/>
![img]({{assetRoot}}assets/deployment-manager/deploymentmgr-share.png)<br/>
_Image: The SpatialOS Console Share Application pop-up window, showing terms of service_<br/>
1. After you have accepted the terms of service, you can send your Share Token URL to other people so they can try out your game. <br/>
![img]({{assetRoot}}assets/deployment-manager/deploymentmgr-share2.png)<br/>
_Image: The SpatialOS Console Share Application screen with terms of service accepted_<br/>

Congratulations! You have successfully launched and shared a SpatialOS deployment using the Deployment Manager! 

If you have an existing Unreal multiplayer project, follow our detailed [porting guide]({{urlRoot}}/content/tutorials/tutorial-porting-guide) to get it onto the GDK.

If you want to start a project from scratch, follow our [Template guide](https://docs-staging.improbable.io/unreal/1.0/content/get-started/gdk-template) to set up a blank project using the GDK.

--------<br/>

_2019-05-21 Page added with full review_