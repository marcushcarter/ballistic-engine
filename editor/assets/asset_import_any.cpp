#include <editor/assets/asset_import_any.h>
#include <editor/assets/texture_cooker.h>
#include <algorithm>
#include <string>

namespace lumen {

static std::string lower_ext(const std::filesystem::path& p)
{
    std::string e = p.extension().string();
    std::transform(e.begin(), e.end(), e.begin(), [](unsigned char c){ return (char)std::tolower(c); });
    return e;
}

bool is_importable(const std::filesystem::path& p_src)
{
    const std::string e = lower_ext(p_src);
    return e == ".png" || e == ".jpg" || e == ".jpeg" || e == ".tga" || e == ".bmp";
}

bool asset_import_any(EditorContext& ctx, const std::filesystem::path& p_src, const std::filesystem::path& p_dst)
{
    const std::string e = lower_ext(p_src);

    if (e == ".png" || e == ".jpg" || e == ".jpeg" || e == ".tga" || e == ".bmp") {
        const std::filesystem::path dst = p_dst / (p_src.stem().wstring() + L".btexture");
        TextureCooker::import_async(ctx, p_src, dst);
        return true;
    }

    return false;
}

}