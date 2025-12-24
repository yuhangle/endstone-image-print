//
// Created by yuhang on 2025/12/24.
//

#include "../include/img_action.h"
#include <iostream>
#include <fstream>
#include <sstream>

bool ImgAction::isImageFile(const std::filesystem::path& p)
{
    static const std::vector<std::string> exts = {".png", ".jpg", ".jpeg", ".bmp", ".gif", ".tiff", ".webp"};
    auto ext = p.extension().string();
    ranges::transform(ext, ext.begin(),
                      [](const unsigned char c) { return std::tolower(c); });
    return ranges::find(exts, ext) != exts.end();
}

bool ImgAction::isCsvFile(const std::filesystem::path& p)
{
    static const std::string csv_ext = "csv";
    auto ext = p.extension().string();
    ranges::transform(ext, ext.begin(),
                      [](const unsigned char c){ return std::tolower(c); });
    if (ext == "." + csv_ext)
        return true;
    return false;
}


std::pair<std::vector<std::pair<int, std::string>>,
          std::vector<std::pair<int, std::string>>>
ImgAction::listImageAndOutputFiles(const std::string& img_dir, const std::string& out_dir)
{
    std::vector<std::pair<int, std::string>> img_list;
    std::vector<std::pair<int, std::string>> out_list;

    auto collectImageFiles = [](const std::string& dir) -> std::vector<std::string> {
        std::vector<std::string> files;
        try {
            if (!std::filesystem::exists(dir) || !std::filesystem::is_directory(dir)) {
                return files;
            }
            for (const auto& entry : std::filesystem::directory_iterator(dir)) {
                if (entry.is_regular_file() && ImgAction::isImageFile(entry.path())) {
                    files.push_back(entry.path().filename().string());
                }
            }
            std::ranges::sort(files);
        } catch (...) {}
        return files;
    };

    auto collectCsvFiles = [](const std::string& dir) -> std::vector<std::string> {
        std::vector<std::string> files;
        try {
            if (!std::filesystem::exists(dir) || !std::filesystem::is_directory(dir)) {
                return files;
            }
            for (const auto& entry : std::filesystem::directory_iterator(dir)) {
                if (entry.is_regular_file() && ImgAction::isCsvFile(entry.path())) {
                    files.push_back(entry.path().filename().string());
                }
            }
            std::ranges::sort(files);
        } catch (...) {}
        return files;
    };

    const auto raw_img = collectImageFiles(img_dir);
    const auto raw_out = collectCsvFiles(out_dir);  // ← 关键：只收 CSV！

    img_list.reserve(raw_img.size());
    for (size_t i = 0; i < raw_img.size(); ++i) {
        img_list.emplace_back(static_cast<int>(i + 1), raw_img[i]);
    }

    out_list.reserve(raw_out.size());
    for (size_t i = 0; i < raw_out.size(); ++i) {
        out_list.emplace_back(static_cast<int>(i + 1), raw_out[i]);
    }

    return {img_list, out_list};
}


