//
// Created by yuhang on 2025/3/20.
//
#pragma once

#include <endstone/plugin/plugin.h>
#include <string>
#include "translate.hpp"

// 数据路径
inline std::string data_path = "plugins/image_print";
//语言文件路径
inline std::string language_path = data_path + "/language/";
//待处理图像文件路径
inline std::string img_path = data_path + "/images";
//处理后输出文件路径
inline std::string out_path = data_path + "/output";
//输入文件缓存
inline vector<pair<int,std::string>> img_files;
//输出文件缓存
inline vector<pair<int,std::string>> out_files;

//语言实例
inline translate Tran;

class ImgPrint : public endstone::Plugin {
public:
    void onLoad() override;

    void onEnable() override;

    void onDisable() override;

    bool onCommand(endstone::CommandSender &sender, const endstone::Command &command, const std::vector<std::string> &args) override;
private:
    void _task_build_map();
    string _in_task_pllayer_name;
    vector<vector<string>> _in_task_build_commands;
    int _in_task_chunk_index = 0;
    vector<vector<string>> _in_task_build_retry_commands;
    bool _is_retry = false;
};