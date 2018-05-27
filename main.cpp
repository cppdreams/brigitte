#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include <iostream>

#include "Commits.h"
#include "Settings.h"


using namespace std;


int main(int argc, char *argv[])
{
    ProjectList projects;
    projects.reset(loadProjects());

    Commits commits;

    FilteredCommits filteredCommits(&commits);

    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    engine.rootContext()->setContextProperty("cpp_allcommits", &commits);
    engine.rootContext()->setContextProperty("cpp_commits", &filteredCommits);
    engine.rootContext()->setContextProperty("cpp_projects", &projects);

    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;


    return app.exec();
}
