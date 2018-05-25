#pragma once

#include <QAbstractListModel>
#include <QSortFilterProxyModel>

#include "Git.h"


class Commits : public QAbstractListModel
{
Q_OBJECT

public slots:
    int getBranchIndex(int row) const;
    QVector<int> getParents(int row) const;
    QVector<int> getChildren(int row) const;

    void loadFrom(QString path);

public:
    void reset(std::vector<Commit> commits);

    virtual int rowCount(const QModelIndex &parent) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual QHash<int, QByteArray> roleNames() const override;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    enum Roles {
        ShaRole = Qt::UserRole + 1,
        ShortShaRole,
        MessageRole,
        BranchIndexRole,
        ChildrenRole,
        ParentsRole,
        SelectionRole,
        AuthorRole,
        TimeRole
    };

private:
    Git m_git;
    std::vector<char>   m_selected;
    std::vector<Commit> m_commits;
};

class FilteredCommits : public QSortFilterProxyModel
{
Q_OBJECT

Q_PROPERTY(int selectionRole READ getSelectionRole)

public slots:
    int getActiveBranchIndex(int row) const;
    QVector<int> getParents(int row) const;
    QVector<int> getChildren(int row) const;
    bool         isSelected(int row) const;

    void         setSelected(int row, bool selected);

    /**
        Show only nodes that have multiple parents and/or children, and those
        parents/children
    */
\
    void         filterOnBranching();
    /**
        Show only commits that belong to the branch the commit with index 'row'
        is in, + direct parents/children.
    */
    void         filterOnBranch(int row);
    void         resetFilter();

public:
    FilteredCommits();

    int getSelectionRole() const;

    QVector<int> getSourceParents(int sourceRow) const;
    QVector<int> getSourceChildren(int sourceRow) const;

private:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
//    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;

    /**
        Make parents and children visible
    */
    void makeDirectRelationsVisible(int sourceRow);
    void makeChildrenVisible(int sourceRow);
    void makeParentsVisible(int sourceRow);

private:
    std::vector<char> m_visible;
};
