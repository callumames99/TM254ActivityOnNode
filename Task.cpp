#include "Task.h"
#include "Brushes.h"

#include <algorithm>


Brushes Brushes::g_;


struct TaskDrawAlignment
{
    unsigned int c[3], r[3];
    unsigned int x[3], y[3];

    inline void pad(unsigned int amount) noexcept
    {
        c[0] += amount;   c[1] += amount;   c[2] += amount;
        r[0] += amount;   r[1] += amount;   r[2] += amount;
    }

    inline void derive(unsigned int atX, unsigned int atY) noexcept
    {
        x[0] = atX + c[0];
        y[0] = atY + r[0];

        x[1] = x[0] + c[1];
        y[1] = y[0] + r[1];

        x[2] = x[1] + c[2];
        y[2] = y[1] + r[2];
    }
};



static void uiNumWH(HDC dc, unsigned int& w, unsigned int& h, unsigned int num)
{
    RECT rct = {0, 0, 0, 0};
    wchar_t buf[64];
    unsigned int nw, nh;

    _ultow_s(num, buf, 64, 10);
    DrawTextW(dc, buf, -1, &rct, DT_CALCRECT);
    nw = rct.right - rct.left;
    nh = rct.bottom - rct.top;

    if (nw > w) w = nw;
    if (nh > h) h = nh;
}

static void textWH(HDC dc, unsigned int& w, unsigned int& h, wchar_t const *txt) noexcept
{
    RECT rct = {0, 0, 0, 0};
    unsigned int nw, nh;

    DrawTextW(dc, txt, -1, &rct, DT_CALCRECT);
    nw = rct.right - rct.left;
    nh = rct.bottom - rct.top;

    if (nw > w) w = nw;
    if (nh > h) h = nh;
}

static void uiNumdraw(HDC dc, RECT& rct, unsigned int num) noexcept
{
    wchar_t buf[64];
    _ui64tow_s(num, buf, 64, 10);
    DrawTextW(dc, buf, -1, &rct, DT_CENTER | DT_NOCLIP);
}


