/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 5.01.0164 */
/* at Wed Mar 26 17:38:32 2003
 */
/* Compiler settings for D:\Dest_SDK_201\DestDLL\DestDLL.idl:
    Oicf (OptLev=i2), W1, Zp8, env=Win32, ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
*/
//@@MIDL_FILE_HEADING(  )


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __DestDLL_h__
#define __DestDLL_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

#ifndef __IDest_FWD_DEFINED__
#define __IDest_FWD_DEFINED__
typedef interface IDest IDest;
#endif 	/* __IDest_FWD_DEFINED__ */


#ifndef __IDest2_FWD_DEFINED__
#define __IDest2_FWD_DEFINED__
typedef interface IDest2 IDest2;
#endif 	/* __IDest2_FWD_DEFINED__ */


#ifndef ___IDestEvents_FWD_DEFINED__
#define ___IDestEvents_FWD_DEFINED__
typedef interface _IDestEvents _IDestEvents;
#endif 	/* ___IDestEvents_FWD_DEFINED__ */


#ifndef ___IDestEvents2_FWD_DEFINED__
#define ___IDestEvents2_FWD_DEFINED__
typedef interface _IDestEvents2 _IDestEvents2;
#endif 	/* ___IDestEvents2_FWD_DEFINED__ */


#ifndef __Dest_FWD_DEFINED__
#define __Dest_FWD_DEFINED__

#ifdef __cplusplus
typedef class Dest Dest;
#else
typedef struct Dest Dest;
#endif /* __cplusplus */

#endif 	/* __Dest_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

/* interface __MIDL_itf_DestDLL_0000 */
/* [local] */ 

typedef 
enum CREATE_DESTINATOR_FLAG
    {	F_DEFAULT	= 0,
	F_ABOUT_DIALOG	= 1,
	F_HIDE_ON_PRESS_EXIT	= 2,
	F_EMBEDED_VB_CLIENT	= 4
    }	CREATE_DESTINATOR_FLAG;

typedef 
enum CommandResult
    {	API_COMMAND_IS_DONE	= 0,
	API_COMMAND_IS_FAILED	= -1,
	API_COMMAND_DESTINATOR_OBJECT_NOT_EXIST	= -2,
	API_ROUTE_START_POINT_IS_BAD	= -3,
	API_ROUTE_FINISH_POINT_IS_BAD	= -4,
	API_ROUTE_PATH_NOT_FOUND	= -5,
	API_SHOW_POINT_OUT_OF_MAP	= -6,
	API_DESTINATOR_WINDOW_NOT_EXIST	= -7,
	API_DESTINATOR_INVALID_SCALE	= -8,
	API_COMMAND_WRONG_IMAGETYPE	= -9,
	API_RESTART_DESTINATOR	= -10,
	API_DESTINATOR_FATAL_ERROR	= -11,
	API_ROUTE_NOT_EXIST	= -12,
	API_INVALID_PARAMETER	= -13
    }	CommandResult;

typedef struct  DESTAPIMAINSETTINGS
    {
    long CarManual;
    long DayNight;
    long AutoZoom;
    long HeadingUpNorth;
    long QuickestShortest;
    long KilometersMiles;
    long OverSpeedLimitPrompt;
    long RecalculationPrompt;
    }	DESTAPIMAINSETTINGS;

typedef struct  DESTAPIGPSDATA
    {
    double Longitude;
    double Latitude;
    double EastVelocity;
    double NorthVelocity;
    short Year;
    short Month;
    short Day;
    short Hour;
    short Minute;
    short Second;
    long N_Satellites;
    long PositionQuality;
    long TimeQuality;
    }	DESTAPIGPSDATA;

typedef struct  DESTAPIPOINT
    {
    double Longitude;
    double Latitude;
    }	DESTAPIPOINT;

typedef struct  DESTAPIADDRESS
    {
    BSTR Zip;
    BSTR City;
    BSTR Street;
    BSTR House;
    }	DESTAPIADDRESS;

typedef struct  DESTAPIMAPLIST
    {
    long Count;
    SAFEARRAY __RPC_FAR * MapName;
    SAFEARRAY __RPC_FAR * MapLegend;
    }	DESTAPIMAPLIST;

typedef struct  DESTAPIMAPINFO
    {
    BSTR MapName;
    BSTR MapLegend;
    double MinLongitude;
    double MaxLongitude;
    double MinLatitude;
    double MaxLatitude;
    double StartLongitude;
    double StartLatitude;
    }	DESTAPIMAPINFO;

typedef struct  DESTAPIMAPSTATE
    {
    double Longitude;
    double Latitude;
    double Angle;
    double Scale;
    }	DESTAPIMAPSTATE;

typedef struct  DESTAPIROUTEINFO
    {
    BSTR Name;
    BSTR Comment;
    DESTAPIADDRESS DestinationAddress;
    double StartLongitude;
    double StartLatitude;
    double FinishLongitude;
    double FinishLatitude;
    long TimeInSec;
    long DistanceInMeters;
    VARIANT_BOOL IsPath;
    }	DESTAPIROUTEINFO;

typedef struct  DESTAPIMANEUVERINFO
    {
    long Count;
    SAFEARRAY __RPC_FAR * ManeuverID;
    SAFEARRAY __RPC_FAR * StreetName;
    SAFEARRAY __RPC_FAR * Distance;
    }	DESTAPIMANEUVERINFO;

typedef struct  DESTAPIADDRESSBOOKDATA
    {
    long Count;
    SAFEARRAY __RPC_FAR * Longitude;
    SAFEARRAY __RPC_FAR * Latitude;
    SAFEARRAY __RPC_FAR * ItemName;
    }	DESTAPIADDRESSBOOKDATA;

typedef struct  DESTAPIPOIKEYINFO
    {
    long Count;
    SAFEARRAY __RPC_FAR * Code;
    SAFEARRAY __RPC_FAR * SubCode;
    SAFEARRAY __RPC_FAR * SubFood;
    SAFEARRAY __RPC_FAR * CodeName;
    SAFEARRAY __RPC_FAR * SubCodeName;
    SAFEARRAY __RPC_FAR * SubFoodName;
    }	DESTAPIPOIKEYINFO;

typedef struct  DESTAPIPOIKEY
    {
    long Code;
    long SubCode;
    long SubFood;
    }	DESTAPIPOIKEY;

typedef struct  DESTAPIPOILISTREQUEST
    {
    DESTAPIPOIKEY POIKey;
    DESTAPIPOINT StrobeCenter;
    long StrobeRadius;
    }	DESTAPIPOILISTREQUEST;

typedef struct  DESTAPIPOILIST
    {
    long Count;
    SAFEARRAY __RPC_FAR * Longitude;
    SAFEARRAY __RPC_FAR * Latitude;
    SAFEARRAY __RPC_FAR * Name;
    SAFEARRAY __RPC_FAR * Address;
    SAFEARRAY __RPC_FAR * House;
    SAFEARRAY __RPC_FAR * Street;
    SAFEARRAY __RPC_FAR * City;
    SAFEARRAY __RPC_FAR * Phone;
    }	DESTAPIPOILIST;

typedef 
enum IMAGE_TYPE
    {	IMAGE_BMP	= 0,
	IMAGE_JPEG	= 1,
	IMAGE_PNG	= 2
    }	IMAGE_TYPE;

typedef struct  DESTAPIIMAGEREQUEST
    {
    DESTAPIPOINT Position;
    long ImageWidth;
    long ImageHeight;
    IMAGE_TYPE ImageType;
    double Scale;
    }	DESTAPIIMAGEREQUEST;

typedef struct  DESTAPIIMAGEDATA
    {
    long ImageSize;
    IMAGE_TYPE ImageType;
    long ImageWidth;
    long ImageHeight;
    double Scale;
    double MapCos;
    SAFEARRAY __RPC_FAR * Image;
    }	DESTAPIIMAGEDATA;

typedef struct  DESTAPIIDARRAY
    {
    long Count;
    SAFEARRAY __RPC_FAR * IDArray;
    }	DESTAPIIDARRAY;

typedef 
enum ManoeuvreTypes
    {	NO_SOLUTION	= 0,
	STRAIGHT	= 1,
	TURN_LEFT	= 2,
	TURN_RIGHT	= 3,
	U_TURN	= 4,
	KEEP_LEFT	= 5,
	KEEP_RIGHT	= 6,
	KEEP_STRAIGHT	= 7,
	DESTINATION	= 8,
	CIRCLE_MANOEUVRE	= 9
    }	ManoeuvreTypes;

typedef 
enum VAR_BOOL
    {	False	= 0,
	True	= 0xffff
    }	VAR_BOOL;



extern RPC_IF_HANDLE __MIDL_itf_DestDLL_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_DestDLL_0000_v0_0_s_ifspec;

#ifndef __IDest_INTERFACE_DEFINED__
#define __IDest_INTERFACE_DEFINED__

