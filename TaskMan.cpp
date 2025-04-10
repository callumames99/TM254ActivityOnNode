#include "TaskMan.h"

#include <queue>


Task TaskMan::start_(0);
Task TaskMan::finish_(0, 512, 0);


TaskMan::TaskMan()
    : dragx_(0)
    , dragy_(0)
{
    std::wstring const strStart (L"Start");
    std::wstring const strFinish(L"Finish");

    map_.insert(std::pair<std::wstring const &, Task *>(strStart,  &start_));
    map_.insert(std::pair<std::wstring const &, Task *>(strFinish, &finish_));

    graph(strStart, strFinish);
}


TaskMan::~TaskMan()
{
    clearMapOnly();
}


void TaskMan::clearMap() noexcept
{
    clearMapOnly();
    graph_.clear();
    rgraph_.clear();
    selected_.clear();
}


void TaskMan::clearMapOnly() noexcept
{
    std::map<std::wstring, Task *>::iterator mit;
    
    for (mit = map_.begin(); mit != map_.end();)
    {
        auto current = mit++;
        Task *t = current->second;

        /* Start and Finish are static objects that cannot be deleted */
        if (t == &start_ || t == &finish_) continue;

        /* Delete task */
        delete t;

        /* Erase from map */
        map_.erase(current);
    }
}


bool TaskMan::addTask(
    std::wstring&& name,
    completion_time ct)
    noexcept
{
    Task *t;

    /* Allocate task object on the heap */
    t = new(std::nothrow) Task(ct);
    if (!t) return false;

    /* Attempt to insert into the map */
    auto r = map_.insert(std::pair<std::wstring&&, Task *>(std::move(name), t));
    if (!r.second)
    {
        delete t;
        return false;
    }

    return true;
}


bool TaskMan::delTask(std::wstring const& name) noexcept
{
    std::map<std::wstring, Task *>::iterator mit;
    std::multimap<Task *, Task *>::iterator git, sit;
    Task *t;

    /* Start and Finish cannot be deleted */
    if (name.compare(L"Finish") == 0 ||
        name.compare(L"Start")  == 0)
    {
        return false;
    }

    /* Find task in map */
    mit = map_.find(name);
    if (mit == map_.end()) return false;
    t = mit->second;

    /* Find graphs out of 'from' */
    auto range = graph_.equal_range(t);

    for (git = range.first; git != range.second; ++git)
    {
        /* Find matching graph 'into' */
        auto subrange = rgraph_.equal_range(git->second);

        for (sit = subrange.first; sit != subrange.second; ++sit)
        {
            if (git->first == sit->second)
            {
                /* SCORE! Erase these, free memory and finish */
                graph_. erase(git);
                rgraph_.erase(sit);
                map_.   erase(mit);
                delete t;
                return true;
            }
        }
    }

    return false;
}


bool TaskMan::delTask(Task *t) noexcept
{
    std::map<std::wstring, Task *>::iterator mit;
    std::multimap<Task *, Task *>::iterator git, sit;

    /* Reverse find in map */
    for (mit = map_.begin(); mit != map_.end(); ++mit)
    {
        if (mit->first.compare(L"Start") == 0 ||
            mit->first.compare(L"Finish") == 0)
        {
            continue;
        }
        if (mit->second == t) break;
    }
    if (mit == map_.end()) return false;

    /* Find graphs out of 'from' */
    auto range = graph_.equal_range(t);

    for (git = range.first; git != range.second; ++git)
    {
        /* Find matching graph 'into' */
        auto subrange = rgraph_.equal_range(git->second);

        for (sit = subrange.first; sit != subrange.second; ++sit)
        {
            if (git->first == sit->second)
            {
                /* SCORE! Erase these and finish */
                graph_. erase(git);
                rgraph_.erase(sit);
                map_.   erase(mit);
                delete t;
                return true;
            }
        }
    }

    return false;
}


bool TaskMan::discover(
    TaskGraph& out,
    std::wstring const& from,
    std::wstring const& to)
    noexcept
{
    std::map<std::wstring, Task *>::const_iterator rf, rt;

    /* Discover whether named nodes exist */
    rf = map_.find(from);
    rt = map_.find(to);

    if (rf == map_.cend() || rt == map_.cend())
        return false;

    /* Aggregate pointers into graph */
    out.from = rf->second;
    out.to   = rt->second;

    return true;
}


