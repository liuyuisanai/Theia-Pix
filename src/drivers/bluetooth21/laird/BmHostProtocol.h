/******************************************************************************
**              Copyright (C) 2003 TDK Systems Europe Ltd
**
** Module:      MPHOSTPROTOCOL.H
**
*******************************************************************************/

#if !defined(_BMHOSTPROTOCOL_H)     /* prevent multiple inclusions */
#define _BMHOSTPROTOCOL_H

/******************************************************************************/
/* Include Files*/
/******************************************************************************/

#include <stdint.h>

/******************************************************************************/
/* Defines*/
/******************************************************************************/

/*
    The following #if block allows some gcc compilers to generate
    code that packs the structs so that each uint8 enitity is stored
    in a byte. Hence a sizeof() will result in the expected value

    If that GCC is used please change the #if 1 to #if 0.

    Please do not change any other code in this file as you are
    likely to get out of sync with new code that will deployed
    in the future.
*/
#if defined(ON_WIN32)
#define ATTR_PACKED         /* */
#else
#define ATTR_PACKED         __attribute__ ((packed))
#endif

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* Miscellaneous defines*/
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
#define MAX_PACKET_SIZE                             255
#define MIN_CHANNEL_ID                              1
#define MAX_CHANNEL_ID                              7
#define MAX_PCM_SLOT_ID                             2
#define MAX_PIN_CODE_LENGTH                         16
#define MAX_INFORMATION_LEN                         8
#define MAX_FNAME_EVENT_DATA_BLOCK                  10
#define MAX_LOCAL_FRIENDLY_NAME_SIZE                30
#define MAX_ASSERT_EVENT_DATA_BLOCK                 11
#define MPHOST_RX_MAX_GAP_TIME_MS                   1000
#define MAX_DEBUGINGO_DATA_BLOCK                    8
#define MAX_CHANNELS_IN_LIST                        16
#define MAX_HDP_SDP_RECORD_NAMELEN                  15

#define MAX_SNIFF_MAXINTERVAL_MSEC                  2000
#define MIN_SNIFF_MAXINTERVAL_MSEC                  100
#define MIN_SNIFF_MININTERVAL_MSEC                  100
#define MIN_SNIFF_ATTEMPT_MSEC                      12
#define MIN_SNIFF_TIMEOUT_MSEC                      12

#define MIN_PARK_MININTERVAL_MSEC                   10
#define MIN_PARK_MAXINTERVAL_MSEC                   10
#define MAX_PARK_MAXINTERVAL_MSEC                   40000

#define STREAM_TYPE_SCO                             0x01
#define STREAM_TYPE_PCM                             0x02

#define SSP_ACTION_COMPLETE                         0
#define SSP_ACTION_DISPLAY_ONLY                     1
#define SSP_ACTION_DISPLAY_YESNO                    2
#define SSP_ACTION_ENTER_PASSCODE                   3


/*
#define SSP_ACTION_
*/

#define BLOBMANAGEID_CLEAR                          0
#define BLOBMANAGEID_GETSIZE                        1
#define BLOBMANAGEID_READ                           2
#define BLOBMANAGEID_HID_DESC_SET                   3
#define BLOBMANAGEID_HID_DESC_GET                   4
#define BLOBMANAGEID_HID_SERVICENAME_SET            5
#define BLOBMANAGEID_HID_SERVICENAME_GET            6
#define BLOBMANAGEID_SEND_EIR_TO_STACK              7
#define BLOBMANAGEID_SAVE_EIR_TO_NONVOL             8
#define BLOBMANAGEID_LOAD_EIR_FROM_NONVOL           9
#if defined(LTMODULE_INC_SPP_SUBPROTOCOLS)
#define BLOBMANAGEID_SAVE_SYSCONFIG_TO_NONVOL       0x80
#define BLOBMANAGEID_LOAD_SYSCONFIG_FROM_NONVOL     0x81
#endif

/*
#define BLOBMANAGEID_
*/

#define HDP_ROLE_AGENT                              0
#define HDP_ROLE_MANAGER                            1
#define HDP_ROLE_INACTIVE                           (0xFF)

#define HDPINFOTYPE_ATTRVALUE                       0
#define HDPINFOTYPE_SCANREPORT                      1

#define HDP_SCANREPORT_INFOTYPE_OBJECT              0
#define HDP_SCANREPORT_INFOTYPE_ATTRIBUTE           1

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* Defines for UUIDs used in MAKE_CONNECTION command. And also can be used*/
/* to infer profile mentioned in coming connection event messages*/
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#define UUID_SPP                                    0x1101  /*Serial Port Profil*/
#define UUID_LAP                                    0x1102  /*LAN Access using PPP*/
#define UUID_DUN                                    0x1103  /*Dial Up Networking*/
#define UUID_IRMCSYNC                               0x1104  /*IrMC Sync*/
#define UUID_OBEX_OBJECT_PUSH                       0x1105  /*OBEX Object Push*/
#define UUID_OBEX_FTP                               0x1106  /*OBEX File Transfer*/
#define UUID_IRMCSYNC_CMD                           0x1107  /*IrMC Sync Command*/
#define UUID_HEADSET                                0x1108  /*Headset*/
#define UUID_CTP                                    0x1109  /*Cordless Telephony*/
#define UUID_AUDIO_SOURCE                           0x110A
#define UUID_AUDIO_SINK                             0x110B
#define UUID_INTERCOM                               0x1110
#define UUID_HEADSET_AUDIO_GATEWAY                  0x1112
#define UUID_WAP                                    0x1113
#define UUID_PAN_U                                  0x1115
#define UUID_PAN_NAP                                0x1116
#define UUID_PAN_GN                                 0x1117
#define UUID_HANDSFREE                              0x111E
#define UUID_HANDSFREE_AUDIO_GATEWAY                0x1112
#define UUID_SAP                                    0x112D  /*Sim Access*/
#define UUID_HID                                    0x1124  /* host or device */
#define UUID_HDP                                    0x1400
#define UUID_HDP_SOURCE                             0x1401
#define UUID_HDP_SINK                               0x1402

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* Sizes of various multibyte fields in the host protocol fields*/
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
#define SIZEOF_HOST_FORMAT_BDADDR                   6
#define SIZEOF_HOST_FORMAT_UUID                     2
#define SIZEOF_HOST_FORMAT_DEVCLASS                 3
#define SIZEOF_HOST_FORMAT_SREG_VAL                 4
#define SIZEOF_HOST_FORMAT_BAUDRATE                 4
#define SIZEOF_SCO_HANDLE                           2
#define SIZEOF_ECHO_HANDLE                          2
#define SIZEOF_PCM_HANDLE                           2
#define SIZEOF_FEATURE_LIST                         8
#define SIZEOF_SSP_ACTION_VALUE                     4
#define SIZEOF_LINK_KEY                             16
#define SIZEOF_LINK_KEY_FLAGS                       4
#define SIZEOF_OOB_INFO_C                           16
#define SIZEOF_OOB_INFO_R                           16
#define SIZEOF_UINT16                               2

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* If data is to be sent to all open connections, then the channel field*/
/* in the data packet should be set to this value*/
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
#define CHANNEL_BROADCAST_ALL                       0xFF