/* interface IDest */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IDest;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("4A0ED4F2-1F7F-4EAC-8387-50E05FABADDF")
    IDest : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE CreateDestinatorWindow( 
            /* [in] */ long Flag,
            /* [retval][out] */ long __RPC_FAR *CommandResult) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ShowDestinatorWindow( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *PTResult) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE HideDestinatorWindow( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *PTResult) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ShowCurrentRoute( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *PTResult) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE CalculateRouteToCoordinates( 
            /* [in] */ double Longitude,
            /* [in] */ double Latitude,
            /* [in] */ BSTR Description,
            /* [retval][out] */ long __RPC_FAR *CommandResult) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetArrayOfCitiesID( 
            /* [out] */ VARIANT __RPC_FAR *CitiesIDArray,
            /* [out] */ VARIANT __RPC_FAR *AmountOfCities,
            /* [retval][out] */ long __RPC_FAR *CommandResult) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetCityNameFromID( 
            /* [in] */ long CityID,
            /* [out] */ VARIANT __RPC_FAR *CityName,
            /* [retval][out] */ long __RPC_FAR *CommandResult) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetArrayOfZipCodesID( 
            /* [out] */ VARIANT __RPC_FAR *ZipCodesIDArray,
            /* [out] */ VARIANT __RPC_FAR *AmountOfZipCodes,
            /* [retval][out] */ long __RPC_FAR *CommandResult) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetZipCodeFromID( 
            /* [in] */ long ZipCodeID,
            /* [out] */ VARIANT __RPC_FAR *ZipCode,
            /* [retval][out] */ long __RPC_FAR *CommandResult) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetArrayOfStreetsID( 
            /* [out] */ VARIANT __RPC_FAR *StreetsIDArray,
            /* [out] */ VARIANT __RPC_FAR *AmountOfStreets,
            /* [retval][out] */ long __RPC_FAR *CommandResult) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetStreetNameFromID( 
            /* [in] */ long StreetID,
            /* [out] */ VARIANT __RPC_FAR *StreetName,
            /* [retval][out] */ long __RPC_FAR *CommandResult) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetArrayOfStreetsIDForCity( 
            /* [in] */ long CityID,
            /* [out] */ VARIANT __RPC_FAR *StreetsIDArray,
            /* [out] */ VARIANT __RPC_FAR *AmountOfStreets,
            /* [retval][out] */ long __RPC_FAR *CommandResult) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetArrayOfStreetsIDForZip( 
            /* [in] */ long ZipID,
            /* [out] */ VARIANT __RPC_FAR *StreetsIDArray,
            /* [out] */ VARIANT __RPC_FAR *AmountOfStreets,
            /* [retval][out] */ long __RPC_FAR *CommandResult) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetArrayOfCitiesIDForSelectedStreet( 
            /* [in] */ long StreetID,
            /* [out] */ VARIANT __RPC_FAR *CitiesIDArray,
            /* [out] */ VARIANT __RPC_FAR *AmountOfCities,
            /* [retval][out] */ long __RPC_FAR *CommandResult) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetArrayOfZipCodesIDForSelectedStreet( 
            /* [in] */ long StreetID,
            /* [out] */ VARIANT __RPC_FAR *ZipCodesIDArray,
            /* [out] */ VARIANT __RPC_FAR *AmountOfZipCodes,
            /* [retval][out] */ long __RPC_FAR *CommandResult) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetMapsList( 
            /* [out] */ VARIANT __RPC_FAR *MapsList,
            /* [out] */ VARIANT __RPC_FAR *LegendsList,
            /* [out] */ VARIANT __RPC_FAR *AmountOfMaps) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetCurrentMapInfo( 
            /* [out] */ VARIANT __RPC_FAR *MapName,
            /* [out] */ VARIANT __RPC_FAR *MapLegend,
            /* [out] */ VARIANT __RPC_FAR *MinLongitude,
            /* [out] */ VARIANT __RPC_FAR *MaxLongitude,
            /* [out] */ VARIANT __RPC_FAR *MinLatitude,
            /* [out] */ VARIANT __RPC_FAR *MaxLatitude,
            /* [out] */ VARIANT __RPC_FAR *StartLongitude,
            /* [out] */ VARIANT __RPC_FAR *StartLatitude) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SwapMaps( 
            /* [in] */ BSTR MapName,
            /* [retval][out] */ long __RPC_FAR *CommandResult) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetCurrentMapState( 
            /* [out] */ VARIANT __RPC_FAR *Longitude,
            /* [out] */ VARIANT __RPC_FAR *Latitude,
            /* [out] */ VARIANT __RPC_FAR *Angle,
            /* [out] */ VARIANT __RPC_FAR *Scale) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetCurrentCarPosition( 
            /* [out] */ VARIANT __RPC_FAR *Longitude,
            /* [out] */ VARIANT __RPC_FAR *Latitude) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetGPSData( 
            /* [out] */ VARIANT __RPC_FAR *Longitude,
            /* [out] */ VARIANT __RPC_FAR *Latitude,
            /* [out] */ VARIANT __RPC_FAR *EastVelocity,
            /* [out] */ VARIANT __RPC_FAR *NorthVelocity,
            /* [out] */ VARIANT __RPC_FAR *Year,
            /* [out] */ VARIANT __RPC_FAR *Month,
            /* [out] */ VARIANT __RPC_FAR *Day,
            /* [out] */ VARIANT __RPC_FAR *Hour,
            /* [out] */ VARIANT __RPC_FAR *Minute,
            /* [out] */ VARIANT __RPC_FAR *Second,
            /* [out] */ VARIANT __RPC_FAR *N_Satellites,
            /* [out] */ VARIANT __RPC_FAR *PositionQuality,
            /* [out] */ VARIANT __RPC_FAR *TimeQuality) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetRouteInfo( 
            /* [out] */ VARIANT __RPC_FAR *Name,
            /* [out] */ VARIANT __RPC_FAR *Comment,
            /* [out] */ VARIANT __RPC_FAR *City,
            /* [out] */ VARIANT __RPC_FAR *Street,
            /* [out] */ VARIANT __RPC_FAR *Zip,
            /* [out] */ VARIANT __RPC_FAR *House,
            /* [out] */ VARIANT __RPC_FAR *StartLongitude,
            /* [out] */ VARIANT __RPC_FAR *StartLatitude,
            /* [out] */ VARIANT __RPC_FAR *FinishLongitude,
            /* [out] */ VARIANT __RPC_FAR *FinishLatitude,
            /* [out] */ VARIANT __RPC_FAR *TimeInSec,
            /* [out] */ VARIANT __RPC_FAR *DistanceInMeters,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *IsPath) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetDestinatorSettings( 
            /* [out] */ VARIANT __RPC_FAR *CarManual,
            /* [out] */ VARIANT __RPC_FAR *DayNight,
            /* [out] */ VARIANT __RPC_FAR *AutoZoom,
            /* [out] */ VARIANT __RPC_FAR *HeadingUpNorth,
            /* [out] */ VARIANT __RPC_FAR *QuickestShortest,
            /* [out] */ VARIANT __RPC_FAR *KilometersMiles,
            /* [out] */ VARIANT __RPC_FAR *OverSpeedLimitPrompt,
            /* [out] */ VARIANT __RPC_FAR *RecalculationPrompt) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetDestinatorSettings( 
            /* [in] */ long CarManual,
            /* [in] */ long DayNight,
            /* [in] */ long AutoZoom,
            /* [in] */ long HeadingUpNorth,
            /* [in] */ long QuickestShortest,
            /* [in] */ long KilometersMiles,
            /* [in] */ long OverSpeedLimitPrompt,
            /* [in] */ long RecalculationPrompt) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE CalculateRouteToAddress( 
            /* [in] */ BSTR City,
            /* [in] */ BSTR Street,
            /* [in] */ BSTR House,
            /* [in] */ BSTR Zip,
            /* [in] */ BSTR Description,
            /* [retval][out] */ long __RPC_FAR *CommandResult) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ShowDesiredPosition( 
            /* [in] */ double Longitude,
            /* [in] */ double Latitude,
            /* [in] */ BSTR Description,
            /* [retval][out] */ long __RPC_FAR *CommandResult) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetReverseGeoCoding( 
            /* [in] */ double Longitude,
            /* [in] */ double Latitude,
            /* [out] */ VARIANT __RPC_FAR *City,
            /* [out] */ VARIANT __RPC_FAR *Street,
            /* [out] */ VARIANT __RPC_FAR *House,
            /* [out] */ VARIANT __RPC_FAR *Zip,
            /* [retval][out] */ long __RPC_FAR *CommandResult) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE AddItemToAddressBook( 
            /* [in] */ double XDeg,
            /* [in] */ double YDeg,
            /* [in] */ BSTR ItemName) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetCoordinatesFromAddress( 
            /* [in] */ BSTR City,
            /* [in] */ BSTR Street,
            /* [in] */ BSTR House,
            /* [in] */ BSTR Zip,
            /* [out] */ VARIANT __RPC_FAR *XDeg,
            /* [out] */ VARIANT __RPC_FAR *YDeg,
            /* [retval][out] */ long __RPC_FAR *CommandResult) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE OpenCOMPort( 
            /* [retval][out] */ long __RPC_FAR *CommandResult) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE CloseCOMPort( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ReturnToMainDestinatorWindow( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE DestroyDestinatorWindow( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE CalculateRouteToStreetIntersection( 
            /* [in] */ BSTR Description,
            /* [in] */ long CityID,
            /* [in] */ long Street1ID,
            /* [in] */ long Street2ID,
            /* [retval][out] */ long __RPC_FAR *CommandResult) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetCoordinatesOfStreetIntersection( 
            /* [in] */ long CityID,
            /* [in] */ long Street1ID,
            /* [in] */ long Street2ID,
            /* [out] */ VARIANT __RPC_FAR *XDeg,
            /* [out] */ VARIANT __RPC_FAR *YDeg,
            /* [retval][out] */ long __RPC_FAR *CommandResult) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetManeuverInfo( 
            /* [out] */ VARIANT __RPC_FAR *AmountOfManeuvers,
            /* [out] */ VARIANT __RPC_FAR *ManeuverIDArray,
            /* [out] */ VARIANT __RPC_FAR *StreetNameArray,
            /* [out] */ VARIANT __RPC_FAR *DistanceArray) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetPoiKeyInfo( 
            /* [out] */ VARIANT __RPC_FAR *AmountOfItems,
            /* [out] */ VARIANT __RPC_FAR *PoiCodeArray,
            /* [out] */ VARIANT __RPC_FAR *PoiSubCodeArray,
            /* [out] */ VARIANT __RPC_FAR *PoiSubFoodArray,
            /* [out] */ VARIANT __RPC_FAR *CodeNameArray,
            /* [out] */ VARIANT __RPC_FAR *SubCodeNameArray,
            /* [out] */ VARIANT __RPC_FAR *SubFoodNameArray) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetPoiListForKey( 
            /* [in] */ long Code,
            /* [in] */ long SubCode,
            /* [in] */ long SubFood,
            /* [in] */ double Longitude,
            /* [in] */ double Latitude,
            /* [in] */ long RadiusInMeters,
            /* [out] */ VARIANT __RPC_FAR *NumberOfPoi,
            /* [out] */ VARIANT __RPC_FAR *XPoi,
            /* [out] */ VARIANT __RPC_FAR *YPoi,
            /* [out] */ VARIANT __RPC_FAR *Name,
            /* [out] */ VARIANT __RPC_FAR *Address,
            /* [out] */ VARIANT __RPC_FAR *House,
            /* [out] */ VARIANT __RPC_FAR *StreetName,
            /* [out] */ VARIANT __RPC_FAR *CityName,
            /* [out] */ VARIANT __RPC_FAR *Phone) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetViewState( 
            /* [out] */ VARIANT __RPC_FAR *View3D2D) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetViewState( 
            /* [in] */ long View3D2D) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE CancelCurrentRoute( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetAddressBookList( 
            /* [out] */ VARIANT __RPC_FAR *XDegArray,
            /* [out] */ VARIANT __RPC_FAR *YDegArray,
            /* [out] */ VARIANT __RPC_FAR *ItemNameArray,
            /* [out] */ VARIANT __RPC_FAR *AmountOfItems) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetMapImage( 
            /* [in] */ double Longitude,
            /* [in] */ double Latitude,
            /* [in] */ long Width,
            /* [in] */ long Height,
            /* [out][in] */ VARIANT __RPC_FAR *Scale,
            /* [out] */ VARIANT __RPC_FAR *MapCos,
            /* [in] */ long ImageType,
            /* [out] */ VARIANT __RPC_FAR *pImage,
            /* [out] */ VARIANT __RPC_FAR *ImageSize,
            /* [retval][out] */ long __RPC_FAR *ErrorCode) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDestVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IDest __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IDest __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IDest __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IDest __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IDest __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IDest __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IDest __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CreateDestinatorWindow )( 
            IDest __RPC_FAR * This,
            /* [in] */ long Flag,
            /* [retval][out] */ long __RPC_FAR *CommandResult);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ShowDestinatorWindow )( 
            IDest __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *PTResult);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *HideDestinatorWindow )( 
            IDest __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *PTResult);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ShowCurrentRoute )( 
            IDest __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *PTResult);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CalculateRouteToCoordinates )( 
            IDest __RPC_FAR * This,
            /* [in] */ double Longitude,
            /* [in] */ double Latitude,
            /* [in] */ BSTR Description,
            /* [retval][out] */ long __RPC_FAR *CommandResult);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetArrayOfCitiesID )( 
            IDest __RPC_FAR * This,
            /* [out] */ VARIANT __RPC_FAR *CitiesIDArray,
            /* [out] */ VARIANT __RPC_FAR *AmountOfCities,
            /* [retval][out] */ long __RPC_FAR *CommandResult);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCityNameFromID )( 
            IDest __RPC_FAR * This,
            /* [in] */ long CityID,
            /* [out] */ VARIANT __RPC_FAR *CityName,
            /* [retval][out] */ long __RPC_FAR *CommandResult);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetArrayOfZipCodesID )( 
            IDest __RPC_FAR * This,
            /* [out] */ VARIANT __RPC_FAR *ZipCodesIDArray,
            /* [out] */ VARIANT __RPC_FAR *AmountOfZipCodes,
            /* [retval][out] */ long __RPC_FAR *CommandResult);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetZipCodeFromID )( 
            IDest __RPC_FAR * This,
            /* [in] */ long ZipCodeID,
            /* [out] */ VARIANT __RPC_FAR *ZipCode,
            /* [retval][out] */ long __RPC_FAR *CommandResult);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetArrayOfStreetsID )( 
            IDest __RPC_FAR * This,
            /* [out] */ VARIANT __RPC_FAR *StreetsIDArray,
            /* [out] */ VARIANT __RPC_FAR *AmountOfStreets,
            /* [retval][out] */ long __RPC_FAR *CommandResult);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetStreetNameFromID )( 
            IDest __RPC_FAR * This,
            /* [in] */ long StreetID,
            /* [out] */ VARIANT __RPC_FAR *StreetName,
            /* [retval][out] */ long __RPC_FAR *CommandResult);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetArrayOfStreetsIDForCity )( 
            IDest __RPC_FAR * This,
            /* [in] */ long CityID,
            /* [out] */ VARIANT __RPC_FAR *StreetsIDArray,
            /* [out] */ VARIANT __RPC_FAR *AmountOfStreets,
            /* [retval][out] */ long __RPC_FAR *CommandResult);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetArrayOfStreetsIDForZip )( 
            IDest __RPC_FAR * This,
            /* [in] */ long ZipID,
            /* [out] */ VARIANT __RPC_FAR *StreetsIDArray,
            /* [out] */ VARIANT __RPC_FAR *AmountOfStreets,
            /* [retval][out] */ long __RPC_FAR *CommandResult);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetArrayOfCitiesIDForSelectedStreet )( 
            IDest __RPC_FAR * This,
            /* [in] */ long StreetID,
            /* [out] */ VARIANT __RPC_FAR *CitiesIDArray,
            /* [out] */ VARIANT __RPC_FAR *AmountOfCities,
            /* [retval][out] */ long __RPC_FAR *CommandResult);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetArrayOfZipCodesIDForSelectedStreet )( 
            IDest __RPC_FAR * This,
            /* [in] */ long StreetID,
            /* [out] */ VARIANT __RPC_FAR *ZipCodesIDArray,
            /* [out] */ VARIANT __RPC_FAR *AmountOfZipCodes,
            /* [retval][out] */ long __RPC_FAR *CommandResult);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetMapsList )( 
            IDest __RPC_FAR * This,
            /* [out] */ VARIANT __RPC_FAR *MapsList,
            /* [out] */ VARIANT __RPC_FAR *LegendsList,
            /* [out] */ VARIANT __RPC_FAR *AmountOfMaps);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCurrentMapInfo )( 
            IDest __RPC_FAR * This,
            /* [out] */ VARIANT __RPC_FAR *MapName,
            /* [out] */ VARIANT __RPC_FAR *MapLegend,
            /* [out] */ VARIANT __RPC_FAR *MinLongitude,
            /* [out] */ VARIANT __RPC_FAR *MaxLongitude,
            /* [out] */ VARIANT __RPC_FAR *MinLatitude,
            /* [out] */ VARIANT __RPC_FAR *MaxLatitude,
            /* [out] */ VARIANT __RPC_FAR *StartLongitude,
            /* [out] */ VARIANT __RPC_FAR *StartLatitude);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SwapMaps )( 
            IDest __RPC_FAR * This,
            /* [in] */ BSTR MapName,
            /* [retval][out] */ long __RPC_FAR *CommandResult);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCurrentMapState )( 
            IDest __RPC_FAR * This,
            /* [out] */ VARIANT __RPC_FAR *Longitude,
            /* [out] */ VARIANT __RPC_FAR *Latitude,
            /* [out] */ VARIANT __RPC_FAR *Angle,
            /* [out] */ VARIANT __RPC_FAR *Scale);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCurrentCarPosition )( 
            IDest __RPC_FAR * This,
            /* [out] */ VARIANT __RPC_FAR *Longitude,
            /* [out] */ VARIANT __RPC_FAR *Latitude);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetGPSData )( 
            IDest __RPC_FAR * This,
            /* [out] */ VARIANT __RPC_FAR *Longitude,
            /* [out] */ VARIANT __RPC_FAR *Latitude,
            /* [out] */ VARIANT __RPC_FAR *EastVelocity,
            /* [out] */ VARIANT __RPC_FAR *NorthVelocity,
            /* [out] */ VARIANT __RPC_FAR *Year,
            /* [out] */ VARIANT __RPC_FAR *Month,
            /* [out] */ VARIANT __RPC_FAR *Day,
            /* [out] */ VARIANT __RPC_FAR *Hour,
            /* [out] */ VARIANT __RPC_FAR *Minute,
            /* [out] */ VARIANT __RPC_FAR *Second,
            /* [out] */ VARIANT __RPC_FAR *N_Satellites,
            /* [out] */ VARIANT __RPC_FAR *PositionQuality,
            /* [out] */ VARIANT __RPC_FAR *TimeQuality);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetRouteInfo )( 
            IDest __RPC_FAR * This,
            /* [out] */ VARIANT __RPC_FAR *Name,
            /* [out] */ VARIANT __RPC_FAR *Comment,
            /* [out] */ VARIANT __RPC_FAR *City,
            /* [out] */ VARIANT __RPC_FAR *Street,
            /* [out] */ VARIANT __RPC_FAR *Zip,
            /* [out] */ VARIANT __RPC_FAR *House,
            /* [out] */ VARIANT __RPC_FAR *StartLongitude,
            /* [out] */ VARIANT __RPC_FAR *StartLatitude,
            /* [out] */ VARIANT __RPC_FAR *FinishLongitude,
            /* [out] */ VARIANT __RPC_FAR *FinishLatitude,
            /* [out] */ VARIANT __RPC_FAR *TimeInSec,
            /* [out] */ VARIANT __RPC_FAR *DistanceInMeters,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *IsPath);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDestinatorSettings )( 
            IDest __RPC_FAR * This,
            /* [out] */ VARIANT __RPC_FAR *CarManual,
            /* [out] */ VARIANT __RPC_FAR *DayNight,
            /* [out] */ VARIANT __RPC_FAR *AutoZoom,
            /* [out] */ VARIANT __RPC_FAR *HeadingUpNorth,
            /* [out] */ VARIANT __RPC_FAR *QuickestShortest,
            /* [out] */ VARIANT __RPC_FAR *KilometersMiles,
            /* [out] */ VARIANT __RPC_FAR *OverSpeedLimitPrompt,
            /* [out] */ VARIANT __RPC_FAR *RecalculationPrompt);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetDestinatorSettings )( 
            IDest __RPC_FAR * This,
            /* [in] */ long CarManual,
            /* [in] */ long DayNight,
            /* [in] */ long AutoZoom,
            /* [in] */ long HeadingUpNorth,
            /* [in] */ long QuickestShortest,
            /* [in] */ long KilometersMiles,
            /* [in] */ long OverSpeedLimitPrompt,
            /* [in] */ long RecalculationPrompt);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CalculateRouteToAddress )( 
            IDest __RPC_FAR * This,
            /* [in] */ BSTR City,
            /* [in] */ BSTR Street,
            /* [in] */ BSTR House,
            /* [in] */ BSTR Zip,
            /* [in] */ BSTR Description,
            /* [retval][out] */ long __RPC_FAR *CommandResult);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ShowDesiredPosition )( 
            IDest __RPC_FAR * This,
            /* [in] */ double Longitude,
            /* [in] */ double Latitude,
            /* [in] */ BSTR Description,
            /* [retval][out] */ long __RPC_FAR *CommandResult);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetReverseGeoCoding )( 
            IDest __RPC_FAR * This,
            /* [in] */ double Longitude,
            /* [in] */ double Latitude,
            /* [out] */ VARIANT __RPC_FAR *City,
            /* [out] */ VARIANT __RPC_FAR *Street,
            /* [out] */ VARIANT __RPC_FAR *House,
            /* [out] */ VARIANT __RPC_FAR *Zip,
            /* [retval][out] */ long __RPC_FAR *CommandResult);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AddItemToAddressBook )( 
            IDest __RPC_FAR * This,
            /* [in] */ double XDeg,
            /* [in] */ double YDeg,
            /* [in] */ BSTR ItemName);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCoordinatesFromAddress )( 
            IDest __RPC_FAR * This,
            /* [in] */ BSTR City,
            /* [in] */ BSTR Street,
            /* [in] */ BSTR House,
            /* [in] */ BSTR Zip,
            /* [out] */ VARIANT __RPC_FAR *XDeg,
            /* [out] */ VARIANT __RPC_FAR *YDeg,
            /* [retval][out] */ long __RPC_FAR *CommandResult);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OpenCOMPort )( 
            IDest __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *CommandResult);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CloseCOMPort )( 
            IDest __RPC_FAR * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ReturnToMainDestinatorWindow )( 
            IDest __RPC_FAR * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DestroyDestinatorWindow )( 
            IDest __RPC_FAR * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CalculateRouteToStreetIntersection )( 
            IDest __RPC_FAR * This,
            /* [in] */ BSTR Description,
            /* [in] */ long CityID,
            /* [in] */ long Street1ID,
            /* [in] */ long Street2ID,
            /* [retval][out] */ long __RPC_FAR *CommandResult);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCoordinatesOfStreetIntersection )( 
            IDest __RPC_FAR * This,
            /* [in] */ long CityID,
            /* [in] */ long Street1ID,
            /* [in] */ long Street2ID,
            /* [out] */ VARIANT __RPC_FAR *XDeg,
            /* [out] */ VARIANT __RPC_FAR *YDeg,
            /* [retval][out] */ long __RPC_FAR *CommandResult);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetManeuverInfo )( 
            IDest __RPC_FAR * This,
            /* [out] */ VARIANT __RPC_FAR *AmountOfManeuvers,
            /* [out] */ VARIANT __RPC_FAR *ManeuverIDArray,
            /* [out] */ VARIANT __RPC_FAR *StreetNameArray,
            /* [out] */ VARIANT __RPC_FAR *DistanceArray);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetPoiKeyInfo )( 
            IDest __RPC_FAR * This,
            /* [out] */ VARIANT __RPC_FAR *AmountOfItems,
            /* [out] */ VARIANT __RPC_FAR *PoiCodeArray,
            /* [out] */ VARIANT __RPC_FAR *PoiSubCodeArray,
            /* [out] */ VARIANT __RPC_FAR *PoiSubFoodArray,
            /* [out] */ VARIANT __RPC_FAR *CodeNameArray,
            /* [out] */ VARIANT __RPC_FAR *SubCodeNameArray,
            /* [out] */ VARIANT __RPC_FAR *SubFoodNameArray);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetPoiListForKey )( 
            IDest __RPC_FAR * This,
            /* [in] */ long Code,
            /* [in] */ long SubCode,
            /* [in] */ long SubFood,
            /* [in] */ double Longitude,
            /* [in] */ double Latitude,
            /* [in] */ long RadiusInMeters,
            /* [out] */ VARIANT __RPC_FAR *NumberOfPoi,
            /* [out] */ VARIANT __RPC_FAR *XPoi,
            /* [out] */ VARIANT __RPC_FAR *YPoi,
            /* [out] */ VARIANT __RPC_FAR *Name,
            /* [out] */ VARIANT __RPC_FAR *Address,
            /* [out] */ VARIANT __RPC_FAR *House,
            /* [out] */ VARIANT __RPC_FAR *StreetName,
            /* [out] */ VARIANT __RPC_FAR *CityName,
            /* [out] */ VARIANT __RPC_FAR *Phone);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetViewState )( 
            IDest __RPC_FAR * This,
            /* [out] */ VARIANT __RPC_FAR *View3D2D);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetViewState )( 
            IDest __RPC_FAR * This,
            /* [in] */ long View3D2D);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CancelCurrentRoute )( 
            IDest __RPC_FAR * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetAddressBookList )( 
            IDest __RPC_FAR * This,
            /* [out] */ VARIANT __RPC_FAR *XDegArray,
            /* [out] */ VARIANT __RPC_FAR *YDegArray,
            /* [out] */ VARIANT __RPC_FAR *ItemNameArray,
            /* [out] */ VARIANT __RPC_FAR *AmountOfItems);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetMapImage )( 
            IDest __RPC_FAR * This,
            /* [in] */ double Longitude,
            /* [in] */ double Latitude,
            /* [in] */ long Width,
            /* [in] */ long Height,
            /* [out][in] */ VARIANT __RPC_FAR *Scale,
            /* [out] */ VARIANT __RPC_FAR *MapCos,
            /* [in] */ long ImageType,
            /* [out] */ VARIANT __RPC_FAR *pImage,
            /* [out] */ VARIANT __RPC_FAR *ImageSize,
            /* [retval][out] */ long __RPC_FAR *ErrorCode);
        
        END_INTERFACE
    } IDestVtbl;

    interface IDest
    {
        CONST_VTBL struct IDestVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDest_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDest_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDest_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDest_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IDest_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IDest_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IDest_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IDest_CreateDestinatorWindow(This,Flag,CommandResult)	\
    (This)->lpVtbl -> CreateDestinatorWindow(This,Flag,CommandResult)

#define IDest_ShowDestinatorWindow(This,PTResult)	\
    (This)->lpVtbl -> ShowDestinatorWindow(This,PTResult)

#define IDest_HideDestinatorWindow(This,PTResult)	\
    (This)->lpVtbl -> HideDestinatorWindow(This,PTResult)

#define IDest_ShowCurrentRoute(This,PTResult)	\
    (This)->lpVtbl -> ShowCurrentRoute(This,PTResult)

#define IDest_CalculateRouteToCoordinates(This,Longitude,Latitude,Description,CommandResult)	\
    (This)->lpVtbl -> CalculateRouteToCoordinates(This,Longitude,Latitude,Description,CommandResult)

#define IDest_GetArrayOfCitiesID(This,CitiesIDArray,AmountOfCities,CommandResult)	\
    (This)->lpVtbl -> GetArrayOfCitiesID(This,CitiesIDArray,AmountOfCities,CommandResult)

#define IDest_GetCityNameFromID(This,CityID,CityName,CommandResult)	\
    (This)->lpVtbl -> GetCityNameFromID(This,CityID,CityName,CommandResult)

#define IDest_GetArrayOfZipCodesID(This,ZipCodesIDArray,AmountOfZipCodes,CommandResult)	\
    (This)->lpVtbl -> GetArrayOfZipCodesID(This,ZipCodesIDArray,AmountOfZipCodes,CommandResult)

#define IDest_GetZipCodeFromID(This,ZipCodeID,ZipCode,CommandResult)	\
    (This)->lpVtbl -> GetZipCodeFromID(This,ZipCodeID,ZipCode,CommandResult)

#define IDest_GetArrayOfStreetsID(This,StreetsIDArray,AmountOfStreets,CommandResult)	\
    (This)->lpVtbl -> GetArrayOfStreetsID(This,StreetsIDArray,AmountOfStreets,CommandResult)

#define IDest_GetStreetNameFromID(This,StreetID,StreetName,CommandResult)	\
    (This)->lpVtbl -> GetStreetNameFromID(This,StreetID,StreetName,CommandResult)

#define IDest_GetArrayOfStreetsIDForCity(This,CityID,StreetsIDArray,AmountOfStreets,CommandResult)	\
    (This)->lpVtbl -> GetArrayOfStreetsIDForCity(This,CityID,StreetsIDArray,AmountOfStreets,CommandResult)

#define IDest_GetArrayOfStreetsIDForZip(This,ZipID,StreetsIDArray,AmountOfStreets,CommandResult)	\
    (This)->lpVtbl -> GetArrayOfStreetsIDForZip(This,ZipID,StreetsIDArray,AmountOfStreets,CommandResult)

#define IDest_GetArrayOfCitiesIDForSelectedStreet(This,StreetID,CitiesIDArray,AmountOfCities,CommandResult)	\
    (This)->lpVtbl -> GetArrayOfCitiesIDForSelectedStreet(This,StreetID,CitiesIDArray,AmountOfCities,CommandResult)

#define IDest_GetArrayOfZipCodesIDForSelectedStreet(This,StreetID,ZipCodesIDArray,AmountOfZipCodes,CommandResult)	\
    (This)->lpVtbl -> GetArrayOfZipCodesIDForSelectedStreet(This,StreetID,ZipCodesIDArray,AmountOfZipCodes,CommandResult)

#define IDest_GetMapsList(This,MapsList,LegendsList,AmountOfMaps)	\
    (This)->lpVtbl -> GetMapsList(This,MapsList,LegendsList,AmountOfMaps)

#define IDest_GetCurrentMapInfo(This,MapName,MapLegend,MinLongitude,MaxLongitude,MinLatitude,MaxLatitude,StartLongitude,StartLatitude)	\
    (This)->lpVtbl -> GetCurrentMapInfo(This,MapName,MapLegend,MinLongitude,MaxLongitude,MinLatitude,MaxLatitude,StartLongitude,StartLatitude)

#define IDest_SwapMaps(This,MapName,CommandResult)	\
    (This)->lpVtbl -> SwapMaps(This,MapName,CommandResult)

#define IDest_GetCurrentMapState(This,Longitude,Latitude,Angle,Scale)	\
    (This)->lpVtbl -> GetCurrentMapState(This,Longitude,Latitude,Angle,Scale)

#define IDest_GetCurrentCarPosition(This,Longitude,Latitude)	\
    (This)->lpVtbl -> GetCurrentCarPosition(This,Longitude,Latitude)

#define IDest_GetGPSData(This,Longitude,Latitude,EastVelocity,NorthVelocity,Year,Month,Day,Hour,Minute,Second,N_Satellites,PositionQuality,TimeQuality)	\
    (This)->lpVtbl -> GetGPSData(This,Longitude,Latitude,EastVelocity,NorthVelocity,Year,Month,Day,Hour,Minute,Second,N_Satellites,PositionQuality,TimeQuality)

#define IDest_GetRouteInfo(This,Name,Comment,City,Street,Zip,House,StartLongitude,StartLatitude,FinishLongitude,FinishLatitude,TimeInSec,DistanceInMeters,IsPath)	\
    (This)->lpVtbl -> GetRouteInfo(This,Name,Comment,City,Street,Zip,House,StartLongitude,StartLatitude,FinishLongitude,FinishLatitude,TimeInSec,DistanceInMeters,IsPath)

#define IDest_GetDestinatorSettings(This,CarManual,DayNight,AutoZoom,HeadingUpNorth,QuickestShortest,KilometersMiles,OverSpeedLimitPrompt,RecalculationPrompt)	\
    (This)->lpVtbl -> GetDestinatorSettings(This,CarManual,DayNight,AutoZoom,HeadingUpNorth,QuickestShortest,KilometersMiles,OverSpeedLimitPrompt,RecalculationPrompt)

#define IDest_SetDestinatorSettings(This,CarManual,DayNight,AutoZoom,HeadingUpNorth,QuickestShortest,KilometersMiles,OverSpeedLimitPrompt,RecalculationPrompt)	\
    (This)->lpVtbl -> SetDestinatorSettings(This,CarManual,DayNight,AutoZoom,HeadingUpNorth,QuickestShortest,KilometersMiles,OverSpeedLimitPrompt,RecalculationPrompt)

#define IDest_CalculateRouteToAddress(This,City,Street,House,Zip,Description,CommandResult)	\
    (This)->lpVtbl -> CalculateRouteToAddress(This,City,Street,House,Zip,Description,CommandResult)

#define IDest_ShowDesiredPosition(This,Longitude,Latitude,Description,CommandResult)	\
    (This)->lpVtbl -> ShowDesiredPosition(This,Longitude,Latitude,Description,CommandResult)

#define IDest_GetReverseGeoCoding(This,Longitude,Latitude,City,Street,House,Zip,CommandResult)	\
    (This)->lpVtbl -> GetReverseGeoCoding(This,Longitude,Latitude,City,Street,House,Zip,CommandResult)

#define IDest_AddItemToAddressBook(This,XDeg,YDeg,ItemName)	\
    (This)->lpVtbl -> AddItemToAddressBook(This,XDeg,YDeg,ItemName)

#define IDest_GetCoordinatesFromAddress(This,City,Street,House,Zip,XDeg,YDeg,CommandResult)	\
    (This)->lpVtbl -> GetCoordinatesFromAddress(This,City,Street,House,Zip,XDeg,YDeg,CommandResult)

#define IDest_OpenCOMPort(This,CommandResult)	\
    (This)->lpVtbl -> OpenCOMPort(This,CommandResult)

#define IDest_CloseCOMPort(This)	\
    (This)->lpVtbl -> CloseCOMPort(This)

#define IDest_ReturnToMainDestinatorWindow(This)	\
    (This)->lpVtbl -> ReturnToMainDestinatorWindow(This)

#define IDest_DestroyDestinatorWindow(This)	\
    (This)->lpVtbl -> DestroyDestinatorWindow(This)

#define IDest_CalculateRouteToStreetIntersection(This,Description,CityID,Street1ID,Street2ID,CommandResult)	\
    (This)->lpVtbl -> CalculateRouteToStreetIntersection(This,Description,CityID,Street1ID,Street2ID,CommandResult)

#define IDest_GetCoordinatesOfStreetIntersection(This,CityID,Street1ID,Street2ID,XDeg,YDeg,CommandResult)	\
    (This)->lpVtbl -> GetCoordinatesOfStreetIntersection(This,CityID,Street1ID,Street2ID,XDeg,YDeg,CommandResult)

#define IDest_GetManeuverInfo(This,AmountOfManeuvers,ManeuverIDArray,StreetNameArray,DistanceArray)	\
    (This)->lpVtbl -> GetManeuverInfo(This,AmountOfManeuvers,ManeuverIDArray,StreetNameArray,DistanceArray)

#define IDest_GetPoiKeyInfo(This,AmountOfItems,PoiCodeArray,PoiSubCodeArray,PoiSubFoodArray,CodeNameArray,SubCodeNameArray,SubFoodNameArray)	\
    (This)->lpVtbl -> GetPoiKeyInfo(This,AmountOfItems,PoiCodeArray,PoiSubCodeArray,PoiSubFoodArray,CodeNameArray,SubCodeNameArray,SubFoodNameArray)

#define IDest_GetPoiListForKey(This,Code,SubCode,SubFood,Longitude,Latitude,RadiusInMeters,NumberOfPoi,XPoi,YPoi,Name,Address,House,StreetName,CityName,Phone)	\
    (This)->lpVtbl -> GetPoiListForKey(This,Code,SubCode,SubFood,Longitude,Latitude,RadiusInMeters,NumberOfPoi,XPoi,YPoi,Name,Address,House,StreetName,CityName,Phone)

#define IDest_GetViewState(This,View3D2D)	\
    (This)->lpVtbl -> GetViewState(This,View3D2D)

#define IDest_SetViewState(This,View3D2D)	\
    (This)->lpVtbl -> SetViewState(This,View3D2D)

#define IDest_CancelCurrentRoute(This)	\
    (This)->lpVtbl -> CancelCurrentRoute(This)

#define IDest_GetAddressBookList(This,XDegArray,YDegArray,ItemNameArray,AmountOfItems)	\
    (This)->lpVtbl -> GetAddressBookList(This,XDegArray,YDegArray,ItemNameArray,AmountOfItems)

#define IDest_GetMapImage(This,Longitude,Latitude,Width,Height,Scale,MapCos,ImageType,pImage,ImageSize,ErrorCode)	\
    (This)->lpVtbl -> GetMapImage(This,Longitude,Latitude,Width,Height,Scale,MapCos,ImageType,pImage,ImageSize,ErrorCode)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IDest_CreateDestinatorWindow_Proxy( 
    IDest __RPC_FAR * This,
    /* [in] */ long Flag,
    /* [retval][out] */ long __RPC_FAR *CommandResult);


void __RPC_STUB IDest_CreateDestinatorWindow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IDest_ShowDestinatorWindow_Proxy( 
    IDest __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *PTResult);


void __RPC_STUB IDest_ShowDestinatorWindow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IDest_HideDestinatorWindow_Proxy( 
    IDest __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *PTResult);


void __RPC_STUB IDest_HideDestinatorWindow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IDest_ShowCurrentRoute_Proxy( 
    IDest __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *PTResult);


void __RPC_STUB IDest_ShowCurrentRoute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IDest_CalculateRouteToCoordinates_Proxy( 
    IDest __RPC_FAR * This,
    /* [in] */ double Longitude,
    /* [in] */ double Latitude,
    /* [in] */ BSTR Description,
    /* [retval][out] */ long __RPC_FAR *CommandResult);


void __RPC_STUB IDest_CalculateRouteToCoordinates_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IDest_GetArrayOfCitiesID_Proxy( 
    IDest __RPC_FAR * This,
    /* [out] */ VARIANT __RPC_FAR *CitiesIDArray,
    /* [out] */ VARIANT __RPC_FAR *AmountOfCities,
    /* [retval][out] */ long __RPC_FAR *CommandResult);


void __RPC_STUB IDest_GetArrayOfCitiesID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IDest_GetCityNameFromID_Proxy( 
    IDest __RPC_FAR * This,
    /* [in] */ long CityID,
    /* [out] */ VARIANT __RPC_FAR *CityName,
    /* [retval][out] */ long __RPC_FAR *CommandResult);


void __RPC_STUB IDest_GetCityNameFromID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IDest_GetArrayOfZipCodesID_Proxy( 
    IDest __RPC_FAR * This,
    /* [out] */ VARIANT __RPC_FAR *ZipCodesIDArray,
    /* [out] */ VARIANT __RPC_FAR *AmountOfZipCodes,
    /* [retval][out] */ long __RPC_FAR *CommandResult);


void __RPC_STUB IDest_GetArrayOfZipCodesID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IDest_GetZipCodeFromID_Proxy( 
    IDest __RPC_FAR * This,
    /* [in] */ long ZipCodeID,
    /* [out] */ VARIANT __RPC_FAR *ZipCode,
    /* [retval][out] */ long __RPC_FAR *CommandResult);


void __RPC_STUB IDest_GetZipCodeFromID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IDest_GetArrayOfStreetsID_Proxy( 
    IDest __RPC_FAR * This,
    /* [out] */ VARIANT __RPC_FAR *StreetsIDArray,
    /* [out] */ VARIANT __RPC_FAR *AmountOfStreets,
    /* [retval][out] */ long __RPC_FAR *CommandResult);


void __RPC_STUB IDest_GetArrayOfStreetsID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IDest_GetStreetNameFromID_Proxy( 
    IDest __RPC_FAR * This,
    /* [in] */ long StreetID,
    /* [out] */ VARIANT __RPC_FAR *StreetName,
    /* [retval][out] */ long __RPC_FAR *CommandResult);


void __RPC_STUB IDest_GetStreetNameFromID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IDest_GetArrayOfStreetsIDForCity_Proxy( 
    IDest __RPC_FAR * This,
    /* [in] */ long CityID,
    /* [out] */ VARIANT __RPC_FAR *StreetsIDArray,
    /* [out] */ VARIANT __RPC_FAR *AmountOfStreets,
    /* [retval][out] */ long __RPC_FAR *CommandResult);


void __RPC_STUB IDest_GetArrayOfStreetsIDForCity_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IDest_GetArrayOfStreetsIDForZip_Proxy( 
    IDest __RPC_FAR * This,
    /* [in] */ long ZipID,
    /* [out] */ VARIANT __RPC_FAR *StreetsIDArray,
    /* [out] */ VARIANT __RPC_FAR *AmountOfStreets,
    /* [retval][out] */ long __RPC_FAR *CommandResult);


void __RPC_STUB IDest_GetArrayOfStreetsIDForZip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IDest_GetArrayOfCitiesIDForSelectedStreet_Proxy( 
    IDest __RPC_FAR * This,
    /* [in] */ long StreetID,
    /* [out] */ VARIANT __RPC_FAR *CitiesIDArray,
    /* [out] */ VARIANT __RPC_FAR *AmountOfCities,
    /* [retval][out] */ long __RPC_FAR *CommandResult);


void __RPC_STUB IDest_GetArrayOfCitiesIDForSelectedStreet_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IDest_GetArrayOfZipCodesIDForSelectedStreet_Proxy( 
    IDest __RPC_FAR * This,
    /* [in] */ long StreetID,
    /* [out] */ VARIANT __RPC_FAR *ZipCodesIDArray,
    /* [out] */ VARIANT __RPC_FAR *AmountOfZipCodes,
    /* [retval][out] */ long __RPC_FAR *CommandResult);


void __RPC_STUB IDest_GetArrayOfZipCodesIDForSelectedStreet_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IDest_GetMapsList_Proxy( 
    IDest __RPC_FAR * This,
    /* [out] */ VARIANT __RPC_FAR *MapsList,
    /* [out] */ VARIANT __RPC_FAR *LegendsList,
    /* [out] */ VARIANT __RPC_FAR *AmountOfMaps);


void __RPC_STUB IDest_GetMapsList_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IDest_GetCurrentMapInfo_Proxy( 
    IDest __RPC_FAR * This,
    /* [out] */ VARIANT __RPC_FAR *MapName,
    /* [out] */ VARIANT __RPC_FAR *MapLegend,
    /* [out] */ VARIANT __RPC_FAR *MinLongitude,
    /* [out] */ VARIANT __RPC_FAR *MaxLongitude,
    /* [out] */ VARIANT __RPC_FAR *MinLatitude,
    /* [out] */ VARIANT __RPC_FAR *MaxLatitude,
    /* [out] */ VARIANT __RPC_FAR *StartLongitude,
    /* [out] */ VARIANT __RPC_FAR *StartLatitude);


void __RPC_STUB IDest_GetCurrentMapInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IDest_SwapMaps_Proxy( 
    IDest __RPC_FAR * This,
    /* [in] */ BSTR MapName,
    /* [retval][out] */ long __RPC_FAR *CommandResult);


void __RPC_STUB IDest_SwapMaps_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IDest_GetCurrentMapState_Proxy( 
    IDest __RPC_FAR * This,
    /* [out] */ VARIANT __RPC_FAR *Longitude,
    /* [out] */ VARIANT __RPC_FAR *Latitude,
    /* [out] */ VARIANT __RPC_FAR *Angle,
    /* [out] */ VARIANT __RPC_FAR *Scale);


void __RPC_STUB IDest_GetCurrentMapState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IDest_GetCurrentCarPosition_Proxy( 
    IDest __RPC_FAR * This,
    /* [out] */ VARIANT __RPC_FAR *Longitude,
    /* [out] */ VARIANT __RPC_FAR *Latitude);


void __RPC_STUB IDest_GetCurrentCarPosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IDest_GetGPSData_Proxy( 
    IDest __RPC_FAR * This,
    /* [out] */ VARIANT __RPC_FAR *Longitude,
    /* [out] */ VARIANT __RPC_FAR *Latitude,
    /* [out] */ VARIANT __RPC_FAR *EastVelocity,
    /* [out] */ VARIANT __RPC_FAR *NorthVelocity,
    /* [out] */ VARIANT __RPC_FAR *Year,
    /* [out] */ VARIANT __RPC_FAR *Month,
    /* [out] */ VARIANT __RPC_FAR *Day,
    /* [out] */ VARIANT __RPC_FAR *Hour,
    /* [out] */ VARIANT __RPC_FAR *Minute,
    /* [out] */ VARIANT __RPC_FAR *Second,
    /* [out] */ VARIANT __RPC_FAR *N_Satellites,
    /* [out] */ VARIANT __RPC_FAR *PositionQuality,
    /* [out] */ VARIANT __RPC_FAR *TimeQuality);


void __RPC_STUB IDest_GetGPSData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IDest_GetRouteInfo_Proxy( 
    IDest __RPC_FAR * This,
    /* [out] */ VARIANT __RPC_FAR *Name,
    /* [out] */ VARIANT __RPC_FAR *Comment,
    /* [out] */ VARIANT __RPC_FAR *City,
    /* [out] */ VARIANT __RPC_FAR *Street,
    /* [out] */ VARIANT __RPC_FAR *Zip,
    /* [out] */ VARIANT __RPC_FAR *House,
    /* [out] */ VARIANT __RPC_FAR *StartLongitude,
    /* [out] */ VARIANT __RPC_FAR *StartLatitude,
    /* [out] */ VARIANT __RPC_FAR *FinishLongitude,
    /* [out] */ VARIANT __RPC_FAR *FinishLatitude,
    /* [out] */ VARIANT __RPC_FAR *TimeInSec,
    /* [out] */ VARIANT __RPC_FAR *DistanceInMeters,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *IsPath);


void __RPC_STUB IDest_GetRouteInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IDest_GetDestinatorSettings_Proxy( 
    IDest __RPC_FAR * This,
    /* [out] */ VARIANT __RPC_FAR *CarManual,
    /* [out] */ VARIANT __RPC_FAR *DayNight,
    /* [out] */ VARIANT __RPC_FAR *AutoZoom,
    /* [out] */ VARIANT __RPC_FAR *HeadingUpNorth,
    /* [out] */ VARIANT __RPC_FAR *QuickestShortest,
    /* [out] */ VARIANT __RPC_FAR *KilometersMiles,
    /* [out] */ VARIANT __RPC_FAR *OverSpeedLimitPrompt,
    /* [out] */ VARIANT __RPC_FAR *RecalculationPrompt);


void __RPC_STUB IDest_GetDestinatorSettings_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IDest_SetDestinatorSettings_Proxy( 
    IDest __RPC_FAR * This,
    /* [in] */ long CarManual,
    /* [in] */ long DayNight,
    /* [in] */ long AutoZoom,
    /* [in] */ long HeadingUpNorth,
    /* [in] */ long QuickestShortest,
    /* [in] */ long KilometersMiles,
    /* [in] */ long OverSpeedLimitPrompt,
    /* [in] */ long RecalculationPrompt);


void __RPC_STUB IDest_SetDestinatorSettings_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IDest_CalculateRouteToAddress_Proxy( 
    IDest __RPC_FAR * This,
    /* [in] */ BSTR City,
    /* [in] */ BSTR Street,
    /* [in] */ BSTR House,
    /* [in] */ BSTR Zip,
    /* [in] */ BSTR Description,
    /* [retval][out] */ long __RPC_FAR *CommandResult);


void __RPC_STUB IDest_CalculateRouteToAddress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IDest_ShowDesiredPosition_Proxy( 
    IDest __RPC_FAR * This,
    /* [in] */ double Longitude,
    /* [in] */ double Latitude,
    /* [in] */ BSTR Description,
    /* [retval][out] */ long __RPC_FAR *CommandResult);


void __RPC_STUB IDest_ShowDesiredPosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IDest_GetReverseGeoCoding_Proxy( 
    IDest __RPC_FAR * This,
    /* [in] */ double Longitude,
    /* [in] */ double Latitude,
    /* [out] */ VARIANT __RPC_FAR *City,
    /* [out] */ VARIANT __RPC_FAR *Street,
    /* [out] */ VARIANT __RPC_FAR *House,
    /* [out] */ VARIANT __RPC_FAR *Zip,
    /* [retval][out] */ long __RPC_FAR *CommandResult);


void __RPC_STUB IDest_GetReverseGeoCoding_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IDest_AddItemToAddressBook_Proxy( 
    IDest __RPC_FAR * This,
    /* [in] */ double XDeg,
    /* [in] */ double YDeg,
    /* [in] */ BSTR ItemName);


void __RPC_STUB IDest_AddItemToAddressBook_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IDest_GetCoordinatesFromAddress_Proxy( 
    IDest __RPC_FAR * This,
    /* [in] */ BSTR City,
    /* [in] */ BSTR Street,
    /* [in] */ BSTR House,
    /* [in] */ BSTR Zip,
    /* [out] */ VARIANT __RPC_FAR *XDeg,
    /* [out] */ VARIANT __RPC_FAR *YDeg,
    /* [retval][out] */ long __RPC_FAR *CommandResult);


void __RPC_STUB IDest_GetCoordinatesFromAddress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IDest_OpenCOMPort_Proxy( 
    IDest __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *CommandResult);


void __RPC_STUB IDest_OpenCOMPort_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IDest_CloseCOMPort_Proxy( 
    IDest __RPC_FAR * This);


void __RPC_STUB IDest_CloseCOMPort_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IDest_ReturnToMainDestinatorWindow_Proxy( 
    IDest __RPC_FAR * This);


void __RPC_STUB IDest_ReturnToMainDestinatorWindow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IDest_DestroyDestinatorWindow_Proxy( 
    IDest __RPC_FAR * This);


void __RPC_STUB IDest_DestroyDestinatorWindow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IDest_CalculateRouteToStreetIntersection_Proxy( 
    IDest __RPC_FAR * This,
    /* [in] */ BSTR Description,
    /* [in] */ long CityID,
    /* [in] */ long Street1ID,
    /* [in] */ long Street2ID,
    /* [retval][out] */ long __RPC_FAR *CommandResult);


void __RPC_STUB IDest_CalculateRouteToStreetIntersection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IDest_GetCoordinatesOfStreetIntersection_Proxy( 
    IDest __RPC_FAR * This,
    /* [in] */ long CityID,
    /* [in] */ long Street1ID,
    /* [in] */ long Street2ID,
    /* [out] */ VARIANT __RPC_FAR *XDeg,
    /* [out] */ VARIANT __RPC_FAR *YDeg,
    /* [retval][out] */ long __RPC_FAR *CommandResult);


void __RPC_STUB IDest_GetCoordinatesOfStreetIntersection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IDest_GetManeuverInfo_Proxy( 
    IDest __RPC_FAR * This,
    /* [out] */ VARIANT __RPC_FAR *AmountOfManeuvers,
    /* [out] */ VARIANT __RPC_FAR *ManeuverIDArray,
    /* [out] */ VARIANT __RPC_FAR *StreetNameArray,
    /* [out] */ VARIANT __RPC_FAR *DistanceArray);


void __RPC_STUB IDest_GetManeuverInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IDest_GetPoiKeyInfo_Proxy( 
    IDest __RPC_FAR * This,
    /* [out] */ VARIANT __RPC_FAR *AmountOfItems,
    /* [out] */ VARIANT __RPC_FAR *PoiCodeArray,
    /* [out] */ VARIANT __RPC_FAR *PoiSubCodeArray,
    /* [out] */ VARIANT __RPC_FAR *PoiSubFoodArray,
    /* [out] */ VARIANT __RPC_FAR *CodeNameArray,
    /* [out] */ VARIANT __RPC_FAR *SubCodeNameArray,
    /* [out] */ VARIANT __RPC_FAR *SubFoodNameArray);


void __RPC_STUB IDest_GetPoiKeyInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IDest_GetPoiListForKey_Proxy( 
    IDest __RPC_FAR * This,
    /* [in] */ long Code,
    /* [in] */ long SubCode,
    /* [in] */ long SubFood,
    /* [in] */ double Longitude,
    /* [in] */ double Latitude,
    /* [in] */ long RadiusInMeters,
    /* [out] */ VARIANT __RPC_FAR *NumberOfPoi,
    /* [out] */ VARIANT __RPC_FAR *XPoi,
    /* [out] */ VARIANT __RPC_FAR *YPoi,
    /* [out] */ VARIANT __RPC_FAR *Name,
    /* [out] */ VARIANT __RPC_FAR *Address,
    /* [out] */ VARIANT __RPC_FAR *House,
    /* [out] */ VARIANT __RPC_FAR *StreetName,
    /* [out] */ VARIANT __RPC_FAR *CityName,
    /* [out] */ VARIANT __RPC_FAR *Phone);


void __RPC_STUB IDest_GetPoiListForKey_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IDest_GetViewState_Proxy( 
    IDest __RPC_FAR * This,
    /* [out] */ VARIANT __RPC_FAR *View3D2D);


void __RPC_STUB IDest_GetViewState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IDest_SetViewState_Proxy( 
    IDest __RPC_FAR * This,
    /* [in] */ long View3D2D);


void __RPC_STUB IDest_SetViewState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IDest_CancelCurrentRoute_Proxy( 
    IDest __RPC_FAR * This);


void __RPC_STUB IDest_CancelCurrentRoute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IDest_GetAddressBookList_Proxy( 
    IDest __RPC_FAR * This,
    /* [out] */ VARIANT __RPC_FAR *XDegArray,
    /* [out] */ VARIANT __RPC_FAR *YDegArray,
    /* [out] */ VARIANT __RPC_FAR *ItemNameArray,
    /* [out] */ VARIANT __RPC_FAR *AmountOfItems);


void __RPC_STUB IDest_GetAddressBookList_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IDest_GetMapImage_Proxy( 
    IDest __RPC_FAR * This,
    /* [in] */ double Longitude,
    /* [in] */ double Latitude,
    /* [in] */ long Width,
    /* [in] */ long Height,
    /* [out][in] */ VARIANT __RPC_FAR *Scale,
    /* [out] */ VARIANT __RPC_FAR *MapCos,
    /* [in] */ long ImageType,
    /* [out] */ VARIANT __RPC_FAR *pImage,
    /* [out] */ VARIANT __RPC_FAR *ImageSize,
    /* [retval][out] */ long __RPC_FAR *ErrorCode);


void __RPC_STUB IDest_GetMapImage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDest_INTERFACE_DEFINED__ */


#ifndef __IDest2_INTERFACE_DEFINED__
#define __IDest2_INTERFACE_DEFINED__

/* interface IDest2 */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IDest2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("98B75860-AD57-4b2f-A241-5B33365904CA")
    IDest2 : public IUnknown
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE CreateDestinatorWindow( 
            /* [in] */ long Flag,
            /* [retval][out] */ long __RPC_FAR *CommandResult) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE ShowDestinatorWindow( 
            /* [retval][out] */ long __RPC_FAR *CommandResult) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE HideDestinatorWindow( 
            /* [retval][out] */ long __RPC_FAR *CommandResult) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE ShowCurrentRoute( 
            /* [retval][out] */ long __RPC_FAR *CommandResult) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE CalculateRouteToCoordinates( 
            /* [in] */ double Longitude,
            /* [in] */ double Latitude,
            /* [in] */ BSTR Description,
            /* [retval][out] */ long __RPC_FAR *CommandResult) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE CalculateRouteToAddress( 
            /* [in] */ DESTAPIADDRESS __RPC_FAR *Address,
            /* [in] */ BSTR Description,
            /* [retval][out] */ long __RPC_FAR *CommandResult) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetGPSData( 
            /* [out] */ DESTAPIGPSDATA __RPC_FAR *pGPSData) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetDestinatorSettings( 
            /* [out] */ DESTAPIMAINSETTINGS __RPC_FAR *pDestinatorSettings) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetDestinatorSettings( 
            /* [in] */ DESTAPIMAINSETTINGS __RPC_FAR *pDestinatorSettings) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE ShowDesiredPosition( 
            /* [in] */ double Longitude,
            /* [in] */ double Latitude,
            /* [in] */ BSTR Description,
            /* [retval][out] */ long __RPC_FAR *CommandResult) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE CancelCurrentRoute( 
            /* [retval][out] */ long __RPC_FAR *CommandResult) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetViewState( 
            /* [out] */ long __RPC_FAR *View3D2D) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetViewState( 
            /* [in] */ long View3D2D) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE ReturnToMainDestinatorWindow( 
            /* [retval][out] */ long __RPC_FAR *CommandResult) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE DestroyDestinatorWindow( 
            /* [retval][out] */ long __RPC_FAR *CommandResult) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE OpenCOMPort( 
            /* [retval][out] */ long __RPC_FAR *CommandResult) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE CloseCOMPort( void) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetArrayOfCitiesID( 
            /* [out] */ DESTAPIIDARRAY __RPC_FAR *CitiesIDArray,
            /* [retval][out] */ long __RPC_FAR *CommandResult) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetArrayOfZipCodesID( 
            /* [out] */ DESTAPIIDARRAY __RPC_FAR *ZipCodesIDArray,
            /* [retval][out] */ long __RPC_FAR *CommandResult) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetArrayOfStreetsID( 
            /* [out] */ DESTAPIIDARRAY __RPC_FAR *StreetsIDArray,
            /* [retval][out] */ long __RPC_FAR *CommandResult) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetCityNameFromID( 
            /* [in] */ long CityID,
            /* [out] */ BSTR __RPC_FAR *CityName,
            /* [retval][out] */ long __RPC_FAR *CommandResult) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetZipCodeFromID( 
            /* [in] */ long ZipCodeID,
            /* [out] */ BSTR __RPC_FAR *ZipCode,
            /* [retval][out] */ long __RPC_FAR *CommandResult) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetStreetNameFromID( 
            /* [in] */ long StreetID,
            /* [out] */ BSTR __RPC_FAR *StreetName,
            /* [retval][out] */ long __RPC_FAR *CommandResult) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetArrayOfStreetsIDForCity( 
            /* [in] */ long CityID,
            /* [out] */ DESTAPIIDARRAY __RPC_FAR *StreetsIDArray,
            /* [retval][out] */ long __RPC_FAR *CommandResult) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetArrayOfStreetsIDForZip( 
            /* [in] */ long ZipID,
            /* [out] */ DESTAPIIDARRAY __RPC_FAR *StreetsIDArray,
            /* [retval][out] */ long __RPC_FAR *CommandResult) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetArrayOfCitiesIDForSelectedStreet( 
            /* [in] */ long StreetID,
            /* [out] */ DESTAPIIDARRAY __RPC_FAR *CitiesIDArray,
            /* [retval][out] */ long __RPC_FAR *CommandResult) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetArrayOfZipCodesIDForSelectedStreet( 
            /* [in] */ long StreetID,
            /* [out] */ DESTAPIIDARRAY __RPC_FAR *ZipCodesIDArray,
            /* [retval][out] */ long __RPC_FAR *CommandResult) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetMapsList( 
            /* [out] */ DESTAPIMAPLIST __RPC_FAR *MapsList) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetCurrentMapInfo( 
            /* [out] */ DESTAPIMAPINFO __RPC_FAR *MapInfo) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SwapMaps( 
            /* [in] */ BSTR MapName,
            /* [retval][out] */ long __RPC_FAR *CommandResult) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetCurrentMapState( 
            /* [out] */ DESTAPIMAPSTATE __RPC_FAR *MapState) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetCurrentCarPosition( 
            /* [out] */ DESTAPIPOINT __RPC_FAR *Position) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetRouteInfo( 
            /* [out] */ DESTAPIROUTEINFO __RPC_FAR *RouteInfo) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetReverseGeoCoding( 
            /* [in] */ DESTAPIPOINT __RPC_FAR *Position,
            /* [out] */ DESTAPIADDRESS __RPC_FAR *Address,
            /* [retval][out] */ long __RPC_FAR *CommandResult) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE AddItemToAddressBook( 
            /* [in] */ DESTAPIPOINT __RPC_FAR *Position,
            /* [in] */ BSTR ItemName) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetCoordinatesFromAddress( 
            /* [in] */ DESTAPIADDRESS __RPC_FAR *Address,
            /* [out] */ DESTAPIPOINT __RPC_FAR *Position,
            /* [retval][out] */ long __RPC_FAR *CommandResult) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE CalculateRouteToStreetIntersection( 
            /* [in] */ BSTR Description,
            /* [in] */ long CityID,
            /* [in] */ long Street1ID,
            /* [in] */ long Street2ID,
            /* [retval][out] */ long __RPC_FAR *CommandResult) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetCoordinatesOfStreetIntersection( 
            /* [in] */ long CityID,
            /* [in] */ long Street1ID,
            /* [in] */ long Street2ID,
            /* [out] */ DESTAPIPOINT __RPC_FAR *Position,
            /* [retval][out] */ long __RPC_FAR *CommandResult) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetManeuverInfo( 
            /* [out] */ DESTAPIMANEUVERINFO __RPC_FAR *ManeuverInfo) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetAddressBookList( 
            /* [out] */ DESTAPIADDRESSBOOKDATA __RPC_FAR *AddressBookData) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetPoiKeyInfo( 
            /* [out] */ DESTAPIPOIKEYINFO __RPC_FAR *PoiKeyInfo) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetPoiListForKey( 
            /* [in] */ DESTAPIPOILISTREQUEST __RPC_FAR *Request,
            /* [out] */ DESTAPIPOILIST __RPC_FAR *POIList) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetMapImage( 
            /* [in] */ DESTAPIIMAGEREQUEST __RPC_FAR *Request,
            /* [out] */ DESTAPIIMAGEDATA __RPC_FAR *ImageData,
            /* [retval][out] */ long __RPC_FAR *ErrorCode) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDest2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IDest2 __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IDest2 __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IDest2 __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CreateDestinatorWindow )( 
            IDest2 __RPC_FAR * This,
            /* [in] */ long Flag,
            /* [retval][out] */ long __RPC_FAR *CommandResult);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ShowDestinatorWindow )( 
            IDest2 __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *CommandResult);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *HideDestinatorWindow )( 
            IDest2 __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *CommandResult);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ShowCurrentRoute )( 
            IDest2 __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *CommandResult);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CalculateRouteToCoordinates )( 
            IDest2 __RPC_FAR * This,
            /* [in] */ double Longitude,
            /* [in] */ double Latitude,
            /* [in] */ BSTR Description,
            /* [retval][out] */ long __RPC_FAR *CommandResult);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CalculateRouteToAddress )( 
            IDest2 __RPC_FAR * This,
            /* [in] */ DESTAPIADDRESS __RPC_FAR *Address,
            /* [in] */ BSTR Description,
            /* [retval][out] */ long __RPC_FAR *CommandResult);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetGPSData )( 
            IDest2 __RPC_FAR * This,
            /* [out] */ DESTAPIGPSDATA __RPC_FAR *pGPSData);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDestinatorSettings )( 
            IDest2 __RPC_FAR * This,
            /* [out] */ DESTAPIMAINSETTINGS __RPC_FAR *pDestinatorSettings);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetDestinatorSettings )( 
            IDest2 __RPC_FAR * This,
            /* [in] */ DESTAPIMAINSETTINGS __RPC_FAR *pDestinatorSettings);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ShowDesiredPosition )( 
            IDest2 __RPC_FAR * This,
            /* [in] */ double Longitude,
            /* [in] */ double Latitude,
            /* [in] */ BSTR Description,
            /* [retval][out] */ long __RPC_FAR *CommandResult);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CancelCurrentRoute )( 
            IDest2 __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *CommandResult);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetViewState )( 
            IDest2 __RPC_FAR * This,
            /* [out] */ long __RPC_FAR *View3D2D);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetViewState )( 
            IDest2 __RPC_FAR * This,
            /* [in] */ long View3D2D);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ReturnToMainDestinatorWindow )( 
            IDest2 __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *CommandResult);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DestroyDestinatorWindow )( 
            IDest2 __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *CommandResult);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OpenCOMPort )( 
            IDest2 __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *CommandResult);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CloseCOMPort )( 
            IDest2 __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetArrayOfCitiesID )( 
            IDest2 __RPC_FAR * This,
            /* [out] */ DESTAPIIDARRAY __RPC_FAR *CitiesIDArray,
            /* [retval][out] */ long __RPC_FAR *CommandResult);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetArrayOfZipCodesID )( 
            IDest2 __RPC_FAR * This,
            /* [out] */ DESTAPIIDARRAY __RPC_FAR *ZipCodesIDArray,
            /* [retval][out] */ long __RPC_FAR *CommandResult);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetArrayOfStreetsID )( 
            IDest2 __RPC_FAR * This,
            /* [out] */ DESTAPIIDARRAY __RPC_FAR *StreetsIDArray,
            /* [retval][out] */ long __RPC_FAR *CommandResult);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCityNameFromID )( 
            IDest2 __RPC_FAR * This,
            /* [in] */ long CityID,
            /* [out] */ BSTR __RPC_FAR *CityName,
            /* [retval][out] */ long __RPC_FAR *CommandResult);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetZipCodeFromID )( 
            IDest2 __RPC_FAR * This,
            /* [in] */ long ZipCodeID,
            /* [out] */ BSTR __RPC_FAR *ZipCode,
            /* [retval][out] */ long __RPC_FAR *CommandResult);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetStreetNameFromID )( 
            IDest2 __RPC_FAR * This,
            /* [in] */ long StreetID,
            /* [out] */ BSTR __RPC_FAR *StreetName,
            /* [retval][out] */ long __RPC_FAR *CommandResult);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetArrayOfStreetsIDForCity )( 
            IDest2 __RPC_FAR * This,
            /* [in] */ long CityID,
            /* [out] */ DESTAPIIDARRAY __RPC_FAR *StreetsIDArray,
            /* [retval][out] */ long __RPC_FAR *CommandResult);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetArrayOfStreetsIDForZip )( 
            IDest2 __RPC_FAR * This,
            /* [in] */ long ZipID,
            /* [out] */ DESTAPIIDARRAY __RPC_FAR *StreetsIDArray,
            /* [retval][out] */ long __RPC_FAR *CommandResult);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetArrayOfCitiesIDForSelectedStreet )( 
            IDest2 __RPC_FAR * This,
            /* [in] */ long StreetID,
            /* [out] */ DESTAPIIDARRAY __RPC_FAR *CitiesIDArray,
            /* [retval][out] */ long __RPC_FAR *CommandResult);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetArrayOfZipCodesIDForSelectedStreet )( 
            IDest2 __RPC_FAR * This,
            /* [in] */ long StreetID,
            /* [out] */ DESTAPIIDARRAY __RPC_FAR *ZipCodesIDArray,
            /* [retval][out] */ long __RPC_FAR *CommandResult);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetMapsList )( 
            IDest2 __RPC_FAR * This,
            /* [out] */ DESTAPIMAPLIST __RPC_FAR *MapsList);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCurrentMapInfo )( 
            IDest2 __RPC_FAR * This,
            /* [out] */ DESTAPIMAPINFO __RPC_FAR *MapInfo);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SwapMaps )( 
            IDest2 __RPC_FAR * This,
            /* [in] */ BSTR MapName,
            /* [retval][out] */ long __RPC_FAR *CommandResult);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCurrentMapState )( 
            IDest2 __RPC_FAR * This,
            /* [out] */ DESTAPIMAPSTATE __RPC_FAR *MapState);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCurrentCarPosition )( 
            IDest2 __RPC_FAR * This,
            /* [out] */ DESTAPIPOINT __RPC_FAR *Position);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetRouteInfo )( 
            IDest2 __RPC_FAR * This,
            /* [out] */ DESTAPIROUTEINFO __RPC_FAR *RouteInfo);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetReverseGeoCoding )( 
            IDest2 __RPC_FAR * This,
            /* [in] */ DESTAPIPOINT __RPC_FAR *Position,
            /* [out] */ DESTAPIADDRESS __RPC_FAR *Address,
            /* [retval][out] */ long __RPC_FAR *CommandResult);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AddItemToAddressBook )( 
            IDest2 __RPC_FAR * This,
            /* [in] */ DESTAPIPOINT __RPC_FAR *Position,
            /* [in] */ BSTR ItemName);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCoordinatesFromAddress )( 
            IDest2 __RPC_FAR * This,
            /* [in] */ DESTAPIADDRESS __RPC_FAR *Address,
            /* [out] */ DESTAPIPOINT __RPC_FAR *Position,
            /* [retval][out] */ long __RPC_FAR *CommandResult);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CalculateRouteToStreetIntersection )( 
            IDest2 __RPC_FAR * This,
            /* [in] */ BSTR Description,
            /* [in] */ long CityID,
            /* [in] */ long Street1ID,
            /* [in] */ long Street2ID,
            /* [retval][out] */ long __RPC_FAR *CommandResult);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCoordinatesOfStreetIntersection )( 
            IDest2 __RPC_FAR * This,
            /* [in] */ long CityID,
            /* [in] */ long Street1ID,
            /* [in] */ long Street2ID,
            /* [out] */ DESTAPIPOINT __RPC_FAR *Position,
            /* [retval][out] */ long __RPC_FAR *CommandResult);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetManeuverInfo )( 
            IDest2 __RPC_FAR * This,
            /* [out] */ DESTAPIMANEUVERINFO __RPC_FAR *ManeuverInfo);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetAddressBookList )( 
            IDest2 __RPC_FAR * This,
            /* [out] */ DESTAPIADDRESSBOOKDATA __RPC_FAR *AddressBookData);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetPoiKeyInfo )( 
            IDest2 __RPC_FAR * This,
            /* [out] */ DESTAPIPOIKEYINFO __RPC_FAR *PoiKeyInfo);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetPoiListForKey )( 
            IDest2 __RPC_FAR * This,
            /* [in] */ DESTAPIPOILISTREQUEST __RPC_FAR *Request,
            /* [out] */ DESTAPIPOILIST __RPC_FAR *POIList);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetMapImage )( 
            IDest2 __RPC_FAR * This,
            /* [in] */ DESTAPIIMAGEREQUEST __RPC_FAR *Request,
            /* [out] */ DESTAPIIMAGEDATA __RPC_FAR *ImageData,
            /* [retval][out] */ long __RPC_FAR *ErrorCode);
        
        END_INTERFACE
    } IDest2Vtbl;

    interface IDest2
    {
        CONST_VTBL struct IDest2Vtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDest2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDest2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDest2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDest2_CreateDestinatorWindow(This,Flag,CommandResult)	\
    (This)->lpVtbl -> CreateDestinatorWindow(This,Flag,CommandResult)

#define IDest2_ShowDestinatorWindow(This,CommandResult)	\
    (This)->lpVtbl -> ShowDestinatorWindow(This,CommandResult)

#define IDest2_HideDestinatorWindow(This,CommandResult)	\
    (This)->lpVtbl -> HideDestinatorWindow(This,CommandResult)

#define IDest2_ShowCurrentRoute(This,CommandResult)	\
    (This)->lpVtbl -> ShowCurrentRoute(This,CommandResult)

#define IDest2_CalculateRouteToCoordinates(This,Longitude,Latitude,Description,CommandResult)	\
    (This)->lpVtbl -> CalculateRouteToCoordinates(This,Longitude,Latitude,Description,CommandResult)

#define IDest2_CalculateRouteToAddress(This,Address,Description,CommandResult)	\
    (This)->lpVtbl -> CalculateRouteToAddress(This,Address,Description,CommandResult)

#define IDest2_GetGPSData(This,pGPSData)	\
    (This)->lpVtbl -> GetGPSData(This,pGPSData)

#define IDest2_GetDestinatorSettings(This,pDestinatorSettings)	\
    (This)->lpVtbl -> GetDestinatorSettings(This,pDestinatorSettings)

#define IDest2_SetDestinatorSettings(This,pDestinatorSettings)	\
    (This)->lpVtbl -> SetDestinatorSettings(This,pDestinatorSettings)

#define IDest2_ShowDesiredPosition(This,Longitude,Latitude,Description,CommandResult)	\
    (This)->lpVtbl -> ShowDesiredPosition(This,Longitude,Latitude,Description,CommandResult)

#define IDest2_CancelCurrentRoute(This,CommandResult)	\
    (This)->lpVtbl -> CancelCurrentRoute(This,CommandResult)

#define IDest2_GetViewState(This,View3D2D)	\
    (This)->lpVtbl -> GetViewState(This,View3D2D)

#define IDest2_SetViewState(This,View3D2D)	\
    (This)->lpVtbl -> SetViewState(This,View3D2D)

#define IDest2_ReturnToMainDestinatorWindow(This,CommandResult)	\
    (This)->lpVtbl -> ReturnToMainDestinatorWindow(This,CommandResult)

#define IDest2_DestroyDestinatorWindow(This,CommandResult)	\
    (This)->lpVtbl -> DestroyDestinatorWindow(This,CommandResult)

#define IDest2_OpenCOMPort(This,CommandResult)	\
    (This)->lpVtbl -> OpenCOMPort(This,CommandResult)

#define IDest2_CloseCOMPort(This)	\
    (This)->lpVtbl -> CloseCOMPort(This)

#define IDest2_GetArrayOfCitiesID(This,CitiesIDArray,CommandResult)	\
    (This)->lpVtbl -> GetArrayOfCitiesID(This,CitiesIDArray,CommandResult)

#define IDest2_GetArrayOfZipCodesID(This,ZipCodesIDArray,CommandResult)	\
    (This)->lpVtbl -> GetArrayOfZipCodesID(This,ZipCodesIDArray,CommandResult)

#define IDest2_GetArrayOfStreetsID(This,StreetsIDArray,CommandResult)	\
    (This)->lpVtbl -> GetArrayOfStreetsID(This,StreetsIDArray,CommandResult)

#define IDest2_GetCityNameFromID(This,CityID,CityName,CommandResult)	\
    (This)->lpVtbl -> GetCityNameFromID(This,CityID,CityName,CommandResult)

#define IDest2_GetZipCodeFromID(This,ZipCodeID,ZipCode,CommandResult)	\
    (This)->lpVtbl -> GetZipCodeFromID(This,ZipCodeID,ZipCode,CommandResult)

#define IDest2_GetStreetNameFromID(This,StreetID,StreetName,CommandResult)	\
    (This)->lpVtbl -> GetStreetNameFromID(This,StreetID,StreetName,CommandResult)

#define IDest2_GetArrayOfStreetsIDForCity(This,CityID,StreetsIDArray,CommandResult)	\
    (This)->lpVtbl -> GetArrayOfStreetsIDForCity(This,CityID,StreetsIDArray,CommandResult)

#define IDest2_GetArrayOfStreetsIDForZip(This,ZipID,StreetsIDArray,CommandResult)	\
    (This)->lpVtbl -> GetArrayOfStreetsIDForZip(This,ZipID,StreetsIDArray,CommandResult)

#define IDest2_GetArrayOfCitiesIDForSelectedStreet(This,StreetID,CitiesIDArray,CommandResult)	\
    (This)->lpVtbl -> GetArrayOfCitiesIDForSelectedStreet(This,StreetID,CitiesIDArray,CommandResult)

#define IDest2_GetArrayOfZipCodesIDForSelectedStreet(This,StreetID,ZipCodesIDArray,CommandResult)	\
    (This)->lpVtbl -> GetArrayOfZipCodesIDForSelectedStreet(This,StreetID,ZipCodesIDArray,CommandResult)

#define IDest2_GetMapsList(This,MapsList)	\
    (This)->lpVtbl -> GetMapsList(This,MapsList)

#define IDest2_GetCurrentMapInfo(This,MapInfo)	\
    (This)->lpVtbl -> GetCurrentMapInfo(This,MapInfo)

#define IDest2_SwapMaps(This,MapName,CommandResult)	\
    (This)->lpVtbl -> SwapMaps(This,MapName,CommandResult)

#define IDest2_GetCurrentMapState(This,MapState)	\
    (This)->lpVtbl -> GetCurrentMapState(This,MapState)

#define IDest2_GetCurrentCarPosition(This,Position)	\
    (This)->lpVtbl -> GetCurrentCarPosition(This,Position)

#define IDest2_GetRouteInfo(This,RouteInfo)	\
    (This)->lpVtbl -> GetRouteInfo(This,RouteInfo)

#define IDest2_GetReverseGeoCoding(This,Position,Address,CommandResult)	\
    (This)->lpVtbl -> GetReverseGeoCoding(This,Position,Address,CommandResult)

#define IDest2_AddItemToAddressBook(This,Position,ItemName)	\
    (This)->lpVtbl -> AddItemToAddressBook(This,Position,ItemName)

#define IDest2_GetCoordinatesFromAddress(This,Address,Position,CommandResult)	\
    (This)->lpVtbl -> GetCoordinatesFromAddress(This,Address,Position,CommandResult)

#define IDest2_CalculateRouteToStreetIntersection(This,Description,CityID,Street1ID,Street2ID,CommandResult)	\
    (This)->lpVtbl -> CalculateRouteToStreetIntersection(This,Description,CityID,Street1ID,Street2ID,CommandResult)

#define IDest2_GetCoordinatesOfStreetIntersection(This,CityID,Street1ID,Street2ID,Position,CommandResult)	\
    (This)->lpVtbl -> GetCoordinatesOfStreetIntersection(This,CityID,Street1ID,Street2ID,Position,CommandResult)

#define IDest2_GetManeuverInfo(This,ManeuverInfo)	\
    (This)->lpVtbl -> GetManeuverInfo(This,ManeuverInfo)

#define IDest2_GetAddressBookList(This,AddressBookData)	\
    (This)->lpVtbl -> GetAddressBookList(This,AddressBookData)

#define IDest2_GetPoiKeyInfo(This,PoiKeyInfo)	\
    (This)->lpVtbl -> GetPoiKeyInfo(This,PoiKeyInfo)

#define IDest2_GetPoiListForKey(This,Request,POIList)	\
    (This)->lpVtbl -> GetPoiListForKey(This,Request,POIList)

#define IDest2_GetMapImage(This,Request,ImageData,ErrorCode)	\
    (This)->lpVtbl -> GetMapImage(This,Request,ImageData,ErrorCode)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring] */ HRESULT STDMETHODCALLTYPE IDest2_CreateDestinatorWindow_Proxy( 
    IDest2 __RPC_FAR * This,
    /* [in] */ long Flag,
    /* [retval][out] */ long __RPC_FAR *CommandResult);


