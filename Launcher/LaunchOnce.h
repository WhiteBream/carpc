/*
 $Id:  $
***************************************************************************
 White Bream
 Terborchdreef 26
 3262NB  Oud-Beijerland
 The Netherlands
 www.whitebream.com
 info@whitebream.nl
***************************************************************************

 Launchonce

***************************************************************************
 Copyright (c) 2003, White Bream
***************************************************************************
 File:          launchonce.h
 Project id.:   -

 Version:       0.10
 Creation date: June 9, 2003
 Revison date:  June 9, 2003
 Author:        Henk Bliek
***************************************************************************
 Revisionlog:
 0.10     June 9, 2003
          * Initial
**************************************************************************/


#define HKEY_WHITEBREAM		TEXT("SOFTWARE\\White Bream\\Cargo\\Launch")
#define HKEY_PROCESS		TEXT("Processes")
#define HKEY_PROCESS_ID		TEXT("ProcessId")
#define HKEY_THREAD_ID		TEXT("ThreadId")

#define PROCESS_INFOSTR		TEXT("Contains the process and thread Id's of the applications")


int LaunchOnce(LPTSTR lpCmdLine, BOOL setSize, BOOL minimize);
void KillApp(LPTSTR lpCmdLine);
