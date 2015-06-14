#include "paramhandler.h"

#define ERROR -1
#define OK 0

cParamHandler::cParamHandler(void) :
	m_orb_set_param(-1),
	m_orb_get_param(-1),

	//TODO? Make target_* modifiable?
	target_system(1),
	target_component(50),

	is_requested(false),
	request_time(0),

	load_request_fail_count(0),
	max_load_request_fail_count(0),
	load_update_timeout(100),
	load_update_fail_count(0),
	max_load_update_fail_count(0),

	loaded_count(0),

	upload_requred(false)
{
	get_param_s.param_index = -1;
	get_param_s.target_system = target_system;
	get_param_s.target_component = target_component;

	set_param_s.target_system = target_system;
	set_param_s.target_component = target_component;
}

cParamHandler::~cParamHandler(void) {
	if(m_orb_get_param >= 0)
		orb_unsubscribe(m_orb_get_param);
	if(m_orb_set_param >= 0)
		orb_unsubscribe(m_orb_set_param);
}

bool cParamHandler::init(void) {
	passed_param_sub = orb_subscribe(ORB_ID(pass_drone_parameter));
	return passed_param_sub != ERROR;
}

void cParamHandler::setupSave(orb_advert_t *orb_cmd, int32_t system_id, int32_t component_id) {
	m_orb_cmd = orb_cmd;

	memset(&save_cmd, 0, sizeof(save_cmd));
	save_cmd.command = VEHICLE_CMD_PREFLIGHT_STORAGE;
	save_cmd.param1 = 1;
	save_cmd.confirmation = false;
	save_cmd.source_system = system_id;
	save_cmd.source_component = component_id;
	save_cmd.target_system = target_system;
	save_cmd.target_component = target_component;
}

void cParamHandler::loadCycle(void) {

	if(is_requested) {
		int res;
		bool updated;

		res = orb_check(passed_param_sub, &updated);
		if(res == -1) {
            //orb check failed
		} else if(updated) {

			ad_param_t *pparam;
			struct pass_drone_param_s param;
			orb_copy(ORB_ID(pass_drone_parameter), passed_param_sub, &param);
			pparam = &paramTable[loaded_count];

			if(!strncmp(param.param_id, pparam->name, sizeof(param.param_id))) {

				pparam->type = (enum param_type)param.param_type;
				switch(pparam->type) {
					case PTYPE_INT:
						pparam->value = *(int*)&param.param_value;
						break;
					case PTYPE_FLOAT:
						pparam->value = param.param_value;
						break;
				}
				pparam->value *= pparam->mul;
				pparam->editing_value = pparam->value;
				loaded_count++;
			} else {
				//Must not happen ?
			}

			is_requested = false;
		} else if((hrt_absolute_time() - request_time) / 10000.0f > load_update_timeout) {
			load_update_fail_count++;
			if(load_update_fail_count >= max_load_update_fail_count) {
				//TODO: Do something?
			}
			is_requested = false;
		}
	} else if(loaded_count < getParamCount()) {
		if(!request((enum param_id)loaded_count)) {
			load_request_fail_count++;
			if(load_request_fail_count >= max_load_request_fail_count) {
				//TODO: Do something?
			}
		}
	}
}

float cParamHandler::get(int id) {
	return paramTable[id].value;
}

bool cParamHandler::request(enum param_id id) {

	strncpy(get_param_s.param_id, paramTable[id].name, sizeof(get_param_s.param_id));

	if (m_orb_get_param < 0) {
		m_orb_get_param = orb_advertise(ORB_ID(get_drone_parameter), &get_param_s);
		is_requested = m_orb_get_param != ERROR;
	} else {
		is_requested = orb_publish(ORB_ID(get_drone_parameter), m_orb_get_param, &get_param_s) == OK;
	}

	if(is_requested) {
		request_time = hrt_absolute_time();
	}

	return is_requested;
}

bool cParamHandler::send(ad_param_t *param, bool upload) {
	bool result;

	strncpy(set_param_s.param_id, param->name, sizeof(set_param_s.param_id));
	set_param_s.param_type = param->type;
	set_param_s.param_value = convertValue(param->editing_value / param->mul, param->type);

	if(m_orb_set_param < 0) {
		m_orb_set_param = orb_advertise(ORB_ID(set_drone_parameter), &set_param_s);
		result = m_orb_set_param != ERROR;
	} else {
		result = orb_publish(ORB_ID(set_drone_parameter), m_orb_set_param, &set_param_s) == OK;
	}

	if(result) {
		param->value = param->editing_value;
		upload_requred = true;
	} else {
		param->editing_value = param->value;
		return false;
	}

	if(upload) {
		return uploadAllParams();
	}

	return true;
}

bool cParamHandler::send(enum param_id id, float new_value, bool upload) {
	ad_param_t *param = &paramTable[id];
	param->editing_value = new_value;
	return send(param, upload);
}

bool cParamHandler::sendCustomParam(const char *name, enum param_type type, float new_value, bool upload) {
	bool result;

	strncpy(set_param_s.param_id, name, sizeof(set_param_s.param_id));
	set_param_s.param_type = type;
	set_param_s.param_value = convertValue(new_value, type);

	if(m_orb_set_param < 0) {
		m_orb_set_param = orb_advertise(ORB_ID(set_drone_parameter), &set_param_s);
		result = m_orb_set_param != ERROR;
	} else {
		result = orb_publish(ORB_ID(set_drone_parameter), m_orb_set_param, &set_param_s) == OK;
	}

	if(result) {
		upload_requred = true;
		if(upload)
			uploadAllParams();
	}

	return result;
}

void cParamHandler::incParam(int param_id) {
	ad_param_t *param = &paramTable[param_id];
	param->editing_value += param->step;
	if(param->editing_value > param->max)
		param->editing_value = param->min;
}

void cParamHandler::decParam(int param_id) {
	ad_param_t *param = &paramTable[param_id];
	param->editing_value -= param->step;
	if(param->editing_value < param->min)
		param->editing_value = param->max;
}

bool cParamHandler::setParam(int param_id) {
	ad_param_t *param = &paramTable[param_id];
	if(param->value == param->editing_value)
		return true;

	if(!send(param))
		return false;

	return true;
}

void cParamHandler::resetParam(int param_id) {
	ad_param_t *param = &paramTable[param_id];
	param->editing_value = param->value;
}

bool cParamHandler::uploadAllParams(void) {
	if(!upload_requred)
		return true;

	if(m_orb_cmd) {
		bool result;
		if(*m_orb_cmd < 0) {
			*m_orb_cmd = orb_advertise(ORB_ID(vehicle_command), &save_cmd);
			result = *m_orb_cmd != ERROR;
		} else {
			result = orb_publish(ORB_ID(vehicle_command), *m_orb_cmd, &save_cmd) == OK;
		}
		upload_requred = !result;
		return result;
	} else {
		return false;
	}
}

float cParamHandler::convertValue(float value, enum param_type type) {
	switch(type) {
		case PTYPE_INT: {
			int int_value = (int)value;
			return *(float*)&int_value;
			break;
		}
		case PTYPE_FLOAT:
		default:
			return value;
			break;
	}
}