void __RPC_STUB IDest2_CreateDestinatorWindow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IDest2_ShowDestinatorWindow_Proxy( 
    IDest2 __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *CommandResult);


void __RPC_STUB IDest2_ShowDestinatorWindow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IDest2_HideDestinatorWindow_Proxy( 
    IDest2 __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *CommandResult);


void __RPC_STUB IDest2_HideDestinatorWindow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IDest2_ShowCurrentRoute_Proxy( 
    IDest2 __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *CommandResult);


void __RPC_STUB IDest2_ShowCurrentRoute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IDest2_CalculateRouteToCoordinates_Proxy( 
    IDest2 __RPC_FAR * This,
    /* [in] */ double Longitude,
    /* [in] */ double Latitude,
    /* [in] */ BSTR Description,
    /* [retval][out] */ long __RPC_FAR *CommandResult);


void __RPC_STUB IDest2_CalculateRouteToCoordinates_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IDest2_CalculateRouteToAddress_Proxy( 
    IDest2 __RPC_FAR * This,
    /* [in] */ DESTAPIADDRESS __RPC_FAR *Address,
    /* [in] */ BSTR Description,
    /* [retval][out] */ long __RPC_FAR *CommandResult);


void __RPC_STUB IDest2_CalculateRouteToAddress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IDest2_GetGPSData_Proxy( 
    IDest2 __RPC_FAR * This,
    /* [out] */ DESTAPIGPSDATA __RPC_FAR *pGPSData);


