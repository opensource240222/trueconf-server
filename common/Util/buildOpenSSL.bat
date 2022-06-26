@ECHO OFF
CALL "C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" x86
nmake clean
perl Configure VC-WIN32
nmake build_libs build_engines
mkdir ..\openssl_buildresult\msvc140\win32-release-dll\engines
move *.dll ..\openssl_buildresult\msvc140\win32-release-dll\
move *.exp ..\openssl_buildresult\msvc140\win32-release-dll\
move *.lib ..\openssl_buildresult\msvc140\win32-release-dll\
move *.pdb ..\openssl_buildresult\msvc140\win32-release-dll\
move engines\*.dll ..\openssl_buildresult\msvc140\win32-release-dll\engines\
move engines\*.exp ..\openssl_buildresult\msvc140\win32-release-dll\engines\
move engines\*.lib ..\openssl_buildresult\msvc140\win32-release-dll\engines\
move engines\*.pdb ..\openssl_buildresult\msvc140\win32-release-dll\engines\


nmake clean
perl Configure no-shared VC-WIN32
nmake build_libs build_engines
mkdir ..\openssl_buildresult\msvc140
mkdir ..\openssl_buildresult\msvc140\win32-release-static
move *.lib ..\openssl_buildresult\msvc140\win32-release-static\


nmake clean
perl Configure --debug VC-WIN32
nmake build_libs build_engines
mkdir ..\openssl_buildresult\msvc140\win32-debug-dll\engines
move *.dll ..\openssl_buildresult\msvc140\win32-debug-dll\
move *.exp ..\openssl_buildresult\msvc140\win32-debug-dll\
move *.lib ..\openssl_buildresult\msvc140\win32-debug-dll\
move *.pdb ..\openssl_buildresult\msvc140\win32-debug-dll\
move engines\*.dll ..\openssl_buildresult\msvc140\win32-debug-dll\engines\
move engines\*.exp ..\openssl_buildresult\msvc140\win32-debug-dll\engines\
move engines\*.lib ..\openssl_buildresult\msvc140\win32-debug-dll\engines\
move engines\*.pdb ..\openssl_buildresult\msvc140\win32-debug-dll\engines\


nmake clean
perl Configure no-shared --debug VC-WIN32
nmake build_libs build_engines
mkdir ..\openssl_buildresult\msvc140\win32-debug-static
move *.lib ..\openssl_buildresult\msvc140\win32-debug-static\

mkdir ..\openssl_buildresult\inc32
xcopy /E include\openssl ..\openssl_buildresult\inc32\openssl\
copy ms\applink.c ..\openssl_buildresult\inc32\openssl\

CALL "C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" x64
nmake clean
perl Configure VC-WIN64A
nmake build_libs build_engines
mkdir ..\openssl_buildresult\msvc140\win64-release-dll\engines
move *-x64.dll ..\openssl_buildresult\msvc140\win64-release-dll\
move *.exp ..\openssl_buildresult\msvc140\win64-release-dll\
move *.lib ..\openssl_buildresult\msvc140\win64-release-dll\
move *-x64.pdb ..\openssl_buildresult\msvc140\win64-release-dll\
move engines\*.dll ..\openssl_buildresult\msvc140\win64-release-dll\engines\
move engines\*.exp ..\openssl_buildresult\msvc140\win64-release-dll\engines\
move engines\*.lib ..\openssl_buildresult\msvc140\win64-release-dll\engines\
move engines\*.pdb ..\openssl_buildresult\msvc140\win64-release-dll\engines\


nmake clean
perl Configure no-shared VC-WIN64A
nmake build_libs build_engines
mkdir ..\openssl_buildresult\msvc140\win64-release-static
move *.lib ..\openssl_buildresult\msvc140\win64-release-static\


nmake clean
perl Configure --debug VC-WIN64A
nmake build_libs build_engines
mkdir ..\openssl_buildresult\msvc140\win64-debug-dll\engines
move *-x64.dll ..\openssl_buildresult\msvc140\win64-debug-dll\
move *.exp ..\openssl_buildresult\msvc140\win64-debug-dll\
move *.lib ..\openssl_buildresult\msvc140\win64-debug-dll\
move *-x64.pdb ..\openssl_buildresult\msvc140\win64-debug-dll\
move engines\*.dll ..\openssl_buildresult\msvc140\win64-debug-dll\engines\
move engines\*.exp ..\openssl_buildresult\msvc140\win64-debug-dll\engines\
move engines\*.lib ..\openssl_buildresult\msvc140\win64-debug-dll\engines\
move engines\*.pdb ..\openssl_buildresult\msvc140\win64-debug-dll\engines\


nmake clean
perl Configure no-shared --debug VC-WIN64A
nmake build_libs build_engines
mkdir ..\openssl_buildresult\msvc140
mkdir ..\openssl_buildresult\msvc140\win64-debug-static
move *.lib ..\openssl_buildresult\msvc140\win64-debug-static\

mkdir ..\openssl_buildresult\inc64
xcopy /E include\openssl ..\openssl_buildresult\inc64\openssl\
copy ms\applink.c ..\openssl_buildresult\inc64\openssl\
