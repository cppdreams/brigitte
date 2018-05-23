#pragma once

#include <QAbstractListModel>
#include <QSortFilterProxyModel>

#include "Git.h"


class Commits : public QAbstractListModel
{
Q_OBJECT

public slots:
    int getActiveBranchIndex(int row) const;
    QVector<int> getParents(int row) const;
    QVector<int> getChildren(int row) const;

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
        ActiveBranchIndexRole,
        ChildrenRole,
        ParentsRole,
        SelectionRole
    };

private:
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

public:
    FilteredCommits();

    int getSelectionRole() const;

private:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
//    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;
};