void __RPC_STUB IDest2_GetGPSData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IDest2_GetDestinatorSettings_Proxy( 
    IDest2 __RPC_FAR * This,
    /* [out] */ DESTAPIMAINSETTINGS __RPC_FAR *pDestinatorSettings);


void __RPC_STUB IDest2_GetDestinatorSettings_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IDest2_SetDestinatorSettings_Proxy( 
    IDest2 __RPC_FAR * This,
    /* [in] */ DESTAPIMAINSETTINGS __RPC_FAR *pDestinatorSettings);


void __RPC_STUB IDest2_SetDestinatorSettings_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IDest2_ShowDesiredPosition_Proxy( 
    IDest2 __RPC_FAR * This,
    /* [in] */ double Longitude,
    /* [in] */ double Latitude,
    /* [in] */ BSTR Description,
    /* [retval][out] */ long __RPC_FAR *CommandResult);


void __RPC_STUB IDest2_ShowDesiredPosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IDest2_CancelCurrentRoute_Proxy( 
    IDest2 __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *CommandResult);


void __RPC_STUB IDest2_CancelCurrentRoute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IDest2_GetViewState_Proxy( 
    IDest2 __RPC_FAR * This,
    /* [out] */ long __RPC_FAR *View3D2D);


void __RPC_STUB IDest2_GetViewState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IDest2_SetViewState_Proxy( 
    IDest2 __RPC_FAR * This,
    /* [in] */ long View3D2D);