#define CHANNELID_HID_DEVICE                        0x20
#define CHANNELID_HID_DEVICE_DONOTUSE               0x80
#define CHANNELID_HID_HOST_FIRST                    0x90
#define CHANNELID_HID_HOST_LAST                     0x97
#define CHANNELID_BLOB_FIRST                        0x98
#define CHANNELID_BLOB_LAST                         0x9F
#define CHANNELID_HID_DEVICE_REPORT                 0xA0
#define CHANNELID_HDP                               0xB0
#define CHANNELID_HDP_CONTINUATION                  0xB1
#define CHANNELID_PANU_FIRST                        0xC0
#define CHANNELID_PANU_FIRST_CONTINUATION           0xC1
#define CHANNELID_PANU_LAST                         0xCC
#define CHANNELID_PANU_LAST_CONTINUATION            0xCD
#define CHANNELID_MISC_EIR_INQ_RESP                 0xF0

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*The following are COMMAND (octet 2) values in command/response packets*/
/*They are guaranteed to be in the range 0x01 to 0x7F (ie bit 7 == 0 )*/
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
#define CMD_NO_OPERATION                            0x01
#define CMD_READ_BDADDR                             0x02
#define CMD_READ_SREG                               0x03
#define CMD_WRITE_SREG                              0x04
#define CMD_STORE_SREG                              0x05
#define CMD_READ_COMMSPARM                          0x06
#define CMD_WRITE_COMMSPARM                         0x07
#define CMD_INQUIRY_REQ                             0x08
#define CMD_DISCOVERABLE_MODE                       0x09
#define CMD_CONNECTABLE_MODE                        0x0A
#define CMD_DROP_CONNECTION                         0x0B
#define CMD_CONTROLMODEMLINES                       0x0C
#define CMD_MAKE_CONNECTION                         0x0D
#define CMD_CONNECTION_SETUP                        0x0E
#define CMD_RSSI_LINKQUAL                           0x0F
#define CMD_PAIR_INITIATE                           0x10
#define CMD_PAIR_ACCEPT                             0x11
#define CMD_TRUSTED_DB_COUNT                        0x12
#define CMD_TRUSTED_DB_READ                         0x13
#define CMD_TRUSTED_DB_DELETE                       0x14
#define CMD_TRUSTED_DB_CHANGETYPE                   0x15
#define CMD_TRUSTED_DB_ISTRUSTED                    0x16
/* Appended at end of list
#define CMD_TRUSTED_DB_ADD                          0x38
*/
#define CMD_INFORMATION                             0x17
#define CMD_SECURITY_MODE                           0x18
#define CMD_GET_REM_FNAME                           0x19
#define CMD_SET_LCL_FNAME                           0x1A
#define CMD_GET_LCL_FNAME                           0x1B
#define CMD_DEFAULT_SREG                            0x1C
#define CMD_GET_MODES                               0x1D
#define CMD_SCO_CONNECT                             0x1E
#define CMD_SCO_DISCONNECT                          0x1F
#define CMD_SCO_INCOMING_SETUP                      0x20
#define CMD_SNIFF_REQUEST                           0x21
#define CMD_PARK_REQUEST                            0x22
#define CMD_PINCODE                                 0x23
#define CMD_GET_IO                                  0x24
#define CMD_SET_IO                                  0x25
#define CMD_STREAM_CONNECT                          0x26
#define CMD_STREAM_DISCONNECT                       0x27
#define CMD_SET_DEVCLASS                            0x28
#define CMD_RESET                                   0x29
#define CMD_CHANNEL_LIST                            0x2A
#define CMD_FACTORYDEFAULT                          0x2B
#define CMD_SET_IO_EX                               0x2C
#define CMD_BLOBMANAGE                              0x2D
#define CMD_HDP_ENDPOINT                            0x2E
#define CMD_HDP_SDPREGISTER                         0x2F
#define CMD_HDP_DISASSOCIATE                        0x30
#define CMD_HDP_ASSOCIATE                           0x31
#define CMD_HDP_BIND                                0x32
#define CMD_HDP_SCANREPORT_FIXED                    0x33
#define CMD_HDP_SCANREPORT_VAR                      0x34
#define CMD_HDP_ATTRIBUTE_READ                      0x35
#define CMD_HDP_ATTRIBUTE_WRITE                     0x36
#define CMD_HDP_SET_TIME                            0x37
#define CMD_TRUSTED_DB_ADD                          0x38
#define CMD_SDP_INQUIRY                             0x39

#define CMD_DEBUGINFO                               0x3F

/* The following are confirmation packets sent to the module */
/* as a result of having received an event, and as such do   */
/* not generate a response from the module                   */
/* All CNF commands have bit 6 set                           */
#define CNF_PACKET_MASK                             0x40

#define CNF_SIMPLE_PAIRING                          0x41
#define CNF_PINCODE                                 0x42
#define CNF_OOBDATA_ACK                             0x43
#define CNF_OOBDATA_NACK                            0x44


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*The following are EVENT (octet 2) values in event packets*/
/*They are guaranteed to be in the range 0x81 to 0xFF (ie bit 7 == 1 )*/
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
#define EVT_STATUS                                  0x81
#define EVT_INVALID_PKTSIZE                         0x82
#define EVT_UNKNOWN_COMMAND                         0x83
#define EVT_INQUIRY_RESULT                          0x84
#define EVT_MODEM_STATUS                            0x85
#define EVT_DISCONNECT                              0x86
#define EVT_CONNECTION_SETUP                        0x87
#define EVT_INCOMING_CONNECTION                     0x88
#define EVT_LINK_KEY                                0x89
#define EVT_LCL_FNAME                               0x8A
#define EVT_REM_FNAME                               0x8B
#define EVT_DEBUG_PACKET                            0x8C
#define EVT_SCO_CONNECT                             0x8D
#define EVT_SCO_DISCONNECT                          0x8E
#define EVT_SCO_INCOMING_SETUP                      0x8F
#define EVT_LOWPOWER_MODE                           0x90
#define EVT_PINCODE_REQUEST                         0x91
#define EVT_ADC                                     0x92
#define EVT_REMOTE_FEATURES                         0x93
#define EVT_PINCODE_EXPIRE                          0x94
#define EVT_SIMPLE_PAIRING                          0x95
#define EVT_HDP_DISASSOCIATED                       0x96
#define EVT_HDP_ASSOCIATED                          0x97
#define EVT_HDP_TIMEUPDATE                          0x98
#define EVT_LINK_KEY_EX                             0x99
#define EVT_OOB_DATA                                0x9A
#define EVT_OOBDATA_REQUEST                         0x9B
#define EVT_SDP_INQUIRY                             0x9C
#define EVT_MEMHEAP_CHOKE                           0x9D

#define EVT_PMALLOC_STAT0                           0xEF


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* Masks used to enable throwing of certain events to the host*/
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
#define EVENT_ENABLEMASK_REMOTE_FEATURES            0x01

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* Masks used to define information bits in CMD_SET_MODEM_LINES command*/
/* and the event EVT_MODEM_STATUS*/
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
#define MODEM_OUT_MASK_DTR                          0x01
#define MODEM_OUT_MASK_RTS                          0x02
#define MODEM_OUT_MASK_DCD                          0x04
#define MODEM_OUT_MASK_RI                           0x08

#define MODEM_IN_MASK_DSR                           0x01
#define MODEM_IN_MASK_CTS                           0x02
#define MODEM_IN_MASK_DCD                           0x04
#define MODEM_IN_MASK_RI                            0x08


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* Values for 'infoType' for command CMD_INFORMATION*/
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
typedef enum {
	  INFORMATION_VERSION                             = 0    /*0*/
	, INFORMATION_MANUFACTURER                               /*1*/
	, INFORMATION_CHIP_DESIGNATION                           /*2*/
	, INFORMATION_PHYSICAL_MEDIUM                            /*3*/
	, INFORMATION_RSSI_RANGE_LIMITS                          /*4*/
	, INFORMATION_ACTUAL_TX_POWER                            /*5*/
	, INFORMATION_MAX_CODEC_OUTPUT_GAIN                      /*6*/

	, INFORMATION_MAX

	, INFORMATION_GET_OOB_DATA                        = 64   /* 0x40 */

	, INFORMATION_HARDWARE_PLATFORM_ID                = 100  /* 0x64 */

	, INFORMATION_AUTHENTICATION_CHIP                 = 149  /* 0x95 */
	, INFORMATION_SYSCONFIG_FLAG                             /* 0x96 */

	, INFORMATION_PMALLOC_FREE_BLOCKS                 = 0xE0 /*224*/
	/* up to EF is used reserved for pmalloc block sizes */
	, INFORMATION_VENDOR_VERSION_0                    = 0xF0 /*240*/
	, INFORMATION_VENDOR_VERSION_1
	, INFORMATION_VENDOR_VERSION_2
	, INFORMATION_VENDOR_VERSION_3
	, INFORMATION_VENDOR_VERSION_4
	, INFORMATION_VENDOR_VERSION_5
	, INFORMATION_VENDOR_VERSION_6
	, INFORMATION_VENDOR_VERSION_7
	, INFORMATION_VENDOR_VERSION_8
	, INFORMATION_VENDOR_VERSION_9
	, INFORMATION_VENDOR_VERSION_10
	, INFORMATION_VENDOR_VERSION_11
	, INFORMATION_VENDOR_VERSION_12
	, INFORMATION_VENDOR_VERSION_13
	, INFORMATION_VENDOR_VERSION_14                          /* 0xFE 254*/

	, INFORMATION_CONTEXT_SIZE                        = 255
}
INFORMATION_TYPE;

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* Values for INFORMATION_PHYSICAL_MEDIUM (infoType) for command CMD_INFORMATION*/
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
#define PHYSICAL_MEDIUM_BLUETOOTH                   0x00
#define PHYSICAL_MEDIUM_WIFI                        0x01
#define PHYSICAL_MEDIUM_ETHERNET                    0x02

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* Values for secMode in command CMD_SECURITY_MODE*/
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
#define SECURITY_MODE_UNSECURE                      0x00
#define SECURITY_MODE_AUTH                          0x01
#define SECURITY_AUTH_ENCRYPT                       0x02
#define SECURITY_NO_CHANGE                          0xFF

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* Values for defining sco packet types*/
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
#define SCO_PACKET_TYPEMASK_HV1_EV3                 0x01
#define SCO_PACKET_TYPEMASK_HV2_EV4                 0x02
#define SCO_PACKET_TYPEMASK_HV3_EV5                 0x04
#define SCO_PACKET_TYPEMASK_ENHANCED                0x08
#define SCO_PACKET_TYPEMASK_ALL                     0x0F

