// DestButton.cpp: implementation of the DestButton class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DestButton.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
//#define new DEBUG_NEW
#endif

#include "resource.h"
#define SRC_SIZE	36

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

DestButton::DestButton(HINSTANCE hInstance, int width, int height)
{
   int     n;
   DbDescr dbDescr;
   LONG    retVal;
   DWORD   registryResult, idSize, regValueType;
   HKEY    regKeyHandle;
   WCHAR   files[3][128];
   Rect    sizes[3];

   hInst = hInstance;

   /* Read the registry stuff */
   retVal = RegCreateKeyEx(HKEY_LOCAL_MACHINE,		//HKEY hKey,                        // handle to open key
							HKEY_WHITEBREAM,		//LPCTSTR lpSubKey,                 // subkey name
							0,						//DWORD Reserved,                   // reserved
							"",						//LPTSTR lpClass,                   // class string
							REG_OPTION_NON_VOLATILE,//DWORD dwOptions,                  // special options
							KEY_QUERY_VALUE,		//REGSAM samDesired,                // desired security access
							NULL,					//LPSECURITY_ATTRIBUTES lpSecAttr,	// inheritance
							&regKeyHandle,			//PHKEY phkResult,                  // key handle 
							&registryResult);		//LPDWORD lpdwDisposition           // disposition value buffer

   regValueType = REG_SZ;
   idSize = sizeof(files[0]);
	
   retVal = RegQueryValueExW(regKeyHandle,			//HKEY hKey,            // handle to key
							L"IdleFile",			//LPCTSTR lpValueName,  // value name
							NULL,					//LPDWORD lpReserved,   // reserved
							&regValueType,			//LPDWORD lpType,       // type buffer
							(LPBYTE)files[0],		//LPBYTE lpData,        // data buffer
							&idSize);				//LPDWORD lpcbData      // size of data buffer

   retVal = RegQueryValueExW(regKeyHandle,			//HKEY hKey,            // handle to key
							L"PressedFile",			//LPCTSTR lpValueName,  // value name
							NULL,					//LPDWORD lpReserved,   // reserved
							&regValueType,			//LPDWORD lpType,       // type buffer
							(LPBYTE)files[1],		//LPBYTE lpData,        // data buffer
							&idSize);				//LPDWORD lpcbData      // size of data buffer

   retVal = RegQueryValueExW(regKeyHandle,			//HKEY hKey,            // handle to key
							L"DisabledFile",		//LPCTSTR lpValueName,  // value name
							NULL,					//LPDWORD lpReserved,   // reserved
							&regValueType,			//LPDWORD lpType,       // type buffer
							(LPBYTE)files[2],		//LPBYTE lpData,        // data buffer
							&idSize);				//LPDWORD lpcbData      // size of data buffer

   dbDescr.height = height;
   dbDescr.width = width;

   if((retVal == ERROR_SUCCESS) && (registryResult == REG_OPENED_EXISTING_KEY))
   {
	   regValueType = REG_DWORD;
	   idSize = sizeof(DWORD);

	   RegQueryValueExW(regKeyHandle,			//HKEY hKey,            // handle to key
						L"IdleRect.X",			//LPCTSTR lpValueName,  // value name
						NULL,					//LPDWORD lpReserved,   // reserved
						&regValueType,			//LPDWORD lpType,       // type buffer
						(LPBYTE)sizes[0].X,		//LPBYTE lpData,        // data buffer
						&idSize);				//LPDWORD lpcbData      // size of data buffer

	   RegQueryValueExW(regKeyHandle,			//HKEY hKey,            // handle to key
						L"IdleRect.Y",			//LPCTSTR lpValueName,  // value name
						NULL,					//LPDWORD lpReserved,   // reserved
						&regValueType,			//LPDWORD lpType,       // type buffer
						(LPBYTE)sizes[0].Y,		//LPBYTE lpData,        // data buffer
						&idSize);				//LPDWORD lpcbData      // size of data buffer

	   RegQueryValueExW(regKeyHandle,			//HKEY hKey,            // handle to key
						L"IdleRect.Width",		//LPCTSTR lpValueName,  // value name
						NULL,					//LPDWORD lpReserved,   // reserved
						&regValueType,			//LPDWORD lpType,       // type buffer
						(LPBYTE)sizes[0].Width,	//LPBYTE lpData,        // data buffer
						&idSize);				//LPDWORD lpcbData      // size of data buffer

	   RegQueryValueExW(regKeyHandle,			//HKEY hKey,            // handle to key
						L"IdleRect.Height",		//LPCTSTR lpValueName,  // value name
						NULL,					//LPDWORD lpReserved,   // reserved
						&regValueType,			//LPDWORD lpType,       // type buffer
						(LPBYTE)sizes[0].Height,//LPBYTE lpData,        // data buffer
						&idSize);				//LPDWORD lpcbData      // size of data buffer

	   RegQueryValueExW(regKeyHandle,			//HKEY hKey,            // handle to key
						L"PressedRect.X",		//LPCTSTR lpValueName,  // value name
						NULL,					//LPDWORD lpReserved,   // reserved
						&regValueType,			//LPDWORD lpType,       // type buffer
						(LPBYTE)sizes[1].X,		//LPBYTE lpData,        // data buffer
						&idSize);				//LPDWORD lpcbData      // size of data buffer

	   RegQueryValueExW(regKeyHandle,			//HKEY hKey,            // handle to key
						L"PressedRect.Y",		//LPCTSTR lpValueName,  // value name
						NULL,					//LPDWORD lpReserved,   // reserved
						&regValueType,			//LPDWORD lpType,       // type buffer
						(LPBYTE)sizes[1].Y,		//LPBYTE lpData,        // data buffer
						&idSize);				//LPDWORD lpcbData      // size of data buffer

	   RegQueryValueExW(regKeyHandle,			//HKEY hKey,            // handle to key
						L"PressedRect.Width",	//LPCTSTR lpValueName,  // value name
						NULL,					//LPDWORD lpReserved,   // reserved
						&regValueType,			//LPDWORD lpType,       // type buffer
						(LPBYTE)sizes[1].Width,	//LPBYTE lpData,        // data buffer
						&idSize);				//LPDWORD lpcbData      // size of data buffer

	   RegQueryValueExW(regKeyHandle,			//HKEY hKey,            // handle to key
						L"PressedRect.Height",	//LPCTSTR lpValueName,  // value name
						NULL,					//LPDWORD lpReserved,   // reserved
						&regValueType,			//LPDWORD lpType,       // type buffer
						(LPBYTE)sizes[1].Height,//LPBYTE lpData,        // data buffer
						&idSize);				//LPDWORD lpcbData      // size of data buffer

	   RegQueryValueExW(regKeyHandle,			//HKEY hKey,            // handle to key
						L"DisabledRect.X",		//LPCTSTR lpValueName,  // value name
						NULL,					//LPDWORD lpReserved,   // reserved
						&regValueType,			//LPDWORD lpType,       // type buffer
						(LPBYTE)sizes[2].X,		//LPBYTE lpData,        // data buffer
						&idSize);				//LPDWORD lpcbData      // size of data buffer

	   RegQueryValueExW(regKeyHandle,			//HKEY hKey,            // handle to key
						L"DisabledRect.Y",		//LPCTSTR lpValueName,  // value name
						NULL,					//LPDWORD lpReserved,   // reserved
						&regValueType,			//LPDWORD lpType,       // type buffer
						(LPBYTE)sizes[2].Y,		//LPBYTE lpData,        // data buffer
						&idSize);				//LPDWORD lpcbData      // size of data buffer

	   RegQueryValueExW(regKeyHandle,			//HKEY hKey,            // handle to key
						L"DisabledRect.Width",	//LPCTSTR lpValueName,  // value name
						NULL,					//LPDWORD lpReserved,   // reserved
						&regValueType,			//LPDWORD lpType,       // type buffer
						(LPBYTE)sizes[2].Width,	//LPBYTE lpData,        // data buffer
						&idSize);				//LPDWORD lpcbData      // size of data buffer

	   RegQueryValueExW(regKeyHandle,			//HKEY hKey,            // handle to key
						L"DisabledRect.Height",	//LPCTSTR lpValueName,  // value name
						NULL,					//LPDWORD lpReserved,   // reserved
						&regValueType,			//LPDWORD lpType,       // type buffer
						(LPBYTE)sizes[2].Height,//LPBYTE lpData,        // data buffer
						&idSize);				//LPDWORD lpcbData      // size of data buffer

	   for(n = 0; n < 3; n++)
	   {
		  dbDescr.srcFile = files[n];
		  dbDescr.srcRect = sizes[n];
		  btn[n] = CreateButton(&dbDescr);
	   }
   }
   else
   {
	   dbDescr.srcFile = NULL;

	   for(n = 0; n < 3; n++)
	   {
		  switch(n)
		  {
			 case 0:
				dbDescr.rcBmp = IDB_DESTUP;
				break;
			 case 1:
				dbDescr.rcBmp = IDB_DESTDOWN;
				break;
			 case 2:
				dbDescr.rcBmp = IDB_DESTOFF;
				break;
		  }
		  btn[n] = CreateButton(&dbDescr);
	   }
   }
   RegCloseKey(regKeyHandle);
}

