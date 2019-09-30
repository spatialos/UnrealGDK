# expects $gdk_home to be defined
param (
    [string] $unreal_path = "$gdk_home\UnrealEngine"
)
$gdk_in_engine = "$unreal_path\Engine\Plugins\UnrealGDK"

    # clean up the symlink 
cmd /c rmdir $gdk_in_engine