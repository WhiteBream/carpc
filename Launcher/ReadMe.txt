This software is Copyright (c) 2003 by White Bream / Henk Bliek
ABOUT:
This program is written for the CarPC project, which can be found at

                         www.CarPC.nl

Owned by Henk Bliek & White Bream. Copyright (c) 2003


DISCLAIMERS
This may only be used for non-commercial applications.

This is beta software, use at your own risk.
If your system gets f*cked up, the computer does not like you anymore or 
whatever else can happen, Bad luck. No fear though. It sometimes works :)


INSTALL:
- Create new account named CarPC in Windows XP, give it administrator rights
- Logon to new account
- Modify CarPC.ini to suit your needs
- Open registry editor:
  find "HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\IniFileMapping\system.ini\boot"
  change the key "Shell=SYS:Microsoft\Windows NT\CurrentVersion\Winlogon" into
                 "Shell=SYS:Software\Microsoft\Windows NT\CurrentVersion\Winlogon"
  goto "HKEY_CURRENT_USER\Software\Microsoft\Windows NT\CurrentVersion\Winlogon"
  add the key "Shell=C:\yourpath\CarPC.exe"
  (if the new account is actually named CarPC, with admin rights, and you have copied 
   the CarPC files into C:\Programs\CarPC\* then you can also load CarPC.reg)
- Now logoff and logon again,
- System should startup with CarPC. To logoff again, hit Ctrl-Alt-Del and select logoff in the menu


TODO:
(v) = done, (*) = pending, (-) = to do, (c) = cancelled
- Detect application that was started last, and bright it to foreground on startup
- Fix small memory leak in launcher
- Add clock on statusbar
v Add idle timer on opened menu
v Make startbutton pressed when menu open (like Destinator)
- Add keepout area for lousy TV-outputs (Epia...)

- Add telephone sub-application
- Add Settings panel
c Add shutdown dialog, instead of quitting (will do in settings window)
- Integrate Destinator launcher
* Add image to desktop window (increase performance)