/******************************************************************************/
/* Macros*/
/******************************************************************************/

/******************************************************************************/
/* Forward decleration of class*/
/******************************************************************************/

/******************************************************************************/
/* Simple (non struct/union) Typedefs*/
/******************************************************************************/

/******************************************************************************/
/* Enum Typedefs*/
/******************************************************************************/

/******************************************************************************/
/* Forward declaration of Class, Struct, Unions and Functions*/
/******************************************************************************/

/******************************************************************************/
/* Struct definitions*/
/******************************************************************************/

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	uint8_t                 length;
	uint8_t                 channel;
	uint8_t                 command;
	uint8_t                 flow;
} ATTR_PACKED
BMCOMMAND_HDR;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	uint8_t                 length;
	uint8_t                 channel;
	uint8_t                 command;
	uint8_t                 flow;
	uint8_t                 status;
} ATTR_PACKED
BMRESPONSE_HDR;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	uint8_t                 length;
	uint8_t                 channel;
	uint8_t                 event;
	uint8_t                 flow;
} ATTR_PACKED
BMEVENT_HDR;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	uint8_t                 length;
	uint8_t                 channel;
} ATTR_PACKED
BMDATA_HDR;


/******************************************************************************/
/* COMMAND Structs*/
/******************************************************************************/

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMCOMMAND_HDR           hdr;
} ATTR_PACKED
COMMAND_SIMPLE;

/*=============================================================================*/
/*=============================================================================*/
typedef COMMAND_SIMPLE      COMMAND_UNKNOWN;

/*=============================================================================*/
/*=============================================================================*/
typedef COMMAND_SIMPLE      COMMAND_NO_OPERATION;

/*=============================================================================*/
/*=============================================================================*/
typedef COMMAND_SIMPLE      COMMAND_READ_BDADDR;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMCOMMAND_HDR           hdr;
	uint8_t                 regNo;
} ATTR_PACKED
COMMAND_READ_SREG;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMCOMMAND_HDR           hdr;
	uint8_t                 regNo;
	uint8_t                 regVal[SIZEOF_HOST_FORMAT_SREG_VAL];
} ATTR_PACKED
COMMAND_WRITE_SREG;

/*=============================================================================*/
/*=============================================================================*/
typedef COMMAND_SIMPLE      COMMAND_STORE_SREG;

/*=============================================================================*/
/*=============================================================================*/
typedef COMMAND_SIMPLE      COMMAND_DEFAULT_SREG;

/*=============================================================================*/
/*=============================================================================*/
typedef COMMAND_SIMPLE      COMMAND_READ_COMMSPARM;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMCOMMAND_HDR           hdr;
	uint8_t                 baud[SIZEOF_HOST_FORMAT_BAUDRATE];
	/*1200..921600 baud[0] = MSB*/
	uint8_t                 stopbits;   /*1..2*/
	uint8_t                 parity;     /*0..2*/
	uint8_t                 flow;       /*0..1*/
} ATTR_PACKED
COMMAND_WRITE_COMMSPARM;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMCOMMAND_HDR           hdr;
	uint8_t                 maxResponses;
	uint8_t                 timeout_sec;
	uint8_t                 flags;
} ATTR_PACKED
COMMAND_INQUIRY_REQ;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMCOMMAND_HDR           hdr;
	uint8_t                 enable;
} ATTR_PACKED
COMMAND_DISCOVERABLE_MODE;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMCOMMAND_HDR           hdr;
	uint8_t                 enable;
	uint8_t                 autoAccept; /*bit 0 for channel*/
	/*bit 1 for mux*/
} ATTR_PACKED
COMMAND_CONNECTABLE_MODE;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMCOMMAND_HDR           hdr;
	uint8_t                 hostHandle;
	uint8_t                 bdAddr[SIZEOF_HOST_FORMAT_BDADDR];
	uint8_t                 uuid[SIZEOF_HOST_FORMAT_UUID];
	uint8_t                 instanceIndex;
} ATTR_PACKED
COMMAND_MAKE_CONNECTION;

typedef struct {
	BMCOMMAND_HDR           hdr;
	uint8_t                 hostHandle;
	uint8_t                 bdAddr[SIZEOF_HOST_FORMAT_BDADDR];
	uint8_t                 uuid[SIZEOF_HOST_FORMAT_UUID];
	uint8_t                 RfuA;
	uint8_t                 nSubProtocol;
	uint8_t                 RfuB[2];
} ATTR_PACKED
COMMAND_MAKE_CONNECTION_EX;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMCOMMAND_HDR           hdr;
	uint8_t                 hostHandle;
	uint8_t                 channelId;      /*1..N*/
} ATTR_PACKED
COMMAND_DROP_CONNECTION;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMCOMMAND_HDR           hdr;
	uint8_t                 channelId;      /*1..N*/
	uint8_t                 modemStates;    /*See defines MODEM_OUT_MASK_XXX*/
	uint8_t                 breakSignal;    /*reserved for future - set to 0*/
} ATTR_PACKED
COMMAND_CONTROLMODEMLINES;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMCOMMAND_HDR           hdr;
	uint8_t                 bdAddr[SIZEOF_HOST_FORMAT_BDADDR];
	uint8_t                 hostHandle;
	uint8_t                 fAccept;    /*non-zero to accept the connection*/
} ATTR_PACKED
COMMAND_CONNECTION_SETUP;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMCOMMAND_HDR           hdr;
	uint8_t                 bdAddr[SIZEOF_HOST_FORMAT_BDADDR];
} ATTR_PACKED
COMMAND_RSSI_LINKQUAL;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMCOMMAND_HDR           hdr;
	uint8_t                 timeoutSec;
	uint8_t                 bdAddr[SIZEOF_HOST_FORMAT_BDADDR];
	uint8_t                 pinCode[MAX_PIN_CODE_LENGTH + 1];  /*null terminated. must be at least 1 long*/
} ATTR_PACKED
COMMAND_PAIR_INITIATE;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMCOMMAND_HDR           hdr;
	uint8_t                 timeoutSec;
	uint8_t                 pinCode[MAX_PIN_CODE_LENGTH + 1];  /*null terminated. must be at least 1 long*/
} ATTR_PACKED
COMMAND_PAIR_ACCEPT;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMCOMMAND_HDR           hdr;
	uint8_t                 dbType;
} ATTR_PACKED
COMMAND_TRUSTED_DB_COUNT;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMCOMMAND_HDR           hdr;
	uint8_t                 dbType;
	uint8_t                 itemNo;
} ATTR_PACKED
COMMAND_TRUSTED_DB_READ;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMCOMMAND_HDR           hdr;
	uint8_t                 bdAddr[SIZEOF_HOST_FORMAT_BDADDR];
} ATTR_PACKED
COMMAND_TRUSTED_DB_DELETE;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMCOMMAND_HDR           hdr;
	uint8_t                 dbType;
	uint8_t                 bdAddr[SIZEOF_HOST_FORMAT_BDADDR];
} ATTR_PACKED
COMMAND_TRUSTED_DB_CHANGETYPE;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMCOMMAND_HDR           hdr;
	uint8_t                 bdAddr[SIZEOF_HOST_FORMAT_BDADDR];
} ATTR_PACKED
COMMAND_TRUSTED_DB_ISTRUSTED;

