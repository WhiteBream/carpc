# Microsoft Developer Studio Project File - Name="Player" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=Player - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Player.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Player.mak" CFG="Player - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Player - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Player - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Player - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "C:\PROGRA~1\MIC977~1\Include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386 /libpath:"C:\PROGRA~1\MIC977~1\Lib"

!ELSEIF  "$(CFG)" == "Player - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "C:\PROGRA~1\MIC977~1\Include" /I "C:\PROGRA~1\MICROS~4\Include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FR /Yu"stdafx.h" /FD /GZ /c
# SUBTRACT CPP /X
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"C:\PROGRA~1\MIC977~1\Lib"

!ENDIF 

# Begin Target

# Name "Player - Win32 Release"
# Name "Player - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Client.cpp
# End Source File
# Begin Source File

SOURCE=.\Database.cpp
# End Source File
# Begin Source File

SOURCE=.\MainWindow.cpp
# End Source File
# Begin Source File

SOURCE=.\MediaDB.cpp
# End Source File
# Begin Source File

SOURCE=.\Player.cpp
# End Source File
# Begin Source File

SOURCE=.\Playlist.cpp
# End Source File
# Begin Source File

SOURCE=.\PlayWin.cpp
# End Source File
# Begin Source File

SOURCE=.\Popup.cpp
# End Source File
# Begin Source File

SOURCE=.\Settings.cpp
# End Source File
# Begin Source File

SOURCE=.\Sorted.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\Ticker.cpp
# End Source File
# Begin Source File

SOURCE=.\Video.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\Client.h
# End Source File
# Begin Source File

SOURCE=.\Database.h
# End Source File
# Begin Source File

SOURCE=.\MainWindow.h
# End Source File
# Begin Source File

SOURCE=.\MediaDB.h
# End Source File
# Begin Source File

SOURCE=.\Playlist.h
# End Source File
# Begin Source File

SOURCE=.\PlayWin.h
# End Source File
# Begin Source File

SOURCE=.\Popup.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\Settings.h
# End Source File
# Begin Source File

SOURCE=.\Sorted.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\Ticker.h
# End Source File
# Begin Source File

SOURCE=.\Video.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\Resources\Album.ico
# End Source File
# Begin Source File

SOURCE=.\Resources\artist.ico
# End Source File
# Begin Source File

SOURCE=.\Resources\cd.ico
# End Source File
# Begin Source File

SOURCE=.\Resources\cfg.ico
# End Source File
# Begin Source File

SOURCE=.\Resources\del.ico
# End Source File
# Begin Source File

SOURCE=.\Resources\exit.ico
# End Source File
# Begin Source File

SOURCE=.\Resources\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\Resources\lib.ico
# End Source File
# Begin Source File

SOURCE=.\Resources\listalbum.ico
# End Source File
# Begin Source File

SOURCE=.\Resources\listtrack.ico
# End Source File
# Begin Source File

SOURCE=.\Resources\lst.ico
# End Source File
# Begin Source File

SOURCE=.\Player.rc
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Resources\mlib.ico
# End Source File
# Begin Source File

SOURCE=.\Resources\mxr.ico
# End Source File
# Begin Source File

SOURCE=.\Resources\next.ico
# End Source File
# Begin Source File

SOURCE=.\Resources\osk.ico
# End Source File
# Begin Source File

SOURCE=.\Resources\pause.ico
# End Source File
# Begin Source File

SOURCE=.\Resources\paused.ico
# End Source File
# Begin Source File

SOURCE=.\Resources\play.ico
# End Source File
# Begin Source File

SOURCE=.\Resources\playing.ico
# End Source File
# Begin Source File

SOURCE=.\Resources\playnext.ico
# End Source File
# Begin Source File

SOURCE=.\Resources\playnow.ico
# End Source File
# Begin Source File

SOURCE=.\Resources\playqueue.ico
# End Source File
# Begin Source File

SOURCE=.\Resources\plr.ico
# End Source File
# Begin Source File

SOURCE=.\Resources\prev.ico
# End Source File
# Begin Source File

SOURCE=.\Resources\sort.ico
# End Source File
# Begin Source File

SOURCE=.\Resources\spk_off.ico
# End Source File
# Begin Source File

SOURCE=.\Resources\spk_on.ico
# End Source File
# Begin Source File

SOURCE=.\Resources\stop.ico
# End Source File
# Begin Source File

SOURCE=.\Resources\stopped.ico
# End Source File
# Begin Source File

SOURCE=.\Resources\vid.ico
# End Source File
# Begin Source File

SOURCE=.\Resources\vis.ico
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
