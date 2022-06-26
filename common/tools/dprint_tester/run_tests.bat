@echo off

rem Constants
set Executable=dprint_tester.exe
set FullExecutablePath="%~dp0%Executable%"
set Time=3
set ReportFileNamePrefix=%~dp0\report_%USERDOMAIN%_
set OutFileNamePrefix=%~dp0\out_%USERDOMAIN%_

rem Set title
title dprint() performance testing

rem Delete old data
del /q /f "%ReportFileNamePrefix%*.txt"
del /q /f "%OutFileNamePrefix%*.txt"

rem Run tests
set ThreadsCount=1
call :run_test_no_out
call :run_test_with_out
set ThreadsCount=2
call :run_test_no_out
call :run_test_with_out
set ThreadsCount=4
call :run_test_no_out
call :run_test_with_out
set ThreadsCount=8
call :run_test_no_out
call :run_test_with_out
set ThreadsCount=16
call :run_test_no_out
call :run_test_with_out

echo Testing complete. Thank You!
pause

:quit
goto :end

rem !!! Testing functions !!!

rem run_test_no_out() - run test with output to screen
:run_test_no_out

%FullExecutablePath% -n %ThreadsCount% -t %Time% -r "%ReportFileNamePrefix%%ThreadsCount%t.txt"

EXIT /b 0
rem END run_test_no_out()


rem run_test_with_out() - run test with output to file
:run_test_with_out

%FullExecutablePath% -n %ThreadsCount% -t %Time%  -r "%ReportFileNamePrefix%%ThreadsCount%t_out.txt" -o "%OutFileNamePrefix%%ThreadsCount%t.txt"

rem Remove output file
del /q /f "%OutFileNamePrefix%%ThreadsCount%t.txt"

EXIT /b 0
rem END run_test_with_out() 

rem Exit from script
:end

