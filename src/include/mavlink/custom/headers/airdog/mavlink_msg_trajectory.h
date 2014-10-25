// MESSAGE TRAJECTORY PACKING

#define MAVLINK_MSG_ID_TRAJECTORY 180

typedef struct __mavlink_trajectory_t
{
 uint32_t time_boot_ms; ///< Timestamp (milliseconds since system boot)
 int32_t lat; ///< Latitude, expressed as * 1E7
 int32_t lon; ///< Longitude, expressed as * 1E7
 int32_t alt; ///< Altitude in meters, expressed as * 1000 (millimeters), WGS84 (not AMSL)
 int32_t relative_alt; ///< Altitude above ground in meters, expressed as * 1000 (millimeters)
 int16_t vx; ///< Ground X Speed (Latitude), expressed as m/s * 100
 int16_t vy; ///< Ground Y Speed (Longitude), expressed as m/s * 100
 int16_t vz; ///< Ground Z Speed (Altitude), expressed as m/s * 100
 uint16_t hdg; ///< Compass heading in degrees * 100, 0.0..359.99 degrees. If unknown, set to: UINT16_MAX
 uint8_t point_type; ///< Indicates type of the trajectory reference point. Currently 0 is for "still point" and 1 is for "valid point".
} mavlink_trajectory_t;

#define MAVLINK_MSG_ID_TRAJECTORY_LEN 29
#define MAVLINK_MSG_ID_180_LEN 29

#define MAVLINK_MSG_ID_TRAJECTORY_CRC 86
#define MAVLINK_MSG_ID_180_CRC 86



#define MAVLINK_MESSAGE_INFO_TRAJECTORY { \
	"TRAJECTORY", \
	10, \
	{  { "time_boot_ms", NULL, MAVLINK_TYPE_UINT32_T, 0, 0, offsetof(mavlink_trajectory_t, time_boot_ms) }, \
         { "lat", NULL, MAVLINK_TYPE_INT32_T, 0, 4, offsetof(mavlink_trajectory_t, lat) }, \
         { "lon", NULL, MAVLINK_TYPE_INT32_T, 0, 8, offsetof(mavlink_trajectory_t, lon) }, \
         { "alt", NULL, MAVLINK_TYPE_INT32_T, 0, 12, offsetof(mavlink_trajectory_t, alt) }, \
         { "relative_alt", NULL, MAVLINK_TYPE_INT32_T, 0, 16, offsetof(mavlink_trajectory_t, relative_alt) }, \
         { "vx", NULL, MAVLINK_TYPE_INT16_T, 0, 20, offsetof(mavlink_trajectory_t, vx) }, \
         { "vy", NULL, MAVLINK_TYPE_INT16_T, 0, 22, offsetof(mavlink_trajectory_t, vy) }, \
         { "vz", NULL, MAVLINK_TYPE_INT16_T, 0, 24, offsetof(mavlink_trajectory_t, vz) }, \
         { "hdg", NULL, MAVLINK_TYPE_UINT16_T, 0, 26, offsetof(mavlink_trajectory_t, hdg) }, \
         { "point_type", NULL, MAVLINK_TYPE_UINT8_T, 0, 28, offsetof(mavlink_trajectory_t, point_type) }, \
         } \
}