DestButton::~DestButton()
{
	if(btn[0]) delete btn[0];
	if(btn[1]) delete btn[1];
	if(btn[2]) delete btn[2];
}

Bitmap * DestButton::Disable()
{
	return(btn[2]);
}

Bitmap * DestButton::Idle()
{
	return(btn[0]);
}

Bitmap * DestButton::Pressed()
{
	return(btn[1]);
}

Bitmap * DestButton::CreateButton(DbDescr * dbDescr)
{
   Bitmap * myImage, * retBtn;
   Rect     srcRect;
   Color    pixColor;
   int      x, y;

   // Load the bitmap
   if(dbDescr->srcFile != NULL)
   {
	   myImage = new Bitmap(dbDescr->srcFile);
	   srcRect = dbDescr->srcRect;
	   if(myImage == NULL)
	   {
		   MessageBox(NULL, "Could not open bitmap", "Error", MB_OK);
		   return(FALSE);
	   }
   }
   else
   {
	   myImage = new Bitmap(hInst, (WCHAR *)dbDescr->rcBmp);
	   srcRect.X = srcRect.Y = 0;
	   srcRect.Width = srcRect.Height = SRC_SIZE;
	   if(myImage == NULL)
	   {
		   MessageBox(NULL, "Could not open bitmap resource!", "Error", MB_OK);
		   return(FALSE);
	   }
   }

   // Create a scaled bitmap portion
   retBtn = new Bitmap(dbDescr->width, dbDescr->height, PixelFormat24bppRGB);
   if(retBtn == NULL)
   {
      MessageBox(NULL, "Could not create new bitmap", "Error", MB_OK);
	  delete myImage;
	  return(FALSE);
   }
   for(y = 0; y < (srcRect.Height >> 1); y++)
   {
	  // Left upper corner
      for(x = 0; x < (srcRect.Width >> 1); x++)
	  {
	     myImage->GetPixel(x + srcRect.X, y + srcRect.Y, &pixColor);
		 retBtn->SetPixel(x, y, pixColor);
	  }
	  // Mid upper edge
      myImage->GetPixel(srcRect.X + (srcRect.Width >> 1), y + srcRect.Y, &pixColor);
	  for(x = (srcRect.Width >> 1); x < (dbDescr->width - (srcRect.Width >> 1)); x++)
	  {
		 retBtn->SetPixel(x, y, pixColor);
	  }
	  // Right upper corner
	  for(x = (srcRect.Width >> 1); x < srcRect.Width; x++)
	  {
	     myImage->GetPixel(x + srcRect.X, y + srcRect.Y, &pixColor);
		 retBtn->SetPixel(x + (dbDescr->width - srcRect.Width), y, pixColor);
	  }
   }

   // Left edge
   for(x = 0; x < (srcRect.Width >> 1); x++)
   {
	  myImage->GetPixel(x + srcRect.X, (srcRect.Width >> 1), &pixColor); //!!!
	  for(y = (srcRect.Height >> 1); y < (dbDescr->height - (srcRect.Height >> 1)); y++)
	  {
		 retBtn->SetPixel(x, y, pixColor);
	  }
   } 
   // Mid area
   myImage->GetPixel(srcRect.X + (srcRect.Width >> 1), (srcRect.Height >> 1) + srcRect.Y, &pixColor);
   for(x = (srcRect.Width >> 1); x < (dbDescr->width - (srcRect.Width >> 1)); x++)
   {
	  for(y = (srcRect.Height >> 1); y < (dbDescr->height - (srcRect.Height >> 1)); y++)
	  {
		retBtn->SetPixel(x, y, pixColor);
	  }
   }

   // Right edge
   for(x = (srcRect.Width >> 1); x < srcRect.Width; x++)
   {
	  myImage->GetPixel(x + srcRect.X, (srcRect.Height >> 1) + srcRect.Y, &pixColor);
	  for(y = (srcRect.Height >> 1); y < (dbDescr->height - (srcRect.Height >> 1)); y++)
	  {
		 retBtn->SetPixel(x + (dbDescr->width - srcRect.Height), y, pixColor);
	  }
   }

   for(y = (srcRect.Width >> 1); y < srcRect.Width; y++)
   {
	  // Left lower corner
	  for(x = 0; x < (srcRect.Width >> 1); x++)
	  {
	     myImage->GetPixel(x + srcRect.X, y + srcRect.Y, &pixColor);
		 retBtn->SetPixel(x, y + (dbDescr->height - srcRect.Height), pixColor);
	  }
	  // Mid lower edge
      myImage->GetPixel(srcRect.X + (srcRect.Width >> 1), y + srcRect.Y, &pixColor);
	  for(x = (srcRect.Width >> 1); x < (dbDescr->width - (srcRect.Width >> 1)); x++)
	  {
		 retBtn->SetPixel(x, y + (dbDescr->height - srcRect.Height), pixColor);
	  }
	  // Right lower corner
	  for(x = (srcRect.Width >> 1); x < srcRect.Width; x++)
	  {
	     myImage->GetPixel(x + srcRect.X, y + srcRect.Y, &pixColor);
		 retBtn->SetPixel(x + (dbDescr->width - srcRect.Width), y + (dbDescr->height - srcRect.Height), pixColor);
	  }
   }
   delete myImage;
   return(retBtn);
}