Task *TaskMan::discover(std::wstring const& name) noexcept
{
    std::map<std::wstring, Task *>::const_iterator it;

    it = map_.find(name);
    if (it == map_.cend()) return nullptr;

    return it->second;
}


Task *TaskMan::discover(wchar_t const *name) noexcept
{
    std::map<std::wstring, Task *>::const_iterator it;

    it = map_.find(name);
    if (it == map_.cend()) return nullptr;

    return it->second;
}


bool TaskMan::toggleGraph(Task *a, Task *b) noexcept
{
    std::multimap<Task *, Task *>::iterator fit, bit;
    bool nofit, nobit;

    /* Cannot (un)graph to/from oneself */
    if (a == b) return false;

    /* Find match in 'from' (a) connections, to b */
    auto range = graph_.equal_range(a);
    for (fit = range.first; fit != range.second; ++fit)
    {
        if (fit->second == b)
            break;
    }
    nofit = (fit == range.second);

    /* Find match in 'to' (b) connections, to a */
    range = rgraph_.equal_range(b);
    for (bit = range.first; bit != range.second; ++bit)
    {
        if (bit->second == a)
            break;
    }
    nobit = (bit == range.second);

    /* nobit and nofit should be equal */
    if (nofit) {
        if (nobit) {
            /* GOOD! Make graph */
            return graph(a, b);
        } else {
            /* BUG! */
            _ASSERT(FALSE);
            /* But easy fix */
            rgraph_.erase(bit);
        }
    } else {
        if (nobit) {
            /* BUG! */
            _ASSERT(FALSE);
            /* But easy fix */
            graph_.erase(fit);
        } else {
            /* GOOD! Destroy graph */
            graph_. erase(fit);
            rgraph_.erase(bit);
        }
    }

    return true;
}


bool TaskMan::graph(
    std::wstring const& from,
    std::wstring const& to)
    noexcept
{
    std::map<Task *, Task *>::iterator mit;
    TaskGraph g;

    /* Get task pointers from names */
    if (!discover(g, from, to)) return false;

    /* Cannot graph to oneself */
    if (g.from == g.to) return false;

    /* Cannot graph to start or from finish */
    if (g.from == &finish_ || g.to == &start_) return false;

    /* Obtain range of graphs out of 'from' */
    auto range = graph_.equal_range(g.from);

    /* Find (if exists) corresponding graph into 'to' */
    for (mit = range.first; mit != range.second; ++mit)
    {
        /* If exists graph */
        if (mit->second == g.to)
        {
            /* Already done */
            return true;
        }
    }

    /* Call subroutine */
    return graph(g);
}


bool TaskMan::graph(Task *a, Task *b) noexcept
{
    std::map<Task *, Task *>::iterator git;

    /* Cannot graph from finish or to start */
    if (a == &finish_ || b == &start_) return false;

    /* Create forward graph */
    try {
        git = graph_.insert(std::make_pair(a, b));
    } catch (...) {
        return false;
    }

    /* Create reverse graph */
    try {
        rgraph_.insert(std::make_pair(b, a));
    } catch (...) {
        graph_.erase(git);
        return false;
    }

    return true;
}


bool TaskMan::graph(TaskGraph const& g) noexcept
{
    std::map<Task *, Task *>::iterator git;

    /* Cannot graph from finish or to start */
    if (g.from == &finish_ || g.to == &start_) return false;

    /* Create forward graph */
    try {
        git = graph_.insert(std::make_pair(g.from, g.to));
    } catch (...) {
        return false;
    }

    /* Create reverse graph */
    try {
        rgraph_.insert(std::make_pair(g.to, g.from));
    } catch (...) {
        graph_.erase(git);
        return false;
    }

    return true;
}


