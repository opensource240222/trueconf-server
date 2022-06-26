:: Collecting all arguments in one variable, passing string with spaces as one argument fails with unknown reasons
@set configuration=%*
net stop "TrueConf Web Manager"
if [%1]==[] (
	copy /Y "%~dp0\MSVC141-Win32-Release-Default\php_trueconf.dll" "C:\Program Files (x86)\TrueConf Server\php\ext\"
) else (
	copy /Y "%~dp0\%configuration: =-%\php_trueconf.dll" "C:\Program Files (x86)\TrueConf Server\php\ext\"
)
net start "TrueConf Web Manager"