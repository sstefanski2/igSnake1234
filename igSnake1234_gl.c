/*
Notes:
    * On first frame color of snake's head is not defined, so it's black.
    * On the beginning bg color is 0.0f, 0.0f, 0.0f, after first apple
      'r', 'g' or 'b' is changed to 0.1f.
    * Apple should probably never show up near right or bottom border.
*/

#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <windows.h>
#include <mmsystem.h>
#include <GL/gl.h>

#define SNAKE_WIDTH 20
#define SNAKE_HEIGHT 20
#define SNAKE_MAX_LENGTH 125
#define DELAY_MS 80
#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080

// 40 bytes
PIXELFORMATDESCRIPTOR pfd =
{
    0,
    1, PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER, 32, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 32, 0, 0, 0, 0, 0, 0, 0
};
// 156 bytes
DEVMODE g_ScreenSettings =
{
    { 0 }, 0, 0, sizeof(DEVMODE), 0,
    DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL, { 0 }, 0, 0, 0, 0, 0,
    { 0 }, 0, 32, SCREEN_WIDTH, SCREEN_HEIGHT, { 0 }, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
// 8 bytes each
struct S_SNAKE
{
    GLfloat x, y, r, g, b;
} snake_blocks[SNAKE_MAX_LENGTH];
// 4 bytes each
HWND hwnd;
HDC hDC;
enum E_DIR { LEFT, RIGHT, UP, DOWN };
enum E_DIR direction = RIGHT;
enum E_DIR real_direction = RIGHT;
enum E_FADE_STATE { NONE, EIN, EOUT };
enum E_FADE_STATE fade_state = NONE;
GLfloat apple_x;
GLfloat apple_y;
GLfloat bg_r = 0.1f;
GLfloat bg_g = 0.1f;
GLfloat bg_b = 0.1f;
GLfloat *bg_attr = &bg_r;
unsigned int snake_length = 3;
unsigned int now, last;
BOOL game_over = FALSE;

// https://stackoverflow.com/questions/7602919/how-do-i-generate-random-numbers-without-rand-function
unsigned short lfsr = 0xACE1u;
unsigned rand()
{
    unsigned bit = ((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5) ) & 1;
    return lfsr = (lfsr >> 1) | (bit << 15);
}

void generate_apple()
{
generate:
    apple_x = (rand() % 95 + 1) * SNAKE_WIDTH;
    apple_y = (rand() % 53 + 1) * SNAKE_HEIGHT;
    for (unsigned int i = 0; i < snake_length; i++)
        if ((snake_blocks[i].x == apple_x) &&
                (snake_blocks[i].y == apple_y))
            goto generate;
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
        if (snake_blocks[0].x < 0.0f)
            snake_blocks[0].x = SCREEN_WIDTH-SNAKE_WIDTH;
        break;
    case RIGHT:
        snake_blocks[0].x += SNAKE_WIDTH;
        if (snake_blocks[0].x > SCREEN_WIDTH-SNAKE_WIDTH)
            snake_blocks[0].x = 0.0f;
        break;
    case DOWN:
        snake_blocks[0].y -= SNAKE_HEIGHT;
        if (snake_blocks[0].y < 0.0f)
            snake_blocks[0].y = SCREEN_HEIGHT-SNAKE_HEIGHT;
        break;
    case UP:
        snake_blocks[0].y += SNAKE_HEIGHT;
        if (snake_blocks[0].y > SCREEN_HEIGHT-SNAKE_HEIGHT)
            snake_blocks[0].y = 0.0f;
        break;
    }
    snake_blocks[0].r = ((rand() % 1000) / 1000.0f);
    snake_blocks[0].g = ((rand() % 1000) / 1000.0f);
    snake_blocks[0].b = ((rand() % 1000) / 1000.0f);
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
        *bg_attr+=0.1f;
        if (*bg_attr >= 0.5f)
            fade_state = EOUT;
    }
    if (fade_state == EOUT)
    {
        *bg_attr-=0.1f;
        if (*bg_attr <= 0.1f)
            fade_state = NONE;
    }

    // Check if snake has bitten himself
    for (unsigned int i = snake_length-1; i > 1; i--)
        if ((snake_blocks[0].x == snake_blocks[i].x) &&
                (snake_blocks[0].y == snake_blocks[i].y))
            game_over = TRUE;
}

void WINAPI WinMainCRTStartup()
{
    ChangeDisplaySettings(&g_ScreenSettings, CDS_FULLSCREEN);
    hwnd = CreateWindow("static", 0, WS_POPUP | WS_VISIBLE | WS_MAXIMIZE,
                        0, 0, 0, 0, 0, 0, 0, 0);
    hDC = GetDC(hwnd);
    SetPixelFormat(hDC, ChoosePixelFormat (hDC, &pfd), &pfd);
    wglMakeCurrent(hDC, wglCreateContext(hDC));
    ShowCursor(FALSE);

    // Init values
    snake_blocks[0].x = SCREEN_WIDTH / 2;
    snake_blocks[0].y = SCREEN_HEIGHT / 2;
    generate_apple();

    // glMatrixMode(GL_PROJECTION);
    // glLoadIdentity();
    glOrtho(0.0f, SCREEN_WIDTH, 0.0f, SCREEN_HEIGHT, 1.0f, -1.0f);
    // glMatrixMode(GL_MODELVIEW);
    // glLoadIdentity();
    // glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

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
        if (GetAsyncKeyState(VK_LEFT) && real_direction != RIGHT)
            direction = LEFT;
        if (GetAsyncKeyState(VK_RIGHT) && real_direction != LEFT)
            direction = RIGHT;
        if (GetAsyncKeyState(VK_UP) && real_direction != DOWN)
            direction = UP;
        if (GetAsyncKeyState(VK_DOWN) && real_direction != UP)
            direction = DOWN;

        // Game logic
        if (!game_over)
            game_loop();

        // Update timer
        last = GetTickCount();
    }

    // Process keys
    if (GetAsyncKeyState(VK_ESCAPE))
        ExitProcess(0);

    glClearColor(bg_r, bg_g, bg_b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw a snake
    for (unsigned int i = 0; i < snake_length; i++)
    {
        if (game_over)
            snake_blocks[i].r = snake_blocks[i].g = snake_blocks[i].b = 0;

        glColor3f(snake_blocks[i].r, snake_blocks[i].g, snake_blocks[i].b);
        glRectf(snake_blocks[i].x, snake_blocks[i].y,
                snake_blocks[i].x+SNAKE_WIDTH, snake_blocks[i].y+SNAKE_HEIGHT);
    }

    // Draw an apple
    glColor3f(0.0f, 0.0f, 1.0f);
    glRectf(apple_x, apple_y, apple_x+SNAKE_WIDTH, apple_y+SNAKE_HEIGHT);

    SwapBuffers(hDC);
    goto loop;
}
