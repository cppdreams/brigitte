#include "Settings.h"

#include <iostream>

#include "yaml-cpp/yaml.h"

using namespace std;


//QString loadProjectPath()
//{
//    YAML::Node config = YAML::LoadFile("config.yaml");

//    return config["project"]["path"].as<string>().c_str();
//}

std::vector<Project> loadProjects()
{
    YAML::Node config = YAML::LoadFile("config.yaml");
    YAML::Node projectsNode = config["projects"];

    std::vector<Project> projects;
    for (const auto& projectNode : projectsNode){
        projects.emplace_back(Project{projectNode["name"].as<string>().c_str(),
                                      projectNode["path"].as<string>().c_str()});
    }

    return projects;

//    for(YAML::const_iterator it=characterType.begin();it != characterType.end();++it) {
//       std::string key = it->first.as<std::string>();       // <- key
//       cTypeList.push_back(it->second.as<CharacterType>()); // <- value
//    }
}
