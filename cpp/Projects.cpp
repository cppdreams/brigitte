#include "Projects.h"


using namespace std;

void ProjectList::reset(std::vector<Project> projects)
{
    beginResetModel();
    m_projects = move(projects);
    endResetModel();
}

int ProjectList::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()){
        return 0;
    }

    return m_projects.size();
}

QVariant ProjectList::data(const QModelIndex &index, int role) const
{
    if (! index.isValid()){
        return QVariant();
    }

    switch(role){
    case NameRole: return m_projects[index.row()].name;
    case PathRole: return m_projects[index.row()].path;
    }

    return QVariant();
}

QHash<int, QByteArray> ProjectList::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[NameRole] = "projectname";
    roles[PathRole] = "path";

    return roles;
}
