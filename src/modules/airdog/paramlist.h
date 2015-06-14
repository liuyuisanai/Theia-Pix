/*
 * Column 0:		Parameter name
 * Parameter id
 * cv				Current value
 * ev				Editing value (for menu)
 * mul				Multiplier (param is divied by this value on load, and multiplied on saving)
 * Type
 * Min
 * Max
 * Step
 * Display symbols	(for menu)
 */

#define PARAM_LIST \
	X(0,ATT_MAG_DECL,		0, 0, 1.0f,		PTYPE_FLOAT,	    -30.0f,		30.0f,		0.5f,		"DEC" ) \
	X(1,BAT_WARN_LVL,		0, 0, 1.0f,		PTYPE_FLOAT,		0.0f,	    1.0f,		0.02f,		"BT1" ) \
	X(2,BAT_CRIT_LVL,		0, 0, 1.0f,		PTYPE_FLOAT,		0.0f,	    1.0f,		0.02f,		"BT2" ) \
	X(3,BAT_FLAT_LVL,		0, 0, 1.0f,		PTYPE_FLOAT,		0.0f,	    1.0f,		0.02f,		"BT3" ) \
	X(4,FOL_LPF,		    0, 0, 1.0f,		PTYPE_FLOAT,		0.0f,	    1.0f,		0.05f,		"LPF" ) \
	X(5,FOL_RPT_ALT,		0, 0, 1.0f,		PTYPE_INT,		    0,	        1,		    1,		    "RPT" ) \
	X(6,FOL_VEL_FF,		    0, 0, 1.0f,		PTYPE_FLOAT,	    0.0f,       1.0f,	    0.05f,	    " FF" ) \
	X(7,LOI_STEP_LEN,	    0, 0, 1.0f,		PTYPE_FLOAT,	    1.0f,       15.0f,	    1.00f,	    "5TP" ) \
	X(8,MPC_LAND_SPD,	    0, 0, 1.0f,		PTYPE_FLOAT,	    1.0f,       8.0f,	    1.00f,	    "LD5" ) \
	X(9,MPC_TAKEOFF_SPD,    0, 0, 1.0f,		PTYPE_FLOAT,	    1.0f,       8.0f,	    1.00f,	    "T05" ) \
	X(10,NAV_TAKEOFF_ALT,   0, 0, 1.0f,	    PTYPE_FLOAT,	    3.0f,       30.0f,	    1.00f,	    "T0A" ) \
	X(11,RTL_RET_ALT,       0, 0, 1.0f,	    PTYPE_FLOAT,	    5.0f,       40.0f,	    1.00f,	    "RTA" ) \
	X(12,SENS_SON_MIN,		0, 0, 1.0f,		PTYPE_FLOAT,	    0.5f,       4.0f,		0.5f,	    "50A" ) \
	X(13,SENS_SON_ON,		0, 0, 1.0f,		PTYPE_INT,		    0,	        1,		    1,		    " 50" ) \
	X(14,MPC_Z_VEL_MAX,		0, 0, 1.0f,		PTYPE_FLOAT,	    1.0f,       20.0f,	    1.0f,	    "UUE" ) \
	X(15,MPC_XY_VEL_MAX,	0, 0, 1.0f,		PTYPE_FLOAT,	    1.0f,       10.0f,	    1.0f,	    "HUE" ) \
    X(16,NAV_AFOL_MODE,     0, 0, 1.0f,     PTYPE_INT,          0,          2,          1,          "AF0")
//	X(0,	/* FTO */				FOL_TALT_OFF,		0, 0, 1.0f,		PTYPE_INT,		0,		1,		1,		"FTO" ) \
	X(1,	/* FRA */				FOL_RPT_ALT,		0, 0, 1.0f,		PTYPE_INT,		0,		1,		1,		"FRA" ) \
	X(2,	/* FUA */				FOL_USE_ALT,		0, 0, 1.0f,		PTYPE_INT,		0,		1,		1,		"FUA" ) \
	X(3,	/* FF */				FOL_VEL_FF,			0, 0, 1.0f,		PTYPE_FLOAT,	0.0f,	1.0f,	0.05f,	" FF" ) \
	X(4,	/* LPF */				FOL_LPF,			0, 0, 1.0f,		PTYPE_FLOAT,	0.0f,	10.0f,	0.05f,	"LPF" )

	/*								Parameter id		cv ev mul		Type			Min		Max		Step	Display symbols */\