bool TaskMan::ungraph(
    std::wstring const& from,
    std::wstring const& to)
    noexcept
{
    std::map<Task *, Task *>::iterator fit, bit;
    TaskGraph g;

    /* Get task pointers from names */
    if (!discover(g, from, to)) return false;

    /* Cannot (un)graph to/from oneself */
    if (g.from == g.to) return false;

    /* Obtain range of graphs out of 'from' */
    auto range = graph_.equal_range(g.from);

    /* Find (if exists) corresponding graph into 'to' */
    for (fit = range.first; fit != range.second; ++fit)
    {
        /* If exists graph */
        if (fit->second == g.to)
            break;
    }
    /* No hit? */
    if (fit == range.second) return false;

    /* Find (if exists) corresponding graph into 'to' */
    for (fit = range.first; fit != range.second; ++fit)
    {
        /* If exists graph */
        if (fit->second == g.to)
            break;
    }
    /* No hit? */
    if (fit == range.second)
        return false;

    /* Obtain range of graphs in to 'to' */
    range = rgraph_.equal_range(g.to);

    /* Find (if exists) corresponding graph from 'from' */
    for (bit = range.first; bit != range.second; ++bit)
    {
        /* If exists graph */
        if (bit->second == g.from)
            break;
    }
    /* No hit is a bug */
    _ASSERT(bit != range.second);

    /* Erase */
    graph_. erase(fit);
    rgraph_.erase(bit);

    return true;
}


/* Initialisation */
void TaskMan::passInit() noexcept
{
    /* For each task, do:
     * - Reset stats
     * - Ensure inputs
     * - Ensure outputs */

    std::map<std::wstring, Task *>::iterator mit;
    Task *t;

    /* For each task */
    for (mit = map_.begin(); mit != map_.end(); ++mit)
    {
        t = mit->second;

        /* Reset stats */
        t->clear();

        /* Ensure at least one input (except for start) */
        if (t != &start_ && rgraph_.count(t) == 0)
        {
            graph(&start_, t);
        }

        /* Ensure at least one output (except for finish) */
        if (t != &finish_ && graph_.count(t) == 0)
        {
            graph(t, &finish_);
        }
    }
}


bool TaskMan::cyclicCheck() const noexcept
{
    std::queue<Task *> q;
    std::set<TaskGraph> closed;

    struct TaskDFS
    {
        TaskDFS const *parent;
        Task          *node;
    };

    TaskDFS init {nullptr, &start_};

    auto dfs = [&](auto& self, TaskDFS const& link)->bool
    {
        std::multimap<Task *, Task *>::const_iterator git;
        TaskDFS nextit {};
        TaskDFS const *c, *n;

        /* Linear walk parents, searching for equality */
        for (c = &link;;)
        {
            n = c->parent;
            if (!n) break;
            if (n->node == link.node)
                return true;
            c = n;
        }

        /* Recursively walk children */
        nextit.parent = &link;
        auto range = graph_.equal_range(link.node);
        for (git = range.first; git != range.second; ++git)
        {
            nextit.node = git->second;
            if (self(self, nextit))
                return true;
        }

        return false;
    };

    return dfs(dfs, init);
}


bool TaskMan::passFwd() noexcept
{
    std::multimap<Task *, Task *>::const_iterator git;
    std::queue<Task *> q;
    Task *t, *n;

    /* Initialise start node */
    start_.setStart(0);

    /* Add it to queue */
    q.push(&start_);

    /* For each node still in the queue */
    while (!q.empty())
    {
        /* Obtain next node */
        t = q.front();
        q.pop();

        /* Gather its range */
        auto range = graph_.equal_range(t);

        /* Iterate through outbound connected nodes */
        for (git = range.first; git != range.second; ++git)
        {
            n = git->second;

            /* Add to queue */
            q.push(n);

            /* Set its start */
            n->setStart(t->getEnd());
        }
    }

    return true;
}


bool TaskMan::passBack() noexcept
{
    std::multimap<Task *, Task *>::const_iterator git;
    std::queue<Task *> q;
    Task *t, *n;

    /* Set up finish node */
    finish_.setBackfloat(finish_.getEnd());

    /* Add finish to queue */
    q.push(&finish_);

    /* For each node still in the queue */
    while (!q.empty())
    {
        /* Obtain next node */
        t = q.front();
        q.pop();

        /* Gather its range */
        auto range = rgraph_.equal_range(t);

        /* Iterate through outbound connected nodes */
        for (git = range.first; git != range.second; ++git)
        {
            n = git->second;

            /* Add to queue */
            q.push(n);

            /* Set its float */
            n->setBackfloat(t->getBackfloat());
        }
    }

    return true;
}