/**
 * @brief Pack a trajectory message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param point_type Indicates type of the trajectory reference point. Currently 0 is for "still point" and 1 is for "valid point".
 * @param time_boot_ms Timestamp (milliseconds since system boot)
 * @param lat Latitude, expressed as * 1E7
 * @param lon Longitude, expressed as * 1E7
 * @param alt Altitude in meters, expressed as * 1000 (millimeters), WGS84 (not AMSL)
 * @param relative_alt Altitude above ground in meters, expressed as * 1000 (millimeters)
 * @param vx Ground X Speed (Latitude), expressed as m/s * 100
 * @param vy Ground Y Speed (Longitude), expressed as m/s * 100
 * @param vz Ground Z Speed (Altitude), expressed as m/s * 100
 * @param hdg Compass heading in degrees * 100, 0.0..359.99 degrees. If unknown, set to: UINT16_MAX
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_trajectory_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
						       uint8_t point_type, uint32_t time_boot_ms, int32_t lat, int32_t lon, int32_t alt, int32_t relative_alt, int16_t vx, int16_t vy, int16_t vz, uint16_t hdg)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[MAVLINK_MSG_ID_TRAJECTORY_LEN];
	_mav_put_uint32_t(buf, 0, time_boot_ms);
	_mav_put_int32_t(buf, 4, lat);
	_mav_put_int32_t(buf, 8, lon);
	_mav_put_int32_t(buf, 12, alt);
	_mav_put_int32_t(buf, 16, relative_alt);
	_mav_put_int16_t(buf, 20, vx);
	_mav_put_int16_t(buf, 22, vy);
	_mav_put_int16_t(buf, 24, vz);
	_mav_put_uint16_t(buf, 26, hdg);
	_mav_put_uint8_t(buf, 28, point_type);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_TRAJECTORY_LEN);
#else
	mavlink_trajectory_t packet;
	packet.time_boot_ms = time_boot_ms;
	packet.lat = lat;
	packet.lon = lon;
	packet.alt = alt;
	packet.relative_alt = relative_alt;
	packet.vx = vx;
	packet.vy = vy;
	packet.vz = vz;
	packet.hdg = hdg;
	packet.point_type = point_type;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_TRAJECTORY_LEN);
#endif

	msg->msgid = MAVLINK_MSG_ID_TRAJECTORY;
#if MAVLINK_CRC_EXTRA
    return mavlink_finalize_message(msg, system_id, component_id, MAVLINK_MSG_ID_TRAJECTORY_LEN, MAVLINK_MSG_ID_TRAJECTORY_CRC);
#else
    return mavlink_finalize_message(msg, system_id, component_id, MAVLINK_MSG_ID_TRAJECTORY_LEN);
#endif
}

/**
 * @brief Pack a trajectory message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param point_type Indicates type of the trajectory reference point. Currently 0 is for "still point" and 1 is for "valid point".
 * @param time_boot_ms Timestamp (milliseconds since system boot)
 * @param lat Latitude, expressed as * 1E7
 * @param lon Longitude, expressed as * 1E7
 * @param alt Altitude in meters, expressed as * 1000 (millimeters), WGS84 (not AMSL)
 * @param relative_alt Altitude above ground in meters, expressed as * 1000 (millimeters)
 * @param vx Ground X Speed (Latitude), expressed as m/s * 100
 * @param vy Ground Y Speed (Longitude), expressed as m/s * 100
 * @param vz Ground Z Speed (Altitude), expressed as m/s * 100
 * @param hdg Compass heading in degrees * 100, 0.0..359.99 degrees. If unknown, set to: UINT16_MAX
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_trajectory_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
							   mavlink_message_t* msg,
						           uint8_t point_type,uint32_t time_boot_ms,int32_t lat,int32_t lon,int32_t alt,int32_t relative_alt,int16_t vx,int16_t vy,int16_t vz,uint16_t hdg)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[MAVLINK_MSG_ID_TRAJECTORY_LEN];
	_mav_put_uint32_t(buf, 0, time_boot_ms);
	_mav_put_int32_t(buf, 4, lat);
	_mav_put_int32_t(buf, 8, lon);
	_mav_put_int32_t(buf, 12, alt);
	_mav_put_int32_t(buf, 16, relative_alt);
	_mav_put_int16_t(buf, 20, vx);
	_mav_put_int16_t(buf, 22, vy);
	_mav_put_int16_t(buf, 24, vz);
	_mav_put_uint16_t(buf, 26, hdg);
	_mav_put_uint8_t(buf, 28, point_type);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_TRAJECTORY_LEN);
#else
	mavlink_trajectory_t packet;
	packet.time_boot_ms = time_boot_ms;
	packet.lat = lat;
	packet.lon = lon;
	packet.alt = alt;
	packet.relative_alt = relative_alt;
	packet.vx = vx;
	packet.vy = vy;
	packet.vz = vz;
	packet.hdg = hdg;
	packet.point_type = point_type;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_TRAJECTORY_LEN);
#endif

	msg->msgid = MAVLINK_MSG_ID_TRAJECTORY;
#if MAVLINK_CRC_EXTRA
    return mavlink_finalize_message_chan(msg, system_id, component_id, chan, MAVLINK_MSG_ID_TRAJECTORY_LEN, MAVLINK_MSG_ID_TRAJECTORY_CRC);
#else
    return mavlink_finalize_message_chan(msg, system_id, component_id, chan, MAVLINK_MSG_ID_TRAJECTORY_LEN);
#endif
}

/**
 * @brief Encode a trajectory struct
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param trajectory C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_trajectory_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_trajectory_t* trajectory)
{
	return mavlink_msg_trajectory_pack(system_id, component_id, msg, trajectory->point_type, trajectory->time_boot_ms, trajectory->lat, trajectory->lon, trajectory->alt, trajectory->relative_alt, trajectory->vx, trajectory->vy, trajectory->vz, trajectory->hdg);
}

/**
 * @brief Encode a trajectory struct on a channel
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param trajectory C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_trajectory_encode_chan(uint8_t system_id, uint8_t component_id, uint8_t chan, mavlink_message_t* msg, const mavlink_trajectory_t* trajectory)
{
	return mavlink_msg_trajectory_pack_chan(system_id, component_id, chan, msg, trajectory->point_type, trajectory->time_boot_ms, trajectory->lat, trajectory->lon, trajectory->alt, trajectory->relative_alt, trajectory->vx, trajectory->vy, trajectory->vz, trajectory->hdg);
}

/**
 * @brief Send a trajectory message
 * @param chan MAVLink channel to send the message
 *
 * @param point_type Indicates type of the trajectory reference point. Currently 0 is for "still point" and 1 is for "valid point".
 * @param time_boot_ms Timestamp (milliseconds since system boot)
 * @param lat Latitude, expressed as * 1E7
 * @param lon Longitude, expressed as * 1E7
 * @param alt Altitude in meters, expressed as * 1000 (millimeters), WGS84 (not AMSL)
 * @param relative_alt Altitude above ground in meters, expressed as * 1000 (millimeters)
 * @param vx Ground X Speed (Latitude), expressed as m/s * 100
 * @param vy Ground Y Speed (Longitude), expressed as m/s * 100
 * @param vz Ground Z Speed (Altitude), expressed as m/s * 100
 * @param hdg Compass heading in degrees * 100, 0.0..359.99 degrees. If unknown, set to: UINT16_MAX
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_trajectory_send(mavlink_channel_t chan, uint8_t point_type, uint32_t time_boot_ms, int32_t lat, int32_t lon, int32_t alt, int32_t relative_alt, int16_t vx, int16_t vy, int16_t vz, uint16_t hdg)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[MAVLINK_MSG_ID_TRAJECTORY_LEN];
	_mav_put_uint32_t(buf, 0, time_boot_ms);
	_mav_put_int32_t(buf, 4, lat);
	_mav_put_int32_t(buf, 8, lon);
	_mav_put_int32_t(buf, 12, alt);
	_mav_put_int32_t(buf, 16, relative_alt);
	_mav_put_int16_t(buf, 20, vx);
	_mav_put_int16_t(buf, 22, vy);
	_mav_put_int16_t(buf, 24, vz);
	_mav_put_uint16_t(buf, 26, hdg);
	_mav_put_uint8_t(buf, 28, point_type);

#if MAVLINK_CRC_EXTRA
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_TRAJECTORY, buf, MAVLINK_MSG_ID_TRAJECTORY_LEN, MAVLINK_MSG_ID_TRAJECTORY_CRC);
#else
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_TRAJECTORY, buf, MAVLINK_MSG_ID_TRAJECTORY_LEN);
#endif
#else
	mavlink_trajectory_t packet;
	packet.time_boot_ms = time_boot_ms;
	packet.lat = lat;
	packet.lon = lon;
	packet.alt = alt;
	packet.relative_alt = relative_alt;
	packet.vx = vx;
	packet.vy = vy;
	packet.vz = vz;
	packet.hdg = hdg;
	packet.point_type = point_type;

#if MAVLINK_CRC_EXTRA
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_TRAJECTORY, (const char *)&packet, MAVLINK_MSG_ID_TRAJECTORY_LEN, MAVLINK_MSG_ID_TRAJECTORY_CRC);
#else
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_TRAJECTORY, (const char *)&packet, MAVLINK_MSG_ID_TRAJECTORY_LEN);
#endif
#endif
}

#if MAVLINK_MSG_ID_TRAJECTORY_LEN <= MAVLINK_MAX_PAYLOAD_LEN
/*
  This varient of _send() can be used to save stack space by re-using
  memory from the receive buffer.  The caller provides a
  mavlink_message_t which is the size of a full mavlink message. This
  is usually the receive buffer for the channel, and allows a reply to an
  incoming message with minimum stack space usage.
 */
