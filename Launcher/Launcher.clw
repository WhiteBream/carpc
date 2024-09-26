; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=DestBtn
LastTemplate=CButton
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "startbtn1.h"
LastPage=0

ClassCount=4

ResourceCount=3
Resource1=IDC_STARTBTN1
Class1=StartKnob
Class2=StartBtn
Class3=StartWin
Class4=DestBtn
Resource2="IDC_STARTBTN1"
Resource3=IDD_START

[ACL:IDC_STARTBTN1]
Type=1
Class=?
Command1=IDM_ABOUT
Command2=IDM_ABOUT
CommandCount=2

[DLG:IDD_START]
Type=1
Class=StartKnob
ControlCount=2
Control1=IDC_STATIC,static,1342177294
Control2=IDC_STATIC,static,1342177287

[CLS:StartKnob]
Type=0
HeaderFile=StartKnob.h
ImplementationFile=StartKnob.cpp
BaseClass=CDialog
Filter=D
LastObject=IDM_ABOUT

[CLS:StartBtn]
Type=0
HeaderFile=StartBtn.h
ImplementationFile=StartBtn.cpp
BaseClass=CButton
Filter=W
LastObject=IDM_ABOUT

[CLS:StartWin]
Type=0
HeaderFile=StartWin.h
ImplementationFile=StartWin.cpp
BaseClass=CWnd
Filter=W
VirtualFilter=WC
LastObject=IDM_ABOUT

[CLS:DestBtn]
Type=0
HeaderFile=DestBtn.h
ImplementationFile=DestBtn.cpp
BaseClass=CButton
Filter=W
VirtualFilter=BWC
LastObject=DestBtn

[ACL:"IDC_STARTBTN1"]
Type=1
Class=?
Command1=IDM_ABOUT
Command2=IDM_ABOUT
CommandCount=2