void Task::draw(PAINTSTRUCT& ps, wchar_t const *name) const noexcept
{
    unsigned int const padding = 16U;
    unsigned int const halfpad = padding>>1;

    TaskDrawAlignment al {};
    RECT rct {};
    HGDIOBJ oldBrush = NULL;
    COLORREF oldPen;
    HBRUSH brush = NULL;
    bool isCritical = false;


    if (wcscmp(name, L"Start") == 0)
    {
        drawStart(ps);
        return;
    }
    if (wcscmp(name, L"Finish") == 0)
    {
        drawFinish(ps);
        return;
    }

    /* Critical node? */
    isCritical = (flt_ == 0);

    /* Get row and column extents */
    uiNumWH(ps.hdc, al.c[0], al.r[0], start_);
    uiNumWH(ps.hdc, al.c[1], al.r[0], ct_);
    uiNumWH(ps.hdc, al.c[2], al.r[0], end_);
    textWH (ps.hdc, al.c[0], al.r[1], L"IN");
    textWH (ps.hdc, al.c[1], al.r[1], name);
    textWH (ps.hdc, al.c[2], al.r[1], L"OUT");
    uiNumWH(ps.hdc, al.c[0], al.r[2], fla_);
    uiNumWH(ps.hdc, al.c[1], al.r[2], flt_);
    uiNumWH(ps.hdc, al.c[2], al.r[2], flb_);
    al.pad(padding);
    al.derive(px_, py_);

    /* Assign in and out coordinates */
    outx_ = al.x[2];
    midy_ = al.y[0] + (al.r[1]>>1);
    boty_ = al.y[2];

    /* Light checker colours */
    if (isCritical)
    {
        oldBrush = SelectObject(ps.hdc, Brushes::getCritBrush());
        oldPen   = SetTextColor(ps.hdc, RGB( 32,   0,   0));
    }
    else
    {
        oldBrush = SelectObject(ps.hdc, Brushes::getLightBrush());
        oldPen   = SetTextColor(ps.hdc, RGB(255, 255, 255));
    }

    /* Light checker row 0 */
    Rectangle(ps.hdc, px_, py_, al.x[0], al.y[0]);
    Rectangle(ps.hdc, al.x[1], py_, al.x[2], al.y[0]);
    /* Light checker row 1 */
    Rectangle(ps.hdc, al.x[0], al.y[0], al.x[1], al.y[1]);
    /* Light checker row 2 */
    Rectangle(ps.hdc, px_, al.y[1], al.x[0], al.y[2]);
    Rectangle(ps.hdc, al.x[1], al.y[1], al.x[2], al.y[2]);

    /* Dark checker colours */
    if (!isCritical)
    {
        SelectObject(ps.hdc, Brushes::getDarkBrush());
    }

    /* Dark checker row 0 */
    Rectangle(ps.hdc, al.x[0], py_, al.x[1], al.y[0]);
    /* Dark checker row 1 */
    Rectangle(ps.hdc, px_, al.y[0], al.x[0], al.y[1]);
    Rectangle(ps.hdc, al.x[1], al.y[0], al.x[2], al.y[1]);
    /* Dark checker row 2 */
    Rectangle(ps.hdc, al.x[0], al.y[1], al.x[1], al.y[2]);

    /* Draw Text (row 0) */
    rct.top    = py_ + halfpad;
    rct.bottom = al.y[0] - halfpad;

    rct.left   = px_ + halfpad;
    rct.right  = al.x[0] - halfpad;
    uiNumdraw(ps.hdc, rct, start_);

    rct.left   = al.x[0] + halfpad;
    rct.right  = al.x[1] - halfpad;
    uiNumdraw(ps.hdc, rct, ct_);

    rct.left   = al.x[1] + halfpad;
    rct.right  = al.x[2] - halfpad;
    uiNumdraw(ps.hdc, rct, end_);

    /* Draw Text (row 1) */
    rct.top    = al.y[0] + halfpad;
    rct.bottom = al.y[1] - halfpad;

    rct.left   = px_ + halfpad;
    rct.right  = al.x[0] - halfpad;
    DrawTextW(ps.hdc, L"IN", -1, &rct, DT_CENTER | DT_NOCLIP);

    rct.left   = al.x[0] + halfpad;
    rct.right  = al.x[1] - halfpad;
    DrawTextW(ps.hdc, name, -1, &rct, DT_CENTER | DT_NOCLIP);

    rct.left   = al.x[1] + halfpad;
    rct.right  = al.x[2] - halfpad;
    DrawTextW(ps.hdc, L"OUT", -1, &rct, DT_CENTER | DT_NOCLIP);

    /* Draw Text (row 2) */
    rct.top    = al.y[1] + halfpad;
    rct.bottom = al.y[2] - halfpad;

    rct.left   = px_ + halfpad;
    rct.right  = al.x[0] - halfpad;
    uiNumdraw(ps.hdc, rct, fla_);

    rct.left   = al.x[0] + halfpad;
    rct.right  = al.x[1] - halfpad;
    uiNumdraw(ps.hdc, rct, flt_);

    rct.left   = al.x[1] + halfpad;
    rct.right  = al.x[2] - halfpad;
    uiNumdraw(ps.hdc, rct, flb_);

    /* Restore old colours */
    SelectObject(ps.hdc, oldBrush);
    SetTextColor(ps.hdc, oldPen);
}


