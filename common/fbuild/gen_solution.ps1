param([String] $WorkDirs, [String] $RootDir, [String] $ProjTemplate)

########################################################################

$ALL_SLN = "All.sln"
$PROJECT_FILE_EXTENSION = 'vcxproj'
$PROJECTS_FOLDER_LINUX = 'projects\linux'
$PROJECTS_LINUX = 'projects_linux.ini'

$PROJECT_NAME = '__PROJECT_NAME__'
$PROJECT_GUID = '__PROJECT_GUID__'
$PROJECT_PATH = '__PROJECT_PATH__'
$EXEC_FILE    = '__EXEC_FILE__'
$ROOT_DIR	  = '__ROOT_DIR__'
$REMOTE_DEBUGGER_COMMAND = '__REMOTE_DEBUGGER_COMMAND__'

$REMOTE_DEBUGGER_COMMAND_TEMPLATE = '<RemoteDebuggerCommand>$(RemoteRootDir)/__PROJECT_PATH__/$(Configuration)/__EXEC_FILE__</RemoteDebuggerCommand>'

########################################################################

function Get-IniContent ($filePath)
{
    $ini = @{}
    switch -regex -file $FilePath
    {
        "^\[(.+)\]" # Section
        {
            $section = $matches[1]
            $ini[$section] = @{}
        }

        "(.+?)\s*=(.*)" # Key
        {
            $name,$value = $matches[1..2]
            $ini[$section][$name] = $value
        }
    }
    return $ini
}

function Gen-Proj ($workDir)
{
	$suffix = ""
	
	if($workDir)
	{
		$suffix = "$($workDir)\"
	}

	$iniContent = Get-IniContent "$($suffix)$($PROJECTS_LINUX)"
 
	$projectTemplate = Get-Content $ProjTemplate

	foreach($key in $iniContent.Keys)
	{   
		$tmpProj = $projectTemplate -replace "$($ROOT_DIR)", "$($RootDir)" -replace "$($PROJECT_NAME)", "$($key)" -replace "$($PROJECT_GUID)", "$((New-Guid).ToString().ToUpper())"
		
		$projectPath = $iniContent[$key]['project_path']
		if($projectPath)
		{
			if($workDir)
			{
				$projectPath = $workDir + "/" + $projectPath
			}
			
			$tmpProj = $tmpProj -replace "$($REMOTE_DEBUGGER_COMMAND)", "$($REMOTE_DEBUGGER_COMMAND_TEMPLATE)" -replace "$($PROJECT_PATH)", "$($projectPath)" 
       
			$execFile = $iniContent[$key]['exec_file']
			if($execFile)
			{
				$tmpProj = $tmpProj -replace "$($EXEC_FILE)", "$($execFile)"
			}
			else 
			{
				$tmpProj = $tmpProj -replace "$($EXEC_FILE)", "$($key)"
			}
		}
		else 
		{
			$tmpProj = $tmpProj -replace ".$($REMOTE_DEBUGGER_COMMAND)", ""
		}
    
		$pathSaveProj = "$($PROJECTS_FOLDER_LINUX)\$($key)_Linux.$($PROJECT_FILE_EXTENSION)"
		$tmpProj | Out-File -FilePath $pathSaveProj

		Write-Host "VCXProj: $((Get-Item -Path ".\").FullName)\$($pathSaveProj)"
	}
}

########################################################################

