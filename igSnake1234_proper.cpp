#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <windows.h>

#ifdef FULLSCREEN
#define SNAKE_WIDTH 15
#else
#define SNAKE_WIDTH 20
#endif
#define SNAKE_HEIGHT 20
#define SNAKE_MAX_LENGTH 125
#define DELAY_MS 80
// Alternative: TCHAR game_over_text[] = "Game over";
#define GAME_OVER_TEXT "Game over"
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

// Window globals
RECT rect;
HWND hwnd;
HDC hdc;
HDC hdcBackBuffer;
HBITMAP hBitmap;

#ifndef FULLSCREEN
unsigned int win_width;
unsigned int win_height;
#endif

// Snake vars
// then changing size of snake and apple, update apple_* in generate_apple()
// to avoid drawing apples outside of screen
struct snake_s
{
    int x, y;
} snake_blocks[SNAKE_MAX_LENGTH];
unsigned int snake_length = 3;
unsigned int apple_x, apple_y;
enum E_DIR { LEFT, RIGHT, UP, DOWN };
E_DIR direction = RIGHT;
E_DIR real_direction = direction;
unsigned char r, g, b;

// Timer
unsigned int now, last;

// Game loop
bool running = TRUE;
bool game_over = FALSE;

#ifdef FULLSCREEN
static DEVMODE g_ScreenSettings =
{
    { 0 }, 0, 0, sizeof(DEVMODE), 0,
    DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL, { 0 }, 0, 0, 0, 0, 0,
    { 0 }, 0, 32, SCREEN_WIDTH, SCREEN_HEIGHT, { 0 }, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
#endif

unsigned int rand()
{
    // GetTickCount() can be used similar to rand() function;
    // stdlib is not linked so use of rand() was not possible
    return GetTickCount();
}

void generate_apple()
{
generate:
    apple_x = (rand() % 40 + 1) * SNAKE_WIDTH;
    apple_y = (rand() % 30 + 1) * SNAKE_HEIGHT;
    for (unsigned int i = 0; i < snake_length; i++)
    {
        if ((snake_blocks[i].x == apple_x) &&
                (snake_blocks[i].y == apple_y))
            // Watch out for "stack overflow"!
            // It can happen on the beginning of this
            // function or a few lines after it
            // in worst case scenario, but it will not happen :)
            // generate_apple();
            goto generate;
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);

    switch (msg)
    {
    case WM_KEYDOWN:
        switch (wParam)
        {
        case VK_ESCAPE:
            PostMessage(hwnd, WM_CLOSE, 0, 0);
            break;
        case VK_LEFT:
            if (real_direction != RIGHT)
                direction = LEFT;
            break;
        case VK_RIGHT:
            if (real_direction != LEFT)
                direction = RIGHT;
            break;
        case VK_UP:
            if (real_direction != DOWN)
                direction = UP;
            break;
        case VK_DOWN:
            if (real_direction != UP)
                direction = DOWN;
            break;
#ifdef _DEBUG
        case VK_ADD:
            if (snake_length < SNAKE_MAX_LENGTH-1)
            {
                snake_blocks[snake_length] = snake_blocks[snake_length-1];
                ++snake_length;
            }
            break;
        case VK_SUBTRACT:
            if (snake_length > 1)
                --snake_length;
            break;
#endif
        }
        break;
    case WM_CREATE:
        // Create the back buffer
        hdcBackBuffer = CreateCompatibleDC(NULL);

        // Get the device context
        hdc = GetDC(hwnd);

        // Create a bitmap
        hBitmap = CreateCompatibleBitmap(hdc, SCREEN_WIDTH, SCREEN_HEIGHT);

        // Select the bitmap
        SelectObject(hdcBackBuffer, hBitmap);

        ReleaseDC(hwnd, hdc);
        break;
    case WM_PAINT:
        PAINTSTRUCT ps;
        // Get the device context
        hdc = BeginPaint(hwnd, &ps);

        BitBlt(hdcBackBuffer, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT,
               0, 0, 0, BLACKNESS);
        HBRUSH color;
        // Draw a snake
        for (unsigned int i = 0; i < snake_length; i++)
        {
            r += rand() % 16 * 16;
            g += rand() % 16 * 8;
            b += rand() % 16 * 4;
            color = CreateSolidBrush(RGB(r, g, b));
            SelectObject(hdcBackBuffer, color);

            Rectangle(hdcBackBuffer,
                      snake_blocks[i].x,
                      snake_blocks[i].y,
                      snake_blocks[i].x+SNAKE_WIDTH,
                      snake_blocks[i].y+SNAKE_HEIGHT);
            DeleteObject(color);
        }

        // Draw an apple
        color = CreateSolidBrush(RGB(0, 0, 255));
        SelectObject(hdcBackBuffer, color);
        Rectangle(hdcBackBuffer, apple_x, apple_y,
                  apple_x+SNAKE_WIDTH, apple_y+SNAKE_HEIGHT);
        DeleteObject(color);

        // Display text
        if (game_over)
        {
            SetTextColor(hdcBackBuffer, RGB(r, g, b));

#ifdef _DEBUG
            // This text will not be displayed, it's used only for calc
            RECT rc = { 0 };
            DrawText(hdc, GAME_OVER_TEXT, ARRAYSIZE(GAME_OVER_TEXT), &rc, DT_CALCRECT);

            // Text size debug info
            TCHAR buf[100];
            wsprintf(buf, "W: %d H: %d",
                     (SCREEN_WIDTH/2)-rc.right/2,
                     (SCREEN_HEIGHT/2)-rc.bottom/2);
            SetWindowText(hwnd, buf);
#endif
            // Display "Game over" in the centre of the screen
            TextOut(hdcBackBuffer, 347, 292,
                    GAME_OVER_TEXT, 9); // ARRAYSIZE(GAME_OVER_TEXT)
        }

        // Display the back buffer
        BitBlt(hdc, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, hdcBackBuffer,
               0, 0, SRCCOPY);

        EndPaint(hwnd, &ps);
        break;
    case WM_CLOSE:
    case WM_DESTROY:
        running = FALSE;
        break;
    };
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void game_loop()
{
    // Move snake parts
    for (unsigned int i = snake_length-1; i > 0; i--)
        snake_blocks[i] = snake_blocks[i-1];

    // Move snake's head
    switch(direction)
    {
    case LEFT:
        snake_blocks[0].x -= SNAKE_WIDTH;
        if (snake_blocks[0].x < 0)
            snake_blocks[0].x = SCREEN_WIDTH-SNAKE_WIDTH;
        break;
    case RIGHT:
        snake_blocks[0].x += SNAKE_WIDTH;
        if (snake_blocks[0].x > SCREEN_WIDTH-SNAKE_WIDTH)
            snake_blocks[0].x = 0;
        break;
    case UP:
        snake_blocks[0].y -= SNAKE_HEIGHT;
        if (snake_blocks[0].y < 0)
            snake_blocks[0].y = SCREEN_HEIGHT-SNAKE_HEIGHT;
        break;
    case DOWN:
        snake_blocks[0].y += SNAKE_HEIGHT;
        if (snake_blocks[0].y > SCREEN_HEIGHT-SNAKE_HEIGHT)
            snake_blocks[0].y = 0;
        break;
    }
    real_direction = direction;

    // Check if snake ate an apple
    if ((snake_blocks[0].x == apple_x) &&
            (snake_blocks[0].y == apple_y))
    {
        if (snake_length < SNAKE_MAX_LENGTH-1)
        {
            snake_blocks[snake_length] = snake_blocks[snake_length-1];
            ++snake_length;
        }
        generate_apple();
    }

    // Check if snake has bitten himself
    for (unsigned int i = 1; i < snake_length-1; i++)
        if ((snake_blocks[0].x == snake_blocks[i].x) &&
                (snake_blocks[0].y == snake_blocks[i].y))
            game_over = TRUE;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Register window class
    WNDCLASS wc = { 0 };
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = " ";
    RegisterClass(&wc);

    // Create window
#ifdef FULLSCREEN
    ChangeDisplaySettings(&g_ScreenSettings, CDS_FULLSCREEN);
    hwnd = CreateWindow(" ", 0, WS_POPUP | WS_VISIBLE | WS_MAXIMIZE,
                        0, 0, 0, 0, 0, 0, hInstance, 0);

    // about 8 bytes
    ShowCursor(FALSE);
#else
    // Set window style, get real size
    DWORD style = WS_VISIBLE | WS_CAPTION;
    rect = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };

    AdjustWindowRect(&rect, style, FALSE);
    win_width = rect.right - rect.left;
    win_height = rect.bottom - rect.top;
    int x = (GetSystemMetrics(SM_CXSCREEN) - win_width) / 2;
    int y = (GetSystemMetrics(SM_CYSCREEN) - win_height) / 2;

    hwnd = CreateWindowEx(WS_EX_APPWINDOW | WS_EX_WINDOWEDGE, " ", " ", style,
                          x, y, win_width, win_height, 0, 0, hInstance, 0);
#endif

#ifdef _DEBUG
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
#endif

    // Init values
    snake_blocks[0].x = SNAKE_WIDTH*19;
    snake_blocks[0].y = SNAKE_HEIGHT*15;

    generate_apple();

    MSG msg;
    do
    {
        // GetTickCount has poor resolution, but using QueryPerformanceCounter
        // msvcrt.lib ucrtd.lib needs to be imported,
        // which makes it a heavy solution; GetTickCount can work properly
        // for a limited time (~49 days)
        now = GetTickCount();
        if (now-last > DELAY_MS)
        {
            // Game logic
            if (!game_over)
                game_loop();

            // Repaint the screen
            InvalidateRect(hwnd, NULL, FALSE);

            // Update timer
            last = GetTickCount();
        }

        // Process messages (key presses, drawing etc.)
        // 1b can be saved after changing 'while' to 'if'
        while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    while (running);

    // This way process doesn't hang in background
    // ExitProcess(msg.wParam); would be nicer
    ExitProcess(0);
}
