//  diskid32.cpp

//  for displaying the details of hard drives in a command window

//  06/11/00  Lynn McGuire  written with many contributions from others,
//                            IDE drives only under Windows NT/2K and 9X,
//                            maybe SCSI drives later
//  11/20/03  Lynn McGuire  added ReadPhysicalDriveInNTWithZeroRights
//  10/26/05  Lynn McGuire  fix the flipAndCodeBytes function
//  12/21/07  Konstantin Tchoumak adopted for VS2005

#if defined(_WIN32) // Not ported yet

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>

#include <windows.h>
#include <winioctl.h>


#include "diskid.h"

namespace Utils
{
	//  Required to ensure correct PhysicalDrive IOCTL structure setup

#define  IDENTIFY_BUFFER_SIZE  512
	//  Valid values for the bCommandReg member of IDEREGS.
#define  IDE_ATAPI_IDENTIFY  0xA1  //  Returns ID sector for ATAPI.
#define  IDE_ATA_IDENTIFY    0xEC  //  Returns ID sector for ATA.

	struct disk_t
	{
		int             num_controller; // 0-3
		bool            master_slave;
		char            vendor[256];    // vendor ??
		char            model[256];     // Product Id
		char            serial[256];    //
		char            revision[256];
		unsigned int    buffer;         //Controller Buffer Size on Drive in bytes
		int             type;           // REMOVABLE_DISK =  0, FIXED_DISK = 1, UNKNOWN_DISK = -1
		__int64         sectors;
		__int64         size;           // Drive Size

		disk_t(){ memset( this, 0, sizeof(disk_t) ); };
	};

	class DiskInfo
	{
	private:
#pragma pack(push, 1)
		//  GETVERSIONOUTPARAMS contains the data returned from the
		//  Get Driver Version function.
		typedef struct _GETVERSIONOUTPARAMS
		{
			unsigned __int8 bVersion;      // Binary driver version.
			unsigned __int8 bRevision;     // Binary driver revision.
			unsigned __int8 bReserved;     // Not used.
			unsigned __int8 bIDEDeviceMap; // Bit map of IDE devices.
			unsigned __int32 fCapabilities; // Bit mask of driver capabilities.
			unsigned __int32 dwReserved[4]; // For future use.
		} GETVERSIONOUTPARAMS, *PGETVERSIONOUTPARAMS, *LPGETVERSIONOUTPARAMS;

		//  IDE registers
		typedef struct _IDEREGS
		{
			unsigned __int8 bFeaturesReg;       // Used for specifying SMART "commands".
			unsigned __int8 bSectorCountReg;    // IDE sector count register
			unsigned __int8 bSectorNumberReg;   // IDE sector number register
			unsigned __int8 bCylLowReg;         // IDE low order cylinder value
			unsigned __int8 bCylHighReg;        // IDE high order cylinder value
			unsigned __int8 bDriveHeadReg;      // IDE drive/head register
			unsigned __int8 bCommandReg;        // Actual IDE command.
			unsigned __int8 bReserved;          // reserved for future use.  Must be zero.
		} IDEREGS, *PIDEREGS, *LPIDEREGS;

		//  SENDCMDINPARAMS contains the input parameters for the
		//  Send Command to Drive function.
		typedef struct _SENDCMDINPARAMS
		{
			unsigned __int32     cBufferSize;   //  Buffer size in bytes
			IDEREGS   irDriveRegs;   //  Structure with drive register values.
			unsigned __int8 bDriveNumber;       //  Physical drive number to send
			//  command to (0,1,2,3).
			unsigned __int8 bReserved[3];       //  Reserved for future expansion.
			unsigned __int32     dwReserved[4]; //  For future use.
			unsigned __int8      bBuffer[1];    //  Input buffer.
		} SENDCMDINPARAMS, *PSENDCMDINPARAMS, *LPSENDCMDINPARAMS;

		// Status returned from driver
		typedef struct _DRIVERSTATUS
		{
			unsigned __int8  bDriverError;  //  Error code from driver, or 0 if no error.
			unsigned __int8  bIDEStatus;    //  Contents of IDE Error register.
			//  Only valid when bDriverError is SMART_IDE_ERROR.
			unsigned __int8  bReserved[2];  //  Reserved for future expansion.
			unsigned __int32  dwReserved[2];  //  Reserved for future expansion.
		} DRIVERSTATUS, *PDRIVERSTATUS, *LPDRIVERSTATUS;

		// Structure returned by PhysicalDrive IOCTL for several commands
		typedef struct _SENDCMDOUTPARAMS
		{
			unsigned __int32         cBufferSize;   //  Size of bBuffer in bytes
			DRIVERSTATUS  DriverStatus;  //  Driver status structure.
			unsigned __int8          bBuffer[1];    //  Buffer of arbitrary length in which to store the data read from the                                                       // drive.
		} SENDCMDOUTPARAMS, *PSENDCMDOUTPARAMS, *LPSENDCMDOUTPARAMS;

