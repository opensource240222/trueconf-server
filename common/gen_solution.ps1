$proc = Start-Process .\gen_solution.cmd -NoNewWindow -PassThru
Wait-Process -InputObject $proc

if ($proc.ExitCode -ne 0) 
{
    Write-Warning "$_ exited with status code $($proc.ExitCode)"
}
else
{
	.\fbuild\gen_solution.ps1 -WorkDirs "" -RootDir "tc3/common" -ProjTemplate "fbuild\Project_Linux.vcxproj.template"
}