/*=============================================================================*/
/*
**  keyFlags[0] =
**    COMBINATION_KEY                 ((uint8_t)0)
**    LOCAL_UNIT_KEY                  ((uint8_t)1)
**    REMOTE_UNIT_KEY                 ((uint8_t)2)
**    DEBUG_COMBINATION_KEY           ((uint8_t)3)
**    UNAUTHENTICATED_COMBINATION_KEY ((uint8_t)4)
**    AUTHENTICATED_COMBINATION_KEY   ((uint8_t)5)
**    CHANGED_COMBINATION_KEY         ((uint8_t)6)
**    KEY_TYPE_UNKNOWN                ((uint8_t)0xFF)
*/
/*=============================================================================*/
typedef struct {
	BMCOMMAND_HDR           hdr;
	uint8_t                 bdAddr[SIZEOF_HOST_FORMAT_BDADDR];
	uint8_t                 linkKey[SIZEOF_LINK_KEY];
	uint8_t                 keyFlags[SIZEOF_LINK_KEY_FLAGS]; /* [1],[2],[3] reserved for future */
} ATTR_PACKED
COMMAND_TRUSTED_DB_ADD;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMCOMMAND_HDR           hdr;
	uint8_t                 infoReq;    /*see define INFORMATION_XXX*/
} ATTR_PACKED
COMMAND_INFORMATION;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMCOMMAND_HDR           hdr;
	uint8_t                 secMode;    /*see define SECURITY_MODE_XXX*/
} ATTR_PACKED
COMMAND_SECURITY_MODE;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMCOMMAND_HDR           hdr;
	uint8_t                 bdAddr[SIZEOF_HOST_FORMAT_BDADDR];
	uint8_t                 timeout_sec;
	uint8_t                 start;
	uint8_t                 maxOctets;
} ATTR_PACKED
COMMAND_GET_REM_FNAME;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMCOMMAND_HDR           hdr;
	uint8_t                 start;
	uint8_t                 maxOctets;
} ATTR_PACKED
COMMAND_GET_LCL_FNAME;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMCOMMAND_HDR           hdr;
	uint8_t                 flags;      /*Bit 0    : Store in Nonvol Memory*/
	/*Bit 1..7 : Future use*/
	/*At least one bit must be set*/
	uint8_t                 nameLen;
	uint8_t                 name[MAX_LOCAL_FRIENDLY_NAME_SIZE + 1];
} ATTR_PACKED
COMMAND_SET_LCL_FNAME;

/*=============================================================================*/
/*=============================================================================*/
typedef COMMAND_SIMPLE      COMMAND_GET_MODES;


/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMCOMMAND_HDR           hdr;
	uint8_t                 bdAddr[SIZEOF_HOST_FORMAT_BDADDR];
	uint8_t                 packetTypeMask; /*see define SCO_PACKET_TYPEMASK_XXXX*/
} ATTR_PACKED
COMMAND_SCO_CONNECT;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMCOMMAND_HDR           hdr;
	uint8_t                 scoHandle[SIZEOF_SCO_HANDLE];
} ATTR_PACKED
COMMAND_SCO_DISCONNECT;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMCOMMAND_HDR           hdr;
	uint8_t                 bdAddr[SIZEOF_HOST_FORMAT_BDADDR];
	uint8_t                 echoHandle[SIZEOF_ECHO_HANDLE];
	uint8_t                 fAccept;    /*0=reject, 1= Accept SCO, 2= Accept Enhanced SCO*/
} ATTR_PACKED
COMMAND_SCO_INCOMING_SETUP;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMCOMMAND_HDR           hdr;
	uint8_t                 bdAddr[SIZEOF_HOST_FORMAT_BDADDR];
	uint8_t                 attemptMs[2];
	uint8_t                 timeoutMs[2];
	uint8_t                 minIntervalMs[2];
	uint8_t                 maxIntervalMs[2];
} ATTR_PACKED
COMMAND_SNIFF_REQUEST;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMCOMMAND_HDR           hdr;
	uint8_t                 bdAddr[SIZEOF_HOST_FORMAT_BDADDR];
	uint8_t                 minIntervalMs[2];
	uint8_t                 maxIntervalMs[2];   /*max = 40000ms*/
} ATTR_PACKED
COMMAND_PARK_REQUEST;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMCOMMAND_HDR           hdr;
	uint8_t                 bdAddr[SIZEOF_HOST_FORMAT_BDADDR];
	uint8_t                 pinCode[MAX_PIN_CODE_LENGTH + 1];  /*null terminated. must be at least 1 long*/
} ATTR_PACKED
COMMAND_PINCODE;

typedef COMMAND_PINCODE      CONFIRM_PINCODE;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMCOMMAND_HDR           hdr;
	uint8_t                 bdAddr[SIZEOF_HOST_FORMAT_BDADDR];
	uint8_t                 C[SIZEOF_OOB_INFO_C];
	uint8_t                 R[SIZEOF_OOB_INFO_R];
} ATTR_PACKED
CONFIRM_OOBDATA_ACK;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMCOMMAND_HDR           hdr;
	uint8_t                 bdAddr[SIZEOF_HOST_FORMAT_BDADDR];
} ATTR_PACKED
CONFIRM_OOBDATA_NACK;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMCOMMAND_HDR           hdr;
	uint8_t                 digitalId;  /*0 for gpio*/
	uint8_t                 analogId;   /*0=no ADC access, 1 for ADC_1, 2 for ADC_2*/
} ATTR_PACKED
COMMAND_GET_IO;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMCOMMAND_HDR           hdr;
	uint8_t                 ioId;       /*0 for mpio only, 1 for gpio 0x80 for ADC_1, 0x81 for ADC_2*/
	uint8_t                 ioValue[2]; /*0=no ADC access, 1 for ADC_1, 2 for ADC_2*/
} ATTR_PACKED
COMMAND_SET_IO;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMCOMMAND_HDR           hdr;
	uint8_t                 ioId;       /*0 for gpio only*/
	uint8_t                 ioMask[2];
	uint8_t                 ioValue[2];
} ATTR_PACKED
COMMAND_SET_IO_EX;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMCOMMAND_HDR           hdr;
	uint8_t                 streamTypeSource; /* See STREAM_TYPE enum in MpRfComm.h */
	uint8_t                 sourceHandle[2]; /* [0] = msb & [1] = lsb */
	uint8_t                 streamTypeSink; /* See STREAM_TYPE enum in MpRfComm.h */
	uint8_t                 sinkHandle[2]; /* [0] = msb & [1] = lsb */
} ATTR_PACKED
COMMAND_STREAM_CONNECT;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMCOMMAND_HDR           hdr;
	uint8_t                 streamTypeSink; /* See STREAM_TYPE enum in MpRfComm.h */
	uint8_t                 sinkHandle[2]; /* [0] = msb & [1] = lsb */
} ATTR_PACKED
COMMAND_STREAM_DISCONNECT;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMCOMMAND_HDR           hdr;
	uint8_t                 devClass[3]; /* [0] = msb & [2] = lsb */
} ATTR_PACKED
COMMAND_SET_DEVCLASS;

/*=============================================================================*/
/*
 * rebootType = 0 (Cold)
 *            = 1 (Warm)
 */
/*=============================================================================*/
typedef struct {
	BMCOMMAND_HDR           hdr;
	uint8_t                 reserved[3]; /* Always set to 0 */
} ATTR_PACKED
COMMAND_RESET;

/*=============================================================================*/
/*=============================================================================*/
typedef COMMAND_SIMPLE      COMMAND_CHANNEL_LIST;

/*=============================================================================*/
/*
 * flagmask :: For future use (set to FF)
 */
/*=============================================================================*/
typedef struct {
	BMCOMMAND_HDR           hdr;
	uint8_t                 flagmask;
} ATTR_PACKED
COMMAND_FACTORYDEFAULT;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMCOMMAND_HDR           hdr;
	uint8_t                 cmdId;  /* see BLOBMANAGEID_xxx */
	uint8_t                 blobId;
	uint8_t                 parmA[4];
	uint8_t                 parmB[4];
} ATTR_PACKED
COMMAND_BLOBMANAGE;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMCOMMAND_HDR           hdr;
	uint8_t                 specNomCode[2]; /* [0] = msb & [1] = lsb */
	uint8_t                 name[MAX_HDP_SDP_RECORD_NAMELEN + 1]; /* [0] = msb & [1] = lsb */

} ATTR_PACKED
COMMAND_HDP_ENDPOINT;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMCOMMAND_HDR           hdr;

} ATTR_PACKED
COMMAND_HDP_SDPREGISTER;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMCOMMAND_HDR           hdr;
	uint8_t                 handle[2]; /* [0] = msb & [1] = lsb */

} ATTR_PACKED
COMMAND_HDP_DISASSOCIATE;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMCOMMAND_HDR           hdr;
	uint8_t                 handle[2]; /* [0] = msb & [1] = lsb */

} ATTR_PACKED
COMMAND_HDP_ASSOCIATE;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMCOMMAND_HDR           hdr;
	uint8_t                 bdAddr[SIZEOF_HOST_FORMAT_BDADDR];
	uint8_t                 specNomCode[2]; /* [0] = msb & [1] = lsb */
	uint8_t                 assocTimeoutMs[2]; /* [0] = msb & [1] = lsb */

} ATTR_PACKED
COMMAND_HDP_BIND;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMCOMMAND_HDR           hdr;
	uint8_t                 handle[2]; /* [0] = msb & [1] = lsb */
	uint8_t                 personId[2]; /* [0] = msb & [1] = lsb */
	uint8_t                 hostContext;

} ATTR_PACKED
COMMAND_HDP_SCANREPORT_FIXED;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMCOMMAND_HDR           hdr;
	uint8_t                 handle[2]; /* [0] = msb & [1] = lsb */
	uint8_t                 personId[2]; /* [0] = msb & [1] = lsb */
	uint8_t                 hostContext;
	uint8_t                 blobId; /* will contain the list of attr for var report */

} ATTR_PACKED
COMMAND_HDP_SCANREPORT_VAR;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMCOMMAND_HDR           hdr;
	uint8_t                 handle[2]; /* [0] = msb & [1] = lsb */
	uint8_t                 attrNomCode[2]; /* [0] = msb & [1] = lsb */
	uint8_t                 qualifierId[2]; /* [0] = msb & [1] = lsb */
	uint8_t                 hostContext; /* just echoed back */

} ATTR_PACKED
COMMAND_HDP_ATTRIBUTE_READ;

