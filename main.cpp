#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include <iostream>

#include "Commits.h"

using namespace std;


int main(int argc, char *argv[])
{
    Git git;

    Commits commits;
    commits.reset(git.readCommits());

    FilteredCommits filteredCommits;
    filteredCommits.setSourceModel(&commits);

    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    engine.rootContext()->setContextProperty("cpp_commits", &filteredCommits);

    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;


    return app.exec();
}
