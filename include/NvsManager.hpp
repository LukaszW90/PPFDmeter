#pragma once



enum class NvsErrorCode
{
    OK,
    ERROR,
};
namespace NvsManager
{
    NvsErrorCode initNvs();
    #if 0

    NvsErrorCode save(const LampConfig_struct& config);
    NvsErrorCode load(LampConfig_struct& config);
    NvsErrorCode clearNvs();
    #endif
}
