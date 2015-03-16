/******************************************************************************
**              Copyright (C) 2004 TDK Systems Europe Ltd
**
** Project:     TDK Intelligent Serial Multipoint Module
**
** Module:      MpStatus.h
**
*******************************************************************************/

#if !defined(_MPSTATUS_H)     /* prevent multiple inclusions */
#define _MPSTATUS_H

/******************************************************************************/
/* Status Defines*/
/******************************************************************************/

    /*Status values sent up to the vm app*/
#define MPSTATUS_OK                                 0x00

    /*These are provided by the baseband -- actually HCI STATUS CODES */
#define MPSTATUS_ILLEGAL_COMMAND                    0x01
#define MPSTATUS_NO_CONNECTION                      0x02
#define MPSTATUS_HARDWARE_FAIL                      0x03
#define MPSTATUS_PAGE_TIMEOUT                       0x04
#define MPSTATUS_AUTH_FAIL                          0x05
#define MPSTATUS_KEY_MISSING                        0x06
#define MPSTATUS_MEMORY_FULL                        0x07
#define MPSTATUS_CONN_TIMEOUT                       0x08
#define MPSTATUS_MAX_NR_OF_CONNS                    0x09
#define MPSTATUS_MAX_NR_OF_SCO                      0x0A
#define MPSTATUS_MAX_NR_OF_ACL                      0x0B
#define MPSTATUS_COMMAND_DISALLOWED                 0x0C
#define MPSTATUS_REJ_BY_REMOTE_NO_RES               0x0D
#define MPSTATUS_REJ_BY_REMOTE_SEC                  0x0E
#define MPSTATUS_REJ_BY_REMOTE_PERS                 0x0F
#define MPSTATUS_HOST_TIMEOUT                       0x10
#define MPSTATUS_UNSUPPORTED_FEATURE                0x11
#define MPSTATUS_ILLEGAL_FORMAT                     0x12
#define MPSTATUS_OETC_USER                          0x13
#define MPSTATUS_OETC_LOW_RESOURCE                  0x14
#define MPSTATUS_OETC_POWERING_OFF                  0x15
#define MPSTATUS_CONN_TERM_LOCAL_HOST               0x16
#define MPSTATUS_AUTH_REPEATED                      0x17
#define MPSTATUS_PAIRING_NOT_ALLOWED                0x18
#define MPSTATUS_UNKNOWN_LMP_PDU                    0x19
#define MPSTATUS_UNSUPPORTED_REM_FEATURE            0x1A
#define MPSTATUS_SCO_OFFSET_REJECTED                0x1B
#define MPSTATUS_SCO_INTERVAL_REJECTED              0x1C
#define MPSTATUS_SCO_AIR_MODE_REJECTED              0x1D
#define MPSTATUS_INVALID_LMP_PARAMETERS             0x1E
#define MPSTATUS_UNSPECIFIED                        0x1F
#define MPSTATUS_UNSUPP_LMP_PARAM                   0x20
#define MPSTATUS_ROLE_CHANGE_NOT_ALLOWED            0x21
#define MPSTATUS_LMP_RESPONSE_TIMEOUT               0x22
#define MPSTATUS_LMP_TRANSACTION_COLLISION          0x23
#define MPSTATUS_LMP_PDU_NOT_ALLOWED                0x24
#define MPSTATUS_ENC_MODE_NOT_ACCEPTABLE            0x25
#define MPSTATUS_UNIT_KEY_USED                      0x26
#define MPSTATUS_QOS_NOT_SUPPORTED                  0x27
#define MPSTATUS_INSTANT_PASSED                     0x28
#define MPSTATUS_PAIR_UNIT_KEY_NO_SUPPORT           0x29
#define MPSTATUS_CHANNEL_CLASS_NO_SUPPORT           0x2E
#define MPSTATUS_INSUFFICIENT_SECURITY              0x2F
#define MPSTATUS_PARM_NOT_MAND_RANGE                0x30
#define MPSTATUS_SCATTERMODE_NOT_REQUIRED           0x31
#define MPSTATUS_ROLE_SWITCH_PENDING                0x32
#define MPSTATUS_SCATTERMODE_PARM_CHG_PEND          0x33
#define MPSTATUS_RESERVED_SLOT_VIOLATION            0x34
#define MPSTATUS_ROLE_SWITCH_FAILED                 0x35
#define MPSTATUS_INQ_RESP_DATA_TOO_LARGE            0x36
#define MPSTATUS_SP_NOT_SUPPORTED_BY_HOST           0x37
#define MPSTATUS_HOST_BUSY_PAIRING                  0x38

