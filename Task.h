#pragma once
#include <string>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

typedef unsigned int completion_time;

class Task
{
friend struct GraphQueueElem;
public:
    Task(completion_time ct)
        : ct_(ct)
        , start_(0)
        , end_(0)
        , fla_(0)
        , flb_(0)
        , flt_(0)
        , px_(0)
        , py_(0)
        , outx_(0)
        , midy_(0)
        , boty_(0)
    {}
    Task(completion_time ct, unsigned int x, unsigned int y)
        : ct_(ct)
        , start_(0)
        , end_(0)
        , fla_(0)
        , flb_(0)
        , flt_(0)
        , px_(x)
        , py_(y)
        , outx_(0)
        , midy_(0)
        , boty_(0)
    {}

    inline void move(unsigned int x, unsigned int y) noexcept
    {
        px_ = x;
        py_ = y;
    }
    inline void moveBy(int x, int y) noexcept
    {
        px_ += x;
        py_ += y;
    }

    void draw(PAINTSTRUCT& ps, wchar_t const *name) const noexcept;

    inline RECT getRect() const noexcept
    {
        RECT r {};

        r.left   = px_;
        r.top    = py_;
        r.right  = outx_;
        r.bottom = boty_;

        return r;
    }

    inline unsigned int GetX()    const noexcept { return px_; }
    inline unsigned int GetY()    const noexcept { return py_; }
    inline unsigned int GetInX()  const noexcept { return px_; }
    inline unsigned int GetInY()  const noexcept { return midy_; }
    inline unsigned int GetOutX() const noexcept { return outx_; }
    inline unsigned int GetOutY() const noexcept { return midy_; }

    inline void clear() noexcept
    {
        start_ = end_ = fla_ = flt_ = 0;
        flb_ = UINT_MAX;
    }

    inline void setStart(completion_time x) noexcept
    {
        if (x >= start_)
        {
            start_ = x;
            end_ = x + ct_;
        }
    }
    inline void setBackfloat(completion_time x) noexcept
    {
        if (x < flb_)
        {
            flb_ = x;
            fla_ = x - ct_;
            flt_ = fla_ - start_;
        }
    }
    inline completion_time getBackfloat() const noexcept
    {
        return fla_;
    }
    inline completion_time getEnd() const noexcept
    {
        return end_;
    }
    inline completion_time getTime() const noexcept
    {
        return ct_;
    }
    inline void setTime(completion_time t) noexcept
    {
        ct_ = t;
    }

protected:
    void drawStart (PAINTSTRUCT& ps) const noexcept; /* Specific draw case */
    void drawFinish(PAINTSTRUCT& ps) const noexcept; /* Specific draw case */


    completion_time ct_, start_, end_, fla_, flb_, flt_;
    unsigned int px_, py_;
    mutable unsigned int outx_, midy_, boty_;
};

