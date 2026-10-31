#pragma once

#include <cstdint>
#include "common.hpp"

namespace WifiManager
{
    enum class FrameType : uint8_t
    {
        CONTROL   = 1,
        ACK       = 2,
        TELEMETRY = 3,
    };

#pragma pack(push,1) // dont add another empty bytes, dont align - use for wifi frames
    struct ControlFrame
    {
        uint8_t frameType;
        uint32_t seq;

        int16_t leftX;
        int16_t leftY;

        int16_t rightX;
        int16_t rightY;

        uint8_t buttons;
        uint8_t switches;
    };

  struct responseFrame
  {
        uint8_t frameType;
        uint32_t seq;
  };
#pragma pack(pop)


    int init();
#ifdef SENDER
    int send(const ControlFrame& frame);
    bool receiveResponse(responseFrame& frame);
#else
    int send(const responseFrame& frame);
    bool receiveResponse(ControlFrame& frame);
#endif
    void getLastFrame(ControlFrame& frame,responseFrame& frame_resp,bool& txStatus);
    bool isConnected();
    int getLostPacketsNum();
}