//
// Created by yuhang on 2025/12/23.
//

#ifndef IMG_PRINT_IMAGE_CONVERT_H
#define IMG_PRINT_IMAGE_CONVERT_H

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <algorithm>
// 引入图像读取库
#define STB_IMAGE_IMPLEMENTATION
#include "../third_party_lib/stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../third_party_lib/stb/stb_image_write.h"
namespace fs = std::filesystem;

struct MapBlock {
    std::string internal_id;
    int r, g, b;
    double L, a, b_val;
};

// 检查图像格式是否支持
inline bool isSupportedImageFormat(const std::string& filePath) {
    const fs::path path(filePath);
    std::string ext = path.extension().string();

    // 转换为小写比较
    std::ranges::transform(ext, ext.begin(), ::tolower);

    // stb_image支持的格式
    const std::vector<std::string> supportedFormats = {
        ".png", ".jpg", ".jpeg", ".bmp", ".tga",
        ".psd", ".gif", ".hdr", ".pic", ".pnm"
    };

    return std::ranges::find(supportedFormats, ext) != supportedFormats.end();
}

// 简易 RGB 转 Lab 函数
inline void rgbToLab(const int R, const int G, const int B, double &L, double &a, double &b_val) {
    double r = R / 255.0, g = G / 255.0, b = B / 255.0;
    r = (r > 0.04045) ? pow((r + 0.055) / 1.055, 2.4) : (r / 12.92);
    g = (g > 0.04045) ? pow((g + 0.055) / 1.055, 2.4) : (g / 12.92);
    b = (b > 0.04045) ? pow((b + 0.055) / 1.055, 2.4) : (b / 12.92);

    const double x = (r * 0.4124 + g * 0.3576 + b * 0.1805) / 0.95047;
    const double y = (r * 0.2126 + g * 0.7152 + b * 0.0722) / 1.00000;
    const double z = (r * 0.0193 + g * 0.1192 + b * 0.9505) / 1.08883;

    auto f = [](const double t) { return (t > 0.008856) ? pow(t, 1.0/3.0) : (7.787 * t + 16.0/116.0); };
    L = 116.0 * f(y) - 16.0;
    a = 500.0 * (f(x) - f(y));
    b_val = 200.0 * (f(y) - f(z));
}

// 获取文件扩展名
inline std::string getFileExtension(const std::string& filePath) {
    const fs::path path(filePath);
    std::string ext = path.extension().string();
    std::ranges::transform(ext, ext.begin(), ::tolower);
    return ext;
}


