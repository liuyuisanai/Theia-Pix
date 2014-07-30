#ifndef PARAMHANDLER_H
#define PARAMHANDLER_H

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <uORB/uORB.h>
#include <uORB/topics/set_drone_parameter.h>
#include <uORB/topics/get_drone_parameter.h>
#include <uORB/topics/pass_drone_parameter.h>
#include <uORB/topics/vehicle_command.h>
#include <drivers/drv_hrt.h>
#include <systemlib/err.h>
#include "common.h"
#include "paramlist.h"

#define PARAM_REQUEST_DELAY 100

enum param_type {
	PTYPE_INT = 6,
	PTYPE_FLOAT = 9,
};

typedef struct {
	const char *name;
	float value;
	float editing_value;
	float mul;
	enum param_type type;
	float min;
	float max;
	float step;
	const char *display_name;
} ad_param_t;


class cParamHandler
{
public:
	cParamHandler(void);
	~cParamHandler(void);

	bool init(void);
	void setupSave(orb_advert_t *orb_cmd, int32_t system_id, int32_t component_id);

	void loadCycle(void);

	float get(int id);

	bool request(enum param_id id);
	bool send(ad_param_t *param, bool upload = false);
	bool send(enum param_id id, float new_value, bool upload = false);
	bool sendCustomParam(const char *name, enum param_type type, float new_value, bool upload);

	//Menu controls
	inline bool allParamsAreLoaded(void) { return loaded_count == PARAM_COUNT; }
	inline int getParamCount(void) { return PARAM_COUNT; }

	void incParam(int param_id);
	void decParam(int param_id);
	bool setParam(int param_id);
	void resetParam(int param_id);

	inline const char *getDisplaySymbols(int param_id) { return paramTable[param_id].display_name; }
	inline float getEditingValue(int param_id) { return paramTable[param_id].editing_value; }
	inline enum param_type getEditingType(int param_id) { return paramTable[param_id].type; }

	bool uploadAllParams(void);

private:
	float convertValue(float value, enum param_type type);

	struct get_drone_param_s get_param_s;
	struct set_drone_param_s set_param_s;
	struct vehicle_command_s save_cmd;

	orb_advert_t *m_orb_cmd;
	orb_advert_t m_orb_set_param;
	orb_advert_t m_orb_get_param;

	int passed_param_sub;

	const int target_system;
	const int target_component;

	bool is_requested;
	uint64_t request_time;
	int load_request_fail_count;
	int max_load_request_fail_count;
	float load_update_timeout;
	int load_update_fail_count;
	int max_load_update_fail_count;

	int loaded_count;

	bool upload_requred;

	#define X(n, id, cv, ev, mul, type, min, max, step, dsym) { #id, cv, ev, mul, type, min, max, step, dsym },
	ad_param_t paramTable[PARAM_COUNT] = { PARAM_LIST };
	#undef X
};

#undef PARAM_LIST //after paramlist.h

#endif // PARAMHANDLER_H