void TaskMan::draw(PAINTSTRUCT& ps) noexcept
{
    std::map<std::wstring, Task *>::const_iterator it;
    std::multimap<Task *, Task *>::const_iterator git;
    std::set<Task *>::const_iterator sit;
    HGDIOBJ old;

    /* Draw task tables */
    for (it = map_.cbegin(); it != map_.cend(); ++it)
    {
        it->second->draw(ps, it->first.c_str());
    }

    /* Draw graphs */
    for (git = graph_.cbegin(); git != graph_.cend(); ++git)
    {
        MoveToEx(ps.hdc, git->first->GetOutX(), git->first->GetOutY(), NULL);
        LineTo(ps.hdc, git->second->GetInX(), git->second->GetInY());
    }

    /* Draw selections */
    old = SelectObject(ps.hdc, GetStockObject(NULL_BRUSH));
    for (sit = selected_.cbegin(); sit != selected_.cend(); ++sit)
    {
        RECT rct = (*sit)->getRect();

        rct.left -= 2;
        rct.right += 2;
        rct.top -= 2;
        rct.bottom += 2;

        Rectangle(ps.hdc, rct.left, rct.top, rct.right, rct.bottom);
    }
    SelectObject(ps.hdc, old);
}


bool TaskMan::mouseHighlight(unsigned int x, unsigned int y) noexcept
{
    std::map<std::wstring, Task *>::const_iterator mit;

    selected_.clear();

    for (mit = map_.cbegin(); mit != map_.cend(); ++mit)
    {
        RECT bb;
        Task *t = mit->second;

        bb = t->getRect();

        if ((signed)x >= bb.left && (signed)x <= bb.right &&
            (signed)y >= bb.top  && (signed)y <= bb.bottom)
        {
            selected_.insert(t);
            return true;
        }
    }

    return false;
}


bool TaskMan::mouseSelect(unsigned int x, unsigned int y) noexcept
{
    std::map<std::wstring, Task *>::const_iterator mit;

    for (mit = map_.cbegin(); mit != map_.cend(); ++mit)
    {
        RECT bb;
        Task *t = mit->second;

        bb = t->getRect();

        if ((signed)x >= bb.left && (signed)x <= bb.right &&
            (signed)y >= bb.top  && (signed)y <= bb.bottom)
        {
            std::set<Task *>::const_iterator it;

            it = selected_.find(t);
            if (it == selected_.cend())
            {
                selected_.insert(t);
            }
            else
            {
                selected_.erase(it);
            }

            return true;
        }
    }

    return false;
}


void TaskMan::mouseGraph(unsigned int x, unsigned int y) noexcept
{
    std::map<std::wstring, Task *>::const_iterator mit;
    std::set<Task *>::const_iterator it;

    if (selected_.empty()) return;

    for (mit = map_.cbegin(); mit != map_.cend(); ++mit)
    {
        RECT bb;
        Task *t = mit->second;

        bb = t->getRect();

        if ((signed)x >= bb.left && (signed)x <= bb.right &&
            (signed)y >= bb.top  && (signed)y <= bb.bottom)
        {
            break;
        }
    }
    if (mit == map_.cend()) return;

    for (it = selected_.cbegin(); it != selected_.cend(); ++it)
    {
        toggleGraph(*it, mit->second);
    }
}


void TaskMan::deleteSelected() noexcept
{
    std::set<Task *>::iterator it;

    for (it = selected_.begin(); it != selected_.end();)
    {
        std::set<Task *>::iterator current = it++;
        delTask(*current);
    }
}


bool TaskMan::commitDrag(HWND w) noexcept
{
    std::set<Task *>::const_iterator sit;

    if (!dragx_ || !dragy_) return false;

    for (sit = selected_.cbegin(); sit != selected_.cend(); ++sit)
    {
        Task *t = *sit;

        t->moveBy(dragx_, dragy_);
    }

    dragx_ = dragy_ = 0;
    return true;
}


void TaskMan::mouseDrag(HWND w, int x, int y) noexcept
{
    dragx_ = x;
    dragy_ = y;
}


