#include "Git.h"

#include <iostream>

#include "git2.h"

using namespace std;


Git::Git()
{
    git_libgit2_init();
}

Git::~Git()
{
    git_libgit2_shutdown();
}

bool check_error(int error)
{
    if (error < 0) {
        const git_error *e = giterr_last();
        cout << "Error " << error << "/" << e->klass << ": " << e->message << endl;
        return false;
    } else {
        return true;
    }
}

struct RefData
{
};

int each_name_cb(const char *name, void *payload)
{
    RefData* d = reinterpret_cast<RefData*>(payload);
    /* … */

    cout << "name: " << name << endl;

    return 0;
}

QString toString(git_commit* commit, git_repository* repo)
{
    char oidstr[10] = {0};
    git_oid oid;
    git_commit_lookup(&commit, repo, &oid);
    git_oid_tostr(oidstr, 9, &oid);
    return QString("%1 %2").arg(oidstr).arg(git_commit_message(commit));
}

std::vector<Commit> Git::readCommits(const QString& projectPath) const
{

    git_repository *repo = nullptr;
    if (! check_error(git_repository_open(&repo, projectPath.toStdString().c_str()))){
        return {};
    }

//    RefData d = {};
//    if (! check_error(git_reference_foreach_glob(repo, "refs/heads/*", each_name_cb, &d))){
//        return {};
//    }

    git_revwalk *walk;
    if (! check_error(git_revwalk_new(&walk, repo))){
        return {};
    }

    git_revwalk_sorting(walk,
                        GIT_SORT_TOPOLOGICAL |
                        GIT_SORT_TIME);
    git_revwalk_push_head(walk);
    //    git_revwalk_push_glob(walk, "refs/heads");
//    git_revwalk_hide_glob(walk, "tags/*");

    //    git_object *obj;
    //    git_revparse_single(&obj, repo, "HEAD~10");
    //    git_revwalk_hide(walk, git_object_id(obj));
    //    git_object_free(obj);

    git_oid oid;

    struct LastCommitData
    {
        git_oid nextCommit;
        size_t  index;
    };

    vector<Commit> commits;
    vector<LastCommitData> activeBranches;

    while (git_revwalk_next(&oid, walk) == 0) {
        git_commit *c;
        if (! check_error(git_commit_lookup(&c, repo, &oid))){
            return {};
        }

        auto nparents = git_commit_parentcount(c);

        size_t commitIndex = commits.size();
        {
            char oidstr[40] = {0};
            git_oid_tostr(oidstr, 39, &oid);

            Commit commit;
            commit.sha = oidstr;
            commit.message = git_commit_message(c);

            const git_signature* author = git_commit_author(c);
            commit.author.name = author->name;
            commit.time = QDateTime::fromSecsSinceEpoch(git_commit_time(c));

            commits.emplace_back(commit);
        }

        size_t branchIndex = activeBranches.size();
        for (size_t i = 0; i < activeBranches.size();){
            if (git_oid_equal(&oid, &activeBranches[i].nextCommit)){
                commits.back().children.push_back(activeBranches[i].index);
                commits[activeBranches[i].index].parents.push_back(commitIndex);

                activeBranches[i].index = commitIndex;

                if (branchIndex == activeBranches.size()){
                    branchIndex = i;
                    ++i;
                } else {
                    activeBranches.erase(activeBranches.begin() + i);
                }
            } else {
                ++i;
            }
        }

        if (activeBranches.empty()){
            activeBranches.emplace_back(LastCommitData{oid, 0});
        } else if (branchIndex == activeBranches.size()){
            cerr << "Unable to find active branch index " << branchIndex << endl
                 << "i: " << commitIndex << " activeBranches: " << activeBranches.size() << endl;
        } else {
            commits.back().branchIndex = branchIndex;
        }

        if (nparents > 1){
            for (unsigned int i = 1; i < nparents; ++i){
                git_commit* parent = nullptr;
                if (! check_error(git_commit_parent(&parent, c, i))){
                    return {};
                }

                const git_oid* parentId = git_commit_id(parent);

                activeBranches.emplace_back(LastCommitData{*parentId,
                                                           commitIndex});
                git_commit_free(parent);
            }
        }

        // else is not missing here, handle the first parent here
        if (nparents >= 1){
            git_commit* parent = nullptr;
            if (! check_error(git_commit_parent(&parent, c, 0))){
                return {};
            }

            const git_oid* parentId = git_commit_id(parent);

            activeBranches[branchIndex].nextCommit = *parentId;
            activeBranches[branchIndex].index = commitIndex;

            git_commit_free(parent);
        }

        git_commit_free(c);
    }

    git_revwalk_free(walk);

    cout << commits.size() << " commits" << endl;

    git_repository_free(repo);

    return commits;
}
