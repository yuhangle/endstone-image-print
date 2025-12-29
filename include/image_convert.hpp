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
#include <thread>
#include <future>
#include <mutex>
#include <atomic>
#include <queue>
#include <condition_variable>
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
        std::cerr << "Error: File does not exist - " << imagePath << std::endl;
        return -3;
    }

    // 检查格式是否支持
    if (!isSupportedImageFormat(imagePath)) {
        std::cerr << "Error: Unsupported image format - " << imagePath << std::endl;
        std::cerr << "Supported formats: PNG, JPG, JPEG, BMP, TGA, PSD, GIF, HDR, PIC, PNM" << std::endl;
        return -3;
    }

    int w, h, c;
    unsigned char* img = stbi_load(imagePath.c_str(), &w, &h, &c, 0);
    if (!img) {
        std::cerr << "Error: Cannot load image file - " << imagePath << std::endl;
        std::cerr << "Reason: " << stbi_failure_reason() << std::endl;
        return -2; // Load failed
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
        std::cerr << "Error: Unsupported image format - " << inputPath << std::endl;
        return nullptr;
    }

    int w, h, c;
    unsigned char* img = stbi_load(inputPath.c_str(), &w, &h, &c, 3);
    if (!img) {
        std::cerr << "Error: Cannot load image " << inputPath << std::endl;
        std::cerr << "Detailed reason: " << stbi_failure_reason() << std::endl;
        return nullptr;
    }

    std::cout << "Loaded image: " << inputPath
              << " Size: " << w << "x" << h
              << " Channels: " << c << std::endl;

    // 如果已经是128x128，直接返回
    if (w == 128 && h == 128) {
        return img;
    }

    // 缩放图像
    unsigned char* resized_img = resizeTo128x128(img, w, h, 3);
    stbi_image_free(img);

    return resized_img;
}

// 多线程处理行数据的辅助函数
inline void processRowsForCSV(const unsigned char* img, std::vector<std::string>& results,
                              const int start_row, const int end_row, const int thread_id) {
    std::stringstream ss;
    for (int z = start_row; z < end_row; ++z) {
        for (int x = 0; x < 128; ++x) {
            const int idx = (z * 128 + x) * 3;
            const MapBlock& bestBlock = getBestBlock(img[idx], img[idx + 1], img[idx + 2]);
            ss << x << "," << z << "," << bestBlock.internal_id << "\n";
        }
    }
    results[thread_id] = ss.str();
}

// 将128x128图像转换为MC数据CSV
inline bool convert128x128ImageToCSV(const unsigned char* img, const std::string& outputPath) {
    if (!img) return false;

    std::ofstream dataFile(outputPath);
    if (!dataFile.is_open()) {
        std::cerr << "can not create file " << outputPath << std::endl;
        return false;
    }

    // 写入CSV头部
    dataFile << "x,z,block_id\n";

    // 确定线程数
    const int num_threads = static_cast<int>(
    std::min(128u, std::max(1u, std::thread::hardware_concurrency()))
);
    const int rows_per_thread = 128 / num_threads;
    const int extra_rows = 128 % num_threads;

    std::vector<std::thread> threads;
    std::vector<std::string> thread_results(num_threads);

    // 启动线程处理行数据
    int current_row = 0;
    for (unsigned int i = 0; i < num_threads; ++i) {
        const int rows_for_this_thread = rows_per_thread + (i < extra_rows ? 1 : 0);
        int end_row = current_row + rows_for_this_thread;

        threads.emplace_back(processRowsForCSV, img, std::ref(thread_results),
                           current_row, end_row, i);
        current_row = end_row;
    }

    // 等待所有线程完成
    for (auto& thread : threads) {
        thread.join();
    }

    // 合并结果并写入文件
    for (const auto& result : thread_results) {
        dataFile << result;
    }

    dataFile.close();
    return true;
}

// 主转换函数：输入任意图像，输出128x128的MC数据CSV
inline bool convertImageToMinecraftCSV(const std::string& inputPath, const std::string& outputPath) {
    // 检查输入文件是否存在
    if (!fs::exists(inputPath)) {
        std::cerr << "Can not find file " << inputPath << std::endl;
        return false;
    }

    // 检查图像格式
    if (!isSupportedImageFormat(inputPath)) {
        return false;
    }

    // 检查图像尺寸
    if (const int sizeStatus = checkImageSize(inputPath); sizeStatus == -2 || sizeStatus == -3) {
        return false; // 错误信息已在函数内部输出
    }

    // 转换为128x128图像
    const unsigned char* img = convertTo128x128Image(inputPath);
    if (!img) {
        return false;
    }

    // 转换为CSV
    const bool success = convert128x128ImageToCSV(img, outputPath);

    // 清理内存
    delete[] img;

    return success;
}

