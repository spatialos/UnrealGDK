
# [Deprecated] Multiple deployments for session-based games
## 6: Play your game

### Step 1: Launch clients with the Launcher
1.  Select **session_0** from the deployment list. This opens the deployment overview screen.
1. Select **LAUNCH** on the left of the page.<br/></br>
<%(Lightbox image="{{assetRoot}}assets/deployment-manager/deploymentmgr-consoleoverview.png")%></br>
_Image: The SpatialOS Console with the game client **LAUNCH** button highlighted._<br/></br>
1. Select the **Launch** button that appears in the center of the page to open the [SpatialOS Launcher](https://docs.improbable.io/reference/latest/shared/operate/launcher). The Launcher automatically downloads a game client for this deployment and runs it on your local machine. </br></br>
![img]({{assetRoot}}assets/deployment-manager/deploymentmgr-launch.png)
_Image: The SpatialOS Console **Launch a client** pop-up window._<br/></br>
1. Once the game client has launched, you see the Example Project start screen.</br>
Select **QUICK JOIN** to join one of your sessions or select **BROWSE...** to choose from a list of currently running sessions. The default game controls are listed below.
</br>
</br>

**Troubleshooting**</br>

[block:html]
{
  "html": "<button class="collapsible">I can see my deployments in my game client, but I can’t join any of them</button><div>\n\n\nIf you can see your deployments when you select **BROWSE**  but the **QUICK JOIN** button is greyed out, you might need to add the `status_lobby` tag to the deployments. \n\nTo do this:\n\n1. Open the Console.\n1. Select the deployment you need to tag from the deployment list.\n1. On the right-hand side of the screen, under **Details**, select **+ add tag.**\n1. Enter `status_lobby` as the tag name. \n5. Repeat this for each running deployment.\n \nWhen you have done this, re-launch your game client and you should be able to join any of the deployments, provided the number of players has not exceeded the maximum.\n\n</div>"
}
[/block]


<%(Lightbox image="{{assetRoot}}assets/deployment-manager/deploymentmgr-startscreen.png")%></br>
_Image: The Example Project start screen._

### Step 2: Try out the default game controls
| **Key Binding**   | **Function**             |
| ----------------- | ------------------------ |
| W,A,S,D           | Standard movement        |
| Space             | Jump                     |
| Shift             | Sprint                   |
| Left ctrl         | Crouch                   |
| Left click        | Fire                     |
| Right click       | Weapon zoom/ iron sights |
| Number keys 1 - 0 | Select weapon            |

### Step 3: Share your deployment

To share your deployment: 

1. Open the console and select your **session_0** deployment. 
1. Select **Share** on the right-hand side of the screen to see the Share Application screen
1. In the Share Application screen, accept the terms of service to activate the **Get Share URL** button<br/></br>
![img]({{assetRoot}}assets/deployment-manager/deploymentmgr-share.png)<br/>
_Image: The SpatialOS Console Share Application pop-up window, showing terms of service_<br/></br>
1. After you have accepted the terms of service, you can send your Share Token URL to other people so they can try out your game. <br/><br/>
![img]({{assetRoot}}assets/deployment-manager/deploymentmgr-share2.png)</br>
_Image: The SpatialOS Console Share Application screen with terms of service accepted_<br/></br>


</br>
**Congratulations!** You have successfully launched and shared a SpatialOS deployment using the Deployment Manager! 

</br>
</br>
**Next steps:**

If you have an existing Unreal multiplayer project, follow our detailed [porting guide]({{urlRoot}}/content/tutorials/tutorial-porting-guide) to get it onto the GDK.

If you want to start a project from scratch, follow our [Template guide](https://docs-staging.improbable.io/unreal/1.0/content/get-started/gdk-template) to set up a blank project using the GDK.


<br/>------<br/>
_2019-10-16 Tutorial deprecated_<br/>
_2019-05-21 Page added with editorial review_