void __RPC_STUB IDest2_SetViewState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IDest2_ReturnToMainDestinatorWindow_Proxy( 
    IDest2 __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *CommandResult);


void __RPC_STUB IDest2_ReturnToMainDestinatorWindow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IDest2_DestroyDestinatorWindow_Proxy( 
    IDest2 __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *CommandResult);


void __RPC_STUB IDest2_DestroyDestinatorWindow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IDest2_OpenCOMPort_Proxy( 
    IDest2 __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *CommandResult);


void __RPC_STUB IDest2_OpenCOMPort_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IDest2_CloseCOMPort_Proxy( 
    IDest2 __RPC_FAR * This);


void __RPC_STUB IDest2_CloseCOMPort_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IDest2_GetArrayOfCitiesID_Proxy( 
    IDest2 __RPC_FAR * This,
    /* [out] */ DESTAPIIDARRAY __RPC_FAR *CitiesIDArray,
    /* [retval][out] */ long __RPC_FAR *CommandResult);


void __RPC_STUB IDest2_GetArrayOfCitiesID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IDest2_GetArrayOfZipCodesID_Proxy( 
    IDest2 __RPC_FAR * This,
    /* [out] */ DESTAPIIDARRAY __RPC_FAR *ZipCodesIDArray,
    /* [retval][out] */ long __RPC_FAR *CommandResult);


