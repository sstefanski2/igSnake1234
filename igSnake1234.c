#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <windows.h>
#include <mmsystem.h>

#define SNAKE_WIDTH 20
#define SNAKE_HEIGHT 20
#define SNAKE_MAX_LENGTH 125
#define DELAY_MS 80
#define GAME_OVER_TEXT "Game over"
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

// Window globals
RECT rect = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
HWND hwnd;
HDC hdc;
HDC hdcBackBuffer;
HBITMAP hBitmap;
DEVMODE g_ScreenSettings =
{
    { 0 }, 0, 0, sizeof(DEVMODE), 0,
    DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL, { 0 }, 0, 0, 0, 0, 0,
    { 0 }, 0, 32, SCREEN_WIDTH, SCREEN_HEIGHT, { 0 }, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

// Snake vars
struct S_SNAKE
{
    int x, y;
} snake_blocks[SNAKE_MAX_LENGTH];
unsigned int snake_length = 3;
unsigned int apple_x;
unsigned int apple_y;
enum E_DIR { LEFT, RIGHT, UP, DOWN };
enum E_DIR direction = RIGHT;
enum E_DIR real_direction = RIGHT;
unsigned char r, g, b;

// Fade effect
enum E_FADE_STATE { NONE, EIN, EOUT };
enum E_FADE_STATE fade_state = NONE;
unsigned char bg_r = 0;
unsigned char bg_g = 0;
unsigned char bg_b = 0;
unsigned char *bg_attr = &bg_r;

// Timer
unsigned int now, last;

// Game loop
BOOL game_over = FALSE;

unsigned int rand()
{
    return GetTickCount();
}

#ifdef _DEBUG
unsigned strlen(char* text)
{
    int length = 0;
    while(*text++)
        ++length;
    return length;
}
#endif

void generate_apple()
{
generate:
    apple_x = (rand() % 39 + 1) * SNAKE_WIDTH;
    apple_y = (rand() % 29 + 1) * SNAKE_HEIGHT;
    for (unsigned int i = 0; i < snake_length; i++)
        if ((snake_blocks[i].x == apple_x) &&
                (snake_blocks[i].y == apple_y))
            goto generate;
}

#ifdef _DEBUG
int CaptureAnImage(HWND hWnd)
{
    // Paste here code of this function from
    // https://msdn.microsoft.com/en-us/library/windows/desktop/dd183402.aspx
    // And change CreateFile(L" to CreateFile("
}
#endif

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
        PlaySound(TEXT("C:\\Windows\\Media\\chord.wav"), NULL,
                  SND_ASYNC|SND_FILENAME);
        if (fade_state == NONE)
        {
            fade_state = EIN;
            switch (GetTickCount() % 3)
            {
            case 0:
                bg_attr = &bg_r;
                break;
            case 1:
                bg_attr = &bg_g;
                break;
            case 2:
                bg_attr = &bg_b;
                break;
            }
        }
        if (snake_length < SNAKE_MAX_LENGTH-1)
        {
            snake_blocks[snake_length] = snake_blocks[snake_length-1];
            ++snake_length;
        }
        generate_apple();
    }

    // Fade effect
    if (fade_state == EIN)
    {
        *bg_attr+=50;
        if (*bg_attr == 250)
            fade_state = EOUT;
    }
    if (fade_state == EOUT)
    {
        *bg_attr-=50;
        if (*bg_attr == 0)
            fade_state = NONE;
    }

    // Check if snake has bitten himself
    for (unsigned int i = snake_length-1; i > 1; i--)
        if ((snake_blocks[0].x == snake_blocks[i].x) &&
                (snake_blocks[0].y == snake_blocks[i].y))
            game_over = TRUE;
}

void init_backbuffer()
{
    // Create the back buffer
    hdcBackBuffer = CreateCompatibleDC(NULL);
    // Get the device context
    hdc = GetDC(hwnd);
    // Create a bitmap
    hBitmap = CreateCompatibleBitmap(hdc, SCREEN_WIDTH, SCREEN_HEIGHT);
    // Select the bitmap
    SelectObject(hdcBackBuffer, hBitmap);
    ReleaseDC(hwnd, hdc);
}

void WINAPI WinMainCRTStartup()
{
    // Create window (textfield control in fullscreen to be more specific)
    ChangeDisplaySettings(&g_ScreenSettings, CDS_FULLSCREEN);
    hwnd = CreateWindow("static", 0, WS_POPUP | WS_VISIBLE | WS_MAXIMIZE,
                        0, 0, 0, 0, 0, 0, 0, 0);
    ShowCursor(FALSE);

    // Init values
    init_backbuffer();
    snake_blocks[0].x = SNAKE_WIDTH*10;
    snake_blocks[0].y = SNAKE_HEIGHT*15;
    generate_apple();

loop:
    now = GetTickCount();
    if (now-last > DELAY_MS)
    {
#ifdef _DEBUG
        if (GetAsyncKeyState(VK_ADD) && (snake_length < SNAKE_MAX_LENGTH-1))
        {
            snake_blocks[snake_length] = snake_blocks[snake_length-1];
            ++snake_length;
        }
        if (GetAsyncKeyState(VK_SUBTRACT) && (snake_length > 1))
            --snake_length;
#endif
        // Game logic
        if (!game_over)
            game_loop();

        // Repaint the screen
        InvalidateRect(hwnd, NULL, FALSE);

        // Update timer
        last = GetTickCount();
    }

    // Process keys
    if (GetAsyncKeyState(VK_ESCAPE))
        ExitProcess(0);
    if (GetAsyncKeyState(VK_LEFT) && real_direction != RIGHT)
        direction = LEFT;
    if (GetAsyncKeyState(VK_RIGHT) && real_direction != LEFT)
        direction = RIGHT;
    if (GetAsyncKeyState(VK_UP) && real_direction != DOWN)
        direction = UP;
    if (GetAsyncKeyState(VK_DOWN) && real_direction != UP)
        direction = DOWN;

    PAINTSTRUCT ps;
    // Get the device context
    hdc = BeginPaint(hwnd, &ps);

    BitBlt(hdcBackBuffer, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT,
           0, 0, 0, BLACKNESS);
    HBRUSH color;

    SetBkColor(hdcBackBuffer, RGB(bg_r, bg_g, bg_b));
    ExtTextOut(hdcBackBuffer, 0, 0, ETO_OPAQUE, &rect, 0, 0, 0);

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
        DrawText(hdc, GAME_OVER_TEXT, ARRAYSIZE(GAME_OVER_TEXT),
                 &rc, DT_CALCRECT);

        // Text "Game over" center location debug info
        char *buf;
        wsprintf(buf, "W: %d H: %d",
                 (SCREEN_WIDTH/2)-(rc.right/2) - ARRAYSIZE(GAME_OVER_TEXT),
                 (SCREEN_HEIGHT/2)-(rc.bottom/2));
        SetTextColor(hdcBackBuffer, RGB(0, 0, 255));
        TextOut(hdcBackBuffer, 50, 50,
                buf, strlen(buf));
#endif
        // Display "Game over" in the centre of the screen
        TextOut(hdcBackBuffer, 347, 292,
                GAME_OVER_TEXT, ARRAYSIZE(GAME_OVER_TEXT));
    }

    // Display the back buffer
    BitBlt(hdc, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, hdcBackBuffer,
           0, 0, SRCCOPY);

#ifdef _DEBUG
    // Save screenshot to clipboard
    if (GetAsyncKeyState(VK_F12))
        CaptureAnImage(hwnd);
#endif
    EndPaint(hwnd, &ps);
    goto loop;
}