inline std::vector<MapBlock> initPalette() {
    std::vector<MapBlock> palette = {
        // --- 基础方块 ---
        {"minecraft:grass_block", 109, 153, 48},
        {"minecraft:white_wool", 171, 171, 171},
        {"minecraft:white_concrete", 220, 220, 220},
        {"minecraft:stone", 97, 97, 97},
        {"minecraft:dirt", 130, 94, 66},
        {"minecraft:oak_planks", 124, 100, 61},
        {"minecraft:deepslate", 55, 55, 55},

        // --- 初音绿/青色系 (Miku Teal) ---
        {"minecraft:diamond_block", 81, 189, 184},
        {"minecraft:prismarine_bricks", 94, 142, 134},
        {"minecraft:warped_planks", 41, 89, 101},
        {"minecraft:cyan_concrete", 66, 109, 134},
        {"minecraft:warped_wart_block", 20, 180, 133},
        // --- Miku 色域增强 ---
        {"minecraft:sea_lantern", 172, 209, 201},
        {"minecraft:prismarine", 102, 153, 153},
        {"minecraft:cyan_wool", 76, 127, 153},

        // 浅粉色/肤色系 (Light Pink) ---
        {"minecraft:cherry_planks", 233, 175, 175},
        {"minecraft:cherry_leaves", 214, 117, 151},
        {"minecraft:pink_wool", 242, 127, 165},
        {"minecraft:pink_concrete", 210, 109, 138},

        // --- 黄色/明亮系 ---
        {"minecraft:bamboo_planks", 197, 175, 43},
        {"minecraft:yellow_wool", 253, 216, 61},
        {"minecraft:gold_block", 215, 194, 58},
        {"minecraft:sponge", 215, 194, 58},

        // --- 蓝色/冷色系 ---
        {"minecraft:lapis_block", 64, 101, 220},
        {"minecraft:blue_concrete", 43, 55, 134},
        {"minecraft:light_blue_wool", 102, 153, 216},

        // --- 红色/紫色系 ---
        {"minecraft:red_concrete", 134, 31, 31},
        {"minecraft:netherrack", 97, 0, 0},
        {"minecraft:crimson_nylium", 163, 22, 109},
        {"minecraft:magenta_concrete", 154, 61, 179},
        {"minecraft:amethyst_block", 134, 61, 179},

        // --- 绿色系 ---
        {"minecraft:emerald_block", 0, 187, 49},
        {"minecraft:slime_block", 109, 153, 48},
        {"minecraft:moss_block", 101, 119, 64},

        // --- 黑色/深色系 ---
        {"minecraft:black_concrete", 22, 22, 22},
        {"minecraft:obsidian", 22, 13, 31},
        {"minecraft:netherite_block", 43, 39, 43},

        // --- 额外灰阶 ---
        {"minecraft:polished_andesite", 132, 135, 134},
        {"minecraft:polished_diorite", 191, 191, 191},
        {"minecraft:quartz_block", 220, 215, 205},

        // --- 红色/橙色渐变 ---
        {"minecraft:red_wool", 150, 50, 50},
        {"minecraft:redstone_block", 171, 50, 44},
        {"minecraft:orange_wool", 240, 118, 19},
        {"minecraft:acacia_planks", 169, 91, 50},
        {"minecraft:brown_mushroom_block", 149, 85, 50},

        // --- 黄绿色 ---
        {"minecraft:lime_wool", 112, 185, 25},

        // --- 蓝紫色 ---
        {"minecraft:purple_wool", 121, 42, 150},
        {"minecraft:blue_ice", 116, 167, 253},
        {"minecraft:packed_ice", 141, 184, 255},

        // --- 带釉陶瓦系列 ---
        {"minecraft:white_glazed_terracotta", 189, 196, 199},
        {"minecraft:silver_glazed_terracotta", 137, 141, 144}, // 基岩版淡灰色
        {"minecraft:gray_glazed_terracotta", 76, 81, 86},
        {"minecraft:black_glazed_terracotta", 44, 29, 34},
        {"minecraft:brown_glazed_terracotta", 125, 85, 59},
        {"minecraft:red_glazed_terracotta", 179, 59, 57},
        {"minecraft:orange_glazed_terracotta", 224, 117, 51},
        {"minecraft:yellow_glazed_terracotta", 232, 171, 59},
        {"minecraft:lime_glazed_terracotta", 129, 183, 63},
        {"minecraft:green_glazed_terracotta", 95, 139, 70},
        {"minecraft:cyan_glazed_terracotta", 50, 118, 119},
        {"minecraft:light_blue_glazed_terracotta", 103, 138, 169},
        {"minecraft:blue_glazed_terracotta", 71, 74, 142},
        {"minecraft:purple_glazed_terracotta", 118, 70, 142},
        {"minecraft:magenta_glazed_terracotta", 169, 78, 143},
        {"minecraft:pink_glazed_terracotta", 221, 141, 167},

        // --- 铜家族 ---
        {"minecraft:copper_block", 216, 127, 51},
        {"minecraft:exposed_copper", 135, 107, 98},
        {"minecraft:weathered_copper", 58, 142, 140},
        {"minecraft:oxidized_copper", 22, 126, 134},

        // --- 陶瓦家族 ---
        {"minecraft:white_terracotta", 209, 177, 161},
        {"minecraft:light_gray_terracotta", 135, 107, 98},
        {"minecraft:gray_terracotta", 57, 41, 35},
        {"minecraft:black_terracotta", 37, 22, 16},
        {"minecraft:brown_terracotta", 76, 50, 35},
        {"minecraft:red_terracotta", 142, 60, 46},
        {"minecraft:orange_terracotta", 159, 82, 36},
        {"minecraft:yellow_terracotta", 186, 133, 36},
        {"minecraft:lime_terracotta", 103, 117, 53},
        {"minecraft:green_terracotta", 76, 82, 42},
        {"minecraft:cyan_terracotta", 87, 92, 92},
        {"minecraft:light_blue_terracotta", 112, 108, 138},
        {"minecraft:blue_terracotta", 76, 62, 92},
        {"minecraft:purple_terracotta", 122, 73, 88},
        {"minecraft:magenta_terracotta", 149, 87, 108},
        {"minecraft:pink_terracotta", 160, 77, 78}
    };
    // 预计算所有方块的 Lab 值
    for (auto &block : palette) {
        rgbToLab(block.r, block.g, block.b, block.L, block.a, block.b_val);
    }
    return palette;
};