void __RPC_STUB IDest2_GetArrayOfZipCodesID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IDest2_GetArrayOfStreetsID_Proxy( 
    IDest2 __RPC_FAR * This,
    /* [out] */ DESTAPIIDARRAY __RPC_FAR *StreetsIDArray,
    /* [retval][out] */ long __RPC_FAR *CommandResult);


void __RPC_STUB IDest2_GetArrayOfStreetsID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IDest2_GetCityNameFromID_Proxy( 
    IDest2 __RPC_FAR * This,
    /* [in] */ long CityID,
    /* [out] */ BSTR __RPC_FAR *CityName,
    /* [retval][out] */ long __RPC_FAR *CommandResult);


void __RPC_STUB IDest2_GetCityNameFromID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IDest2_GetZipCodeFromID_Proxy( 
    IDest2 __RPC_FAR * This,
    /* [in] */ long ZipCodeID,
    /* [out] */ BSTR __RPC_FAR *ZipCode,
    /* [retval][out] */ long __RPC_FAR *CommandResult);


void __RPC_STUB IDest2_GetZipCodeFromID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IDest2_GetStreetNameFromID_Proxy( 
    IDest2 __RPC_FAR * This,
    /* [in] */ long StreetID,
    /* [out] */ BSTR __RPC_FAR *StreetName,
    /* [retval][out] */ long __RPC_FAR *CommandResult);


