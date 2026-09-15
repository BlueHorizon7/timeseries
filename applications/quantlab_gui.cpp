#define UNICODE
#define _UNICODE

#include "quant/execution/execution_coordinator.hpp"
#include "quant/execution/execution_engine.hpp"
#include "quant/execution/order_generation.hpp"
#include "quant/execution/portfolio_ledger.hpp"
#include "quant/execution/reconciliation.hpp"
#include "quant/execution/simulated_venue.hpp"

#include <windows.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <cstring>

namespace {

using namespace quant::execution;

constexpr UINT kTimerId = 1;
constexpr UINT kTimerPeriodMs = 1000;

struct GuiState {
    ExecutionEngine engine;
    PortfolioLedger ledger;
    SimulatedExecutionVenue venue;
    ExecutionCoordinator coordinator;

    std::int64_t timestamp{1};

    std::vector<double> equity_history;
    std::vector<Position> displayed_positions;
    std::vector<Order> displayed_orders;

    bool reconciled{true};

    std::unordered_map<std::string, double> prices{
        {"KO", 70.0},
        {"PEP", 170.0}
    };

    std::vector<
        std::unordered_map<std::string, double>
    > price_scenarios{
        {{"KO", 70.0}, {"PEP", 170.0}},
        {{"KO", 71.0}, {"PEP", 169.0}},
        {{"KO", 72.0}, {"PEP", 168.0}},
        {{"KO", 71.0}, {"PEP", 169.0}},
        {{"KO", 70.0}, {"PEP", 170.0}},
        {{"KO", 69.0}, {"PEP", 171.0}},
        {{"KO", 71.0}, {"PEP", 169.0}},
        {{"KO", 73.0}, {"PEP", 167.0}},
        {{"KO", 72.0}, {"PEP", 168.0}},
        {{"KO", 70.0}, {"PEP", 170.0}}
    };

    std::vector<
        std::vector<TargetPosition>
    > target_scenarios{
        {
            {"KO", 100.0},
            {"PEP", -41.0}
        },
        {
            {"KO", 100.0},
            {"PEP", -41.0}
        },
        {
            {"KO", 100.0},
            {"PEP", -41.0}
        },
        {
            {"KO", 0.0},
            {"PEP", 0.0}
        },
        {},
        {
            {"KO", -100.0},
            {"PEP", 41.0}
        },
        {
            {"KO", -100.0},
            {"PEP", 41.0}
        },
        {
            {"KO", 0.0},
            {"PEP", 0.0}
        },
        {
            {"KO", 50.0},
            {"PEP", -20.0}
        },
        {
            {"KO", 0.0},
            {"PEP", 0.0}
        }
    };

    std::size_t scenario_index{0};

    GuiState()
        : ledger(),
          venue({
              .buy_slippage_bps = 1.0,
              .sell_slippage_bps = 1.0,
              .commission_rate = 0.0005,
              .partial_fill_fraction = 1.0,
              .latency = 0
          }),
          coordinator(engine, ledger, venue)
    {
        ledger.cash = 100000.0;

        displayed_positions = positions(ledger);
        displayed_orders = engine.all_orders();

        equity_history.push_back(
            equity(ledger, prices)
        );
    }