		// The following struct defines the interesting part of the IDENTIFY
		// buffer:
		typedef struct _IDSECTOR
		{
			unsigned short  wGenConfig;
			unsigned short  wNumCyls;
			unsigned short  wReserved;
			unsigned short  wNumHeads;
			unsigned short  wBytesPerTrack;
			unsigned short  wBytesPerSector;
			unsigned short  wSectorsPerTrack;
			unsigned short  wVendorUnique[3];
			char            sSerialNumber[20];
			unsigned short  wBufferType;
			unsigned short  wBufferSize;
			unsigned short  wECCSize;
			char            sFirmwareRev[8];
			char            sModelNumber[40];
			unsigned short  wMoreVendorUnique;
			unsigned short  wDoubleWordIO;
			unsigned short  wCapabilities;
			unsigned short  wReserved1;
			unsigned short  wPIOTiming;
			unsigned short  wDMATiming;
			unsigned short  wBS;
			unsigned short  wNumCurrentCyls;
			unsigned short  wNumCurrentHeads;
			unsigned short  wNumCurrentSectorsPerTrack;
			unsigned long   ulCurrentSectorCapacity;
			unsigned short  wMultSectorStuff;
			unsigned long   ulTotalAddressableSectors;
			unsigned short  wSingleWordDMA;
			unsigned short  wMultiWordDMA;
			unsigned __int8    bReserved[128];
		} IDSECTOR, *PIDSECTOR;

		// (* Output Bbuffer for the VxD (rt_IdeDinfo record) *)
		typedef struct _rt_IdeDInfo_
		{
			unsigned __int8 IDEExists[4];
			unsigned __int8 DiskExists[8];
			unsigned __int16 DisksRawInfo[8*256];
		} rt_IdeDInfo, *pt_IdeDInfo;


		// (* IdeDinfo "data fields" *)
		typedef struct _rt_DiskInfo_
		{
			bool             DiskExists;
			bool             ATAdevice;
			bool             RemovableDevice;
			unsigned __int16 TotLogCyl;
			unsigned __int16 TotLogHeads;
			unsigned __int16 TotLogSPT;
			char             SerialNumber[20];
			char             FirmwareRevision[8];
			char             ModelNumber[40];
			unsigned __int16 CurLogCyl;
			unsigned __int16 CurLogHeads;
			unsigned __int16 CurLogSPT;
		} rt_DiskInfo;


		typedef struct _SRB_IO_CONTROL
		{
			unsigned long HeaderLength;
			unsigned char Signature[8];
			unsigned long Timeout;
			unsigned long ControlCode;
			unsigned long ReturnCode;
			unsigned long Length;
		} SRB_IO_CONTROL, *PSRB_IO_CONTROL;

		typedef struct _MEDIA_SERAL_NUMBER_DATA
		{
			unsigned long  SerialNumberLength;
			unsigned long  Result;
			unsigned long  Reserved[2];
			unsigned long  SerialNumberData[1];
		} MEDIA_SERIAL_NUMBER_DATA, *PMEDIA_SERIAL_NUMBER_DATA;

#pragma pack(pop)

		void WriteConstantString (char *entry, char *string){ (string); (entry); }
		char *ConvertToString( unsigned __int32 diskdata[256], int firstIndex, int lastIndex );
		bool ReadPhysicalDriveInNTWithAdminRights( disk_t &_disk );
		bool ReadDrivePortsInWin9X( disk_t &disk );
		bool GetIdeInfo( const int drive, unsigned __int32 diskdata[256], disk_t &_disk  );
		bool ReadIdeDriveAsScsiDriveInNT( disk_t &_disk );
		bool ReadPhysicalDriveInNTWithZeroRights( disk_t &_disk );
		char * flipAndCodeBytes (char * str);

		bool DoIDENTIFY (void * hPhysicalDriveIOCTL, PSENDCMDINPARAMS pSCIP,
			PSENDCMDOUTPARAMS pSCOP, unsigned __int8 bIDCmd, unsigned __int8 bDriveNum,
			unsigned __int32 * lpcbBytesReturned);
		static void strMACaddress( unsigned char MACData[], char string[256] );

		// Define global buffers.
		unsigned __int8  IdOutCmd [sizeof (SENDCMDOUTPARAMS) + IDENTIFY_BUFFER_SIZE - 1];
		char             HardDriveSerialNumber[1024];
		char             HardDriveModelNumber[1024];
		char             m_flipped[1024];
		char             m_cv[1024];
	public:
		//std::vector<std::wstring>    errors;

		static unsigned __int64 getHardDriveComputerID( disk_t &_disk );
		bool                getDrivesInfo( disk_t &_disk );
	};
};



#define  TITLE   "DiskId32"

#pragma warning (disable : 4996)

//----------------------------------------------------------------------------------------------------------------------
//  Max number of drives assuming primary/secondary, master/slave topology
#define  MAX_IDE_DRIVES  16

//  IOCTL commands
#define  DFP_GET_VERSION          0x00074080
#define  DFP_SEND_DRIVE_COMMAND   0x0007c084
#define  DFP_RECEIVE_DRIVE_DATA   0x0007c088