bool TaskMan::loadTasks(std::wstring const& filename) noexcept
{
    static char const sig[] = {"TASKFILE"};

    std::map<std::wstring, Task *>::iterator mit;

    FILE *f = NULL;
    size_t namesz = 0;
    unsigned short numTasks, numGraphs, i;
    wchar_t *namebuf = NULL;

    std::map<unsigned short, Task *> id2p;

    /* Clear all */
    clearMap();

    /* Open file */
    _wfopen_s(&f, filename.c_str(), L"rb");
    if (!f)
    {
        MessageBox(NULL, TEXT("Error opening file."), TEXT("Error"), MB_ICONHAND);
        return false;
    }

    /* Check signature */
    do {
        char insig[8];
        if (fread(insig, 1, 8, f) != 8) goto L_ErrorRead;
        if (memcmp(sig, insig, 8) != 0)
        {
            MessageBox(NULL, TEXT("Not a tasks storage file."), TEXT("Error"), MB_ICONHAND);
            goto L_Error;
        }
    } while(0);

    /* Read in numbers */
    if (fread(&numTasks, sizeof(numTasks), 1, f) != 1) goto L_ErrorRead;
    if (fread(&numGraphs, sizeof(numGraphs), 1, f) != 1) goto L_ErrorRead;

    /* Read in task names */
    namesz = 255;
    namebuf = static_cast<wchar_t*>(calloc(256, sizeof(wchar_t)));
    if (!namebuf) goto L_ErrorMemory;
    for (i = 0; i < numTasks; ++i)
    {
        wchar_t *next = namebuf;
        wint_t ch;

        while ((ch = fgetwc(f)) != EOF)
        {
            /* Increase buffer size as needed */
            ptrdiff_t offset = next - namebuf;
            if ((unsigned)offset > namesz)
            {
                size_t newsz = (namesz+1) << 1;
                wchar_t *newbuf = static_cast<wchar_t *>(realloc(namebuf, newsz));
                if (!newbuf) goto L_ErrorMemory;
                namesz = newsz - 1;
                namebuf = newbuf;
                next = namebuf + offset;
            }

            /* Write character */
            *next = static_cast<wchar_t>(ch);

            /* Terminate on NUL */
            if (*next == L'\0') break;

            /* Iterate to next character */
            ++next;
        }

        /* Add task, if not start or finish */
        if (wcscmp(namebuf, L"Start") != 0 &&
            wcscmp(namebuf, L"Finish") != 0)
        {
            if (!addTask(namebuf, 0))
            {
                MessageBox(NULL, TEXT("Error adding task from file."), TEXT("Error"), MB_ICONHAND);
                goto L_Error;
            }
        }

        /* Add to set */
        Task *ptr = discover(namebuf);
        _ASSERT(ptr != nullptr);
        try {
            id2p.insert(std::make_pair(i, ptr));
        } catch (...) {
            goto L_ErrorMemory;
        }
    }
    free(namebuf);
    namebuf = nullptr;

    /* Read in properties */
    for (mit = map_.begin(); mit != map_.end(); ++mit)
    {
        unsigned int ct, x, y;

        if (fread(&ct, sizeof(ct), 1, f) != 1) goto L_ErrorRead;
        if (fread(&x, sizeof(x), 1, f) != 1) goto L_ErrorRead;
        if (fread(&y, sizeof(y), 1, f) != 1) goto L_ErrorRead;
        mit->second->setTime(ct);
        mit->second->move(x, y);
    }

    /* Read in graphs */
    for (i = 0; i < numGraphs; ++i)
    {
        std::map<unsigned short, Task *>::const_iterator idit;
        Task *from, *to;
        unsigned short fromID, toID;

        /* Read IDs */
        if (fread(&fromID, sizeof(fromID), 1, f) != 1) goto L_ErrorRead;
        if (fread(&toID  , sizeof(toID  ), 1, f) != 1) goto L_ErrorRead;

        /* Convert IDs into pointers */
        idit = id2p.find(fromID);
        if (idit == id2p.cend()) goto L_ErrorCorrupted;
        from = idit->second;

        idit = id2p.find(toID);
        if (idit == id2p.cend()) goto L_ErrorCorrupted;
        to = idit->second;

        /* Graph */
        if (!graph(from, to))
        {
            MessageBox(NULL, TEXT("Error adding graph from file."), TEXT("Error"), MB_ICONHAND);
            goto L_Error;
        }
    }

    fclose(f);
    return true;

L_ErrorRead:
    MessageBox(NULL, TEXT("Error reading file."), TEXT("Error"), MB_ICONHAND);
    goto L_Error;

L_ErrorCorrupted:
    MessageBox(NULL, TEXT("File is corrupted."), TEXT("Error"), MB_ICONHAND);
    goto L_Error;

L_ErrorMemory:
    MessageBox(NULL, TEXT("The system ran out of memory performing this task."), TEXT("Fatal"), MB_ICONHAND);
    goto L_Error;

L_Error:
    if (f) fclose(f);
    free(namebuf);
    return false;
}