std::pair<bool, std::vector<std::string>> ImgAction::generateSetblockCommandsFromCsv(
    const std::string& csvPath,
    float originX, float originY, float originZ)
{
    auto trim = [](std::string s) -> std::string {
        auto not_space = [](const unsigned char c) { return !std::isspace(c); };
        s.erase(s.begin(), ranges::find_if(s, not_space));
        s.erase(std::find_if(s.rbegin(), s.rend(), not_space).base(), s.end());
        return s;
    };

    std::ifstream file(csvPath);
    if (!file.is_open()) {
        std::cerr << "错误：无法打开 CSV 文件 \"" << csvPath << "\"\n";
        return {false, {}};
    }

    std::string line;
    if (!std::getline(file, line)) {
        std::cerr << "错误：CSV 文件为空\n";
        return {false, {}};
    }

    // 处理 Windows 行尾 \r
    if (!line.empty() && line.back() == '\r') {
        line.pop_back();
    }

    // 检查 header
    std::istringstream headerStream(line);
    std::string col1, col2, col3;
    if (!std::getline(headerStream, col1, ',') ||
        !std::getline(headerStream, col2, ',') ||
        !std::getline(headerStream, col3, ','))
    {
        std::cerr << "错误：CSV header 列数不足，应为 \"x,z,block_id\"\n";
        return {false, {}};
    }

    col1 = trim(col1);
    col2 = trim(col2);
    col3 = trim(col3);

    if (col1 != "x" || col2 != "z" || col3 != "block_id") {
        std::cerr << "错误：CSV header 不匹配！\n"
                  << "期望: x,z,block_id\n"
                  << "实际: \"" << col1 << "," << col2 << "," << col3 << "\"\n";
        return {false, {}};
    }

    std::vector<PointBlock> points;
    int line_num = 1; // header 是第 1 行

    while (std::getline(file, line)) {
        ++line_num;

        // 跳过空行
        if (line.empty()) continue;

        // 处理 \r
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        std::istringstream ss(line);
        std::string xStr, zStr, blockId;

        if (!std::getline(ss, xStr, ',') ||
            !std::getline(ss, zStr, ',') ||
            !std::getline(ss, blockId, ','))
        {
            std::cerr << "错误：第 " << line_num << " 行字段数量不足（需要 x,z,block_id）\n";
            return {false, {}};
        }

        xStr = trim(xStr);
        zStr = trim(zStr);
        blockId = trim(blockId);

        if (blockId.empty()) {
            std::cerr << "错误：第 " << line_num << " 行 block_id 为空\n";
            return {false, {}};
        }

        try {
            size_t pos;
            int x_val = std::stoi(xStr, &pos);
            if (pos != xStr.size()) {
                throw std::invalid_argument("x 值包含非法字符");
            }
            int z_val = std::stoi(zStr, &pos);
            if (pos != zStr.size()) {
                throw std::invalid_argument("z 值包含非法字符");
            }

            points.emplace_back(x_val, z_val, blockId);
        } catch (const std::exception& e) {
            std::cerr << "错误：第 " << line_num << " 行解析失败 - " << e.what() << "\n";
            return {false, {}};
        }
    }

    // 生成命令
    std::vector<std::string> commands;
    for (const auto& [x, z, block_id] : points) {
        int worldX = static_cast<int>(std::round(originX + static_cast<float>(x)));
        int worldY = static_cast<int>(std::round(originY));
        int worldZ = static_cast<int>(std::round(originZ + static_cast<float>(z)));

        std::ostringstream cmd, tp_cmd;
        cmd << "setblock " << worldX << " " << worldY << " " << worldZ << " " << block_id;
        tp_cmd << "tp @s " << worldX << " " << worldY << " " << worldZ;
        commands.push_back(tp_cmd.str());
        commands.push_back(cmd.str());
    }

    return {true, commands};
}

// 将命令列表按 32x32 区域分组
std::vector<std::vector<std::string>> ImgAction::partitionCommandsIntoChunks(
    const std::vector<std::string>& commands)
{
    constexpr int imageWidth = 128;
    constexpr int numChunksPerSide = 4; // 128 / 32

    // 验证输入
    if (commands.size() % 2 != 0) {
        return {};
    }
    if (commands.size() / 2 != imageWidth * imageWidth) {
        return {};
    }

    std::vector<std::vector<std::string>> result;
    result.reserve(numChunksPerSide * numChunksPerSide);

    for (int cz = 0; cz < numChunksPerSide; ++cz) {
        // 决定当前行是正序还是倒序
        const bool reverseX = (cz % 2 == 1); // 奇数行（第1、3行）倒序

        std::vector<int> xIndices(numChunksPerSide);
        std::iota(xIndices.begin(), xIndices.end(), 0); // [0,1,2,3]
        if (reverseX) {
            ranges::reverse(xIndices); // [3,2,1,0]
        }

        for (const int cx_raw : xIndices) {
            constexpr int chunkSize = 32;
            std::vector<std::string> chunk;
            chunk.reserve(chunkSize * chunkSize * 2);

            const int startX = cx_raw * chunkSize;
            const int startZ = cz * chunkSize;
            const int endX = std::min(startX + chunkSize, imageWidth);
            const int endZ = std::min(startZ + chunkSize, imageWidth);

            for (int z = startZ; z < endZ; ++z) {
                for (int x = startX; x < endX; ++x) {
                    const size_t pixelIndex = z * imageWidth + x;
                    const size_t cmdIndex = pixelIndex * 2;

                    // tp 先，setblock 后
                    chunk.push_back(commands[cmdIndex]);     // tp
                    chunk.push_back(commands[cmdIndex + 1]); // setblock
                }
            }

            if (!chunk.empty()) {
                result.push_back(std::move(chunk));
            }
        }
    }

    return result;
}

std::optional<std::array<float, 3>> ImgAction::extractTpCoordinates(const std::string& command) {
    std::istringstream iss(command);
    std::string token;

    if (!(iss >> token) || token != "tp") return std::nullopt;

    if (!(iss >> token)) return std::nullopt;

    float x;
    if (token == "@s") {
        if (!(iss >> x)) return std::nullopt;
    } else {
        try {
            x = std::stof(token);
        } catch (...) {
            return std::nullopt;
        }
    }

    float y, z;
    if (!(iss >> y >> z)) return std::nullopt;

    if (std::string trailing; iss >> trailing) return std::nullopt;

    return std::array{x, y, z};
}
