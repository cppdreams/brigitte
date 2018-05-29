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

    void loadFrom(QString path, int maxCommits);
    void refresh();

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

    QString m_path;
    int     m_maxCommits = 0;

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
    void         filterOnBranching();
    /**
        Show only commits that belong to the branch the commit with index 'row'
        is in, + direct parents/children.
    */
    void         filterOnBranch(int row);
    void         resetFilter();
    void         search(QString searchString);

public:
    /**
        Take Commits as argument to get access to unfiltered data without having
        to cast SourceModel
    */
    FilteredCommits(Commits* commits);

    int getSelectionRole() const;

    QVector<int> getSourceParents(int sourceRow) const;
    QVector<int> getSourceChildren(int sourceRow) const;

private:
    struct Filter
    {
        enum class Type
        {
            Search,
            Git
        };

        Type type;
        std::vector<char> visible;
    };

private:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
//    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;

    /**
        Make parents and children visible
    */
    void makeDirectRelationsVisible(int sourceRow, Filter &filter);
    void makeChildrenVisible(int sourceRow, Filter &filter);
    void makeParentsVisible(int sourceRow, Filter &filter);

private:
    Commits* m_commits;
    std::vector<Filter> m_filters;
};
