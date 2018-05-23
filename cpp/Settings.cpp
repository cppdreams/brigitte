#include "Settings.h"

#include "yaml-cpp/yaml.h"

using namespace std;


QString loadProjectPath()
{
    YAML::Node config = YAML::LoadFile("config.yaml");

    return config["project"]["path"].as<string>().c_str();
}
