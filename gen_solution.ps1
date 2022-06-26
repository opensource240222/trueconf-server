$proc = Start-Process .\gen_solution.cmd -NoNewWindow -PassThru
Wait-Process -InputObject $proc

if ($proc.ExitCode -ne 0) 
{
    Write-Warning "$_ exited with status code $($proc.ExitCode)"
}
else
{
	.\common\fbuild\gen_solution.ps1 -WorkDirs ",common" -RootDir "tc3" -ProjTemplate "common\fbuild\Project_Linux.vcxproj.template"
}