typedef COMMAND_HDP_ATTRIBUTE_READ      COMMAND_HDP_ATTRIBUTE_WRITE;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMCOMMAND_HDR           hdr;
	uint8_t                 handle[2]; /* [0] = msb & [1] = lsb */
	uint8_t                 time[8];   /* cc yy mm dd hh mm ss xx */

} ATTR_PACKED
COMMAND_HDP_SET_TIME;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMCOMMAND_HDR           hdr;
	uint8_t                 bdAddr[SIZEOF_HOST_FORMAT_BDADDR];
	uint8_t                 action; /* See SSP_ACTION_XXXX #defines above */
	uint8_t                 actionValue[SIZEOF_SSP_ACTION_VALUE];
} ATTR_PACKED
CONFIRM_SIMPLE_PAIRING;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMCOMMAND_HDR           hdr;
	uint8_t                 info0;
	uint8_t                 info1;
	/*
	info0 = 0
	    Get pmalloc pool stats. Stats will be sent via EVENT_POOL_STATS
	*/
} ATTR_PACKED
COMMAND_DEBUGINFO;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMCOMMAND_HDR           hdr;
	uint8_t                 bdAddr[SIZEOF_HOST_FORMAT_BDADDR];
	/*
	info0 = 0
	    Get pmalloc pool stats. Stats will be sent via EVENT_POOL_STATS
	*/
} ATTR_PACKED
COMMAND_SDP_INQUIRY;

/*=============================================================================*/
/*=============================================================================*/
typedef union {
	BMCOMMAND_HDR                   header;
	COMMAND_UNKNOWN                 unknown;
	COMMAND_NO_OPERATION            noOperation;
	COMMAND_READ_BDADDR             readBdAddr;
	COMMAND_READ_SREG               readSReg;
	COMMAND_WRITE_SREG              writeSReg;
	COMMAND_STORE_SREG              storeSReg;
	COMMAND_READ_COMMSPARM          readComms;
	COMMAND_WRITE_COMMSPARM         writeComms;
	COMMAND_INQUIRY_REQ             inqReq;
	COMMAND_DISCOVERABLE_MODE       discMode;
	COMMAND_CONNECTABLE_MODE        connMode;
	COMMAND_DROP_CONNECTION         dropConnection;
	COMMAND_CONTROLMODEMLINES       controlMdmLines;
	COMMAND_MAKE_CONNECTION         makeConnection;
	COMMAND_CONNECTION_SETUP        connSetup;
	COMMAND_RSSI_LINKQUAL           rssiLinkQual;
	COMMAND_PAIR_INITIATE           pairInitiate;
	COMMAND_PAIR_ACCEPT             pairAccept;
	COMMAND_TRUSTED_DB_COUNT        trustedCount;
	COMMAND_TRUSTED_DB_READ         trustedRead;
	COMMAND_TRUSTED_DB_DELETE       trustedDelete;
	COMMAND_TRUSTED_DB_CHANGETYPE   trustedChangeType;
	COMMAND_TRUSTED_DB_ISTRUSTED    trustedIsTrusted;
	COMMAND_TRUSTED_DB_ADD          trustedAdd;
	COMMAND_INFORMATION             information;
	COMMAND_SECURITY_MODE           securityMode;
	COMMAND_GET_REM_FNAME           getRemFname;
	COMMAND_GET_LCL_FNAME           getLclFname;
	COMMAND_SET_LCL_FNAME           setLclFname;
	COMMAND_DEFAULT_SREG            defaultSReg;
	COMMAND_GET_MODES               getModes;
	COMMAND_SCO_CONNECT             scoConnect;
	COMMAND_SCO_DISCONNECT          scoDisconnect;
	COMMAND_SCO_INCOMING_SETUP      scoIncomingSetup;
	COMMAND_SNIFF_REQUEST           sniffReq;
	COMMAND_PARK_REQUEST            parkReq;
	COMMAND_PINCODE                 pinCode;
	COMMAND_GET_IO                  getIO;
	COMMAND_SET_IO                  setIO;
	COMMAND_SET_IO_EX               setIOEx;
	COMMAND_STREAM_CONNECT          streamConnect;
	COMMAND_STREAM_DISCONNECT       streamDisconnect;
	COMMAND_SET_DEVCLASS            setDevClass;
	COMMAND_DEBUGINFO               debugInfo;
	COMMAND_RESET                   reset;
	COMMAND_CHANNEL_LIST            channelList;
	COMMAND_FACTORYDEFAULT          factoryDefault;
	COMMAND_BLOBMANAGE              blobManage;
	COMMAND_HDP_ENDPOINT            hdpEndpoint;
	COMMAND_HDP_SDPREGISTER         hdpSdpRegister;
	COMMAND_HDP_DISASSOCIATE        hdpDisassociate;
	COMMAND_HDP_ASSOCIATE           hdpAssociate;
	COMMAND_HDP_BIND                hdpBind;
	COMMAND_HDP_SCANREPORT_FIXED    hdpScanReportFixed;
	COMMAND_HDP_SCANREPORT_VAR      hdpScanReportVar;
	COMMAND_HDP_ATTRIBUTE_READ      hdpAttributeRead;
	COMMAND_HDP_ATTRIBUTE_WRITE     hdpAttributeWrite;
	COMMAND_HDP_SET_TIME            hdpSetTime;

	CONFIRM_SIMPLE_PAIRING          cnfSimplePairing;
	CONFIRM_PINCODE                 cnfPincode;
	CONFIRM_OOBDATA_ACK             cnfOobDataAck;
	CONFIRM_OOBDATA_NACK            cnfOobDataNack;
}
COMMAND_UNION;

/******************************************************************************/
/* RESPONSE Structs*/
/******************************************************************************/

/*=============================================================================*/
/* This struct exists as a generic template*/
/*=============================================================================*/
typedef struct {
	BMRESPONSE_HDR          hdr;
} ATTR_PACKED
RESPONSE_SIMPLE;

/*=============================================================================*/
/* This struct exists as a generic template*/
/*=============================================================================*/
typedef struct {
	BMRESPONSE_HDR          hdr;
	uint8_t                 bdAddr[SIZEOF_HOST_FORMAT_BDADDR];
} ATTR_PACKED
RESPONSE_SIMPLE_BDADDR;

/*=============================================================================*/
/*=============================================================================*/
typedef RESPONSE_SIMPLE     RESPONSE_NO_OPERATION;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMRESPONSE_HDR          hdr;
	uint8_t                 bdAddr[SIZEOF_HOST_FORMAT_BDADDR];
} ATTR_PACKED
RESPONSE_READ_BDADDR;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMRESPONSE_HDR          hdr;
	uint8_t                 regNo;
	uint8_t                 regVal[SIZEOF_HOST_FORMAT_SREG_VAL];  /*[0] is MSB*/
} ATTR_PACKED
RESPONSE_READ_SREG;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMRESPONSE_HDR          hdr;
	uint8_t                 regNo;
	uint8_t                 regVal[SIZEOF_HOST_FORMAT_SREG_VAL];  /*[0] is MSB*/
} ATTR_PACKED
RESPONSE_WRITE_SREG;

/*=============================================================================*/
/*=============================================================================*/
typedef RESPONSE_SIMPLE     RESPONSE_STORE_SREG;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMRESPONSE_HDR          hdr;
	uint8_t                 baud[SIZEOF_HOST_FORMAT_BAUDRATE];
	/*1200..921600 baud[0] = MSB*/
	uint8_t                 stopbits;   /*1..2*/
	uint8_t                 parity;     /*0..2*/
	uint8_t                 flow;       /*1..1*/
} ATTR_PACKED
RESPONSE_READ_COMMSPARM;