static const std::vector<MapBlock> MC_DATA_PALETTE = initPalette();


// 修改返回值为引用，避免字符串拷贝和二次查找
inline const MapBlock& getBestBlock(const int r, const int g, const int b) {
    double targetL, targeta, targetb;
    rgbToLab(r, g, b, targetL, targeta, targetb);

    int bestIdx = 0;
    double minD = 1e18;

    for (int i = 0; i < MC_DATA_PALETTE.size(); ++i) {
        const double d = std::pow(targetL - MC_DATA_PALETTE[i].L, 2) +
                   std::pow(targeta - MC_DATA_PALETTE[i].a, 2) +
                   std::pow(targetb - MC_DATA_PALETTE[i].b_val, 2);
        if (d < minD) {
            minD = d;
            bestIdx = i;
        }
    }
    return MC_DATA_PALETTE[bestIdx];
}

// 检查图像尺寸
// 返回值: 0-等于128x128, 1-大于128x128, -1-小于128x128, -2-加载失败
inline int checkImageSize(const std::string& imagePath) {
    // 检查文件是否存在
    if (!fs::exists(imagePath)) {
        std::cerr << "错误: 文件不存在 - " << imagePath << std::endl;
        return -3;
    }

    // 检查格式是否支持
    if (!isSupportedImageFormat(imagePath)) {
        std::cerr << "错误: 不支持的图像格式 - " << imagePath << std::endl;
        std::cerr << "支持的格式: PNG, JPG, JPEG, BMP, TGA, PSD, GIF, HDR, PIC, PNM" << std::endl;
        return -3;
    }

    int w, h, c;
    unsigned char* img = stbi_load(imagePath.c_str(), &w, &h, &c, 0);
    if (!img) {
        std::cerr << "错误: 无法加载图像文件 - " << imagePath << std::endl;
        std::cerr << "原因: " << stbi_failure_reason() << std::endl;
        return -2; // 加载失败
    }
    stbi_image_free(img);

    if (w == 128 && h == 128) return 0;
    if (w >= 128 && h >= 128) return 1;
    return -1;
}