#define MPSTATUS_HCISTATUS_UNKNOWN                  0x3F

   /*STATUS values pertaining to application library are mapped to >= 0x40*/
#define MPSTATUS_PACKET_INVALID_SIZE                0x40
#define MPSTATUS_INVALID_REGISTER                   0x41
#define MPSTATUS_INVALID_REGVALUE                   0x42
#define MPSTATUS_NONVOL_ERROR                       0x43
#define MPSTATUS_INVALID_COMMPARMS                  0x44
#define MPSTATUS_INVALID_PARAMETER                  0x45
#define MPSTATUS_INVALID_UUID                       0x46
#define MPSTATUS_MAX_CONNECTIONS_ACTIVE             0x47
#define MPSTATUS_INVALID_CHANNELID                  0x48
#define MPSTATUS_CHANNEL_NOT_OPEN                   0x49
#define MPSTATUS_NO_CONNECTION_PENDING              0x4A
#define MPSTATUS_NOT_IDLE                           0x4B
#define MPSTATUS_INVALID_BDADDR                     0x4C
#define MPSTATUS_DB_PSWRITE_FAIL_KEY                0x4D
#define MPSTATUS_DB_PSWRITE_FAIL_INDEX              0x4E
#define MPSTATUS_DB_PSDELETE_FAIL_KEY               0x4F
#define MPSTATUS_DB_LINKKEY_NOT_PRESENT             0x50
#define MPSTATUS_DB_PERSIST_FULL                    0x51
#define MPSTATUS_DB_UNKNOWN_TYPE                    0x52
#define MPSTATUS_DB_INVALID_ITEMNO                  0x53
#define MPSTATUS_DB_NOT_TRUSTED                     0x54
#define MPSTATUS_INVALID_FNAME_LENGTH               0x55
#define MPSTATUS_DB_PSWRITE_FAIL_FNAME              0x56
#define MPSTATUS_INVALID_LICENSE                    0x57
#define MPSTATUS_SET_TXPOWER_ERROR                  0x58
#define MPSTATUS_INVALID_GPIO_NUM                   0x59
#define MPSTATUS_INVALID_AIO_NUM                    0x5A
#define MPSTATUS_INVALID_HID_DEC                    0x5B
#define MPSTATUS_INVALID_HID_DEC_AIO_TIMEOUT        0x5D

   /*STATUS values pertaining to multipoint library are mapped to >= 0x80*/
