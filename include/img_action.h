//
// Created by yuhang on 2025/12/24.
//

#ifndef IMG_PRINT_IMG_ACTION_H
#define IMG_PRINT_IMG_ACTION_H
#include <filesystem>
#include "img_print.h"


struct PointBlock
{
    int x;
    int z;
    string block_id;
};

class ImgAction
{
public:

    // 判断是否是图像文件
    static bool isImageFile(const std::filesystem::path& p);

    static bool isCsvFile(const std::filesystem::path& p);

    static std::pair<std::vector<pair<int,std::string>>, std::vector<pair<int,std::string>>> listImageAndOutputFiles(const std::string& img_dir,
                                                                                                 const std::string& out_dir);

    static std::pair<bool, std::vector<std::string>> generateSetblockCommandsFromCsv(
    const std::string& csvPath,
    float originX, float originY, float originZ);

    static std::vector<std::vector<std::string>> partitionCommandsIntoChunks(const std::vector<std::string>& commands);

    static std::optional<std::array<float, 3>> extractTpCoordinates(const std::string& command);
};


#endif //IMG_PRINT_IMG_ACTION_H