#define  FILE_DEVICE_SCSI              0x0000001b
#define  IOCTL_SCSI_MINIPORT_IDENTIFY  ((FILE_DEVICE_SCSI << 16) + 0x0501)
#define  IOCTL_SCSI_MINIPORT 0x0004D008  //  see NTDDSCSI.H for definition

//  Bits returned in the fCapabilities member of GETVERSIONOUTPARAMS
#define  CAP_IDE_ID_FUNCTION             1  // ATA ID command supported
#define  CAP_IDE_ATAPI_ID                2  // ATAPI ID command supported
#define  CAP_IDE_EXECUTE_SMART_FUNCTION  4  // SMART commannds supported

namespace Utils
{

	//----------------------------------------------------------------------------------------------------------------------
	// DoIDENTIFY
	// FUNCTION: Send an IDENTIFY command to the drive
	// bDriveNum = 0-3
	// bIDCmd = IDE_ATA_IDENTIFY or IDE_ATAPI_IDENTIFY
	bool DiskInfo::DoIDENTIFY ( void * hPhysicalDriveIOCTL, PSENDCMDINPARAMS pSCIP,
		PSENDCMDOUTPARAMS pSCOP, unsigned __int8 bIDCmd, unsigned __int8 bDriveNum,
		unsigned __int32 * lpcbBytesReturned )
	{
		// Set up data structures for IDENTIFY command.
		pSCIP -> cBufferSize = IDENTIFY_BUFFER_SIZE;
		pSCIP -> irDriveRegs.bFeaturesReg = 0;
		pSCIP -> irDriveRegs.bSectorCountReg = 1;
		//pSCIP -> irDriveRegs.bSectorNumberReg = 1;
		pSCIP -> irDriveRegs.bCylLowReg = 0;
		pSCIP -> irDriveRegs.bCylHighReg = 0;

		// Compute the drive number.
		pSCIP -> irDriveRegs.bDriveHeadReg = 0xA0 | ((bDriveNum & 1) << 4);

		// The command can either be IDE identify or ATAPI identify.
		pSCIP -> irDriveRegs.bCommandReg = bIDCmd;
		pSCIP -> bDriveNumber = bDriveNum;
		pSCIP -> cBufferSize = IDENTIFY_BUFFER_SIZE;

		return ( DeviceIoControl (hPhysicalDriveIOCTL, DFP_RECEIVE_DRIVE_DATA,
			(LPVOID) pSCIP,
			sizeof(SENDCMDINPARAMS) - 1,
			(LPVOID) pSCOP,
			sizeof(SENDCMDOUTPARAMS) + IDENTIFY_BUFFER_SIZE - 1,
			(LPDWORD)lpcbBytesReturned, NULL) ? true : false );
	}
	//----------------------------------------------------------------------------------------------------------------------
	bool DiskInfo::GetIdeInfo( const int drive, unsigned __int32 diskdata [256], disk_t &_disk )
	{
		if( drive < 0 )
		{
			return false;
		}
		char string1 [1024] = {0};

		memset( &_disk, 0, sizeof( _disk ) );

		//  copy the hard drive serial number to the buffer
		char tmp [1024] = {0};
		strncpy( tmp, ConvertToString( diskdata, 10, 19 ), sizeof(tmp)-1 );
		size_t ctr = 0;
		// cut off frontal spaces
		for( size_t i = 0; i < strlen(tmp); i++ )
		{
			if( tmp[i] != ' ' )
			{
				ctr = i;
				break;
			}
		}
		strncpy_s( string1, sizeof(string1)-1, &tmp[ctr], strlen(&tmp[ctr]) );

		//  serial number must be alphanumeric(but there can be leading spaces on IBM drives)
		if( 0 == HardDriveSerialNumber [0] && (isalnum (string1 [0]) || isalnum (string1 [19])) )
		{
			memset( HardDriveSerialNumber, 0, sizeof(HardDriveSerialNumber)/sizeof(HardDriveSerialNumber[0]) );
			strncpy_s( HardDriveSerialNumber, sizeof(HardDriveSerialNumber)-1, string1, strlen(string1) );

			memset( HardDriveModelNumber, 0, sizeof(HardDriveModelNumber)/sizeof(HardDriveModelNumber[0]) );
			strncpy( HardDriveModelNumber, ConvertToString( diskdata, 27, 46 ), sizeof(HardDriveModelNumber)-1 );
		}
		switch (drive / 2)
		{
		case 0: _disk.num_controller = 0; break;  //Primary Controller
		case 1: _disk.num_controller = 1; break;  //Secondary Controller
		case 2: _disk.num_controller = 2; break;  //Tertiary Controller
		case 3: _disk.num_controller = 3; break;  //Quaternary Controller
		}
		switch (drive % 2)
		{
		case 0: _disk.master_slave = true; break;
		case 1: _disk.master_slave = false; break;
		}

		strncpy( _disk.model,    ConvertToString (diskdata, 27, 46), sizeof(_disk.model)-1 );
		strncpy( _disk.serial,   ConvertToString (diskdata, 10, 19), sizeof(_disk.serial)-1 );
		strncpy( _disk.revision, ConvertToString (diskdata, 23, 26), sizeof(_disk.revision)-1 );

		_disk.buffer = diskdata [21] * 512;

		if( diskdata [0] & 0x0080 )
		{
			_disk.type = 0; // REMOVABLE_DISK;
		}else if( diskdata [0] & 0x0040 )
		{
			_disk.type = 1; // FIXED_DISK;
		}else
		{
			_disk.type = -1; // UNKNOWN_DISK;
		}
		//  calculate size based on 28 bit or 48 bit addressing
		//  48 bit addressing is reflected by bit 10 of word 83
		if (diskdata [83] & 0x400)
		{
			_disk.sectors = diskdata [103] * 65536I64 * 65536I64 * 65536I64 +
				diskdata [102] * 65536I64 * 65536I64 +
				diskdata [101] * 65536I64 +
				diskdata [100];
		}else
		{
			_disk.sectors = diskdata [61] * 65536 + diskdata [60];
		}
		//  there are 512 bytes in a sector
		_disk.size = _disk.sectors * 512;

		return true;
	}
	//----------------------------------------------------------------------------------------------------------------------
	char *DiskInfo::ConvertToString( unsigned __int32 diskdata [256], int firstIndex, int lastIndex )
	{
		int position = 0;
		memset( m_cv, 0, sizeof(m_cv) );

		//  each integer has two characters stored in it backwards
		for(int index = firstIndex; index <= lastIndex; index++)
		{
			//  get high byte for 1st character
			m_cv [position] = (char) (diskdata [index] / 256);
			position++;

			//  get low byte for 2nd character
			m_cv [position] = (char) (diskdata [index] % 256);
			position++;
		}
		//  end the string
		m_cv[position] = '\0';

		//  cut off the trailing blanks
		for(int index = position - 1; index > 0 && ' ' == m_cv[index]; index-- )
		{
			m_cv [index] = '\0';
		}
		return m_cv;
	}
	//----------------------------------------------------------------------------------------------------------------------
	bool DiskInfo::ReadPhysicalDriveInNTWithAdminRights(disk_t &lst_disk )
	{
		bool done = false;
		int drive = 0;



		{
			HANDLE hPhysicalDriveIOCTL = 0;
			wchar_t szMsg[512] = {0};

			//  Try to get a handle to PhysicalDrive IOCTL, report failure
			//  and exit if can't.
			wchar_t driveName [256]={0};

			_snwprintf( driveName, sizeof(driveName)-1, L"\\\\.\\PhysicalDrive%d", drive);

			//  Windows NT, Windows 2000, must have admin rights
			hPhysicalDriveIOCTL = CreateFileW (driveName,
				GENERIC_READ | GENERIC_WRITE,
				FILE_SHARE_READ | FILE_SHARE_WRITE , NULL,
				OPEN_EXISTING, 0, NULL);

			if( hPhysicalDriveIOCTL == INVALID_HANDLE_VALUE )
			{
				_snwprintf( szMsg, sizeof(szMsg)-1,
					L"Unable to open physical drive %d, error code: 0x%lX\n",
					drive, GetLastError () );
				//              errors.push_back( szMsg );
				return false;
			}

			if (hPhysicalDriveIOCTL != INVALID_HANDLE_VALUE)
			{
				GETVERSIONOUTPARAMS VersionParams;
				unsigned __int32    cbBytesReturned = 0;

				// Get the version, etc of PhysicalDrive IOCTL
				memset ((void*) &VersionParams, 0, sizeof(VersionParams));

				if ( ! DeviceIoControl (hPhysicalDriveIOCTL, DFP_GET_VERSION,
					NULL,
					0,
					&VersionParams,
					sizeof(VersionParams),
					(LPDWORD)&cbBytesReturned, NULL) )
				{
					_snwprintf( szMsg, sizeof(szMsg)-1, L"DFP_GET_VERSION failed for drive %d\n", drive );
					//                  errors.push_back( szMsg );
					return false;
				}

				// If there is a IDE device at number "drive" issue commands
				// to the device
				if (VersionParams.bIDEDeviceMap > 0)
				{
					BYTE             bIDCmd = 0;   // IDE or ATAPI IDENTIFY cmd
					SENDCMDINPARAMS  scip;
					//SENDCMDOUTPARAMS OutCmd;

					// Now, get the ID sector for all IDE devices in the system.
					// If the device is ATAPI use the IDE_ATAPI_IDENTIFY command,
					// otherwise use the IDE_ATA_IDENTIFY command
					bIDCmd = (VersionParams.bIDEDeviceMap >> drive & 0x10) ? IDE_ATAPI_IDENTIFY : IDE_ATA_IDENTIFY;

					memset (&scip, 0, sizeof(scip));
					memset (IdOutCmd, 0, sizeof(IdOutCmd));

					if ( DoIDENTIFY (hPhysicalDriveIOCTL,
						&scip,
						(PSENDCMDOUTPARAMS)&IdOutCmd,
						(BYTE) bIDCmd,
						(BYTE) drive,
						&cbBytesReturned))
					{
						unsigned __int32 diskdata [256] = {0};
						USHORT *pIdSector = (USHORT *)((PSENDCMDOUTPARAMS) IdOutCmd) -> bBuffer;

						for( int ijk = 0; ijk < 256; ijk++ )
						{
							diskdata [ijk] = pIdSector [ijk];
						}
						done = GetIdeInfo( drive, diskdata, lst_disk );

					}
				}
				CloseHandle (hPhysicalDriveIOCTL);
			}
		}

		return done;
	}
	//----------------------------------------------------------------------------------------------------------------------
	//  Required to ensure correct PhysicalDrive IOCTL structure setup
#pragma pack(4)