static inline void mavlink_msg_trajectory_send_buf(mavlink_message_t *msgbuf, mavlink_channel_t chan,  uint8_t point_type, uint32_t time_boot_ms, int32_t lat, int32_t lon, int32_t alt, int32_t relative_alt, int16_t vx, int16_t vy, int16_t vz, uint16_t hdg)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char *buf = (char *)msgbuf;
	_mav_put_uint32_t(buf, 0, time_boot_ms);
	_mav_put_int32_t(buf, 4, lat);
	_mav_put_int32_t(buf, 8, lon);
	_mav_put_int32_t(buf, 12, alt);
	_mav_put_int32_t(buf, 16, relative_alt);
	_mav_put_int16_t(buf, 20, vx);
	_mav_put_int16_t(buf, 22, vy);
	_mav_put_int16_t(buf, 24, vz);
	_mav_put_uint16_t(buf, 26, hdg);
	_mav_put_uint8_t(buf, 28, point_type);

#if MAVLINK_CRC_EXTRA
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_TRAJECTORY, buf, MAVLINK_MSG_ID_TRAJECTORY_LEN, MAVLINK_MSG_ID_TRAJECTORY_CRC);
#else
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_TRAJECTORY, buf, MAVLINK_MSG_ID_TRAJECTORY_LEN);
#endif
#else
	mavlink_trajectory_t *packet = (mavlink_trajectory_t *)msgbuf;
	packet->time_boot_ms = time_boot_ms;
	packet->lat = lat;
	packet->lon = lon;
	packet->alt = alt;
	packet->relative_alt = relative_alt;
	packet->vx = vx;
	packet->vy = vy;
	packet->vz = vz;
	packet->hdg = hdg;
	packet->point_type = point_type;

