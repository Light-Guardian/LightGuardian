#ifndef __HEADER_H__
#define __HEADER_H__

#pragma pack(1)

namespace Simulator {

#define MAX_IP_LENGTH 128
#define MAX_PATH_LENGTH 1024
#define MAX_HOST_NUM 1024

const int sw_num = 20;
const int ar_num = 3;
const int bu_num = 64;

struct MIH_Header {
    uint16_t switch_id;
    uint64_t tim_epoch;
    uint64_t timestamp;
};

struct SFH_Header {
    uint16_t switch_id;
    uint32_t fgment_id;
    uint32_t delay[8];
};

struct FLAG_Header
{
    uint8_t exists_fg;
};

struct LOAD_Header
{
    uint8_t load[1400];
};

struct COM_Header {
    FLAG_Header flag_header;
	SFH_Header sfh;
    LOAD_Header load_header;

};

struct SEND_Header
{
    FLAG_Header flag_header;
    LOAD_Header load_header;
};

}

#endif
