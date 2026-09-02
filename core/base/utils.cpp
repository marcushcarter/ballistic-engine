#include <core/base/utils.h>
#include <cstdarg>
#include <cstdio>

namespace lumen {
        
const char* fmt_bytes(uint64_t b)
{
    static char pool[8][24];
    static uint32_t next = 0;
    char* buf = pool[next++ & 7];
    if (b >= (1ull << 30)) std::snprintf(buf, 24, "%.2f GiB", double(b) / double(1ull << 30));
    else if (b >= (1ull << 20)) std::snprintf(buf, 24, "%.1f MiB", double(b) / double(1ull << 20));
    else if (b >= (1ull << 10)) std::snprintf(buf, 24, "%.0f KiB", double(b) / double(1ull << 10));
    else std::snprintf(buf, 24, "%llu B", (unsigned long long)b);
    return buf;
}


}