#if MAVLINK_CRC_EXTRA
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_TRAJECTORY, (const char *)packet, MAVLINK_MSG_ID_TRAJECTORY_LEN, MAVLINK_MSG_ID_TRAJECTORY_CRC);
#else
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_TRAJECTORY, (const char *)packet, MAVLINK_MSG_ID_TRAJECTORY_LEN);
#endif
#endif
}
#endif

#endif

// MESSAGE TRAJECTORY UNPACKING


/**
 * @brief Get field point_type from trajectory message
 *
 * @return Indicates type of the trajectory reference point. Currently 0 is for "still point" and 1 is for "valid point".
 */
static inline uint8_t mavlink_msg_trajectory_get_point_type(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint8_t(msg,  28);
}

/**
 * @brief Get field time_boot_ms from trajectory message
 *
 * @return Timestamp (milliseconds since system boot)
 */
static inline uint32_t mavlink_msg_trajectory_get_time_boot_ms(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint32_t(msg,  0);
}

/**
 * @brief Get field lat from trajectory message
 *
 * @return Latitude, expressed as * 1E7
 */
static inline int32_t mavlink_msg_trajectory_get_lat(const mavlink_message_t* msg)
{
	return _MAV_RETURN_int32_t(msg,  4);
}

/**
 * @brief Get field lon from trajectory message
 *
 * @return Longitude, expressed as * 1E7
 */