// 智能缩放图像到128x128（保持宽高比的裁剪或缩放）
inline unsigned char* resizeTo128x128(const unsigned char* img, const int w, const int h, const int c) {
    auto* resized = new unsigned char[128 * 128 * c];

    if (w == 128 && h == 128) {
        // 已经是目标尺寸，直接复制
        memcpy(resized, img, 128 * 128 * c);
        return resized;
    }

    if (const int min_dim = std::min(w, h); min_dim >= 128) {
        // 裁剪模式：先中心裁剪成正方形（边长=min_dim），再缩放到128x128
        const int crop_size = min_dim;
        const int offset_x = (w - crop_size) / 2;
        const int offset_y = (h - crop_size) / 2;

        // 缩放裁剪区域到128x128（最近邻）
        const float scale = 128.0f / static_cast<float>(crop_size);
        for (int y = 0; y < 128; ++y) {
            for (int x = 0; x < 128; ++x) {
                // 映射回裁剪区域坐标
                int src_x_in_crop = static_cast<int>(static_cast<float>(x) / scale);
                int src_y_in_crop = static_cast<int>(static_cast<float>(y) / scale);
                src_x_in_crop = std::min(src_x_in_crop, crop_size - 1);
                src_y_in_crop = std::min(src_y_in_crop, crop_size - 1);

                // 加上裁剪偏移得到原图坐标
                int src_x = offset_x + src_x_in_crop;
                int src_y = offset_y + src_y_in_crop;
                src_x = std::min(src_x, w - 1);
                src_y = std::min(src_y, h - 1);

                const int dst_idx = (y * 128 + x) * c;
                const int src_idx = (src_y * w + src_x) * c;
                for (int channel = 0; channel < c; ++channel) {
                    resized[dst_idx + channel] = img[src_idx + channel];
                }
            }
        }
    } else {
        // 填充模式：按比例缩放，居中填充
        const float scale_w = 128.0f / static_cast<float>(w);
        const float scale_h = 128.0f / static_cast<float>(h);
        const float scale = std::min(scale_w, scale_h); // 保持宽高比

        const int new_w = static_cast<int>(std::round(static_cast<float>(w) * scale));
        const int new_h = static_cast<int>(std::round(static_cast<float>(h) * scale));
        const int offset_x = (128 - new_w) / 2;
        const int offset_y = (128 - new_h) / 2;

        // 填充黑色背景
        memset(resized, 0, 128 * 128 * c);

        for (int y = 0; y < new_h; ++y) {
            for (int x = 0; x < new_w; ++x) {
                int src_x = static_cast<int>(static_cast<float>(x) / scale);
                int src_y = static_cast<int>(static_cast<float>(y) / scale);
                src_x = std::min(src_x, w - 1);
                src_y = std::min(src_y, h - 1);

                const int dst_idx = ((offset_y + y) * 128 + (offset_x + x)) * c;
                const int src_idx = (src_y * w + src_x) * c;

                for (int channel = 0; channel < c; ++channel) {
                    resized[dst_idx + channel] = img[src_idx + channel];
                }
            }
        }
    }

    return resized;
}

// 将图像转换为128x128（自动处理缩放）
inline unsigned char* convertTo128x128Image(const std::string& inputPath) {
    // 检查文件格式
    if (!isSupportedImageFormat(inputPath)) {
        std::cerr << "错误: 不支持的图像格式 - " << inputPath << std::endl;
        return nullptr;
    }

    int w, h, c;
    unsigned char* img = stbi_load(inputPath.c_str(), &w, &h, &c, 3);
    if (!img) {
        std::cerr << "错误: 无法加载图像 " << inputPath << std::endl;
        std::cerr << "详细原因: " << stbi_failure_reason() << std::endl;
        return nullptr;
    }

    std::cout << "已加载图像: " << inputPath
              << " 尺寸: " << w << "x" << h
              << " 通道: " << c << std::endl;

    // 如果已经是128x128，直接返回
    if (w == 128 && h == 128) {
        return img;
    }

    // 缩放图像
    unsigned char* resized_img = resizeTo128x128(img, w, h, 3);
    stbi_image_free(img);

    return resized_img;
}

// 将128x128图像转换为MC数据CSV
inline bool convert128x128ImageToCSV(const unsigned char* img, const std::string& outputPath) {
    if (!img) return false;

    std::ofstream dataFile(outputPath);
    if (!dataFile.is_open()) {
        std::cerr << "错误: 无法创建输出文件 " << outputPath << std::endl;
        return false;
    }

    // 写入CSV头部
    dataFile << "x,z,block_id\n";

    // 遍历像素，以左上角为 (0,0)
    for (int z = 0; z < 128; ++z) {
        for (int x = 0; x < 128; ++x) {
            // 获取对应像素 RGB
            const int idx = (z * 128 + x) * 3;

            const MapBlock& bestBlock = getBestBlock(img[idx], img[idx + 1], img[idx + 2]);

            dataFile << x << "," << z << "," << bestBlock.internal_id << "\n";
        }
    }

    dataFile.close();
    return true;
}