void __RPC_STUB IDest2_GetStreetNameFromID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IDest2_GetArrayOfStreetsIDForCity_Proxy( 
    IDest2 __RPC_FAR * This,
    /* [in] */ long CityID,
    /* [out] */ DESTAPIIDARRAY __RPC_FAR *StreetsIDArray,
    /* [retval][out] */ long __RPC_FAR *CommandResult);


void __RPC_STUB IDest2_GetArrayOfStreetsIDForCity_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IDest2_GetArrayOfStreetsIDForZip_Proxy( 
    IDest2 __RPC_FAR * This,
    /* [in] */ long ZipID,
    /* [out] */ DESTAPIIDARRAY __RPC_FAR *StreetsIDArray,
    /* [retval][out] */ long __RPC_FAR *CommandResult);


void __RPC_STUB IDest2_GetArrayOfStreetsIDForZip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IDest2_GetArrayOfCitiesIDForSelectedStreet_Proxy( 
    IDest2 __RPC_FAR * This,
    /* [in] */ long StreetID,
    /* [out] */ DESTAPIIDARRAY __RPC_FAR *CitiesIDArray,
    /* [retval][out] */ long __RPC_FAR *CommandResult);


void __RPC_STUB IDest2_GetArrayOfCitiesIDForSelectedStreet_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IDest2_GetArrayOfZipCodesIDForSelectedStreet_Proxy( 
    IDest2 __RPC_FAR * This,
    /* [in] */ long StreetID,
    /* [out] */ DESTAPIIDARRAY __RPC_FAR *ZipCodesIDArray,
    /* [retval][out] */ long __RPC_FAR *CommandResult);


