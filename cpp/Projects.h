#pragma once

#include <QAbstractListModel>


struct Project
{
    QString name;
    QString path;
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
        PathRole
    };

private:
    std::vector<Project> m_projects;
};