// 主转换函数：输入任意图像，输出128x128的MC数据CSV
inline bool convertImageToMinecraftCSV(const std::string& inputPath, const std::string& outputPath) {
    // 检查输入文件是否存在
    if (!fs::exists(inputPath)) {
        std::cerr << "错误: 输入文件不存在 - " << inputPath << std::endl;
        return false;
    }

    // 检查图像格式
    if (!isSupportedImageFormat(inputPath)) {
        /*
        std::cerr << "错误: 不支持的图像格式 - " << inputPath << std::endl;
        std::cout << "支持以下格式:" << std::endl;
        std::cout << "  - PNG (.png)" << std::endl;
        std::cout << "  - JPEG/JPG (.jpg, .jpeg)" << std::endl;
        std::cout << "  - BMP (.bmp)" << std::endl;
        std::cout << "  - TGA (.tga)" << std::endl;
        std::cout << "  - GIF (.gif)" << std::endl;
        std::cout << "  - 以及其他stb_image支持的格式" << std::endl;
        */
        return false;
    }

    // 检查图像尺寸
    if (const int sizeStatus = checkImageSize(inputPath); sizeStatus == -2 || sizeStatus == -3) {
        return false; // 错误信息已在函数内部输出
    }

    /*

    std::cout << "图像信息: " << std::endl;
    std::cout << "  文件: " << inputPath << std::endl;
    std::cout << "  格式: " << getFileExtension(inputPath) << std::endl;
    std::cout << "  状态: ";
    switch (sizeStatus) {
    case 0: std::cout << "完美尺寸 (128x128)"; break;
    case 1: std::cout << "大于128x128，将进行缩放"; break;
    case -1: std::cout << "小于128x128，将进行缩放（可能会模糊）"; break;
    default: ;
    }
    std::cout << std::endl;
    */

    // 转换为128x128图像
    const unsigned char* img = convertTo128x128Image(inputPath);
    if (!img) {
        return false;
    }

    // 转换为CSV
    const bool success = convert128x128ImageToCSV(img, outputPath);

    // 清理内存
    delete[] img;

    if (success) {
        //std::cout << "✓ MC数据文件已生成: " << outputPath << std::endl;
        //std::cout << "  包含128x128个方块数据，可用于生成地图画" << std::endl;
    }

    return success;
}

// 生成预览图像
inline bool generatePreviewImage(const unsigned char* img, const std::string& outputPath) {
    if (!img) return false;

    // 创建预览缓冲区
    auto* preview = new unsigned char[128 * 128 * 3];

    for (int z = 0; z < 128; ++z) {
        for (int x = 0; x < 128; ++x) {
            const int idx = (z * 128 + x) * 3;

            // 直接获取匹配到的方块对象
            const MapBlock& best = getBestBlock(img[idx], img[idx + 1], img[idx + 2]);

            // 直接赋值 RGB
            preview[idx]     = static_cast<unsigned char>(best.r);
            preview[idx + 1] = static_cast<unsigned char>(best.g);
            preview[idx + 2] = static_cast<unsigned char>(best.b);
        }
    }

    // 保存图像 (注意：需要确保包含了 stb_image_write.h)
    const int result = stbi_write_png(outputPath.c_str(), 128, 128, 3, preview, 128 * 3);

    delete[] preview;
    return result != 0;
}

// 批量转换函数（新增功能）
inline bool convertMultipleImagesToCSV(const std::vector<std::string>& inputPaths,
                                      const std::string& outputDir = "./output") {
    // 创建输出目录
    fs::create_directories(outputDir);

    int successCount = 0;
    //const int totalCount = static_cast<int>(inputPaths.size());

    for (const auto & inputPath : inputPaths) {
        //std::cout << "\n=== 处理文件 " << (i+1) << "/" << totalCount << " ===" << std::endl;
        //std::cout << "输入: " << inputPath << std::endl;

        // 生成输出文件名
        fs::path inputFilePath(inputPath);
        std::string outputFilename = inputFilePath.stem().string() + "_mc_data.csv";

        if (std::string outputPath = (fs::path(outputDir) / outputFilename).string(); convertImageToMinecraftCSV(inputPath, outputPath)) {
            successCount++;
            //std::cout << "输出: " << outputPath << " ✓" << std::endl;
        } else {
            std::cout << "输出: 转换失败 ✗" << std::endl;
        }
    }

    //std::cout << "\n=== 转换完成 ===" << std::endl;
    //std::cout << "成功: " << successCount << "/" << totalCount << " 个文件" << std::endl;

    return successCount > 0;
}


#endif //IMG_PRINT_IMAGE_CONVERT_H