bool TaskMan::saveTasks(std::wstring const& filename) noexcept
{
    static char const sig[] = {"TASKFILE"};
    FILE *f = NULL;
    std::map<std::wstring, Task *>::const_iterator mit;
    std::multimap<Task *, Task *>::const_iterator  git;
    unsigned short num, id;

    std::map<Task *, unsigned short> p2id;

    /* Size checks */
    if (map_.size() & ~USHRT_MAX)
    {
        MessageBox(NULL, TEXT("Too many tasks to store."), TEXT("Error"), MB_ICONHAND);
        return false;
    }
    if (graph_.size() & ~USHRT_MAX)
    {
        MessageBox(NULL, TEXT("Too many graphs to store."), TEXT("Error"), MB_ICONHAND);
        return false;
    }

    /* Open file */
    _wfopen_s(&f, filename.c_str(), L"wb");
    if (!f)
    {
        MessageBox(NULL, TEXT("Error opening file."), TEXT("Error"), MB_ICONHAND);
        return false;
    }

    /* Write signature */
    if (fwrite(sig, 1, 8, f) != 8) goto L_ErrorWrite;

    /* Write number of tasks */
    num = static_cast<unsigned short>(map_.size());
    if (fwrite(&num, sizeof(num), 1, f) != 1) goto L_ErrorWrite;

    /* Write number of graphs */
    num = static_cast<unsigned short>(graph_.size());
    if (fwrite(&num, sizeof(num), 1, f) != 1) goto L_ErrorWrite;

    /* Write NUL separated list of tasks and record mapping of Task pointers to indices (IDs) */
    for (id = 0, mit = map_.cbegin(); mit != map_.cend(); ++id, ++mit)
    {
        num = static_cast<unsigned short>(mit->first.size() + 1);
        if (fwrite(mit->first.c_str(), sizeof(wchar_t), num, f) != num) goto L_ErrorWrite;

        try {
            p2id.insert(std::make_pair(mit->second, id));
        } catch (...) {
            goto L_ErrorMemory;
        }
    }

    /* Write length of tasks */
    for (mit = map_.cbegin(); mit != map_.cend(); ++mit)
    {
        Task *t = mit->second;
        unsigned int ct, x, y;

        ct = t->getTime();
        x = t->GetX();
        y = t->GetY();

        if (fwrite(&ct, sizeof(ct), 1, f) != 1) goto L_ErrorWrite;
        if (fwrite(&x, sizeof(x), 1, f) != 1) goto L_ErrorWrite;
        if (fwrite(&y, sizeof(y), 1, f) != 1) goto L_ErrorWrite;
    }

    /* Write connections */
    for (git = graph_.cbegin(); git != graph_.cend(); ++git)
    {
        std::map<Task *, unsigned short>::const_iterator from, to;

        /* Pointer to map iterator */
        from = p2id.find(git->first);
        to   = p2id.find(git->second);
        if (from == p2id.cend() || to == p2id.cend())
        {
            _ASSERT(FALSE);
            continue;
        }

        /* IDs from mapped iterator */
        id = from->second;
        if (fwrite(&id, sizeof(id), 1, f) != 1) goto L_ErrorWrite;
        id = to  ->second;
        if (fwrite(&id, sizeof(id), 1, f) != 1) goto L_ErrorWrite;
    }

    /* All good */
    fclose(f);
    return true;

L_ErrorWrite:
    MessageBox(NULL, TEXT("File write error."), TEXT("Error"), MB_ICONHAND);
    goto L_Error;

L_ErrorMemory:
    MessageBox(NULL, TEXT("The system ran out of memory performing this task."), TEXT("Fatal"), MB_ICONHAND);
    goto L_Error;

L_Error:
    if (f) fclose(f);
    return false;
}