/*=============================================================================*/
/*=============================================================================*/
typedef RESPONSE_SIMPLE     RESPONSE_WRITE_COMMSPARM;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMRESPONSE_HDR          hdr;
	uint8_t                 totalResponses;
	uint8_t                 dumpedResponses;
} ATTR_PACKED
RESPONSE_INQUIRY_REQ;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMRESPONSE_HDR          hdr;
	uint8_t                 currentMode; /*0=Off, 1=On, FF=GetCurrentMode*/
} ATTR_PACKED
RESPONSE_DISCOVERABLE_MODE;

/*=============================================================================*/
/*=============================================================================*/
typedef RESPONSE_DISCOVERABLE_MODE RESPONSE_CONNECTABLE_MODE;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMRESPONSE_HDR          hdr;
	uint8_t                 hostHandle;
} ATTR_PACKED
RESPONSE_DROP_CONNECTION;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMRESPONSE_HDR          hdr;
	uint8_t                 channelId;
} ATTR_PACKED
RESPONSE_CONTROLMODEMLINES;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMRESPONSE_HDR          hdr;
	uint8_t                 hostHandle;
	uint8_t                 channelId;      /*1..N*/
} ATTR_PACKED
RESPONSE_MAKE_CONNECTION;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMRESPONSE_HDR          hdr;
	uint8_t                 hostHandle;
} ATTR_PACKED
RESPONSE_CONNECTION_SETUP;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMRESPONSE_HDR          hdr;
	uint8_t                 bdAddr[SIZEOF_HOST_FORMAT_BDADDR];
	uint8_t                 rssi;
	uint8_t                 link_quality;
} ATTR_PACKED
RESPONSE_RSSI_LINKQUAL;

/*=============================================================================*/
/*=============================================================================*/
typedef RESPONSE_SIMPLE     RESPONSE_PAIR_INITIATE;

/*=============================================================================*/
/*=============================================================================*/
typedef RESPONSE_SIMPLE     RESPONSE_PAIR_ACCEPT;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMRESPONSE_HDR          hdr;
	uint8_t                 dbType;
	uint8_t                 count;
	uint8_t                 maxCount;
} ATTR_PACKED
RESPONSE_TRUSTED_DB_COUNT;

/*=============================================================================*/
/*=============================================================================*/
/* The following 2 structs need to have the same command nummber */

typedef struct {
	BMRESPONSE_HDR          hdr;
	uint8_t                 dbType;
	uint8_t                 itemNo;
	uint8_t                 bdAddr[SIZEOF_HOST_FORMAT_BDADDR];
#if defined(LTMODULE_INC_SPP_SUBPROTOCOLS)
	uint8_t                 nDeviceType;
#endif
} ATTR_PACKED
RESPONSE_TRUSTED_DB_READ;

typedef struct {
	BMRESPONSE_HDR          hdr;
	uint8_t                 dbType;
	uint8_t                 itemNo;
	uint8_t                 bdAddr[SIZEOF_HOST_FORMAT_BDADDR];
	uint8_t                 nDeviceType;
} ATTR_PACKED
RESPONSE_TRUSTED_DB_READ_EX;

/*=============================================================================*/
/*=============================================================================*/
typedef RESPONSE_SIMPLE_BDADDR     RESPONSE_TRUSTED_DB_DELETE;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMRESPONSE_HDR          hdr;
	uint8_t                 dbType;
	uint8_t                 bdAddr[SIZEOF_HOST_FORMAT_BDADDR];
} ATTR_PACKED
RESPONSE_TRUSTED_DB_CHANGETYPE;

/*=============================================================================*/
/*=============================================================================*/
typedef RESPONSE_SIMPLE_BDADDR     RESPONSE_TRUSTED_DB_ISTRUSTED;

/*=============================================================================*/
/*=============================================================================*/
typedef RESPONSE_SIMPLE     RESPONSE_TRUSTED_DB_ADD;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMRESPONSE_HDR          hdr;
	uint8_t                 infoReq;        /*see define INFORMATION_XXX*/
	uint8_t                 infoData[MAX_INFORMATION_LEN];    /*number of bytes used as required*/
} ATTR_PACKED
RESPONSE_INFORMATION;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMRESPONSE_HDR          hdr;
	uint8_t                 currSecMode;        /*see define SECURITY_MODE_XXX*/
} ATTR_PACKED
RESPONSE_SECURITY_MODE;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMRESPONSE_HDR          hdr;
	uint8_t                 nameLen;
	uint8_t                 startIndex;
	uint8_t                 sentLen;   /*number of characters sent*/
} ATTR_PACKED
RESPONSE_GET_REM_FNAME;

/*=============================================================================*/
/*=============================================================================*/
typedef RESPONSE_GET_REM_FNAME  RESPONSE_GET_LCL_FNAME;

/*=============================================================================*/
/*=============================================================================*/
typedef RESPONSE_SIMPLE         RESPONSE_SET_LCL_FNAME;

/*=============================================================================*/
/*=============================================================================*/
typedef RESPONSE_SIMPLE         RESPONSE_DEFAULT_SREG;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMRESPONSE_HDR          hdr;
	uint8_t                 discoverable_mode;
	uint8_t                 connectable_mode;
	uint8_t                 security_mode;
} ATTR_PACKED
RESPONSE_GET_MODES;

/*=============================================================================*/
/*=============================================================================*/
typedef RESPONSE_SIMPLE         RESPONSE_SCO_CONNECT;

/*=============================================================================*/
/*=============================================================================*/
typedef RESPONSE_SIMPLE         RESPONSE_SCO_DISCONNECT;

/*=============================================================================*/
/*=============================================================================*/
typedef RESPONSE_SIMPLE         RESPONSE_SCO_INCOMING_SETUP;

/*=============================================================================*/
/*=============================================================================*/
typedef RESPONSE_SIMPLE         RESPONSE_SNIFF_REQUEST;

/*=============================================================================*/
/*=============================================================================*/
typedef RESPONSE_SIMPLE         RESPONSE_PARK_REQUEST;

/*=============================================================================*/
/*=============================================================================*/
typedef RESPONSE_SIMPLE         RESPONSE_PINCODE;


/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMRESPONSE_HDR          hdr;
	uint8_t                 digitalId;  /*echoed from command*/
	uint8_t                 digitalIn[2];  /*[0] is MSB*/
} ATTR_PACKED
RESPONSE_GET_IO;

/*=============================================================================*/
/*=============================================================================*/
typedef RESPONSE_SIMPLE         RESPONSE_SET_IO;

/*=============================================================================*/
/*=============================================================================*/
typedef RESPONSE_SIMPLE         RESPONSE_SET_IO_EX;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMRESPONSE_HDR          hdr;
	uint8_t                 streamTypeSource; /* See STREAM_TYPE enum in MpRfComm.h */
	uint8_t                 sourceHandle[2];
	uint8_t                 streamTypeSink; /* See STREAM_TYPE enum in MpRfComm.h */
	uint8_t                 sinkHandle[2];
} ATTR_PACKED
RESPONSE_STREAM_CONNECT;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMRESPONSE_HDR          hdr;
	uint8_t                 streamTypeSink; /* See STREAM_TYPE enum in MpRfComm.h */
	uint8_t                 sinkHandle[2];
} ATTR_PACKED
RESPONSE_STREAM_DISCONNECT;


/*=============================================================================*/
/*=============================================================================*/
typedef RESPONSE_SIMPLE     RESPONSE_SET_DEVCLASS;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMRESPONSE_HDR          hdr;
	uint8_t                 openChannels; /* count of open channels */
	uint8_t                 channel[MAX_CHANNELS_IN_LIST];
} ATTR_PACKED
RESPONSE_CHANNEL_LIST;

/*=============================================================================*/
/*=============================================================================*/
typedef RESPONSE_SIMPLE     RESPONSE_FACTORYDEFAULT;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMRESPONSE_HDR          hdr;
	uint8_t                 cmdId;      /* echoed from command */
	uint8_t                 blobId;     /* echoed from command */
	uint8_t                 parmA[4];
	uint8_t                 parmB[4];
} ATTR_PACKED
RESPONSE_BLOBMANAGE;