void __RPC_STUB IDest2_GetArrayOfZipCodesIDForSelectedStreet_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IDest2_GetMapsList_Proxy( 
    IDest2 __RPC_FAR * This,
    /* [out] */ DESTAPIMAPLIST __RPC_FAR *MapsList);


void __RPC_STUB IDest2_GetMapsList_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IDest2_GetCurrentMapInfo_Proxy( 
    IDest2 __RPC_FAR * This,
    /* [out] */ DESTAPIMAPINFO __RPC_FAR *MapInfo);


void __RPC_STUB IDest2_GetCurrentMapInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IDest2_SwapMaps_Proxy( 
    IDest2 __RPC_FAR * This,
    /* [in] */ BSTR MapName,
    /* [retval][out] */ long __RPC_FAR *CommandResult);


void __RPC_STUB IDest2_SwapMaps_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IDest2_GetCurrentMapState_Proxy( 
    IDest2 __RPC_FAR * This,
    /* [out] */ DESTAPIMAPSTATE __RPC_FAR *MapState);


void __RPC_STUB IDest2_GetCurrentMapState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IDest2_GetCurrentCarPosition_Proxy( 
    IDest2 __RPC_FAR * This,
    /* [out] */ DESTAPIPOINT __RPC_FAR *Position);


void __RPC_STUB IDest2_GetCurrentCarPosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IDest2_GetRouteInfo_Proxy( 
    IDest2 __RPC_FAR * This,
    /* [out] */ DESTAPIROUTEINFO __RPC_FAR *RouteInfo);


void __RPC_STUB IDest2_GetRouteInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IDest2_GetReverseGeoCoding_Proxy( 
    IDest2 __RPC_FAR * This,
    /* [in] */ DESTAPIPOINT __RPC_FAR *Position,
    /* [out] */ DESTAPIADDRESS __RPC_FAR *Address,
    /* [retval][out] */ long __RPC_FAR *CommandResult);


void __RPC_STUB IDest2_GetReverseGeoCoding_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IDest2_AddItemToAddressBook_Proxy( 
    IDest2 __RPC_FAR * This,
    /* [in] */ DESTAPIPOINT __RPC_FAR *Position,
    /* [in] */ BSTR ItemName);


void __RPC_STUB IDest2_AddItemToAddressBook_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IDest2_GetCoordinatesFromAddress_Proxy( 
    IDest2 __RPC_FAR * This,
    /* [in] */ DESTAPIADDRESS __RPC_FAR *Address,
    /* [out] */ DESTAPIPOINT __RPC_FAR *Position,
    /* [retval][out] */ long __RPC_FAR *CommandResult);


void __RPC_STUB IDest2_GetCoordinatesFromAddress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IDest2_CalculateRouteToStreetIntersection_Proxy( 
    IDest2 __RPC_FAR * This,
    /* [in] */ BSTR Description,
    /* [in] */ long CityID,
    /* [in] */ long Street1ID,
    /* [in] */ long Street2ID,
    /* [retval][out] */ long __RPC_FAR *CommandResult);


void __RPC_STUB IDest2_CalculateRouteToStreetIntersection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IDest2_GetCoordinatesOfStreetIntersection_Proxy( 
    IDest2 __RPC_FAR * This,
    /* [in] */ long CityID,
    /* [in] */ long Street1ID,
    /* [in] */ long Street2ID,
    /* [out] */ DESTAPIPOINT __RPC_FAR *Position,
    /* [retval][out] */ long __RPC_FAR *CommandResult);


void __RPC_STUB IDest2_GetCoordinatesOfStreetIntersection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IDest2_GetManeuverInfo_Proxy( 
    IDest2 __RPC_FAR * This,
    /* [out] */ DESTAPIMANEUVERINFO __RPC_FAR *ManeuverInfo);


void __RPC_STUB IDest2_GetManeuverInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IDest2_GetAddressBookList_Proxy( 
    IDest2 __RPC_FAR * This,
    /* [out] */ DESTAPIADDRESSBOOKDATA __RPC_FAR *AddressBookData);


void __RPC_STUB IDest2_GetAddressBookList_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IDest2_GetPoiKeyInfo_Proxy( 
    IDest2 __RPC_FAR * This,
    /* [out] */ DESTAPIPOIKEYINFO __RPC_FAR *PoiKeyInfo);


void __RPC_STUB IDest2_GetPoiKeyInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IDest2_GetPoiListForKey_Proxy( 
    IDest2 __RPC_FAR * This,
    /* [in] */ DESTAPIPOILISTREQUEST __RPC_FAR *Request,
    /* [out] */ DESTAPIPOILIST __RPC_FAR *POIList);


void __RPC_STUB IDest2_GetPoiListForKey_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IDest2_GetMapImage_Proxy( 
    IDest2 __RPC_FAR * This,
    /* [in] */ DESTAPIIMAGEREQUEST __RPC_FAR *Request,
    /* [out] */ DESTAPIIMAGEDATA __RPC_FAR *ImageData,
    /* [retval][out] */ long __RPC_FAR *ErrorCode);


void __RPC_STUB IDest2_GetMapImage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDest2_INTERFACE_DEFINED__ */



#ifndef __DESTDLLLib_LIBRARY_DEFINED__
#define __DESTDLLLib_LIBRARY_DEFINED__

/* library DESTDLLLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_DESTDLLLib;

#ifndef ___IDestEvents_DISPINTERFACE_DEFINED__
#define ___IDestEvents_DISPINTERFACE_DEFINED__

/* dispinterface _IDestEvents */
/* [helpstring][uuid] */ 


EXTERN_C const IID DIID__IDestEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("9BD46DC5-658B-4102-8B5F-52E0424DAFB1")
    _IDestEvents : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct _IDestEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            _IDestEvents __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            _IDestEvents __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            _IDestEvents __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            _IDestEvents __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            _IDestEvents __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            _IDestEvents __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            _IDestEvents __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        END_INTERFACE
    } _IDestEventsVtbl;

    interface _IDestEvents
    {
        CONST_VTBL struct _IDestEventsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define _IDestEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define _IDestEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define _IDestEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define _IDestEvents_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define _IDestEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define _IDestEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define _IDestEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* ___IDestEvents_DISPINTERFACE_DEFINED__ */


#ifndef ___IDestEvents2_INTERFACE_DEFINED__
#define ___IDestEvents2_INTERFACE_DEFINED__

/* interface _IDestEvents2 */
/* [object][helpstring][uuid] */ 


EXTERN_C const IID IID__IDestEvents2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("A64F2C06-2E8C-4229-8521-A64ADCBD4EC2")
    _IDestEvents2 : public IUnknown
    {
    public:
        virtual /* [helpstring] */ void STDMETHODCALLTYPE OnGetDataFromCOMPort( 
            /* [in] */ long NumberOfBytes,
            /* [in] */ const BYTE __RPC_FAR *ByteArray) = 0;
        
        virtual /* [helpstring] */ void STDMETHODCALLTYPE OnCloseMainDestinatorWindow( void) = 0;
        
        virtual /* [helpstring] */ void STDMETHODCALLTYPE OnDestinatorError( 
            /* [in] */ long ErrorNotification) = 0;
        
        virtual /* [helpstring] */ void STDMETHODCALLTYPE OnYouHaveArrived( void) = 0;
        
        virtual /* [helpstring] */ void STDMETHODCALLTYPE OnCreateDestinatorWindow( 
            /* [in] */ HWND hWnd) = 0;
        
        virtual /* [helpstring] */ void STDMETHODCALLTYPE OnSwapMap( 
            /* [in] */ BSTR MapName) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct _IDestEvents2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            _IDestEvents2 __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            _IDestEvents2 __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            _IDestEvents2 __RPC_FAR * This);
        
        /* [helpstring] */ void ( STDMETHODCALLTYPE __RPC_FAR *OnGetDataFromCOMPort )( 
            _IDestEvents2 __RPC_FAR * This,
            /* [in] */ long NumberOfBytes,
            /* [in] */ const BYTE __RPC_FAR *ByteArray);
        
        /* [helpstring] */ void ( STDMETHODCALLTYPE __RPC_FAR *OnCloseMainDestinatorWindow )( 
            _IDestEvents2 __RPC_FAR * This);
        
        /* [helpstring] */ void ( STDMETHODCALLTYPE __RPC_FAR *OnDestinatorError )( 
            _IDestEvents2 __RPC_FAR * This,
            /* [in] */ long ErrorNotification);
        
        /* [helpstring] */ void ( STDMETHODCALLTYPE __RPC_FAR *OnYouHaveArrived )( 
            _IDestEvents2 __RPC_FAR * This);
        
        /* [helpstring] */ void ( STDMETHODCALLTYPE __RPC_FAR *OnCreateDestinatorWindow )( 
            _IDestEvents2 __RPC_FAR * This,
            /* [in] */ HWND hWnd);
        
        /* [helpstring] */ void ( STDMETHODCALLTYPE __RPC_FAR *OnSwapMap )( 
            _IDestEvents2 __RPC_FAR * This,
            /* [in] */ BSTR MapName);
        
        END_INTERFACE
    } _IDestEvents2Vtbl;

    interface _IDestEvents2
    {
        CONST_VTBL struct _IDestEvents2Vtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define _IDestEvents2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define _IDestEvents2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define _IDestEvents2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define _IDestEvents2_OnGetDataFromCOMPort(This,NumberOfBytes,ByteArray)	\
    (This)->lpVtbl -> OnGetDataFromCOMPort(This,NumberOfBytes,ByteArray)

#define _IDestEvents2_OnCloseMainDestinatorWindow(This)	\
    (This)->lpVtbl -> OnCloseMainDestinatorWindow(This)

#define _IDestEvents2_OnDestinatorError(This,ErrorNotification)	\
    (This)->lpVtbl -> OnDestinatorError(This,ErrorNotification)

#define _IDestEvents2_OnYouHaveArrived(This)	\
    (This)->lpVtbl -> OnYouHaveArrived(This)

#define _IDestEvents2_OnCreateDestinatorWindow(This,hWnd)	\
    (This)->lpVtbl -> OnCreateDestinatorWindow(This,hWnd)

#define _IDestEvents2_OnSwapMap(This,MapName)	\
    (This)->lpVtbl -> OnSwapMap(This,MapName)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring] */ void STDMETHODCALLTYPE _IDestEvents2_OnGetDataFromCOMPort_Proxy( 
    _IDestEvents2 __RPC_FAR * This,
    /* [in] */ long NumberOfBytes,
    /* [in] */ const BYTE __RPC_FAR *ByteArray);


void __RPC_STUB _IDestEvents2_OnGetDataFromCOMPort_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ void STDMETHODCALLTYPE _IDestEvents2_OnCloseMainDestinatorWindow_Proxy( 
    _IDestEvents2 __RPC_FAR * This);


void __RPC_STUB _IDestEvents2_OnCloseMainDestinatorWindow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ void STDMETHODCALLTYPE _IDestEvents2_OnDestinatorError_Proxy( 
    _IDestEvents2 __RPC_FAR * This,
    /* [in] */ long ErrorNotification);


void __RPC_STUB _IDestEvents2_OnDestinatorError_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ void STDMETHODCALLTYPE _IDestEvents2_OnYouHaveArrived_Proxy( 
    _IDestEvents2 __RPC_FAR * This);


void __RPC_STUB _IDestEvents2_OnYouHaveArrived_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ void STDMETHODCALLTYPE _IDestEvents2_OnCreateDestinatorWindow_Proxy( 
    _IDestEvents2 __RPC_FAR * This,
    /* [in] */ HWND hWnd);


void __RPC_STUB _IDestEvents2_OnCreateDestinatorWindow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ void STDMETHODCALLTYPE _IDestEvents2_OnSwapMap_Proxy( 
    _IDestEvents2 __RPC_FAR * This,
    /* [in] */ BSTR MapName);


void __RPC_STUB _IDestEvents2_OnSwapMap_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* ___IDestEvents2_INTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_Dest;

#ifdef __cplusplus

class DECLSPEC_UUID("D3E508C8-211A-4739-BA04-8042D3440A8D")
Dest;
#endif
#endif /* __DESTDLLLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long __RPC_FAR *, unsigned long            , BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long __RPC_FAR *, BSTR __RPC_FAR * ); 

unsigned long             __RPC_USER  LPSAFEARRAY_UserSize(     unsigned long __RPC_FAR *, unsigned long            , LPSAFEARRAY __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  LPSAFEARRAY_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, LPSAFEARRAY __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  LPSAFEARRAY_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, LPSAFEARRAY __RPC_FAR * ); 
void                      __RPC_USER  LPSAFEARRAY_UserFree(     unsigned long __RPC_FAR *, LPSAFEARRAY __RPC_FAR * ); 

unsigned long             __RPC_USER  VARIANT_UserSize(     unsigned long __RPC_FAR *, unsigned long            , VARIANT __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  VARIANT_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, VARIANT __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  VARIANT_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, VARIANT __RPC_FAR * ); 
void                      __RPC_USER  VARIANT_UserFree(     unsigned long __RPC_FAR *, VARIANT __RPC_FAR * ); 

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
