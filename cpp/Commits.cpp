#include "Commits.h"

#include <QDebug>

#include <iostream>

using namespace std;


int Commits::getActiveBranchIndex(int row) const
{
    return m_commits[row].activeBranchIndex;
}

QVector<int> Commits::getParents(int row) const
{
    return m_commits[row].parents;
}

QVector<int> Commits::getChildren(int row) const
{
    return m_commits[row].children;
}

void Commits::reset(std::vector<Commit> commits)
{
    beginResetModel();
    m_commits  = std::move(commits);
    m_selected = std::vector<char>(m_commits.size(), 0);
    endResetModel();
}

int Commits::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()){
        return 0;
    }

    return m_commits.size();
}

QVariant Commits::data(const QModelIndex &index, int role) const
{
    if (! index.isValid()){
        return QVariant();
    }

    switch(role){
    case ShaRole:      return m_commits[index.row()].sha;
    case ShortShaRole: return m_commits[index.row()].sha.left(7);
    case MessageRole:  return m_commits[index.row()].message;
    case ActiveBranchIndexRole: return getActiveBranchIndex(index.row());
    case ChildrenRole: return QVariant::fromValue(getChildren(index.row()));
    case ParentsRole: return QVariant::fromValue(getParents(index.row()));
    case SelectionRole: return m_selected[index.row()];
    }

    return QVariant();
}

QHash<int, QByteArray> Commits::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[ShaRole]      = "sha";
    roles[ShortShaRole] = "shortsha";
    roles[MessageRole]  = "message";
    roles[SelectionRole] = "selected";

    return roles;
}

bool Commits::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (! index.isValid()){
        return false;
    }

    switch(role){
    case SelectionRole:
        m_selected[index.row()] = value.toBool();
        dataChanged(index, index, {SelectionRole});
        return true;
    }

    return false;
}

int FilteredCommits::getActiveBranchIndex(int row) const
{
    return data(index(row, 0), Commits::ActiveBranchIndexRole).toInt();
}

namespace
{

QVector<int> mapListFromSource(const QVector<int>& source, const QSortFilterProxyModel* model)
{
    QVector<int> mapped;
    for (int iparent : source){
        auto mappedIndex = model->mapFromSource(model->sourceModel()->index(iparent, 0));
        if (mappedIndex.isValid()){
            mapped.push_back(mappedIndex.row());
        }
    }

    return mapped;
}

} // namespace

QVector<int> FilteredCommits::getParents(int row) const
{
    return mapListFromSource(getParentsRaw(row), this);
}

QVector<int> FilteredCommits::getChildren(int row) const
{
    return mapListFromSource(getChildrenRaw(row), this);
}

bool FilteredCommits::isSelected(int row) const
{
    return data(index(row, 0), Commits::SelectionRole).toBool();
}

void FilteredCommits::setSelected(int row, bool selected)
{
    sourceModel()->setData(mapToSource(index(row, 0)), selected, Commits::SelectionRole);
}

void FilteredCommits::filterOnBranch(int row)
{
    m_visible = std::vector<char>(sourceModel()->rowCount(), 0);

    m_visible[mapToSource(index(row, 0)).row()] = 1;

    const int branchIndex = getActiveBranchIndex(row);

    auto makeVisible = [this](int sourceIndex){
        for (int parent : getParentsRaw(sourceIndex)){
            m_visible[parent] = 1;
        }

        for (int child : getChildrenRaw(sourceIndex)){
            m_visible[child] = 1;
        }
    };

    int icurrent = row;
    while (icurrent >= 0){
        makeVisible(icurrent);

        // Source model indices
        auto parents = getParentsRaw(icurrent);
        icurrent = -1;

        for (int parent : parents){
            if (sourceModel()->data(sourceModel()->index(parent, 0), Commits::ActiveBranchIndexRole) == branchIndex){
                // Continue if parent is on the same branch
                icurrent = mapFromSource(sourceModel()->index(parent, 0)).row();
                break;
            }
        }
    }

    icurrent = row;
    while (icurrent >= 0){
        makeVisible(icurrent);

        // Source model indices
        auto children = getChildrenRaw(icurrent);
        icurrent = -1;

        for (int child : children){
            if (sourceModel()->data(sourceModel()->index(child, 0), Commits::ActiveBranchIndexRole) == branchIndex){
                icurrent = mapFromSource(sourceModel()->index(child, 0)).row();
                break;
            }
        }
    }

    setSelected(row, false);

    // Call this instead of invalidateFilter because then views can listen to layoutChanged
    invalidate();
}

void FilteredCommits::resetFilter()
{
    m_visible = std::vector<char>(sourceModel()->rowCount(), 1);

    invalidate();
}

FilteredCommits::FilteredCommits()
    : QSortFilterProxyModel ()
{
}

bool FilteredCommits::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    return sourceRow >= m_visible.size() || m_visible[sourceRow];
}

//bool FilteredCommits::lessThan(const QModelIndex &left, const QModelIndex &right) const
//{
//}

int FilteredCommits::getSelectionRole() const
{
    return Commits::SelectionRole;
}

QVector<int> FilteredCommits::getParentsRaw(int row) const
{
    return data(index(row, 0), Commits::ParentsRole).value<QVector<int>>();
}

QVector<int> FilteredCommits::getChildrenRaw(int row) const
{
    return data(index(row, 0), Commits::ChildrenRole).value<QVector<int>>();
}
