//
// Created by yuhang on 2025/3/20.
//

#include "img_print.h"
#include "version.h"
#include <endstone/endstone.hpp>
#include "image_convert.hpp"
#include <filesystem>
#include <thread>

#include "img_action.h"


void ImgPrint::onLoad()
{
    getLogger().info("onLoad is called");
    try
    {

        if (!std::filesystem::exists(data_path))
        {
            std::filesystem::create_directory(data_path);
        }
        if (!std::filesystem::exists(img_path))
        {
            std::filesystem::create_directory(img_path);
        }
        if (!std::filesystem::exists(out_path))
        {
            std::filesystem::create_directory(out_path);
        }
        if (!std::filesystem::exists(language_path))
        {
            std::filesystem::create_directory(language_path);
        }
    }
    catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << '\n';
    }
    catch (const std::exception& e) {
        std::cerr << "Standard exception: " << e.what() << '\n';
    }
    catch (...) {
        std::cerr << "Unknown exception occurred.\n";
    }
    const auto lang = getServer().getLanguage().getLocale();
    const string local_language_file = language_path + lang + ".json";
    Tran = translate(local_language_file);
    Tran.loadLanguage();
    translate::checkLanguageCommon(local_language_file,language_file);
}


void ImgPrint::onEnable()
{
    getLogger().info("onEnable is called");
    //加载文件列表到缓存
    const auto [fst, snd] = ImgAction::listImageAndOutputFiles(img_path, out_path);
    img_files = fst;
    out_files = snd;
    getServer().getScheduler().runTaskTimer(*this,[&]() { _task_build_map(); }, 0, 20);
}


void ImgPrint::onDisable()
{
    getLogger().info("onDisable is called");
    getServer().getScheduler().cancelTasks(*this);
}


bool ImgPrint::onCommand(endstone::CommandSender &sender, const endstone::Command &command, const std::vector<std::string> &args)
{
    if (command.getName() == "img-p")
    {
        if (!args[0].empty() && args[0] == "ls")
        {
            const auto [fst, snd] = ImgAction::listImageAndOutputFiles(img_path, out_path);
            //刷新缓存
            img_files = fst;
            out_files = snd;

            if (!fst.empty())
            {
                sender.sendMessage(Tran.getLocal("Files in images:"));
                for (const auto& [fst_1, snd_1] : fst)
                {
                    std::stringstream msg;
                    msg << "[" << fst_1 << "]" << ": " << snd_1;
                    sender.sendMessage("\n" + msg.str());
                }
            } else
            {
                sender.sendMessage(Tran.getLocal("Nothing in images"));
            }

            if (!snd.empty())
            {
                sender.sendMessage("\n" + Tran.getLocal("Files in output:"));
                for (const auto& [fst_1, snd_1] : snd)
                {
                    std::stringstream msg;
                    msg << "[" << fst_1 << "]" << ": " << snd_1;
                    sender.sendMessage("\n" + msg.str());
                }
            } else
            {
                sender.sendMessage("\n" + Tran.getLocal("Nothing in output"));
            }

        }

        if (!args[0].empty() && args[0] == "convert")
        {
            const int file_index = stoi(args[1]);
            const string file_name = img_files[file_index-1].second;
            if (file_name.empty())
            {
                sender.sendErrorMessage(Tran.getLocal("Unknown file!"));
                return false;
            }
            const string img_file_path = img_path + "/" +file_name;
            if (!ImgAction::isImageFile(img_file_path))
            {
                sender.sendErrorMessage(Tran.getLocal("Not a image file!"));
                return false;
            }
            if (convertImageToMinecraftCSV(img_file_path, out_path + "/" + file_name + ".csv"))
            {
                sender.sendMessage(endstone::ColorFormat::Green+Tran.getLocal("Convert success!"));
                auto [fst, snd] = ImgAction::listImageAndOutputFiles(img_path, out_path);
                img_files = fst;out_files = snd;
                if (unsigned char* img = convertTo128x128Image(img_file_path)) {
                    generatePreviewImage(img, out_path + "/preview_"+ file_name);
                    delete[] img;
                }
            } else
            {
                sender.sendErrorMessage(Tran.getLocal("Convert failed!"));
            }
        }

        if (!args[0].empty() && args[0] == "print")
        {
            if (const auto player = sender.asPlayer())
            {
                if (!_in_task_pllayer_name.empty() && _in_task_pllayer_name != player->getName())
                {
                    sender.sendErrorMessage(Tran.getLocal("A task in running!"));
                    return false;
                }
                float x = player->getLocation().getX();
                float y = player->getLocation().getY();
                float z = player->getLocation().getZ();
                const int file_index = stoi(args[1]);
                const string file_name = out_files[file_index-1].second;
                if (file_name.empty())
                {
                    sender.sendErrorMessage(Tran.getLocal("Unknown file!"));
                    return false;
                }
                if (const auto [fst, snd] = ImgAction::generateSetblockCommandsFromCsv(out_path + "/" + file_name, x, y, z);
                    fst)
                {
                    auto chunks = ImgAction::partitionCommandsIntoChunks(snd);
                    _in_task_build_commands = chunks;
                    _in_task_pllayer_name = player->getName();
                    sender.sendMessage(endstone::ColorFormat::Green+Tran.getLocal("Task commit over!"));
                } else
                {
                    sender.sendErrorMessage(Tran.getLocal("Print failed!"));
                }

            }
            else
            {
                sender.sendErrorMessage(Tran.getLocal("Console can not use this command"));
                return false;
            }
        }
        return true;
    }
    return false;
}

