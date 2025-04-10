#pragma once

#include <set>
#include <map>
#include <vector>

#include "Task.h"

class TaskMan
{
public:
     TaskMan();
    ~TaskMan();

    bool loadTasks(std::wstring const& filename) noexcept;
    bool saveTasks(std::wstring const& filename) noexcept;

    void clearMap() noexcept;

    inline bool addTask(
        wchar_t const *name,
        completion_time ct)
        noexcept
    {
        return addTask(std::wstring(name), ct);
    }

    bool addTask(
        std::wstring&& name,
        completion_time ct)
        noexcept;

    bool delTask(std::wstring const& name) noexcept;

    bool graph(
        std::wstring const& from,
        std::wstring const& to)
        noexcept;

    bool ungraph(
        std::wstring const& from,
        std::wstring const& to)
        noexcept;

    bool mouseHighlight(unsigned int x, unsigned int y) noexcept;
    bool mouseSelect(unsigned int x, unsigned int y) noexcept;
    void mouseDrag(HWND w, int x, int y) noexcept;
    bool commitDrag(HWND w) noexcept;
    void mouseGraph(unsigned int x, unsigned int y) noexcept;
    void deleteSelected() noexcept;

    void draw(PAINTSTRUCT& ps) noexcept;

    void calculate() noexcept
    {
        passInit();
        if (cyclicCheck())
        {
            MessageBox(NULL,
                TEXT("Cyclic Graph Detected.\n\nPlease remove cyclic dependencies."),
                TEXT("Error"), MB_ICONHAND);
            return;
        }
        passFwd();
        passBack();
    }
    
protected:
    struct TaskGraph
    {
        Task *from, *to;

        static TaskGraph makeTaskGraph(Task *f, Task *t) noexcept
        {
            return TaskGraph{f,t};
        }

        inline bool operator<(TaskGraph const& other) const noexcept
        {
            return from <  other.from ? true :
                   from == other.from ? (to < other.to) : false;
        }
        inline bool operator==(TaskGraph const& other) const noexcept
        {
            return from == other.from && to == other.to;
        }
        inline void swap() noexcept
        {
            Task *swp = from;
            from = to;
            to = swp;
        }
    };

    bool discover(
        TaskGraph& out,
        std::wstring const& from,
        std::wstring const& to)
        noexcept;

    Task *discover(wchar_t const *name) noexcept;
    Task *discover(std::wstring const& name) noexcept;

    bool graph(Task *a, Task *b) noexcept;
    bool graph(TaskGraph const& g) noexcept;
    //bool ungraph(Task *a, Task *b) noexcept;
    bool toggleGraph(Task *a, Task *b) noexcept;
    bool delTask(Task *) noexcept;

    void passInit() noexcept;
    bool cyclicCheck() const noexcept;
    bool passFwd() noexcept;
    bool passBack() noexcept;

    void clearMapOnly() noexcept;



    static Task start_, finish_;

    std::map<std::wstring, Task *> map_;
    std::multimap<Task *, Task *> graph_;
    std::multimap<Task *, Task *> rgraph_;
    std::set<Task *> selected_;

    int dragx_, dragy_;
};

