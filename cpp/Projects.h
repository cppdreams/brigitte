#pragma once

#include <QAbstractListModel>


struct Project
{
    QString name;
    QString path;
    /// Max number of commits to read from the repo, 0 for unused
    int     maxCommits = 0;
};

class ProjectList : public QAbstractListModel
{
public:
    void reset(std::vector<Project> projects);

    virtual int rowCount(const QModelIndex &parent) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual QHash<int, QByteArray> roleNames() const override;

private:
    enum RoleNames
    {
        NameRole,
        PathRole,
        MaxCommitsRole
    };

private:
    std::vector<Project> m_projects;
};