#define MPSTATUS_FAIL                               0x80
#define MPSTATUS_BUSY_TRY_LATER                     0x81
#define MPSTATUS_SDP_REGISTER_BUSY                  0x82 
#define MPSTATUS_RFC_REGISTER_FAIL                  0x83 
#define MPSTATUS_TOO_MANY_PROFILES                  0x84 
#define MPSTATUS_MALLOC_FAILED                      0x85
#define MPSTATUS_NULL_POINTER                       0x86
#define MPSTATUS_INQUIRY_IN_PROGRESS                0x87
#define MPSTATUS_INQUIRY_NOT_IN_PROGRESS            0x88
#define MPSTATUS_TIMEOUT                            0x89
#define MPSTATUS_CANCELLED                          0x8A
#define MPSTATUS_WRITESCAN_IN_PROGRESS              0x8B
#define MPSTATUS_INVALID_BLOBID                     0x8B  /* intentionally same as MPSTATUS_WRITESCAN_IN_PROGRESS */
#define MPSTATUS_PAIR_IN_PROGRESS                   0x8C
#define MPSTATUS_PAIR_NOT_IN_PROGRESS               0x8D
#define MPSTATUS_UNKNOWN_CHANNEL                    0x8E
#define MPSTATUS_UNKNOWN_MUX                        0x8F
#define MPSTATUS_ATTRLIST_INVALID                   0x8F /* Intentionally same as MPSTATUS_UNKNOWN_MUX */
#define MPSTATUS_GET_RSSI_PENDING                   0x90
#define MPSTATUS_SDPSEARCH_FAIL                     0x91
#define MPSTATUS_SDP_INVALID_RESPONSE_SIZE          0x92
#define MPSTATUS_SDPSERVICE_FAIL                    0x93
#define MPSTATUS_SDP_INSTANCE_MISSING               0x94
#define MPSTATUS_SDP_ATTRIBUTE_FAIL                 0x95
#define MPSTATUS_REMOTE_REFUSAL                     0x96
#define MPSTATUS_ESTABLISH_FAIL                     0x97
#define MPSTATUS_INCORRECT_STATE                    0x98
#define MPSTATUS_NOT_INITIALISED                    0x99
#define MPSTATUS_MUX_FULL                           0x9A
#define MPSTATUS_REGISTER_FAIL                      0x9B
#define MPSTATUS_UNKNOWN_CONFIG                     0x9C
#define MPSTATUS_NOT_IMPLEMENTED                    0x9D
#define MPSTATUS_SCO_BANDWIDTH_FULL                 0x9E
#define MPSTATUS_PEER_NOT_SCO_CAPABLE               0x9F
#define MPSTATUS_INVALID_SCO_HANDLE                 0xA0
#define MPSTATUS_SCO_NOT_OPEN                       0xA1
#define MPSTATUS_STREAM_ERROR                       0xA2
#define MPSTATUS_INVALID_SINK                       0xA3
#define MPSTATUS_INVALID_SOURCE                     0xA4
#define MPSTATUS_INVALID_STREAMTYPE                 0xA5
#define MPSTATUS_INVALID_PCM_SLOT                   0xA6
#define MPSTATUS_NOT_CONNECTABLE                    0xA7
#define MPSTATUS_TO_BE_CODED                        0xA8
#define MPSTATUS_FNAME_WRITE_FAIL                   0xA9
#define MPSTATUS_IAS_NOHANDLE                       0xAA
#define MPSTATUS_IOCTL_UNHANDLED                    0xAB
#define MPSTATUS_NOT_CONNECTED                      0xAC
#define MPSTATUS_NO_CALLBACK                        0xAD
#define MPSTATUS_NOT_ASSOCIATED                     0xAE
#define MPSTATUS_PHDC_FAILED                        0xAF
#define MPSTATUS_PHDC_INSUFFRESOURCE                0xB0
#define MPSTATUS_PHDC_INVALIDPARM                   0xB1
#define MPSTATUS_PHDC_INVALIDSTATE                  0xB2
#define MPSTATUS_PHDC_UNKNOWN                       0xB3
#define MPSTATUS_PHDC_INVALIDHANDLE                 0xB4
#define MPSTATUS_UNKNOWN_SPECIALISATION             0xB5
#define MPSTATUS_OBJECT_CLOSED                      0xB6
#define MPSTATUS_OBJECT_INCOMPLETE                  0xB7
#define MPSTATUS_PROFILE_NOTACTIVE                  0xB8
#define MPSTATUS_ATTR_NOTFOUND                      0xB9
#define MPSTATUS_TOO_MANY_AGENTS                    0xBA
#define MPSTATUS_ALREADY_ASSOCIATED                 0xBB
#define MPSTATUS_HDP_REPORT_ERROR                   0xBC

    /*Invalid Parameter status codes start 0xC0*/
#define MPSTATUS_INVALID_ADDRESS                    0xC0
#define MPSTATUS_INVALID_TIMEOUT                    0xC1
#define MPSTATUS_INVALID_SERVER_CHANNEL             0xC2
#define MPSTATUS_INVALID_FRAMESIZE                  0xC3
#define MPSTATUS_INVALID_SCAN_VALUE                 0xC4
#define MPSTATUS_INVALID_PINCODE                    0xC5
#define MPSTATUS_INVALID_SECURITYMODE               0xC6
#define MPSTATUS_CHANNELID_UNAVAILABLE              0xC7

   /*STATUS values pertaining to CCL l2cap_response_t codes are mapped to >= 0xD1*/
   /* L2CAP_SUCCESS == 0x00 */
