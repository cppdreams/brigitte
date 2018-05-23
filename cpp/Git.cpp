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

std::vector<Commit> Git::readCommits() const
{

    git_repository *repo = nullptr;
    //    if (! check_error(git_repository_open(&repo, "/home/sander/fast/programming/cppdreams"))){
    //    if (! check_error(git_repository_open(&repo, "/home/sanderv/dev/test/git"))){
//    if (! check_error(git_repository_open(&repo, "/home/sander/fast/programming/testgit"))){
    if (! check_error(git_repository_open(&repo, "/home/sander/fast/programming/github/conan"))){
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
        size_t      index;
        git_commit* nextCommit;
    };

    vector<Commit> commits;
    vector<LastCommitData> activeBranches;

    while (git_revwalk_next(&oid, walk) == 0) {
        git_commit *c;
        if (! check_error(git_commit_lookup(&c, repo, &oid))){
            return {};
        }
//        cout << oidstr << " " << git_commit_message(c);

        auto nparents = git_commit_parentcount(c);
        // append commit here

        size_t commitIndex = commits.size();
        {
            char oidstr[40] = {0};
            git_oid_tostr(oidstr, 39, &oid);

            Commit commit;
            commit.sha = oidstr;
            commit.message = git_commit_message(c);
            commits.emplace_back(commit);
        }


        size_t activeBranchIndex = activeBranches.size();
        for (size_t i = 0; i < activeBranches.size();){
            // Does this work, because we free commit pointers... maybe have to compare sha or oid instead?
            if (c == activeBranches[i].nextCommit){
                commits.back().children.push_back(activeBranches[i].index);
                commits[activeBranches[i].index].parents.push_back(commitIndex);

                activeBranches[i].index = commitIndex;

                if (activeBranchIndex == activeBranches.size()){
                    activeBranchIndex = i;
                    ++i;
                } else {
                    activeBranches.erase(activeBranches.begin() + i);
                }
            } else {
                ++i;
            }
        }

        if (activeBranches.empty()){
            activeBranches.emplace_back(LastCommitData{0, c});
        } else if (activeBranchIndex == activeBranches.size()){
            cerr << "unable to find active branch index" << endl;
            return{};
        } else {
            commits.back().activeBranchIndex = activeBranchIndex;
        }

        if (nparents > 1){
            for (unsigned int i = 1; i < nparents; ++i){
                git_commit* parent = nullptr;
                if (! check_error(git_commit_parent(&parent, c, i))){
                    return {};
                }
                activeBranches.emplace_back(LastCommitData{commitIndex,
                                                           parent});
                git_commit_free(parent);
            }
        }

        // else is not missing here, handle the first parent here
        if (nparents >= 1){
            git_commit* parent = nullptr;
            if (! check_error(git_commit_parent(&parent, c, 0))){
                return {};
            }

            activeBranches[activeBranchIndex].nextCommit = parent;
            activeBranches[activeBranchIndex].index = commitIndex;

            git_commit_free(parent);
        }

        git_commit_free(c);
    }

    git_revwalk_free(walk);

    cout << commits.size() << " commits" << endl;

    git_repository_free(repo);

    return commits;
}
