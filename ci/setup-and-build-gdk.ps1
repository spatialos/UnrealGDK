# Download Unreal Engine 
&$PSScriptRoot"\get-engine.ps1"

# Run the required setup steps
&$PSScriptRoot"\setup-gdk.ps1"

# Build the GDK plugin
&$PSScriptRoot"\build-gdk.ps1"