/*=============================================================================*/
/*=============================================================================*/
typedef RESPONSE_SIMPLE     RESPONSE_HDP_ENDPOINT;
typedef RESPONSE_SIMPLE     RESPONSE_HDP_SDPREGISTER;
typedef RESPONSE_SIMPLE     RESPONSE_HDP_DISASSOCIATE;
typedef RESPONSE_SIMPLE     RESPONSE_HDP_ASSOCIATE;
typedef RESPONSE_SIMPLE     RESPONSE_HDP_SET_TIME;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMRESPONSE_HDR          hdr;
	uint8_t                 handle[2]; /* [0] = msb & [1] = lsb */

} ATTR_PACKED
RESPONSE_HDP_BIND;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMRESPONSE_HDR          hdr;
	uint8_t                 handle[2]; /* [0] = msb & [1] = lsb */
	uint8_t                 hostContext;

} ATTR_PACKED
RESPONSE_HDP_SCANREPORT_FIXED;

typedef RESPONSE_HDP_SCANREPORT_FIXED     RESPONSE_HDP_SCANREPORT_VAR;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMRESPONSE_HDR          hdr;
	uint8_t                 hostContext;
	uint8_t                 dataLen[2]; /* [0] = msb & [1] = lsb */

} ATTR_PACKED
RESPONSE_HDP_ATTRIBUTE_READ;

typedef RESPONSE_HDP_ATTRIBUTE_READ     RESPONSE_HDP_ATTRIBUTE_WRITE;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMRESPONSE_HDR          hdr;
	uint8_t                 infoType[2];
	uint8_t                 data[MAX_DEBUGINGO_DATA_BLOCK];
} ATTR_PACKED
RESPONSE_DEBUGINFO;

typedef struct {
	BMRESPONSE_HDR          hdr;
} ATTR_PACKED
RESPONSE_SDP_INQUIRY;

/******************************************************************************/
/* EVENT Structs*/
/******************************************************************************/

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMEVENT_HDR             hdr;
	uint8_t                 status;
	uint8_t                 discoverable_mode;
	uint8_t                 connectable_mode;
	uint8_t                 security_mode;
} ATTR_PACKED
EVENT_STATUS;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMEVENT_HDR             hdr;
	uint8_t                 command;
	uint8_t                 actualSize;
	uint8_t                 requiredSize;
} ATTR_PACKED
EVENT_INVALID_PKTSIZE;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMEVENT_HDR             hdr;
	uint8_t                 command;
} ATTR_PACKED
EVENT_UNKNOWN_CMD;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMEVENT_HDR             hdr;
	uint8_t                 bdAddr[SIZEOF_HOST_FORMAT_BDADDR];
	uint8_t                 devClass[SIZEOF_HOST_FORMAT_DEVCLASS];
} ATTR_PACKED
EVENT_INQUIRY_RESULT;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMEVENT_HDR             hdr;
	uint8_t                 channelId;      /*1..N*/
	uint8_t                 modemStates;    /*See defines MODEM_IN_MASK_XXX*/
	uint8_t                 breakSignal;
} ATTR_PACKED
EVENT_MODEM_STATUS;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMEVENT_HDR             hdr;
	uint8_t                 channelId;      /*1..N*/
	uint8_t                 reason;
} ATTR_PACKED
EVENT_DISCONNECT;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMEVENT_HDR             hdr;
	uint8_t                 bdAddr[SIZEOF_HOST_FORMAT_BDADDR];
	uint8_t                 uuid[SIZEOF_HOST_FORMAT_UUID];
} ATTR_PACKED
EVENT_CONNECTION_SETUP;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMEVENT_HDR             hdr;
	uint8_t                 channelId;      /*1..N*/
	uint8_t                 bdAddr[SIZEOF_HOST_FORMAT_BDADDR];
	uint8_t                 uuid[SIZEOF_HOST_FORMAT_UUID];
} ATTR_PACKED
EVENT_INCOMING_CONNECTION;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMEVENT_HDR             hdr;
	uint8_t                 bdAddr[SIZEOF_HOST_FORMAT_BDADDR];
	uint8_t                 dbResult;
} ATTR_PACKED
EVENT_LINK_KEY;

/*=============================================================================*/
/*
**  linkKeyType =
**    COMBINATION_KEY                 ((uint8_t)0)
**    LOCAL_UNIT_KEY                  ((uint8_t)1)
**    REMOTE_UNIT_KEY                 ((uint8_t)2)
**    DEBUG_COMBINATION_KEY           ((uint8_t)3)
**    UNAUTHENTICATED_COMBINATION_KEY ((uint8_t)4)
**    AUTHENTICATED_COMBINATION_KEY   ((uint8_t)5)
**    CHANGED_COMBINATION_KEY         ((uint8_t)6)
**    KEY_TYPE_UNKNOWN                ((uint8_t)0xFF)
*/
/*=============================================================================*/
typedef struct {
	BMEVENT_HDR             hdr;
	uint8_t                 bdAddr[SIZEOF_HOST_FORMAT_BDADDR];
	uint8_t                 linkKey[SIZEOF_LINK_KEY];
	uint8_t                 linkKeyType;
	uint8_t                 rfu[3];
} ATTR_PACKED
EVENT_LINK_KEY_EX;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMEVENT_HDR             hdr;
	uint8_t                 c[SIZEOF_OOB_INFO_C];
	uint8_t                 r[SIZEOF_OOB_INFO_R];
} ATTR_PACKED
EVENT_OOB_DATA;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMEVENT_HDR             hdr;
	uint8_t                 index;
	uint8_t                 len;
	uint8_t                 data[MAX_FNAME_EVENT_DATA_BLOCK];
} ATTR_PACKED
EVENT_REM_FNAME;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMEVENT_HDR             hdr;
	uint8_t                 index;
	uint8_t                 len;
	uint8_t                 data[MAX_FNAME_EVENT_DATA_BLOCK];
} ATTR_PACKED
EVENT_LCL_FNAME;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMEVENT_HDR             hdr;
	uint8_t                 nFlag;  /*bit0    : First Event Packet*/
	/*bit1    : Last Event Packet*/
	/*bit2..5 : Spare*/
	/*bit6..7 : Debug Type*/
	uint8_t                 data[MAX_ASSERT_EVENT_DATA_BLOCK];
} ATTR_PACKED
EVENT_DEBUG_PACKET;

/* If the following defines are changed then they must also be modified*/
/* in MpPrivate.h*/
#define DEBUGPKT_FIRST                  0x01
#define DEBUGPKT_LAST                   0x02
#define DEBUGTYPE_MASK                  0xC0
#define DEBUGTYPE_ASSERT                0x00
#define DEBUGTYPE_MESSAGE               0x40

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMEVENT_HDR             hdr;
	uint8_t                 status;
	uint8_t                 bdAddr[SIZEOF_HOST_FORMAT_BDADDR];
	uint8_t                 count;
	uint8_t                 scoHandle[SIZEOF_SCO_HANDLE];
	uint8_t                 fIncoming;
} ATTR_PACKED
EVENT_SCO_CONNECT;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMEVENT_HDR             hdr;
	uint8_t                 scoHandle[SIZEOF_SCO_HANDLE];
	uint8_t                 reason;
} ATTR_PACKED
EVENT_SCO_DISCONNECT;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMEVENT_HDR             hdr;
	uint8_t                 bdAddr[SIZEOF_HOST_FORMAT_BDADDR];
	uint8_t                 instance;
	uint8_t                 echoHandle[SIZEOF_ECHO_HANDLE];
} ATTR_PACKED
EVENT_SCO_INCOMING_SETUP;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMEVENT_HDR             hdr;
	uint8_t                 status;
	uint8_t                 bdAddr[SIZEOF_HOST_FORMAT_BDADDR];
	uint8_t                 mode;
} ATTR_PACKED
EVENT_LOWPOWER_MODE;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMEVENT_HDR             hdr;
	uint8_t                 bdAddr[SIZEOF_HOST_FORMAT_BDADDR];
} ATTR_PACKED
EVENT_PINCODE_REQUEST;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMEVENT_HDR             hdr;
	uint8_t                 bdAddr[SIZEOF_HOST_FORMAT_BDADDR];
} ATTR_PACKED
EVENT_OOBDATA_REQUEST;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMEVENT_HDR             hdr;
	uint8_t                 adcId;  /*1...N*/
	uint8_t                 adcValMsb;
	uint8_t                 adcValLsb;
} ATTR_PACKED
EVENT_ADC;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMEVENT_HDR             hdr;
	uint8_t                 bdAddr[SIZEOF_HOST_FORMAT_BDADDR];
	uint8_t                 features[SIZEOF_FEATURE_LIST];
} ATTR_PACKED
EVENT_REMOTE_FEATURES;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMEVENT_HDR             hdr;
} ATTR_PACKED
EVENT_PINCODE_EXPIRE;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMEVENT_HDR             hdr;
	uint8_t                 role; /* see HDP_ROLE_xxx */
	uint8_t                 handle[2];

} ATTR_PACKED
EVENT_HDP_DISASSOCIATED;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMEVENT_HDR             hdr;
	uint8_t                 role; /* see HDP_ROLE_xxx */
	uint8_t                 handle[2];
	uint8_t                 specNomCode[2]; /* [0] = msb & [1] = lsb */
	uint8_t                 deviceConfigId[2]; /* [0] = msb & [1] = lsb */
	uint8_t                 systemId[8];

} ATTR_PACKED
EVENT_HDP_ASSOCIATED;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMEVENT_HDR             hdr;
	uint8_t                 handle[2];
	uint8_t                 time[8];   /* cc yy mm dd hh mm ss xx */
} ATTR_PACKED
EVENT_HDP_TIMEUPDATE;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMEVENT_HDR             hdr;
	uint8_t                 bdAddr[SIZEOF_HOST_FORMAT_BDADDR];
	uint8_t                 action;  /* See SSP_ACTION_XXXX #defines above */
	uint8_t                 actionValue[SIZEOF_SSP_ACTION_VALUE];
} ATTR_PACKED
EVENT_SIMPLE_PAIRING;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMEVENT_HDR             hdr;

	uint8_t                 elsize_msb;
	uint8_t                 elsize_lsb;
	uint8_t                 nels_msb;
	uint8_t                 nels_lsb;
	uint8_t                 ntaken_msb;
	uint8_t                 ntaken_lsb;
	uint8_t                 maxtaken_msb;
	uint8_t                 maxtaken_lsb;
	uint8_t                 overflow_msb;
	uint8_t                 overflow_lsb;

} ATTR_PACKED
EVENT_PMALLOC_STAT0;