	//
	// IOCTL_STORAGE_QUERY_PROPERTY
	//
	// Input Buffer:
	//      a STORAGE_PROPERTY_QUERY structure which describes what type of query
	//      is being done, what property is being queried for, and any additional
	//      parameters which a particular property query requires.
	//
	//  Output Buffer:
	//      Contains a buffer to place the results of the query into.  Since all
	//      property descriptors can be cast into a STORAGE_DESCRIPTOR_HEADER,
	//      the IOCTL can be called once with a small buffer then again using
	//      a buffer as large as the header reports is necessary.
	//


	//
	// Types of queries
	//

	typedef enum _STORAGE_QUERY_TYPE
	{
		PropertyStandardQuery = 0,          // Retrieves the descriptor
		PropertyExistsQuery,                // Used to test whether the descriptor is supported
		PropertyMaskQuery,                  // Used to retrieve a mask of writeable fields in the descriptor
		PropertyQueryMaxDefined     // use to validate the value
	} STORAGE_QUERY_TYPE, *PSTORAGE_QUERY_TYPE;

	//
	// define some initial property id's
	//

	typedef enum _STORAGE_PROPERTY_ID
	{
		StorageDeviceProperty = 0,
		StorageAdapterProperty
	} STORAGE_PROPERTY_ID, *PSTORAGE_PROPERTY_ID;

