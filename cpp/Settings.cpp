#include "Settings.h"

#include <iostream>

#include "yaml-cpp/yaml.h"

using namespace std;


std::vector<Project> loadProjects()
{
    YAML::Node config = YAML::LoadFile("config.yaml");
    YAML::Node projectsNode = config["projects"];

    std::vector<Project> projects;
    for (const auto& projectNode : projectsNode){
        projects.emplace_back(Project{projectNode["name"].as<string>().c_str(),
                                      projectNode["path"].as<string>().c_str(),
                                      projectNode["max_commits"].as<int>(0)
                              });
    }

    return projects;
}
