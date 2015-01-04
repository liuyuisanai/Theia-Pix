#pragma once

#include <cstdint>

namespace SiKUploader
{

static constexpr uint16_t PROG_MULTI_MAX = 32;

bool AT_UPDATE(int d);
bool flash_from_file(int f, int d);

} // end of namespace SiKUploader