	//
	// Query structure - additional parameters for specific queries can follow
	// the header
	//

	typedef struct _STORAGE_PROPERTY_QUERY {

		//
		// ID of the property being retrieved
		//

		STORAGE_PROPERTY_ID PropertyId;

		//
		// Flags indicating the type of query being performed
		//

		STORAGE_QUERY_TYPE QueryType;

		//
		// Space for additional parameters if necessary
		//

		UCHAR AdditionalParameters[1];

	} STORAGE_PROPERTY_QUERY, *PSTORAGE_PROPERTY_QUERY;


#define IOCTL_STORAGE_QUERY_PROPERTY   CTL_CODE(IOCTL_STORAGE_BASE, 0x0500, METHOD_BUFFERED, FILE_ANY_ACCESS)


	//
	// Device property descriptor - this is really just a rehash of the inquiry
	// data retrieved from a scsi device
	//
	// This may only be retrieved from a target device.  Sending this to the bus
	// will result in an error
	//

#pragma pack(4)

	typedef struct _STORAGE_DEVICE_DESCRIPTOR
	{

		//
		// Sizeof(STORAGE_DEVICE_DESCRIPTOR)
		//

		ULONG Version;

		//
		// Total size of the descriptor, including the space for additional
		// data and id strings
		//

		ULONG Size;

		//
		// The SCSI-2 device type
		//

		UCHAR DeviceType;

		//
		// The SCSI-2 device type modifier (if any) - this may be zero
		//

		UCHAR DeviceTypeModifier;

		//
		// Flag indicating whether the device's media (if any) is removable.  This
		// field should be ignored for media-less devices
		//

		BOOLEAN RemovableMedia;

		//
		// Flag indicating whether the device can support mulitple outstanding
		// commands.  The actual synchronization in this case is the responsibility
		// of the port driver.
		//

		BOOLEAN CommandQueueing;

		//
		// Byte offset to the zero-terminated ascii string containing the device's
		// vendor id string.  For devices with no such ID this will be zero
		//

		ULONG VendorIdOffset;

		//
		// Byte offset to the zero-terminated ascii string containing the device's
		// product id string.  For devices with no such ID this will be zero
		//

		ULONG ProductIdOffset;

		//
		// Byte offset to the zero-terminated ascii string containing the device's
		// product revision string.  For devices with no such string this will be
		// zero
		//

		ULONG ProductRevisionOffset;

		//
		// Byte offset to the zero-terminated ascii string containing the device's
		// serial number.  For devices with no serial number this will be zero
		//

		ULONG SerialNumberOffset;

		//
		// Contains the bus type (as defined above) of the device.  It should be
		// used to interpret the raw device properties at the end of this structure
		// (if any)
		//

		STORAGE_BUS_TYPE BusType;

		//
		// The number of bytes of bus-specific data which have been appended to
		// this descriptor
		//

		ULONG RawPropertiesLength;

		//
		// Place holder for the first byte of the bus specific property data
		//

		UCHAR RawDeviceProperties[1];

	} STORAGE_DEVICE_DESCRIPTOR, *PSTORAGE_DEVICE_DESCRIPTOR;