void ImgPrint::_task_build_map()
{
    // 安全检查：容器是否为空或索引越界
    if (_in_task_build_commands.empty() || _in_task_chunk_index >= _in_task_build_commands.size())
    {
        // 如果索引越界，说明任务已完成或出错，重置状态
        _in_task_chunk_index = 0;
        _in_task_build_commands.clear();
        return;
    }

    const auto player = getServer().getPlayer(_in_task_pllayer_name);
    // 状态检查：玩家是否存在且在线
    if (player == nullptr)
    {
        _in_task_build_commands.clear();
        _in_task_chunk_index = 0;
        _in_task_build_retry_commands.clear();
        return;
    }

    bool error = false;
    string last_cmd = "None"; // 初始化默认值
    vector<string> error_commands;

    // 安全获取当前区块的命令列表
    int fa_times = 0;
    for (const auto &current_chunk = _in_task_build_commands[_in_task_chunk_index]; const auto &cmd : current_chunk)
    {
        bool su =false;
        endstone::CommandSenderWrapper wrap(*player->asCommandSender(),
            [&su](const endstone::Message &){su = true;},
            /*
            [this,&fa_times] (const endstone::Message &msg)
            {
                fa_times++;
                cout << getServer().getLanguage().translate(get<endstone::Translatable>(msg),"zh_CN") << endl;
            });*/
            [&fa_times] (const endstone::Message &)
            {
                fa_times++;
            });

        if (const auto xyz = ImgAction::extractTpCoordinates(cmd))
        {
            player->teleport(endstone::Location(player->getLocation().getDimension(), xyz->at(0), xyz->at(1), xyz->at(2),
                player->getLocation().getPitch(), player->getLocation().getYaw()));
            su = true;
        } else
        {
            (void)getServer().dispatchCommand(wrap, cmd);
        }
        if (!su)
        {
            error = true;
            error_commands.push_back(last_cmd);
            error_commands.push_back(cmd);
        }
        last_cmd = cmd;
    }

    // 错误处理逻辑
    if (error)
    {
        _in_task_build_retry_commands.push_back(error_commands);
        player->sendErrorMessage(to_string(fa_times) + Tran.getLocal(" error!"));
        player->sendErrorMessage(Tran.getLocal("Build failed, will retry remaining tasks later."));
    }

    player->sendMessage(endstone::ColorFormat::Yellow+to_string(_in_task_chunk_index + 1) + " / " + to_string(_in_task_build_commands.size()));

    // 更新索引并检查是否完成
    _in_task_chunk_index++;

    if (_in_task_chunk_index >= _in_task_build_commands.size())
    {
        if (_in_task_build_retry_commands.empty())
        {
            // 成功完成所有任务
            _in_task_chunk_index = 0;
            _in_task_build_commands.clear();
            _in_task_pllayer_name = "";
            _is_retry = false;
            player->sendMessage(endstone::ColorFormat::Green+Tran.getLocal("Build over!"));
        }
        else
        {
            // 进入重试模式
            _in_task_build_commands = _in_task_build_retry_commands;
            _in_task_build_retry_commands.clear();
            _in_task_chunk_index = 0;
            _is_retry = true;
        }
    }
}



ENDSTONE_PLUGIN("img_print",PLUGIN_VERSION, ImgPrint)
{
    description = Tran.getLocal("Print any image into the Minecraft world as a block-based pixel artwork.");

    command("img-p")
            .description(Tran.getLocal("Image print commands"))
            .usages(
                "/img-p <ls>",
                "/img-p <convert> <int: int>",
                "/img-p <print> <int: int>"
                )
            .permissions("img_print.command.op");

    permission("img_print.command.op")
            .description("OP command")
            .default_(endstone::PermissionDefault::Operator);
}