// 多线程处理预览图像生成的辅助函数
inline void processRowsForPreview(const unsigned char* img, unsigned char* preview,
                                  int start_row, int end_row) {
    for (int z = start_row; z < end_row; ++z) {
        for (int x = 0; x < 128; ++x) {
            const int idx = (z * 128 + x) * 3;
            const MapBlock& best = getBestBlock(img[idx], img[idx + 1], img[idx + 2]);
            preview[idx] = static_cast<unsigned char>(best.r);
            preview[idx + 1] = static_cast<unsigned char>(best.g);
            preview[idx + 2] = static_cast<unsigned char>(best.b);
        }
    }
}


// 生成预览图像
inline bool generatePreviewImage(const unsigned char* img, const std::string& outputPath) {
    if (!img) return false;

    // 创建预览缓冲区
    auto* preview = new unsigned char[128 * 128 * 3];

    // 确定线程数
    const int num_threads = static_cast<int>(
        std::min(128u, std::max(1u, std::thread::hardware_concurrency()))
    );
    const int rows_per_thread = 128 / num_threads;
    const int extra_rows = 128 % num_threads;

    std::vector<std::thread> threads;
    int current_row = 0;

    // 启动线程处理行数据
    for (unsigned int i = 0; i < num_threads; ++i) {
        const int rows_for_this_thread = rows_per_thread + (i < extra_rows ? 1 : 0);
        int end_row = current_row + rows_for_this_thread;

        threads.emplace_back(processRowsForPreview, img, preview, current_row, end_row);
        current_row = end_row;
    }

    // 等待所有线程完成
    for (auto& thread : threads) {
        thread.join();
    }

    // 保存图像
    const int result = stbi_write_png(outputPath.c_str(), 128, 128, 3, preview, 128 * 3);

    delete[] preview;
    return result != 0;
}

// 批量转换函数
inline bool convertMultipleImagesToCSV(const std::vector<std::string>& inputPaths,
                                      const std::string& outputDir = "./output",
                                      const bool generatePreview = true) {
    fs::create_directories(outputDir);

    int successCount = 0;
    int previewCount = 0;
    std::mutex count_mutex;

    const unsigned int num_threads = std::max(1u, std::thread::hardware_concurrency());
    std::vector<std::thread> workers;
    std::queue<std::string> task_queue;
    std::mutex queue_mutex;
    std::condition_variable cv;
    bool finished = false;

    // 填充任务队列
    for (const auto& path : inputPaths) {
        task_queue.push(path);
    }

    // 工作线程函数（修改后支持预览图生成）
    auto worker = [&]() {
        while (true) {
            std::string path;
            {
                std::unique_lock lock(queue_mutex);
                cv.wait(lock, [&]() { return !task_queue.empty() || finished; });
                if (task_queue.empty() && finished) return;
                path = std::move(task_queue.front());
                task_queue.pop();
            }

            // 处理单个图像
            fs::path inputFilePath(path);
            std::string baseName = inputFilePath.stem().string();
            std::string csvPath = (fs::path(outputDir) / (baseName + "_mc_data.csv")).string();
            std::string previewPath = (fs::path(outputDir) / (baseName + "_preview.png")).string();

            // 读取并缩放图像到128x128
            if (unsigned char* img = convertTo128x128Image(path)) {
                // 生成CSV
                if (convert128x128ImageToCSV(img, csvPath)) {
                    std::lock_guard lock(count_mutex);
                    successCount++;
                    std::cout << "Converted CSV: " << path << " -> " << csvPath << " ✓" << std::endl;
                } else {
                    std::lock_guard lock(count_mutex);
                    std::cout << "Convert CSV failed: " << path << " ✗" << std::endl;
                }

                // 生成预览图（如果启用）
                if (generatePreview) {
                    if (generatePreviewImage(img, previewPath)) {
                        std::lock_guard lock(count_mutex);
                        previewCount++;
                        std::cout << "Generated preview: " << previewPath << " ✓" << std::endl;
                    } else {
                        std::lock_guard lock(count_mutex);
                        std::cout << "Generate preview failed: " << path << " ✗" << std::endl;
                    }
                }

                // 清理图像内存
                delete[] img;
            } else {
                std::lock_guard lock(count_mutex);
                std::cout << "Load image failed: " << path << " ✗" << std::endl;
            }
        }
    };

    // 启动工作线程
    for (unsigned int i = 0; i < num_threads; ++i) {
        workers.emplace_back(worker);
    }

    // 等待所有任务完成
    {
        std::unique_lock lock(queue_mutex);
        while (!task_queue.empty()) {
            lock.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(10)); // 简单轮询
            lock.lock();
        }
        finished = true;
    }
    cv.notify_all();

    for (auto& t : workers) {
        if (t.joinable()) t.join();
    }

    std::cout << "\nBatch conversion completed:" << std::endl;
    std::cout << "  Total files: " << inputPaths.size() << std::endl;
    std::cout << "  Successful CSV conversions: " << successCount << std::endl;
    if (generatePreview) {
        std::cout << "  Generated preview images: " << previewCount << std::endl;
    }

    return successCount > 0;
}


#endif //IMG_PRINT_IMAGE_CONVERT_H