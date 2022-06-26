# Helper to enable building single source from MSVS (Ctrl-F7) using FASTBuild
# Setup:
#   1. Make symlink to this script at the root of your repo:
#      a. Open console (cmd.exe) (it must be cmd.exe, because mklink is a builtin)
#      b. Change directory to the root of your repo
#      c. Execute, substituting PATH_TO_THIS_SCRIPT to actual path:
#             mklink fbuild-build-item.ps1 PATH_TO_THIS_SCRIPT
#         For example (for tc3): mklink fbuild-build-item.ps1 common\fbuild\build-item.ps1
#   2. Add new external tool:
#      a. Select "TOOLS/External Tools..." in the menu and then click "Add"
#      b. Fill out parameters:
#         * Command: powershell.exe
#         * Arguments: -ExecutionPolicy RemoteSigned $(SolutionDir)\fbuild-build-item.ps1 $(TargetDir) $(ItemPath)
#         * Initial directory: $(SolutionDir)
#         * Use Output window: true
#      c. If you are using a different directory for intermediate build results then path to it must be added to the Arguments
#   3. (optional) Configure hotkey for added external tool:
#      a. Remember position of added external tool in the list of tools
#      b. Select "TOOLS/Options..." in the menu
#      c. Select "Environment/Keyboard" category
#      d. Find "Tools.ExternalCommand" entry with the number from step a.
#      e. Enter desired hotkey (e.g. Ctrl-E, Ctrl-B) in "Press shortcut keys" field and click "Assign"
# Known bugs:
#   1. Curent file isn't saved automaticaly. Hit Ctrl-S yourself.
#   2. Can't build files that are part of unity. Compile whole project.
#   3. Can't build files that were never built as part of a project.
#      Try to compile whole project once then it will work even if file will fail to compile.
if ($Args.Count -lt 2)
{
	$ScriptName = $MyInvocation.MyCommand.Name
	Write "Usage: $ScriptName `$(TargetDir) `$(ItemPath)"
	Write "   or: $ScriptName `$(TargetDir) `$(ItemPath) C:\path\to\build_dir"
    Exit -1
}
$TargetDir = $Args[0]
$ItemPath = $Args[1]
$BuildDir = $Args[2]
if ($BuildDir -eq $null) { $BuildDir = "tmp" }
$Config = $(Split-Path -Leaf "$TargetDir")
$RelativeItemPath = $ItemPath.Replace($(Split-Path -Parent "$TargetDir"),"")
$Target = "$BuildDir\$Config$RelativeItemPath" -replace "\.(?:cpp|cxx|cc|c)$", ".obj"
& fbuild -vs -cache "$Target"
