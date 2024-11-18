#pragma once
// MESSAGE FD PACKING

#define MAVLINK_MSG_ID_FD_CLB 677

#pragma pack(push,1)
typedef struct __mavlink_fd_clb_t {
    uint32_t idnum;      // Unique ID number of this FD board
    uint16_t cmd;
    uint16_t data_num;
    float clb_val[8];
} mavlink_fd_clb_t;
#pragma pack(pop)


#define MAVLINK_MSG_ID_FD_CLB_LEN 			sizeof(mavlink_fd_clb_t)
#define MAVLINK_MSG_ID_FD_CLB_MIN_LEN 		sizeof(mavlink_fd_clb_t)

#define MAVLINK_MSG_ID_FD_CLB_CRC 			0


#if MAVLINK_COMMAND_24BIT
#define MAVLINK_MESSAGE_INFO_FD_CLB { \
    0, \
    "FD_CLB", \
    1, \
    { { "id", NULL, MAVLINK_TYPE_UINT32_T, 0, 0, offsetof(mavlink_fd_clb_t, id) }, \
    } \
}
#else

#endif

/**
 * @brief Pack a fdclb message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param pfd C-struct to write the message contents
 */
static inline uint16_t mavlink_msg_fd_clb_pack( uint8_t system_id, uint8_t component_id, mavlink_message_t *msg, mavlink_fd_clb_t *pfdclb )
{
    memcpy( _MAV_PAYLOAD_NON_CONST(msg), pfdclb, MAVLINK_MSG_ID_FD_CLB_LEN );
    msg->msgid = MAVLINK_MSG_ID_FD_CLB;
    return mavlink_finalize_message( msg, system_id, component_id, MAVLINK_MSG_ID_FD_CLB_MIN_LEN, MAVLINK_MSG_ID_FD_CLB_LEN, MAVLINK_MSG_ID_FD_CLB_CRC );
}

/**
 * @brief Encode a fdclb struct
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param fd C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_fd_clb_encode( uint8_t system_id, uint8_t component_id, mavlink_message_t *msg, mavlink_fd_clb_t *fdclb )
{
    return mavlink_msg_fd_clb_pack( system_id, component_id, msg, fdclb );
}

/**
 * @brief Decode a fdclb message into a struct
 *
 * @param msg The message to decode
 * @param fd C-struct to decode the message contents into
 */
static inline void mavlink_msg_fd_clb_decode( const mavlink_message_t *msg, mavlink_fd_clb_t *fdclb )
{
	uint8_t len = msg->len < MAVLINK_MSG_ID_FD_CLB_LEN ? msg->len : MAVLINK_MSG_ID_FD_CLB_LEN;
	memset( fdclb, 0, MAVLINK_MSG_ID_FD_CLB_LEN );
    memcpy( fdclb, _MAV_PAYLOAD(msg), len );
}