//	X(0,	/* AYAW */				MPC_YAW_OFF,		0, 0, 1.0f,		PTYPE_INT,		0,		1,		1,		"AYA" ) \
	X(1,	/* PI */				NAV_TALT_USE,		0, 0, 1.0f,		PTYPE_INT,		0,		1,		1,		" P1" ) \
	X(2,	/* BA */				NAV_TALT_RPT,		0, 0, 1.0f,		PTYPE_INT,		0,		1,		1,		" BA" ) \
	X(3,	/* FF */				MPC_FW_FF,			0, 0, 1.0f,		PTYPE_FLOAT,	0.0f,	1.0f,	0.05f,	" FF" ) \
	X(4,	/* LPF */				MPC_FW_LPF,			0, 0, 1.0f,		PTYPE_FLOAT,	0.0f,	10.0f,	0.05f,	"LPF" ) \
	X(5,	/* MD */				ATT_MAG_DECL,		0, 0, 1.0f,		PTYPE_FLOAT,	-99.0f,	99.0f,	1.0f,	"DEC" ) \
	X(6,	/* INAV_W_Z_GPS_P */	INAV_W_Z_GPS_P,		0, 0, 1000.0f,	PTYPE_FLOAT,	0.0f,	5.0f,	0.1f,	"6P5" ) \
	X(7,	/* MPC_FW_ALT_OFF */	MPC_FW_ALT_OFF,		0, 0, 1.0f,		PTYPE_FLOAT,	-100.0f,100.0f,	1.0f,	"AL0" ) \
	X(8,	/* MPC_FW_MAX_YAW */	MPC_FW_MAX_YAW,		0, 0, 1.0f,		PTYPE_FLOAT,	45.0f,	180.0f,	5.0f,	" YA" ) \
	X(9,	/* MPC_FW_MIN_DIST */	MPC_FW_MIN_DIST,	0, 0, 1.0f,		PTYPE_FLOAT,	0.0f,	50.0f,	1.0f,	"D15" ) \
	X(10,	/* MPC_THR_MIN */		MPC_THR_MIN,		0, 0, 1.0f,		PTYPE_FLOAT,	0.0f,	0.9f,	0.05f,	"THR" ) \
	X(11,	/* MPC_TILTMAX_AIR */	MPC_TILTMAX_AIR,	0, 0, 1.0f,		PTYPE_FLOAT,	15.0f,	60.0f,	5.0f,	"T1A" ) \
	X(12,	/* MPC_TILTMAX_LND */	MPC_TILTMAX_LND,	0, 0, 1.0f,		PTYPE_FLOAT,	10.0f,	45.0f,	5.0f,	"T1L" ) \
	X(13,	/* MPC_Z_VEL_MAX */		MPC_Z_VEL_MAX,		0, 0, 1.0f,		PTYPE_FLOAT,	1.0f,	10.0f,	0.5f,	"2UE" ) \
	X(14,	/* MPC_XY_VEL_MAX */	MPC_XY_VEL_MAX,		0, 0, 1.0f,		PTYPE_FLOAT,	1.0f,	50.0f,	1.0f,	"YUE" ) \
	X(15,	/* NAV_ACCEPT_RAD */	NAV_ACCEPT_RAD,		0, 0, 1.0f,		PTYPE_FLOAT,	1.0f,	100.0f,	5.0f,	"RAD" ) \
	X(16,	/* NAV_FOL_RTL_TO */	NAV_FOL_RTL_TO,		0, 0, 1.0f,		PTYPE_FLOAT,	5.0f,	50.0f,	1.0f,	"RT0" ) \
	X(17,	/* ALTNAV_TAKEOFF_ALT */	NAV_TAKEOFF_ALT,	0, 0, 1.0f,		PTYPE_FLOAT, 	3.0f,	50.0f,	1.0f,	"TAL" ) \
	X(18,	/* NAV_RTL_ALT */		NAV_RTL_ALT,		0, 0, 1.0f,		PTYPE_FLOAT,	3.0f,	50.0f,	1.0f,	"RAL" ) \
	X(19,	/* NAV_LAND_HOME */		NAV_LAND_HOME,		0, 0, 1.0f,		PTYPE_INT,		0,		1,		1,		"LAH" )

#define PARAM_COUNT 17

#define X(n, id, cv, ev, mul, type, min, max, step, dsym) PARAM_##id = n,
enum param_id {
	PARAM_LIST
};
#undef X