	//--------------------------------------------------------------------------------------------------------
	//  function to decode the serial numbers of IDE hard drives
	//  using the IOCTL_STORAGE_QUERY_PROPERTY command
	char * DiskInfo::flipAndCodeBytes (char * str)
	{
		const size_t num = strlen (str);

		memset( m_flipped, '\0', sizeof(m_flipped) );
		for( size_t i = 0; i < num; i += 4 )
		{
			for( int j = 1; j >= 0; j-- )
			{
				int sum = 0;
				for( int k = 0; k < 2; k++ )
				{
					sum *= 16;
					switch (str [i + j * 2 + k])
					{
					case '0': sum += 0; break;
					case '1': sum += 1; break;
					case '2': sum += 2; break;
					case '3': sum += 3; break;
					case '4': sum += 4; break;
					case '5': sum += 5; break;
					case '6': sum += 6; break;
					case '7': sum += 7; break;
					case '8': sum += 8; break;
					case '9': sum += 9; break;
					case 'a': sum += 10; break;
					case 'b': sum += 11; break;
					case 'c': sum += 12; break;
					case 'd': sum += 13; break;
					case 'e': sum += 14; break;
					case 'f': sum += 15; break;
					case 'A': sum += 10; break;
					case 'B': sum += 11; break;
					case 'C': sum += 12; break;
					case 'D': sum += 13; break;
					case 'E': sum += 14; break;
					case 'F': sum += 15; break;
					}
				}
				if (sum > 0)
				{
					char sub [2];
					sub [0] = (char) sum;
					sub [1] = 0;
					strcat( m_flipped, sub );
				}
			}
		}

		return m_flipped;
	}
	//--------------------------------------------------------------------------------------------------------
	bool DiskInfo::ReadPhysicalDriveInNTWithZeroRights( disk_t &lst_disk )
	{
		bool done = false;


		int drive = 0;
		{
			HANDLE hPhysicalDriveIOCTL = 0;
			wchar_t szMsg[512] = {0};

			//  Try to get a handle to PhysicalDrive IOCTL, report failure
			//  and exit if can't.
			wchar_t driveName [256] = {0};

			_snwprintf( driveName, sizeof(driveName)-1, L"\\\\.\\PhysicalDrive%d", drive );

			//  Windows NT, Windows 2000, Windows XP - admin rights not required
			hPhysicalDriveIOCTL = CreateFileW (driveName, 0,
				FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
				OPEN_EXISTING, 0, NULL);
			if (hPhysicalDriveIOCTL == INVALID_HANDLE_VALUE)
			{
				_snwprintf( szMsg, sizeof(szMsg)-1,
					L"Unable to open physical drive %d, error code: 0x%lX", drive, GetLastError () );
				//              errors.push_back( szMsg );
			}

			if (hPhysicalDriveIOCTL != INVALID_HANDLE_VALUE)
			{
				STORAGE_PROPERTY_QUERY query;
				DWORD cbBytesReturned = 0;
				char buffer [16000] = {0};

				memset ((void *) & query, 0, sizeof (query));
				query.PropertyId = StorageDeviceProperty;
				query.QueryType = PropertyStandardQuery;

				memset (buffer, 0, sizeof (buffer));

				if ( DeviceIoControl (hPhysicalDriveIOCTL, IOCTL_STORAGE_QUERY_PROPERTY,
					& query,
					sizeof (query),
					& buffer,
					sizeof (buffer),
					& cbBytesReturned, NULL) )
				{
					STORAGE_DEVICE_DESCRIPTOR * descrip = (STORAGE_DEVICE_DESCRIPTOR *) & buffer;
					char serialNumber [1000] = {0};
					char modelNumber [1000] = {0};

					strcpy (serialNumber,
						flipAndCodeBytes ( & buffer [descrip -> SerialNumberOffset]));
					strcpy (modelNumber, & buffer [descrip -> ProductIdOffset]);
					if (0 == HardDriveSerialNumber [0] &&
						//  serial number must be alphanumeric
						//  (but there can be leading spaces on IBM drives)
						(isalnum (serialNumber [0]) || isalnum (serialNumber [19])))
					{
						strcpy (HardDriveSerialNumber, serialNumber);
						strcpy (HardDriveModelNumber, modelNumber);
						done = TRUE;
					}

					//disk_t disk;
					lst_disk.num_controller = drive;
					strncpy( lst_disk.vendor,  &buffer [descrip -> VendorIdOffset], sizeof(lst_disk.vendor) );
					strncpy( lst_disk.model,  &buffer [descrip -> ProductIdOffset], sizeof(lst_disk.model) );
					strncpy( lst_disk.revision,  &buffer [descrip -> ProductRevisionOffset], sizeof(lst_disk.revision) );
					strncpy( lst_disk.serial,  serialNumber, sizeof(lst_disk.serial) );


				}
				else
				{
					_snwprintf( szMsg, sizeof(szMsg)-1,
						L"DeviceIOControl IOCTL_STORAGE_QUERY_PROPERTY error = %ld", GetLastError () );
					//                  errors.push_back( szMsg );
				}

				memset (buffer, 0, sizeof (buffer));

				if ( DeviceIoControl (hPhysicalDriveIOCTL, IOCTL_STORAGE_GET_MEDIA_SERIAL_NUMBER,
					NULL,
					0,
					& buffer,
					sizeof (buffer),
					& cbBytesReturned, NULL) )
				{
					MEDIA_SERIAL_NUMBER_DATA * mediaSerialNumber =
						(MEDIA_SERIAL_NUMBER_DATA *) & buffer;
					char serialNumber [1000] = {0};
					//                 char modelNumber [1000] = {0};

					strncpy( serialNumber, (char *) mediaSerialNumber -> SerialNumberData, sizeof(serialNumber)-1 );
					//			     strcpy (modelNumber, & buffer [descrip -> ProductIdOffset], sizeof(modelNumber)-1 );

					if (0 == HardDriveSerialNumber [0] &&
						//  serial number must be alphanumeric
						//  (but there can be leading spaces on IBM drives)
						(isalnum (serialNumber [0]) || isalnum (serialNumber [19])))
					{
						strncpy( HardDriveSerialNumber, serialNumber, sizeof(HardDriveSerialNumber)-1 );
						//				    strncpy (HardDriveModelNumber, modelNumber, sizeof(HardDriveModelNumber)-1 );
						done = true;
					}
				}
				else
				{
					DWORD err = GetLastError ();

					switch (err)
					{
					case 1:
						_snwprintf( szMsg, sizeof(szMsg)-1, L"DeviceIOControl IOCTL_STORAGE_GET_MEDIA_SERIAL_NUMBER error = %ld The request is not valid for this device.",
							GetLastError () );
						break;
					case 50:
						_snwprintf( szMsg, sizeof(szMsg)-1, L"DeviceIOControl IOCTL_STORAGE_GET_MEDIA_SERIAL_NUMBER error = %ld The request is not supported for this device.",
							GetLastError () );
						break;
					default:
						_snwprintf( szMsg, sizeof(szMsg)-1, L"DeviceIOControl IOCTL_STORAGE_GET_MEDIA_SERIAL_NUMBER error = %ld",
							GetLastError () );
					}
					//                 errors.push_back( szMsg );

				}
				CloseHandle (hPhysicalDriveIOCTL);
			}
		}

		return done;
	}
	//  -----------------------------------------------------------------------------------------------------------------



