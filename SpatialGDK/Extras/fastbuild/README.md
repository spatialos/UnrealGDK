# Introduction
[FASTBuild](http://www.fastbuild.org/docs/home.html) is a distributed build and caching system.

The GDK for Unreal integrates with the Unreal Build Tool, using [Unreal_FASTBuild](https://github.com/liamkf/Unreal_FASTBuild)
as the foundation.

# Cache location 

To use Fastbuild outside Improbable, you need to configure your own caching location, where compilation results can be shared across users.

Follow [these instructions](http://www.fastbuild.org/docs/features/caching.html) to set up your cache and update the `$fileshare` variable in `install.ps1` with your value.

# Cache location (internal to Improbable)

Improbable's cache location is `\\lonv-file-01` which is set as the default value in the installation script. To use it, you first need to authorise access to this network drive:
1. Enter `\\lonv-file-01` your Windows Explorer address bar
1. Authenticate with your Windows username and password (not the PIN). Make sure to set `Remember my credentials` so this works after a restart.

# Installation

FASTBuild can be installed in two different ways:
* As a service, which is good for build agents.
* As a GUI, which is good for your local machine.

Installation scripts are provided in this folder: `install.ps1` and `uninstall.ps1`.

> All of these must be run from an **administrator** `powershell` or `cmd` prompt.

## As a service
FASTBuild can be installed as a service, for build agents and other non-interactive scenarios.

**To install**

  `powershell -NoProfile -ExecutionPolicy Bypass -File install.ps1 -service`

**To uninstall**

  `powershell -NoProfile -ExecutionPolicy Bypass -File uninstall.ps1 -service`

# As a GUI
If you're installing on your workstation, it's recommended to install it in interactive mode.

**To install**

  `powershell -NoProfile -ExecutionPolicy Bypass -File install.ps1`

**To uninstall**

  `powershell -NoProfile -ExecutionPolicy Bypass -File uninstall.ps1`

# Useful tools

* A Visual Studio plugin for monitoring the status of builds: [FASTBuildMonitor](https://github.com/yass007/FASTBuildMonitor)
* An alternative, standalone build monitor: [FASTBuild-Dashboard](https://github.com/hillin/FASTBuild-Dashboard)
