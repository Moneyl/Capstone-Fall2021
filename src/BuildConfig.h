#pragma once
#include <string>

namespace BuildConfig
{
    //Path to the asset folder. Varies depending on build type. See src/CMakeLists.txt.
    static const std::string AssetFolderPath = ASSET_FOLDER_PATH;
    static const std::string RobotFolderPath = ROBOT_FOLDER_PATH;
    static const std::string MainFontPath = AssetFolderPath + "fonts/Ruda-Bold.ttf";
    static const std::string FontAwesomePath = AssetFolderPath + "fonts/fa-solid-900.ttf";
}