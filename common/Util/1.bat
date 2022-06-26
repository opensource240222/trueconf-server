@echo off
SET TargedDir=\\dront\dev\Server\
SET ParentDir=..\Servers\

IF %1==as (
	SET src=%ParentDir%AppServer\
	SET dst=%TargedDir%AppServer\
	SET file=vs_appsrv
)
IF %1==bs (
	SET src=%ParentDir%BaseServer\
	SET dst=%TargedDir%BaseServer\
	SET file=vs_basesrv
)
IF %1==rs (
	SET src=%ParentDir%RoutingServer\
	SET dst=%TargedDir%RoutingServer\
	SET file=vs_routersrv
)
IF %1==sm (
	SET src=%ParentDir%ServerManager\
	SET dst=%TargedDir%ServerManager\
	SET file=vs_srvmgr
)

IF %1==all (
call 1 as
call 1 bs
call 1 rs
call 1 sm
goto :EOF
)

IF NOT DEFINED file goto :EOF


filever.exe %src%Release\%file%.exe >> 2.bat
call 2.bat
del 2.bat
MD %dst%%VisicronServerVersion%\debug

copy %src%Release\%file%.exe %dst%%VisicronServerVersion%\%file%.exe
copy %src%Release\%file%.map %dst%%VisicronServerVersion%\%file%.map
copy %src%Release\%file%.pdb %dst%%VisicronServerVersion%\%file%.pdb
copy %src%Debug\%file%.exe   %dst%%VisicronServerVersion%\debug\%file%.exe
copy %src%Debug\%file%.map   %dst%%VisicronServerVersion%\debug\%file%.map
copy %src%Debug\%file%.pdb   %dst%%VisicronServerVersion%\debug\%file%.pdb

rar a -m5 -ep1 -r %dst%%VisicronServerVersion%\%VisicronServerVersion%.rar %dst%%VisicronServerVersion%\*

echo BUILT at %DATE% %TIME%: >> out.txt
echo %dst%%VisicronServerVersion%\ >> out.txt