static inline int32_t mavlink_msg_trajectory_get_lon(const mavlink_message_t* msg)
{
	return _MAV_RETURN_int32_t(msg,  8);
}

/**
 * @brief Get field alt from trajectory message
 *
 * @return Altitude in meters, expressed as * 1000 (millimeters), WGS84 (not AMSL)
 */
static inline int32_t mavlink_msg_trajectory_get_alt(const mavlink_message_t* msg)
{
	return _MAV_RETURN_int32_t(msg,  12);
}

/**
 * @brief Get field relative_alt from trajectory message
 *
 * @return Altitude above ground in meters, expressed as * 1000 (millimeters)
 */
static inline int32_t mavlink_msg_trajectory_get_relative_alt(const mavlink_message_t* msg)
{
	return _MAV_RETURN_int32_t(msg,  16);
}

/**
 * @brief Get field vx from trajectory message
 *
 * @return Ground X Speed (Latitude), expressed as m/s * 100
 */
static inline int16_t mavlink_msg_trajectory_get_vx(const mavlink_message_t* msg)
{
	return _MAV_RETURN_int16_t(msg,  20);
}

/**
 * @brief Get field vy from trajectory message
 *
 * @return Ground Y Speed (Longitude), expressed as m/s * 100
 */
static inline int16_t mavlink_msg_trajectory_get_vy(const mavlink_message_t* msg)
{
	return _MAV_RETURN_int16_t(msg,  22);
}

/**
 * @brief Get field vz from trajectory message
 *
 * @return Ground Z Speed (Altitude), expressed as m/s * 100
 */
static inline int16_t mavlink_msg_trajectory_get_vz(const mavlink_message_t* msg)
{
	return _MAV_RETURN_int16_t(msg,  24);
}

/**
 * @brief Get field hdg from trajectory message
 *
 * @return Compass heading in degrees * 100, 0.0..359.99 degrees. If unknown, set to: UINT16_MAX
 */
static inline uint16_t mavlink_msg_trajectory_get_hdg(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint16_t(msg,  26);
}

/**
 * @brief Decode a trajectory message into a struct
 *
 * @param msg The message to decode
 * @param trajectory C-struct to decode the message contents into
 */
static inline void mavlink_msg_trajectory_decode(const mavlink_message_t* msg, mavlink_trajectory_t* trajectory)
{
#if MAVLINK_NEED_BYTE_SWAP
	trajectory->time_boot_ms = mavlink_msg_trajectory_get_time_boot_ms(msg);
	trajectory->lat = mavlink_msg_trajectory_get_lat(msg);
	trajectory->lon = mavlink_msg_trajectory_get_lon(msg);
	trajectory->alt = mavlink_msg_trajectory_get_alt(msg);
	trajectory->relative_alt = mavlink_msg_trajectory_get_relative_alt(msg);
	trajectory->vx = mavlink_msg_trajectory_get_vx(msg);
	trajectory->vy = mavlink_msg_trajectory_get_vy(msg);
	trajectory->vz = mavlink_msg_trajectory_get_vz(msg);
	trajectory->hdg = mavlink_msg_trajectory_get_hdg(msg);
	trajectory->point_type = mavlink_msg_trajectory_get_point_type(msg);
#else
	memcpy(trajectory, _MAV_PAYLOAD(msg), MAVLINK_MSG_ID_TRAJECTORY_LEN);
#endif
}