#define MPSTATUS_CCL_L2CAP_RESPONSE                 0xD1
#define MPSTATUS_L2CAP_OPEN_PENDING					0xD1
#define MPSTATUS_L2CAP_CONFIG_REJECTED              0xD2
#define MPSTATUS_L2CAP_CONFIG_FAILED                0xD3
#define MPSTATUS_L2CAP_CONFIG_UNKNOWN_OPTIONS       0xD4
#define MPSTATUS_L2CAP_CONFIG_TIMEOUT               0xD5
#define MPSTATUS_L2CAP_CONFIG_UNACCEPTABLE          0xD6
#define MPSTATUS_L2CAP_FLUSH_TO_CANNOT_BE_SET       0xD7
#define MPSTATUS_L2CAP_REJECTED_UNKNOWN_PSM			0xD8
#define MPSTATUS_L2CAP_REJECTED_SECURITY            0xD9
#define MPSTATUS_L2CAP_REJECTED_RESOURCES			0xDA
#define MPSTATUS_L2CAP_FAILED					    0xDB
#define MPSTATUS_L2CAP_PAGE_TIMEOUT                 0xDC
#define MPSTATUS_L2CAP_REMOTE_DEVICE_CLOSED			0xDD
#define MPSTATUS_L2CAP_LINKLOSS                     0xDE
#define MPSTATUS_L2CAP_TIMEOUT				        0xDF
#define MPSTATUS_L2CAP_COMMAND_REJECT				0xE0
#define MPSTATUS_L2CAP_SIGNAL_ERROR		      	    0xE1
#define MPSTATUS_L2CAP_QOS_CANNOT_BE_SET   	    	0xE2
#define MPSTATUS_L2CAP_ECHO_TIMEOUT					0xE3
#define MPSTATUS_L2CAP_ECHO_REJECTED                0xE4
#define MPSTATUS_L2CAP_WRITE_NO_CONNECTION			0xE5
#define MPSTATUS_L2CAP_WRITE_MTU_VIOLATION			0xE6
#define MPSTATUS_L2CAP_READ_FAIL                    0xE7
#define MPSTATUS_L2CAP_READ_INCOMPLETE				0xE8
#define MPSTATUS_L2CAP_GROUP_FAIL					0xE9
#define MPSTATUS_L2CAP_PSM_NOT_REGISTERED			0xEA
#define MPSTATUS_L2CAP_PSM_IN_USE					0xEB
#define MPSTATUS_L2CAP_STRM_FAIL_INFINITE_FLUSH     0xEC

   /*STATUS values pertaining to CCL iasres_t codes are mapped to >= 0xF1*/
   /* IAS_OK == 0x00 */
#define MPSTATUS_CCL_IASRESULT                      0xF1
#define MPSTATUS_IAS_OUTOFMEMORY                    0xF1
#define MPSTATUS_IAS_INVALIDSTATE                   0xF2
#define MPSTATUS_IAS_INVALIDPARAMETER               0xF3
#define MPSTATUS_IAS_MISSINGPARAMETER               0xF4
#define MPSTATUS_IAS_INSTANCELIMIT                  0xF5
#define MPSTATUS_IAS_CONNECTIONLIMIT                0xF6
#define MPSTATUS_IAS_SERVICEDISCOVERYERR            0xF7
#define MPSTATUS_IAS_PAGETIMEOUT                    0xF8
#define MPSTATUS_IAS_REJECTEDSECURITY               0xF9
#define MPSTATUS_IAS_FAIL                           0xFA

                                                
#define MPSTATUS_CONTACT_MANUFACTURER               0xFF


#endif /* Prevention of multiple inclusion */
/******************************************************************************/
/* END OF FILE*/
/******************************************************************************/
