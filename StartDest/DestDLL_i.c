/* this file contains the actual definitions of */
/* the IIDs and CLSIDs */

/* link this file in with the server and any clients */


/* File created by MIDL compiler version 5.01.0164 */
/* at Wed Mar 26 17:38:32 2003
 */
/* Compiler settings for D:\Dest_SDK_201\DestDLL\DestDLL.idl:
    Oicf (OptLev=i2), W1, Zp8, env=Win32, ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data
*/
//@@MIDL_FILE_HEADING(  )
#ifdef __cplusplus
extern "C"{
#endif


#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif // __IID_DEFINED__

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif // CLSID_DEFINED

const IID IID_IDest = {0x4A0ED4F2,0x1F7F,0x4EAC,{0x83,0x87,0x50,0xE0,0x5F,0xAB,0xAD,0xDF}};


const IID IID_IDest2 = {0x98B75860,0xAD57,0x4b2f,{0xA2,0x41,0x5B,0x33,0x36,0x59,0x04,0xCA}};


const IID LIBID_DESTDLLLib = {0xAD0FEB15,0xC9B5,0x4AEC,{0xB2,0xDE,0xD1,0xF0,0xDC,0xDB,0x92,0xF3}};


const IID DIID__IDestEvents = {0x9BD46DC5,0x658B,0x4102,{0x8B,0x5F,0x52,0xE0,0x42,0x4D,0xAF,0xB1}};


const IID IID__IDestEvents2 = {0xA64F2C06,0x2E8C,0x4229,{0x85,0x21,0xA6,0x4A,0xDC,0xBD,0x4E,0xC2}};


const CLSID CLSID_Dest = {0xD3E508C8,0x211A,0x4739,{0xBA,0x04,0x80,0x42,0xD3,0x44,0x0A,0x8D}};


#ifdef __cplusplus
}
#endif