	//  ----------------------------------------------------------------------------------------------
#define  SENDIDLENGTH  sizeof (SENDCMDOUTPARAMS) + IDENTIFY_BUFFER_SIZE


	//  ----------------------------------------------------------------------------------------------
	bool DiskInfo::ReadIdeDriveAsScsiDriveInNT( disk_t &lst_disk )
	{
		bool done = false;



		for( int controller = 0; controller < 16; controller++ )
		{
			HANDLE hScsiDriveIOCTL = 0;
			wchar_t   driveName [256] = {0};

			//  Try to get a handle to PhysicalDrive IOCTL, report failure
			//  and exit if can't.
			_snwprintf( driveName, sizeof(driveName)-1, L"\\\\.\\Scsi%d:", controller );

			//  Windows NT, Windows 2000, any rights should do
			hScsiDriveIOCTL = CreateFileW( driveName,
				GENERIC_READ | GENERIC_WRITE,
				FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
				OPEN_EXISTING, 0, NULL);
			if( hScsiDriveIOCTL == INVALID_HANDLE_VALUE )
			{
				wchar_t szMsg[512] = {0};
				_snwprintf( szMsg, sizeof(szMsg)-1,
					L"Unable to open SCSI controller %d, error code: 0x%lX", controller, GetLastError () );
				// errors.push_back( szMsg );
			}

			if (hScsiDriveIOCTL != INVALID_HANDLE_VALUE)
			{
				int drive = 0;

				for (drive = 0; drive < 2; drive++)
				{
					char buffer [sizeof (SRB_IO_CONTROL) + SENDIDLENGTH];
					SRB_IO_CONTROL *p = (SRB_IO_CONTROL *) buffer;
					SENDCMDINPARAMS *pin =
						(SENDCMDINPARAMS *) (buffer + sizeof (SRB_IO_CONTROL));
					DWORD dummy;

					memset (buffer, 0, sizeof (buffer));
					p -> HeaderLength = sizeof (SRB_IO_CONTROL);
					p -> Timeout = 10000;
					p -> Length = SENDIDLENGTH;
					p -> ControlCode = IOCTL_SCSI_MINIPORT_IDENTIFY;
					strncpy( (char *)p->Signature, "SCSIDISK", 8 );

					pin -> irDriveRegs.bCommandReg = IDE_ATA_IDENTIFY;
					pin -> bDriveNumber = (unsigned char)drive;

					if (DeviceIoControl (hScsiDriveIOCTL, IOCTL_SCSI_MINIPORT,
						buffer,
						sizeof (SRB_IO_CONTROL) +
						sizeof (SENDCMDINPARAMS) - 1,
						buffer,
						sizeof (SRB_IO_CONTROL) + SENDIDLENGTH,
						&dummy, NULL))
					{
						SENDCMDOUTPARAMS *pOut =
							(SENDCMDOUTPARAMS *) (buffer + sizeof (SRB_IO_CONTROL));
						IDSECTOR *pId = (IDSECTOR *) (pOut -> bBuffer);
						if (pId -> sModelNumber [0])
						{
							unsigned __int32 diskdata [256];
							int ijk = 0;
							USHORT *pIdSector = (USHORT *) pId;

							for( ijk = 0; ijk < 256; ijk++ )
							{
								diskdata [ijk] = pIdSector [ijk];
							}
							// disk_t _disk;

							done = GetIdeInfo (controller * 2 + drive, diskdata, lst_disk );
							break;
						}
					}
				}
				CloseHandle (hScsiDriveIOCTL);
			}
		}

		return done;
	}
	//-------------------------------------------------------------------------------------------------------------------
	unsigned __int64 DiskInfo::getHardDriveComputerID( disk_t &_disk )
	{
		unsigned __int64 id = 0ULL;

		char serial[1024] = {0};
		strncpy_s( serial, sizeof(serial)-1, _disk.serial, strlen(_disk.serial) );

		if( serial[0] > 0 )
		{
			char *p = serial;

			//  ignore first 5 characters from western digital hard drives if
			//  the first four characters are WD-W
			if( !strncmp (serial, "WD-W", 4))
			{
				p += 5;
			}
			for( ; p && *p; p++ )
			{
				if( '-' == *p )
				{
					continue;
				}
				id *= 10;
				switch (*p)
				{
				case '0': id += 0; break;
				case '1': id += 1; break;
				case '2': id += 2; break;
				case '3': id += 3; break;
				case '4': id += 4; break;
				case '5': id += 5; break;
				case '6': id += 6; break;
				case '7': id += 7; break;
				case '8': id += 8; break;
				case '9': id += 9; break;
				case 'a': case 'A': id += 10; break;
				case 'b': case 'B': id += 11; break;
				case 'c': case 'C': id += 12; break;
				case 'd': case 'D': id += 13; break;
				case 'e': case 'E': id += 14; break;
				case 'f': case 'F': id += 15; break;
				case 'g': case 'G': id += 16; break;
				case 'h': case 'H': id += 17; break;
				case 'i': case 'I': id += 18; break;
				case 'j': case 'J': id += 19; break;
				case 'k': case 'K': id += 20; break;
				case 'l': case 'L': id += 21; break;
				case 'm': case 'M': id += 22; break;
				case 'n': case 'N': id += 23; break;
				case 'o': case 'O': id += 24; break;
				case 'p': case 'P': id += 25; break;
				case 'q': case 'Q': id += 26; break;
				case 'r': case 'R': id += 27; break;
				case 's': case 'S': id += 28; break;
				case 't': case 'T': id += 29; break;
				case 'u': case 'U': id += 30; break;
				case 'v': case 'V': id += 31; break;
				case 'w': case 'W': id += 32; break;
				case 'x': case 'X': id += 33; break;
				case 'y': case 'Y': id += 34; break;
				case 'z': case 'Z': id += 35; break;
				}
			}
		}

		id %= 100000000ULL;
		if( strstr( serial, "IBM-") )
		{
			id += 300000000ULL;
		}
		else if (strstr (serial, "MAXTOR") ||
			strstr (serial, "Maxtor"))
		{
			id += 400000000ULL;
		}
		else if (strstr (serial, "WDC "))
		{
			id += 500000000ULL;
		}
		else
		{
			id += 600000000ULL;
		}
		return id;
	}
	//-------------------------------------------------------------------------------------------------------------------
	bool DiskInfo::getDrivesInfo( disk_t &_disk )
	{
		bool done = false;
		OSVERSIONINFO version;

		memset( &version, 0, sizeof (version) );
		strcpy (HardDriveSerialNumber, "");

		version.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
		GetVersionEx (&version);
		if( version.dwPlatformId == VER_PLATFORM_WIN32_NT )
		{
			//  this works under WinNT4 or Win2K if you have admin rights
			done = ReadPhysicalDriveInNTWithAdminRights( _disk );

			//  this should work in WinNT or Win2K if previous did not work
			//  this is kind of a backdoor via the SCSI mini port driver into
			//     the IDE drives
			if( !done )
			{
				done = ReadIdeDriveAsScsiDriveInNT( _disk );
			}
			//  this works under WinNT4 or Win2K or WinXP if you have any right
			if( !done )
			{
				done = ReadPhysicalDriveInNTWithZeroRights( _disk );
			}
		}

		return ( done );
	}

};

bool VS_GetDiskString(char *szData, size_t iSize)
{
	if (szData==NULL || iSize<64)
		return false;
	Utils::DiskInfo comp;
	Utils::disk_t _disk;
	if ( comp.getDrivesInfo( _disk )) {
		sprintf_s(szData, iSize, "%s %s %s %s", _disk.vendor, _disk.model, _disk.revision, _disk.serial);
		return true;
	}
	else {
		return false;
	}
}

#endif