function Gen-Sln ()
{
	$linuxGuid = "AF10D104-3697-42D3-80E7-7D599EFD4A45"
	
	$proj = "Project(""{2150E333-8FDC-42A3-9474-1A3956D46DE8}"") = ""Linux"", ""Linux"", ""{$($linuxGuid)}""`r`nEndProject`r`n"
	
	$listSlnPlatforms = @{}
	
	$projCfgPlatforms = "`tGlobalSection(ProjectConfigurationPlatforms) = postSolution`r`n"
	
	$nestedProj = "`tGlobalSection(NestedProjects) = preSolution`r`n"
	
	foreach($nameProj in Get-ChildItem -Path $PROJECTS_FOLDER_LINUX -Filter *.$PROJECT_FILE_EXTENSION -r | % { $_.Name.Replace( ".$($PROJECT_FILE_EXTENSION)","") })
	{
		$pathProj = "$($PROJECTS_FOLDER_LINUX)\$($nameProj).$($PROJECT_FILE_EXTENSION)"
		
		$guid = (New-Guid).ToString().ToUpper()
		
		$proj += "Project(""{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}"") = ""$($nameProj)"", ""$($pathProj)"", ""{$($guid)}""`r`nEndProject`r`n"
		
		[xml]$xmlData = Get-Content $pathProj
		
		foreach($projCfg in $xmldata.Project.ItemGroup.ProjectConfiguration | %{"$($_.Configuration)|$($_.Platform)"} | Select-Object -unique)
		{
			$listSlnPlatforms[$projCfg] += $true; 
			
			$tmp = "`t`t{$($guid)}.$($projCfg)"
			$projCfgPlatforms += "$($tmp).ActiveCfg = $($projCfg)`r`n"
			$projCfgPlatforms += "$($tmp).Build.0 = $($projCfg)`r`n"
		}
		
		if($xmldata.Project.PropertyGroup.RemoteDebuggerCommand -ne $null)
		{
			$nestedProj += "`t`t{$($guid)} = {$($linuxGuid)}`r`n"
		}
	}
	
	$slnCfgPlatforms = "`tGlobalSection(SolutionConfigurationPlatforms) = preSolution`r`n"
	foreach($key in $listSlnPlatforms.Keys)
	{
		$slnCfgPlatforms += "`t`t$($key) = $($key)`r`n"
	}
	$slnCfgPlatforms += "`tEndGlobalSection`r`n"
	
	$projCfgPlatforms += "`tEndGlobalSection`r`n"
	
	$nestedProj += "`tEndGlobalSection`r`n"
	
	
	$sln = "`r`nMicrosoft Visual Studio Solution File, Format Version 12.00`r`n# Visual Studio 15`r`nVisualStudioVersion = 15.0.27703.2026`r`nMinimumVisualStudioVersion = 10.0.40219.1`r`n"
	$sln += $proj
	$sln += "Global`r`n"
	$sln += $slnCfgPlatforms
	$sln += $projCfgPlatforms
	$sln += "`tGlobalSection(SolutionProperties) = preSolution`r`n`t`tHideSolutionNode = FALSE`r`n`tEndGlobalSection`r`n"
	$sln += $nestedProj
	$sln += "`tGlobalSection(ExtensibilityGlobals) = postSolution`r`n`t`tSolutionGuid = {35CE3875-4D27-4849-A063-2C04700C6FC5}`r`n`tEndGlobalSection`r`nEndGlobal"
	
	$fileSln = "$($PROJECTS_FOLDER_LINUX)\$($ALL_SLN)"
	$sln | Out-File -FilePath $fileSln
	
	Write-Host "SLN: $((Get-Item -Path ".\").FullName)\$($fileSln)"
}

########################################################################

function Merge-Sln ($fromSln)
{
	$data = Get-Content $fromSln
	Add-Content -Path $ALL_SLN -Value $data
	
	Write-Host "Merge from $((Get-Item -Path ".\").FullName)\$($fromSln) to $((Get-Item -Path ".\").FullName)\$($ALL_SLN)"
}

########################################################################

Remove-Item -LiteralPath $PROJECTS_FOLDER_LINUX -Force -Recurse -ErrorAction Ignore
New-Item -ItemType Directory -Force -Path $PROJECTS_FOLDER_LINUX
	
foreach($dirPath in $WorkDirs.split(','))
{
	Gen-Proj $dirPath
}
Gen-Sln
Merge-Sln "$($PROJECTS_FOLDER_LINUX)\$($ALL_SLN)"

Write-Host -NoNewLine 'Press any key to continue...';
$null = $Host.UI.RawUI.ReadKey('NoEcho,IncludeKeyDown');

########################################################################