    void step()
    {
        if (scenario_index >= target_scenarios.size()) {
            scenario_index = 0;
        }

        prices = price_scenarios[scenario_index];

        std::vector<TargetPosition> current_positions;

        for (const Position& position_value : positions(ledger)) {
            current_positions.push_back({
                position_value.symbol,
                position_value.quantity
            });
        }

        const auto orders = generate_orders(
            current_positions,
            target_scenarios[scenario_index],
            timestamp
        );

        for (const auto& order : orders) {
           static_cast<void>(
    coordinator.submit_and_execute(
        order,
        prices,
        timestamp
    )
);
        }

        mark_to_market(
            ledger,
            prices,
            timestamp
        );

        displayed_positions = positions(ledger);
        displayed_orders = engine.all_orders();

        const auto reconciliation =
            reconcile(engine, ledger);

        reconciled = reconciliation.consistent;

        equity_history.push_back(
            equity(ledger, prices)
        );

        if (equity_history.size() > 120) {
            equity_history.erase(
                equity_history.begin()
            );
        }

        ++timestamp;
        ++scenario_index;
    }
};

GuiState* g_state = nullptr;

std::wstring to_wstring(double value, int precision = 2)
{
    std::wostringstream stream;
    stream << std::fixed << std::setprecision(precision)
           << value;
    return stream.str();
}

std::wstring format_currency(double value)
{
    return L"$" + to_wstring(value, 2);
}

void set_font(HDC dc, int height, int weight = FW_NORMAL)
{
    HFONT font = CreateFontW(
        height,
        0,
        0,
        0,
        weight,
        FALSE,
        FALSE,
        FALSE,
        DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE,
        L"Segoe UI"
    );

    SelectObject(dc, font);
    DeleteObject(font);
}

void draw_text(
    HDC dc,
    int x,
    int y,
    const std::wstring& text,
    int height = 18,
    int weight = FW_NORMAL)
{
    set_font(dc, height, weight);

    TextOutW(
        dc,
        x,
        y,
        text.c_str(),
        static_cast<int>(text.size())
    );
}

void draw_text(
    HDC dc,
    int x,
    int y,
    const char* text,
    int height = 18,
    int weight = FW_NORMAL)
{
    if (text == nullptr) {
        return;
    }

    const std::wstring wide_text(
        text,
        text + std::strlen(text)
    );

    draw_text(
        dc,
        x,
        y,
        wide_text,
        height,
        weight
    );
}

void draw_panel(
    HDC dc,
    int left,
    int top,
    int right,
    int bottom,
    const wchar_t* title)
{
    Rectangle(dc, left, top, right, bottom);

    draw_text(
        dc,
        left + 14,
        top + 12,
        title,
        18,
        FW_BOLD
    );
}

void draw_equity_chart(
    HDC dc,
    int left,
    int top,
    int right,
    int bottom)
{
    if (g_state == nullptr ||
        g_state->equity_history.size() < 2) {
        return;
    }

    const auto& values =
        g_state->equity_history;

    double minimum =
        *std::min_element(values.begin(), values.end());

    double maximum =
        *std::max_element(values.begin(), values.end());

    if (std::abs(maximum - minimum) < 1e-12) {
        maximum += 1.0;
        minimum -= 1.0;
    }

    MoveToEx(dc, left, bottom, nullptr);
    LineTo(dc, right, bottom);

    MoveToEx(dc, left, top, nullptr);
    LineTo(dc, left, bottom);

    const int width = right - left;
    const int height = bottom - top;

    for (std::size_t i = 0; i < values.size(); ++i) {
        const double normalized =
            (values[i] - minimum) /
            (maximum - minimum);

        const int x =
            left +
            static_cast<int>(
                static_cast<double>(i) /
                static_cast<double>(values.size() - 1) *
                static_cast<double>(width)
            );

        const int y =
            bottom -
            static_cast<int>(
                normalized *
                static_cast<double>(height)
            );

        if (i == 0) {
            MoveToEx(dc, x, y, nullptr);
        } else {
            LineTo(dc, x, y);
        }
    }

    draw_text(
        dc,
        left + 8,
        top + 8,
        L"Equity",
        15,
        FW_BOLD
    );

    draw_text(
        dc,
        right - 120,
        top + 8,
        format_currency(maximum),
        14
    );

    draw_text(
        dc,
        right - 120,
        bottom - 22,
        format_currency(minimum),
        14
    );
}

void paint(HWND window, HDC dc)
{
    RECT client{};
    GetClientRect(window, &client);

    const int width = client.right;
    const int height = client.bottom;

    HBRUSH background =
        CreateSolidBrush(RGB(245, 246, 248));

    FillRect(
        dc,
        &client,
        background
    );

    DeleteObject(background);

    draw_text(
        dc,
        24,
        18,
        L"QUANTLAB",
        28,
        FW_BOLD
    );

    draw_text(
        dc,
        width - 180,
        24,
        L"PAPER MODE  ●",
        17,
        FW_BOLD
    );

    const int margin = 24;
    const int content_top = 70;
    const int left_width = 310;
    const int gap = 18;

    const int left = margin;
    const int mid = left + left_width + gap;
    const int right = width - margin;

    const int top = content_top;
    const int portfolio_bottom = 300;
    const int lower_top = portfolio_bottom + gap;
    const int bottom = height - 70;

    draw_panel(
        dc,
        left,
        top,
        left + left_width,
        portfolio_bottom,
        L"PORTFOLIO"
    );

    draw_panel(
        dc,
        mid,
        top,
        right,
        portfolio_bottom,
        L"EQUITY CURVE"
    );

    draw_panel(
        dc,
        left,
        lower_top,
        right,
        bottom,
        L"POSITIONS / ORDERS"
    );

    if (g_state == nullptr) {
        return;
    }

    int y = top + 52;

    draw_text(
        dc,
        left + 16,
        y,
        L"Equity",
        16
    );

    draw_text(
        dc,
        left + 175,
        y,
        format_currency(
            equity(g_state->ledger, g_state->prices)
        ),
        16,
        FW_BOLD
    );

    y += 32;

    draw_text(
        dc,
        left + 16,
        y,
        L"Cash",
        16
    );

    draw_text(
        dc,
        left + 175,
        y,
        format_currency(
            g_state->ledger.cash
        ),
        16
    );

    y += 32;

    draw_text(
        dc,
        left + 16,
        y,
        L"Realized PnL",
        16
    );

    draw_text(
        dc,
        left + 175,
        y,
        format_currency(
            g_state->ledger.realized_pnl
        ),
        16
    );

    y += 32;

    draw_text(
        dc,
        left + 16,
        y,
        L"Unrealized PnL",
        16
    );

    draw_text(
        dc,
        left + 175,
        y,
        format_currency(
            g_state->ledger.unrealized_pnl
        ),
        16
    );

    y += 32;

    draw_text(
        dc,
        left + 16,
        y,
        L"Commission",
        16
    );

    draw_text(
        dc,
        left + 175,
        y,
        format_currency(
            g_state->ledger.total_commission
        ),
        16
    );

    draw_equity_chart(
        dc,
        mid + 20,
        top + 45,
        right - 20,
        portfolio_bottom - 20
    );

    const int lower_y = lower_top + 46;

    draw_text(
        dc,
        left + 16,
        lower_y,
        L"Symbol",
        14,
        FW_BOLD
    );

    draw_text(
        dc,
        left + 105,
        lower_y,
        L"Qty",
        14,
        FW_BOLD
    );

    draw_text(
        dc,
        left + 170,
        lower_y,
        L"Avg",
        14,
        FW_BOLD
    );

    draw_text(
        dc,
        left + 245,
        lower_y,
        L"Unreal.",
        14,
        FW_BOLD
    );

    int row = lower_y + 28;

    for (const Position& position_value :
         g_state->displayed_positions) {

        draw_text(
            dc,
            left + 16,
            row,
            std::wstring(
                position_value.symbol.begin(),
                position_value.symbol.end()
            ),
            15
        );

        draw_text(
            dc,
            left + 105,
            row,
            to_wstring(
                position_value.quantity,
                2
            ),
            15
        );

        draw_text(
            dc,
            left + 170,
            row,
            to_wstring(
                position_value.average_entry_price,
                2
            ),
            15
        );

        draw_text(
            dc,
            left + 245,
            row,
            format_currency(
                position_value.unrealized_pnl
            ),
            15
        );

        row += 27;
    }

    const int order_x = mid + 18;

    draw_text(
        dc,
        order_x,
        lower_y,
        L"ID",
        14,
        FW_BOLD
    );

    draw_text(
        dc,
        order_x + 45,
        lower_y,
        L"Symbol",
        14,
        FW_BOLD
    );

    draw_text(
        dc,
        order_x + 110,
        lower_y,
        L"Side",
        14,
        FW_BOLD
    );

    draw_text(
        dc,
        order_x + 165,
        lower_y,
        L"Qty",
        14,
        FW_BOLD
    );

    draw_text(
        dc,
        order_x + 220,
        lower_y,
        L"Status",
        14,
        FW_BOLD
    );

    draw_text(
        dc,
        order_x + 315,
        lower_y,
        L"Avg Fill",
        14,
        FW_BOLD
    );

    row = lower_y + 28;

    const std::size_t begin =
        g_state->displayed_orders.size() > 8
            ? g_state->displayed_orders.size() - 8
            : 0;

    for (std::size_t i = begin;
         i < g_state->displayed_orders.size();
         ++i) {

        const Order& order =
            g_state->displayed_orders[i];

        draw_text(
            dc,
            order_x,
            row,
            std::to_wstring(order.id),
            14
        );

        draw_text(
            dc,
            order_x + 45,
            row,
            std::wstring(
                order.symbol.begin(),
                order.symbol.end()
            ),
            14
        );

        draw_text(
            dc,
            order_x + 110,
            row,
            order_side_to_string(
                order.side
            ),
            14
        );

        draw_text(
            dc,
            order_x + 165,
            row,
            to_wstring(
                order.filled_quantity,
                2
            ),
            14
        );

        draw_text(
            dc,
            order_x + 220,
            row,
            order_status_to_string(
                order.status
            ),
            14
        );

        draw_text(
            dc,
            order_x + 315,
            row,
            to_wstring(
                order.average_fill_price,
                2
            ),
            14
        );

        row += 25;

        if (row > bottom - 20) {
            break;
        }
    }

    const int footer_y = height - 42;

    draw_text(
        dc,
        24,
        footer_y,
        L"ENGINE: ONLINE",
        15,
        FW_BOLD
    );

    draw_text(
        dc,
        190,
        footer_y,
        L"EXECUTION: SIMULATED",
        15,
        FW_BOLD
    );

    draw_text(
        dc,
        420,
        footer_y,
        g_state->reconciled
            ? L"RECONCILIATION: OK"
            : L"RECONCILIATION: FAILED",
        15,
        FW_BOLD
    );
}

LRESULT CALLBACK window_proc(
    HWND window,
    UINT message,
    WPARAM w_param,
    LPARAM l_param)
{
    switch (message) {

    case WM_CREATE:
        SetTimer(
            window,
            kTimerId,
            kTimerPeriodMs,
            nullptr
        );
        return 0;

    case WM_TIMER:
        if (g_state != nullptr &&
            w_param == kTimerId) {

            g_state->step();
            InvalidateRect(window, nullptr, FALSE);
        }

        return 0;

    case WM_PAINT:
    {
        PAINTSTRUCT paint_struct{};
        HDC dc = BeginPaint(
            window,
            &paint_struct
        );

        paint(window, dc);

        EndPaint(
            window,
            &paint_struct
        );

        return 0;
    }

    case WM_DESTROY:
        KillTimer(
            window,
            kTimerId
        );

        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(
        window,
        message,
        w_param,
        l_param
    );
}

} // namespace

int WINAPI WinMain(
    HINSTANCE instance,
    HINSTANCE,
    LPSTR,
    int show_command)
{
    GuiState state;
    g_state = &state;

    const wchar_t* class_name =
        L"QuantLabWindowClass";

    WNDCLASSW window_class{};
    window_class.lpfnWndProc = window_proc;
    window_class.hInstance = instance;
    window_class.lpszClassName = class_name;
    window_class.hCursor =
        LoadCursor(nullptr, IDC_ARROW);
    window_class.hbrBackground =
        static_cast<HBRUSH>(
            GetStockObject(WHITE_BRUSH)
        );

    if (!RegisterClassW(&window_class)) {
        return 1;
    }

    HWND window = CreateWindowExW(
        0,
        class_name,
        L"QuantLab",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        1280,
        760,
        nullptr,
        nullptr,
        instance,
        nullptr
    );

    if (window == nullptr) {
        return 1;
    }

    ShowWindow(
        window,
        show_command
    );

    UpdateWindow(window);

    MSG message{};

    while (GetMessageW(
        &message,
        nullptr,
        0,
        0) > 0) {

        TranslateMessage(&message);
        DispatchMessageW(&message);
    }

    g_state = nullptr;

    return static_cast<int>(
        message.wParam
    );
}