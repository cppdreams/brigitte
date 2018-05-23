#pragma once

#include <QString>
#include <QVector>
#include <QDateTime>

struct Author
{
    QString name;
};

struct Commit
{
    QString   sha;
    QString   message;
    Author    author;
    QDateTime time;

    /// This is higher up in the tree i.e. later in time. Multiple: branch.
    QVector<int> children;
    /// Lower in the tree i.e. earlier in time. Multiple: merge.
    QVector<int> parents;
    /// For visualization: number (left-to-right) of the branch of the active branches
    /// this commit is in
    int branchIndex = 0;
};

class Git
{
public:
    Git();
    ~Git();

    Git(const Git& rhs) = delete;
    Git& operator=(const Git& rhs) = delete;

    std::vector<Commit> readCommits(const QString& projectPath) const;
};
