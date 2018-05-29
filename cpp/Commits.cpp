#include "Commits.h"

#include <QDebug>

#include <iostream>

using namespace std;


int Commits::getBranchIndex(int row) const
{
    return m_commits[row].branchIndex;
}

QVector<int> Commits::getParents(int row) const
{
    return m_commits[row].parents;
}

QVector<int> Commits::getChildren(int row) const
{
    return m_commits[row].children;
}

void Commits::loadFrom(QString path, int maxCommits)
{
    m_path = path;
    m_maxCommits = maxCommits;

    reset(m_git.readCommits(path, maxCommits));
}

void Commits::refresh()
{
    reset(m_git.readCommits(m_path, m_maxCommits));
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
    case BranchIndexRole: return getBranchIndex(index.row());
    case ChildrenRole: return QVariant::fromValue(getChildren(index.row()));
    case ParentsRole: return QVariant::fromValue(getParents(index.row()));
    case SelectionRole: return m_selected[index.row()];
    case AuthorRole:  return m_commits[index.row()].author.name;
    case TimeRole: return m_commits[index.row()].time;
    }

    return QVariant();
}

QHash<int, QByteArray> Commits::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[ShaRole]      = "sha";
    roles[ShortShaRole] = "shortsha";
    roles[MessageRole]  = "message";
    roles[BranchIndexRole] = "branchindex";
    roles[SelectionRole] = "selected";
    roles[AuthorRole]   = "author";
    roles[TimeRole]     = "time";

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
    return data(index(row, 0), Commits::BranchIndexRole).toInt();
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
    return mapListFromSource(data(index(row, 0), Commits::ParentsRole).value<QVector<int>>(), this);
}

QVector<int> FilteredCommits::getChildren(int row) const
{
    return mapListFromSource(data(index(row, 0), Commits::ChildrenRole).value<QVector<int>>(), this);
}

bool FilteredCommits::isSelected(int row) const
{
    return data(index(row, 0), Commits::SelectionRole).toBool();
}

void FilteredCommits::setSelected(int row, bool selected)
{
    sourceModel()->setData(mapToSource(index(row, 0)), selected, Commits::SelectionRole);
}

void FilteredCommits::filterOnBranching()
{
    m_filters.clear();

    Filter filter{Filter::Type::Git,
                vector<char>(sourceModel()->rowCount(), 0)};

    for (size_t i = 0; i < filter.visible.size(); ++i){
        if (getSourceChildren(i).size() > 1){
            filter.visible[i] = true;
            makeChildrenVisible(i, filter);
        }

        if (getSourceParents(i).size() > 1){
            filter.visible[i] = true;
            makeParentsVisible(i, filter);
        }
    }

    m_filters.emplace_back(move(filter));
    invalidate();
}

void FilteredCommits::filterOnBranch(int row)
{
    m_filters.clear();

    Filter filter{Filter::Type::Git,
                 vector<char>(sourceModel()->rowCount(), 0)};

    filter.visible[mapToSource(index(row, 0)).row()] = 1;

    const int branchIndex = getActiveBranchIndex(row);

    int icurrent = mapToSource(index(row, 0)).row();
    while (icurrent >= 0){
        makeDirectRelationsVisible(icurrent, filter);

        // Source model indices
        auto parents = getSourceParents(icurrent);
        icurrent = -1;

        for (int parent : parents){
            if (sourceModel()->data(sourceModel()->index(parent, 0), Commits::BranchIndexRole) == branchIndex){
                // Continue if parent is on the same branch
                icurrent = parent;
                break;
            }
        }
    }

    icurrent = mapToSource(index(row, 0)).row();
    while (icurrent >= 0){
        makeDirectRelationsVisible(icurrent, filter);

        // Source model indices
        auto children = getSourceChildren(icurrent);
        icurrent = -1;

        for (int child : children){
            if (sourceModel()->data(sourceModel()->index(child, 0), Commits::BranchIndexRole) == branchIndex){
                icurrent = child;
                break;
            }
        }
    }

    setSelected(row, false);

    // Call this instead of invalidateFilter because then views can listen to layoutChanged
    m_filters.emplace_back(move(filter));
    invalidate();
}

void FilteredCommits::resetFilter()
{
    m_filters.clear();
    invalidate();
}

void FilteredCommits::search(QString searchString)
{
    // We are replacing the previous search filter. Because of this we cannot rely on the
    // rowCount of this object, it is inaccurate until the filtering is reprocessed, which we
    // delay until the end of this function
    if (! m_filters.empty() && m_filters.back().type == Filter::Type::Search){
        m_filters.pop_back();
    }

    // empty search string: clears out previous search filter, if present
    if (! searchString.isEmpty()){
        Filter filter{Filter::Type::Search,
                    vector<char>(sourceModel()->rowCount(), 0)};

        for (int i = 0; i < m_commits->rowCount(QModelIndex()); ++i){
            // Manually apply filtering
            if (! m_filters.empty() && ! m_filters.back().visible[i]){
                continue;
            }

            bool visible = false;

            // If bottle-neck: get data directly
            if (   m_commits->data(m_commits->index(i, 0), Commits::MessageRole).toString().contains(searchString)
                || m_commits->data(m_commits->index(i, 0), Commits::AuthorRole).toString().contains(searchString))
            {
                visible = true;
            }

            filter.visible[i] = visible;
        }

        m_filters.emplace_back(move(filter));
    }

    invalidate();
}

FilteredCommits::FilteredCommits(Commits* commits)
    : QSortFilterProxyModel ()
    , m_commits(commits)
{
    setSourceModel(commits);
}

bool FilteredCommits::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    return m_filters.empty()
            || sourceRow >= m_filters.back().visible.size()
            || m_filters.back().visible[sourceRow];
}

void FilteredCommits::makeDirectRelationsVisible(int sourceRow, Filter& filter)
{
    makeParentsVisible(sourceRow, filter);
    makeChildrenVisible(sourceRow, filter);
}

void FilteredCommits::makeChildrenVisible(int sourceRow, Filter& filter)
{
    for (int child : getSourceChildren(sourceRow)){
        filter.visible[child] = 1;
    }
}

void FilteredCommits::makeParentsVisible(int sourceRow, Filter& filter)
{
    for (int parent : getSourceParents(sourceRow)){
        filter.visible[parent] = 1;
    }
}

//bool FilteredCommits::lessThan(const QModelIndex &left, const QModelIndex &right) const
//{
//}

int FilteredCommits::getSelectionRole() const
{
    return Commits::SelectionRole;
}

QVector<int> FilteredCommits::getSourceParents(int sourceRow) const
{
    return sourceModel()->data(sourceModel()->index(sourceRow, 0),
                               Commits::ParentsRole).value<QVector<int>>();
}

QVector<int> FilteredCommits::getSourceChildren(int sourceRow) const
{
    return sourceModel()->data(sourceModel()->index(sourceRow, 0),
                               Commits::ChildrenRole).value<QVector<int>>();
}