/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMEVENT_HDR             hdr;
	uint8_t                 bdAddr[SIZEOF_HOST_FORMAT_BDADDR];
	uint8_t                 nDeviceType;
	uint8_t                 nStatus;
} ATTR_PACKED
EVENT_SDP_INQUIRY;

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMEVENT_HDR             hdr;
	uint8_t                 nBlocks[SIZEOF_UINT16];
} ATTR_PACKED
EVENT_MEMHEAP_CHOKE;

/******************************************************************************/
/* Outgoing message union*/
/******************************************************************************/
typedef union {
	RESPONSE_READ_BDADDR                rspReadBdAddr;
	RESPONSE_READ_SREG                  rspReadSReg;
	RESPONSE_WRITE_SREG                 rspWriteSReg;
	RESPONSE_STORE_SREG                 rspStoreSReg;
	RESPONSE_READ_COMMSPARM             rspReadCommsParm;
	RESPONSE_WRITE_COMMSPARM            rspWriteCommsParm;
	RESPONSE_INQUIRY_REQ                rspInqReq;
	RESPONSE_DISCOVERABLE_MODE          rspDiscoverableMode;
	RESPONSE_CONNECTABLE_MODE           rspConnectableMode;
	RESPONSE_DROP_CONNECTION            rspDropConnection;
	RESPONSE_CONTROLMODEMLINES          rspCtrlModemLines;
	RESPONSE_MAKE_CONNECTION            rspMakeConnection;
	RESPONSE_CONNECTION_SETUP           rspConnectionSetup;
	RESPONSE_RSSI_LINKQUAL              rspRssiLinkQual;
	RESPONSE_PAIR_INITIATE              rspPairInitiate;
	RESPONSE_PAIR_ACCEPT                rspPairAccept;
	RESPONSE_TRUSTED_DB_COUNT           rspDbCount;
	RESPONSE_TRUSTED_DB_READ            rspDbRead;
	RESPONSE_TRUSTED_DB_DELETE          rspDbDelete;
	RESPONSE_TRUSTED_DB_CHANGETYPE      rspDbChangeType;
	RESPONSE_TRUSTED_DB_ISTRUSTED       rspDbIsTrusted;
	RESPONSE_TRUSTED_DB_ADD             rspDbAdd;
	RESPONSE_INFORMATION                rspInformation;
	RESPONSE_SECURITY_MODE              rspSecurityMode;
	RESPONSE_GET_REM_FNAME              rspGetRemFname;
	RESPONSE_GET_LCL_FNAME              rspGetLclFname;
	RESPONSE_SET_LCL_FNAME              rspSetLclFname;
	RESPONSE_DEFAULT_SREG               rspDefaultSReg;
	RESPONSE_GET_MODES                  rspGetModes;
	RESPONSE_SCO_CONNECT                rspScoConnect;
	RESPONSE_GET_IO                     rspGetIo;
	RESPONSE_SET_IO                     rspSetIo;
	RESPONSE_SET_IO_EX                  rspSetIoEx;
	RESPONSE_STREAM_CONNECT             rspStreamConnect;
	RESPONSE_STREAM_DISCONNECT          rspStreamDisConnect;
	RESPONSE_SET_DEVCLASS               rspSetDevClass;
	RESPONSE_DEBUGINFO                  rspDebugInfo;
	RESPONSE_CHANNEL_LIST               rspChannelList;
	RESPONSE_FACTORYDEFAULT             rspFactoryDefault;
	RESPONSE_BLOBMANAGE                 rspBlobManage;
	RESPONSE_HDP_ENDPOINT               rspHdpEndpoint;
	RESPONSE_HDP_SDPREGISTER            rspHdpSdpRegister;
	RESPONSE_HDP_DISASSOCIATE           rspHdpDisassociate;
	RESPONSE_HDP_ASSOCIATE              rspHdpAssociate;
	RESPONSE_HDP_BIND                   rspHdpBind;
	RESPONSE_HDP_SCANREPORT_FIXED       rspHdpScanReportFixed;
	RESPONSE_HDP_SCANREPORT_VAR         rspHdpScanReportVar;
	RESPONSE_HDP_ATTRIBUTE_READ         rspHdpAttributeRead;
	RESPONSE_HDP_ATTRIBUTE_WRITE        rspHdpAttributeWrite;
	RESPONSE_HDP_SET_TIME               rspHdpSetTime;

	EVENT_STATUS                        evtStatus;
	EVENT_INVALID_PKTSIZE               evtInvPktSize;
	EVENT_UNKNOWN_CMD                   evtUnknownCmd;
	EVENT_INQUIRY_RESULT                evtInqResult;
	EVENT_MODEM_STATUS                  evtModemStatus;
	EVENT_DISCONNECT                    evtDisconnect;
	EVENT_CONNECTION_SETUP              evtConnectionSetup;
	EVENT_INCOMING_CONNECTION           evtIncomingConnection;
	EVENT_LINK_KEY                      evtLinkKey;
	EVENT_LINK_KEY_EX                   evtLinkKeyEx;
	EVENT_REM_FNAME                     evtRemFname;
	EVENT_OOB_DATA                      evtOobData;
	EVENT_LCL_FNAME                     evtLclFname;
	EVENT_DEBUG_PACKET                  evtDebug;
	EVENT_SCO_CONNECT                   evtScoConnect;
	EVENT_SCO_DISCONNECT                evtScoDisconnect;
	EVENT_SCO_INCOMING_SETUP            evtScoIncomingSetup;
	EVENT_LOWPOWER_MODE                 evtLowPowerMode;
	EVENT_PINCODE_REQUEST               evtPinCodeRequest;
	EVENT_OOBDATA_REQUEST               evtOobDataRequest;
	EVENT_ADC                           evtAdc;
	EVENT_REMOTE_FEATURES               evtRemoteFeatures;
	EVENT_PINCODE_EXPIRE                evtPinCodeExpire;
	EVENT_SIMPLE_PAIRING                evtSimplePairing;
	EVENT_HDP_DISASSOCIATED             evtHdpDisassociated;
	EVENT_HDP_ASSOCIATED                evtHdpAssociated;
	EVENT_HDP_TIMEUPDATE                evtTimeUpdate;
	EVENT_PMALLOC_STAT0                 evtPmallocStat0;
	EVENT_SDP_INQUIRY                   evtSdpInquiry;
}
RESPONSE_EVENT_UNION;

/******************************************************************************/
/* DATA Structs*/
/******************************************************************************/

/*=============================================================================*/
/*=============================================================================*/
typedef struct {
	BMDATA_HDR              hdr;
	uint8_t                 data[255 - 2];
} ATTR_PACKED
BMDATAPACKET;

/******************************************************************************/
/* Class definitions*/
/******************************************************************************/

/******************************************************************************/
/* Union definitions*/
/******************************************************************************/

/******************************************************************************/
/* Global Functions (API etc) exported for other modules*/
/******************************************************************************/

#endif /* Prevention of multiple inclusion */
/******************************************************************************/
/* END OF FILE*/
/******************************************************************************/

