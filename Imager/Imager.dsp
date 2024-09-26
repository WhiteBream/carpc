# Microsoft Developer Studio Project File - Name="Imager" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=Imager - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Imager.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Imager.mak" CFG="Imager - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Imager - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Imager - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Imager - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "Imager - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "C:\DXSDK\Include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "Imager - Win32 Release"
# Name "Imager - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Call.cpp
# End Source File
# Begin Source File

SOURCE=.\Call.h
# End Source File
# Begin Source File

SOURCE=.\Imager.cpp
# End Source File
# Begin Source File

SOURCE=.\Imager.rc
# End Source File
# Begin Source File

SOURCE=.\MainWindow.cpp
# End Source File
# Begin Source File

SOURCE=.\Ticker.cpp
# End Source File
# Begin Source File

SOURCE=.\Ticker.h
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\MainWindow.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\Resources\artist.ico
# End Source File
# Begin Source File

SOURCE=.\Resources\cfg.ico
# End Source File
# Begin Source File

SOURCE=.\Resources\ct_err.ico
# End Source File
# Begin Source File

SOURCE=.\Resources\ct_in.ico
# End Source File
# Begin Source File

SOURCE=.\Resources\ct_out.ico
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

SOURCE=.\Resources\lst.ico
# End Source File
# Begin Source File

SOURCE=.\Resources\mlib.ico
# End Source File
# Begin Source File

SOURCE=.\Resources\mxr.ico
# End Source File
# Begin Source File

SOURCE=.\Resources\nt_cell.ico
# End Source File
# Begin Source File

SOURCE=.\Resources\nt_div.ico
# End Source File
# Begin Source File

SOURCE=.\Resources\nt_fax.ico
# End Source File
# Begin Source File

SOURCE=.\Resources\nt_home.ico
# End Source File
# Begin Source File

SOURCE=.\Resources\nt_work.ico
# End Source File
# Begin Source File

SOURCE=.\Resources\osk.ico
# End Source File
# Begin Source File

SOURCE=.\Resources\pause.ico
# End Source File
# Begin Source File

SOURCE=.\Resources\phone.ico
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

SOURCE=.\Resources\vid.ico
# End Source File
# End Group
# End Target
# End Project
