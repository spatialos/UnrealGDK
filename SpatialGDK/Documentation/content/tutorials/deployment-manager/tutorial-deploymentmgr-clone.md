<%(TOC)%>
# [Deprecated] Multiple deployments for session-based games
## 1: Clone the Deployment Manager

First, clone the Deployment Manager repository. 

Clone the Deployment Manager repository into the parent directory of your Example Project repository. For example: ![img]({{assetRoot}}assets/deployment-manager/deploymentmgr-directory.png)_<br/>Image: An example directory structure for the Deployment Manager and Example Project_<br/>

If you do not clone the Deployment Manager into the parent directory of your Example Project repository then you will have to edit a number of .bat scripts later on in this tutorial.

<%(#Expandable title="Using a terminal")%>

1. Open a terminal window and navigate to the parent directory of your Example Project repository. 
1. Run either of these commands to clone the Deployment Manager repository:

|       |                                                              |
| ----- | ------------------------------------------------------------ |
| HTTPS | `git clone https://github.com/spatialos/deployment-manager.git`|
| SSH   | `git clone git@github.com:spatialos/deployment-manager.git`|

<%(/Expandable)%>

<%(#Expandable title="Using Github Desktop")%>

1. In GitHub Desktop, select **File >  Clone  Repository**.<br/>
1. In the Clone a repository window, select **URL.**<br/>
1. In the Repository URL field, enter this url: `https://github.com/spatialos/deployment-manager.git`<br/>
1. In the Local path field, type the file path to the parent directory of your Example Project repository, or select **Choose…** to navigate to the parent directory of your Example Project repository. <br/>
1. Select **Clone**. <br/>
![img]({{assetRoot}}assets/screen-grabs/github-desktop.png)<br/>
_Image: The Github Desktop Clone a repository window_<br/>

<%(/Expandable)%>

Your cloned directory should now have a path similar to `C:\Dev\deployment-manager\...`

</br
</br>
### **> Next**: [2: Generate an authentification token]({{urlRoot}}/content/tutorials/deployment-manager/tutorial-deploymentmgr-authentication)


<br/>------<br/>
_2019-05-21 Page added with editorial review_
