#pragma once

#include "framework.h"

class Brushes
{
public:

    static HBRUSH getLightBrush() noexcept { return g_.getLightBrush_(); }
    static HBRUSH getDarkBrush () noexcept { return g_.getDarkBrush_ (); }
    static HBRUSH getGreenBrush() noexcept { return g_.getGreenBrush_(); }
    static HBRUSH getCritBrush () noexcept { return g_.getCritBrush_ (); }
    static HPEN   getCritPen   () noexcept { return g_.getCritPen_   (); }

protected:
    Brushes() noexcept
        : blt_(NULL)
        , bdk_(NULL)
        , bgr_(NULL)
        , bct_(NULL)
        , pct_(NULL)
        , good_(false)
    {
        blt_ = CreateSolidBrush(RGB(128,  64, 0));
        bdk_ = CreateSolidBrush(RGB( 64,  32, 0));
        bgr_ = CreateSolidBrush(RGB(  0, 128, 0));
        bct_ = CreateSolidBrush(RGB(192,  64, 0));

        pct_ = CreatePen(PS_SOLID, 5, RGB(192, 64, 0));

        good_ = blt_ && bdk_ && bgr_ && pct_;
    }
    ~Brushes() noexcept
    {
#define SAFE_DELETE_OBJECT(x) if (x) DeleteObject(x);

        SAFE_DELETE_OBJECT(pct_);

        SAFE_DELETE_OBJECT(blt_);
        SAFE_DELETE_OBJECT(bdk_);
        SAFE_DELETE_OBJECT(bgr_);
    }

    HBRUSH getLightBrush_() const noexcept
    {
        return good_ ? blt_ : NULL;
    }
    HBRUSH getDarkBrush_() const noexcept
    {
        return good_ ? bdk_ : NULL;
    }
    HBRUSH getGreenBrush_() const noexcept
    {
        return good_ ? bgr_ : NULL;
    }
    HBRUSH getCritBrush_() const noexcept
    {
        return good_ ? bct_ : NULL;
    }
    HPEN getCritPen_() const noexcept
    {
        return good_ ? pct_ : NULL;
    }

    HBRUSH blt_, bdk_, bgr_, bct_;
    HPEN   pct_;
    bool good_;

    static Brushes g_;
};
