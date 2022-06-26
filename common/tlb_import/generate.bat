@echo off
cd /D "%~dp0"
del /Q /S *.tlh *.tli
echo #import "../lib/msado26.tlb" rename("EOF", "ADOEOF") no_implementation > gen.cpp
echo #import "../lib/msado26.tlb" rename("EOF", "ADOEOF") implementation_only >> gen.cpp
echo #import "C:\Program Files\Common Files\system\ado\msado15.dll" no_namespace rename("EOF", "EndOfFile") >> gen.cpp
echo #import ^<cdosys.dll^> no_namespace >> gen.cpp
cl.exe /nologo /Zs gen.cpp
del gen.cpp
powershell -Command "Get-ChildItem *.tlh,*.tli | ForEach { (Get-Content $_) -ireplace [regex]::Escape($_.DirectoryName + '\'), '' | Set-Content $_ }"
