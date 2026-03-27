#ifndef LIDARPACKETSTRUCT_H
#define LIDARPACKETSTRUCT_H


#include <cstdint>

#pragma pack(push,1)


struct LidarUnit {
    uint16_t distance; // 距离单位通常是 2mm
    uint8_t intensity; // 反射强度
};

struct LidarBlock {
    uint16_t header;       // 应为 0xFFEE
    uint16_t azimuth;      // 当前块的起始方位角 (0-35999, 单位 0.01°)
    LidarUnit units[32];   // 32个激光通道的数据
};

//struct LidarPacket {
//    LidarBlock blocks[12]; // 一个 UDP 包通常包含 12 个 Block
//    uint32_t timestamp;    // 雷达自身的时间戳
//    uint16_t factory;      // 工厂标记 (如 0x3738)
//};
struct LidarPacket {
    LidarBlock blocks[12]; // 12 * 100 = 1200 bytes
    uint32_t timestamp;    // 4 bytes
    uint16_t factory;      // 2 bytes
    uint8_t reserved[42];  // 补充剩余的 42 字节，使总大小等于 1248
};


#pragma pack(pop)




#endif // LIDARPACKETSTRUCT_H