void Task::drawStart(PAINTSTRUCT& ps) const noexcept
{
    unsigned int const padding = 16U;
    unsigned int const halfpad = padding >> 1;
    RECT rct {};
    HGDIOBJ oldBrush = NULL;
    COLORREF oldPen;
    HBRUSH brush = NULL;

    /* Get text dimensions */
    DrawTextW(ps.hdc, L"Start", -1, &rct, DT_CALCRECT);
    rct.left   += px_;
    rct.top    += py_;
    rct.right  += px_ + padding;
    rct.bottom += py_ + padding;

    /* Assign in and out coordinates */
    outx_ = rct.right;
    midy_ = (rct.top + rct.bottom) >> 1;
    boty_ = rct.bottom;

    /* Colours */
    oldBrush = SelectObject(ps.hdc, Brushes::getGreenBrush());
    oldPen   = SetTextColor(ps.hdc, RGB(64, 255, 64));

    /* Draw box */
    Rectangle(ps.hdc, rct.left, rct.top, rct.right, rct.bottom);

    /* Draw centred text */
    rct.top += halfpad;
    DrawTextW(ps.hdc, L"Start", -1, &rct, DT_CENTER | DT_NOCLIP);

    /* Restore old colours */
    SelectObject(ps.hdc, oldBrush);
    SetTextColor(ps.hdc, oldPen);
}


void Task::drawFinish(PAINTSTRUCT& ps) const noexcept
{
    unsigned int const padding = 16U;
    unsigned int const halfpad = padding >> 1;
    unsigned int col, row1, row2;
    RECT rct {};
    HGDIOBJ oldBrush = NULL;
    COLORREF oldPen;
    HBRUSH brush = NULL;

    wchar_t finishTimeBuf[64];

    /* Get text dimensions */
    DrawTextW(ps.hdc, L"Finish", -1, &rct, DT_CALCRECT);
    col = padding + rct.right - rct.left;
    row1 = padding + rct.bottom - rct.top;

    /* Get finish time dimensions */
    _ultow_s(end_, finishTimeBuf, 10);

    /* Calculate dimensions */
    DrawTextW(ps.hdc, finishTimeBuf, -1, &rct, DT_CALCRECT);
    col = std::max((unsigned long)col, padding + rct.right - rct.left);
    row2 = padding + rct.bottom - rct.top;

    /* Get full rect */
    rct.left   = px_;
    rct.top    = py_;
    rct.right  = px_ + col;
    rct.bottom = py_ + row1 + row2;

    /* Assign in and out coordinates */
    outx_ = rct.right;
    midy_ = (rct.top + rct.bottom) >> 1;
    boty_ = rct.bottom;

    /* Colours */
    oldBrush = SelectObject(ps.hdc, Brushes::getGreenBrush());
    oldPen   = SetTextColor(ps.hdc, RGB(64, 255, 64));

    /* Draw box */
    rct.bottom = py_ + row1;
    Rectangle(ps.hdc, rct.left, rct.top, rct.right, rct.bottom);

    /* Draw centred text */
    rct.top += halfpad;
    DrawTextW(ps.hdc, L"Finish", -1, &rct, DT_CENTER | DT_NOCLIP);

    /* Draw box */
    rct.top = py_ + row1;
    rct.bottom = rct.top + row2;
    Rectangle(ps.hdc, rct.left, rct.top, rct.right, rct.bottom);

    /* Draw centred text */
    rct.top += halfpad;
    DrawTextW(ps.hdc, finishTimeBuf, -1, &rct, DT_CENTER | DT_NOCLIP);

    /* Restore old colours */
    SelectObject(ps.hdc, oldBrush);
    SetTextColor(ps.hdc, oldPen);
}




static void uiNumRect(HDC dc, RECT& rct, unsigned int num)
{
    wchar_t buf[64];

    _ui64tow_s(num, buf, 64, 10);

    rct.left   = 0;
    rct.top    = 0;
    rct.right  = 0;
    rct.bottom = 0;
    DrawTextW(dc, buf, -1, &rct, DT_CALCRECT);
}

inline static void rectPad(RECT& rct, unsigned int amount)
{
    rct.left   -= amount;
    rct.top    -= amount;
    rct.right  += amount;
    rct.bottom += amount;
}

