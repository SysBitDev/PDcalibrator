#pragma once
// MESSAGE FD PACKING

#define MAVLINK_MSG_ID_FD 666

#pragma pack(push,1)
typedef struct __mavlink_fd_t {
	uint version:11;     // Version in decimal MMDD MM - mounth, DD - day
	uint calibrated:1;   // Calibration: 0 - after reset, 1 - calibrated
    uint error_code:8;   // Error code, RET_xxx, RET_OK = 0 - no error
    uint ch_num:4;       // Fotodiode channels number - 1-2-3-4-6
    uint branches_num:2; // Branches for channels 1-3
    uint vector_mode:2;  // Mode, 0 only
    uint state:4;		 // State,

    uint16_t inc_cnt;     // Infinite incrementor

    uint32_t idnum;      // Unique ID number of this FD board

    // signal level after 1-x branch
    uint32_t b1ch1;
    uint32_t b1ch2;
    uint32_t b1ch3;
    uint32_t b1ch4;

    // signal level after 2-x branch
    uint32_t b2ch1;
    uint32_t b2ch2;
    uint32_t b2ch3;
    uint32_t b2ch4;

    // noise level after 1-x branch
    uint16_t snr_b1ch1;
    uint16_t snr_b1ch2;
    uint16_t snr_b1ch3;
    uint16_t snr_b1ch4;

    // noise level after 2-x branch
    uint16_t snr_b2ch1;
    uint16_t snr_b2ch2;
    uint16_t snr_b2ch3;
    uint16_t snr_b2ch4;

    // direction - common
    float x[2];
    float y[2];

} mavlink_fd_t;
#pragma pack(pop)


#define MAVLINK_MSG_ID_FD_LEN 			sizeof(mavlink_fd_t)
#define MAVLINK_MSG_ID_FD_MIN_LEN 		sizeof(mavlink_fd_t)

#define MAVLINK_MSG_ID_FD_CRC 			0


#if MAVLINK_COMMAND_24BIT
#define MAVLINK_MESSAGE_INFO_FD { \
    0, \
    "FD", \
    1, \
    { { "id", NULL, MAVLINK_TYPE_UINT32_T, 0, 0, offsetof(mavlink_fd_t, id) }, \
    } \
}
#else

#endif

/**
 * @brief Pack a fd message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param pfd C-struct to write the message contents
 */
static inline uint16_t mavlink_msg_fd_pack( uint8_t system_id, uint8_t component_id, mavlink_message_t *msg, mavlink_fd_t *pfd )
{
    memcpy( _MAV_PAYLOAD_NON_CONST(msg), pfd, MAVLINK_MSG_ID_FD_LEN );
    msg->msgid = MAVLINK_MSG_ID_FD;
    return mavlink_finalize_message( msg, system_id, component_id, MAVLINK_MSG_ID_FD_MIN_LEN, MAVLINK_MSG_ID_FD_LEN, MAVLINK_MSG_ID_FD_CRC );
}

/**
 * @brief Encode a fd struct
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param fd C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_fd_encode( uint8_t system_id, uint8_t component_id, mavlink_message_t *msg, mavlink_fd_t *fd )
{
    return mavlink_msg_fd_pack( system_id, component_id, msg, fd );
}

/**
 * @brief Decode a fd message into a struct
 *
 * @param msg The message to decode
 * @param fd C-struct to decode the message contents into
 */
static inline void mavlink_msg_fd_decode( const mavlink_message_t *msg, mavlink_fd_t *fd )
{
	uint8_t len = msg->len < MAVLINK_MSG_ID_FD_LEN ? msg->len : MAVLINK_MSG_ID_FD_LEN;
	memset( fd, 0, MAVLINK_MSG_ID_FD_LEN );
    memcpy( fd, _MAV_PAYLOAD(msg), len );
}

