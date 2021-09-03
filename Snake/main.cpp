/*////////////////////////////////////////////////////
//
//本程序由悠然小赐编写
//转载请注明出处
*/////////////////////////////////////////////////////
#if (defined(_WIN32)||defined(_WIN64))&&!defined(__GNUC__)

#include <Windows.h>
#include <WinUser.h>
#include "resource.h"
#endif

#include <iostream>
#include <sstream>
#include <vector>
#include <cmath>
#include <deque>
#include <algorithm>
#include <random>
#include <ctime>
#include <string>

using namespace std;

#if (defined(_WIN32)||defined(_WIN64))&&!defined(__GNUC__)
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif
#include <SDL_image.h>
#include <SDL_ttf.h>

const int SCREEN_TICK_FPS_FRAMES = 1000 / 60;
const int Map_array_Width = 30, Map_array_Height = 30;
const int piexl_width = 20, piexl_height = 20;
const int screen_width = Map_array_Width * piexl_width + piexl_width * 2,
screen_height = screen_width+ Map_array_Width*2;// Map_array_Width * Map_array_Height;

SDL_Window *gWindow;
SDL_Renderer *gRenderer;
TTF_Font *gFont;
//SDL_Texture *gTexture;

#if (defined(_WIN32)||defined(_WIN64))&&!defined(__GNUC__)
HGLOBAL hFntMem; DWORD len = 0;
/* 加载 */
void LoadResourceFont() {
	HANDLE hMyFont = INVALID_HANDLE_VALUE; // Here, we will (hopefully) get our font handle
	HINSTANCE hInstance = ::GetModuleHandle(nullptr); // Or could even be a DLL's HINSTANCE
	HRSRC  hFntRes = FindResource(hInstance, MAKEINTRESOURCE(IDR_FONT), (LPCSTR)"BINARY");
	if (hFntRes) { // If we have found the resource ... 
		/*HGLOBAL */hFntMem = LoadResource(hInstance, hFntRes); // Load it
		if (hFntMem != nullptr) {
			void*  FntData = LockResource(hFntMem); // Lock it into accessible memory
			DWORD nFonts = 0;
			len = SizeofResource(hInstance, hFntRes);
			hMyFont = AddFontMemResourceEx(FntData, len, nullptr, &nFonts); // Fake install font!
		}
	}
}
/* 释放 */
#define RemoveFont(x) RemoveFontMemResourceEx(x)
#endif
void initSDL() {
#if (defined(_WIN32)||defined(_WIN64))&&!defined(__GNUC__)
	LoadResourceFont();
#endif
	SDL_Init(SDL_INIT_VIDEO);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
	gWindow = SDL_CreateWindow(u8"贪吃蛇 悠然小赐 QQ:108201645",
		/*400,*/SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		screen_width,
		screen_height,
		SDL_WINDOW_SHOWN);
	gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_SOFTWARE);
	IMG_Init(IMG_INIT_PNG);
	TTF_Init();
	
#if (defined(_WIN32)||defined(_WIN64))&&!defined(__GNUC__)
	const int FontSize = 16;
	SDL_RWops* pFontMem = SDL_RWFromConstMem(hFntMem, len);
	gFont = TTF_OpenFontRW(pFontMem, 1, FontSize);
#else
	gFont = TTF_OpenFont("NSimSun.ttf", 12);
#endif
	SDL_SetRenderDrawBlendMode(gRenderer, SDL_BLENDMODE_BLEND);
}

void close() {
	//SDL_DestroyTexture(gTexture);
	TTF_CloseFont(gFont);
#if (defined(_WIN32)||defined(_WIN64))&&!defined(__GNUC__)
	RemoveFont(hFntMem);
#endif
	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
}


int mWidth, mHeight;

void loadText(SDL_Texture *&gTexture,const char *text, SDL_Color textColor) {
	if (gTexture)
		SDL_DestroyTexture(gTexture);
	SDL_Surface *load = TTF_RenderUTF8_Blended(gFont, text, textColor);
	mWidth = load->w, mHeight = load->h;
	gTexture = SDL_CreateTextureFromSurface(gRenderer, load);
	SDL_FreeSurface(load);
}
/* 清屏 */
void ClearScreen() {
	SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);
	SDL_RenderClear(gRenderer);
}

//斜对角线翻转     
template<typename T, int n, int m>
void right90angle_flip(T(&arr)[n][m])
{
	for (int i = 0; i < n; i++) {
		for (int j = i; j < m; j++) {
			swap(arr[i][j], arr[j][i]);
		}
	}

	for (int i = 0; i < n; i++) {
		for (int j = 0; j < m / 2; j++) {
			swap(arr[i][j], arr[i][m - 1 - j]);
		}
	}
}
template<typename T, int n, int m>
void left90angle_flip(T(&arr)[n][m])
{
	for (int i = 0; i < n; i++) {  // 次对角线翻转
		for (int j = 0; j < n - i; j++) {
			swap(arr[i][j], arr[n - j - 1][n - i - 1]);
		}
	}
	for (int i = 0; i < n; i++) {  // 每行按照中点翻转
		for (int j = 0; j < n / 2; j++) {
			swap(arr[i][j], arr[i][n - j - 1]);
		}
	}
}
//左右翻转     
template<typename T, int n, int m>
void horizontalFlip(T(&arr)[n][m])
{
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < m / 2; j++) {
			swap(arr[i][j], arr[i][m - 1 - j]);
		}
	}
}
//上下翻转
template<typename T, int n, int m>
void verticalFlip(T(&arr)[n][m])
{
	for (int i = 0; i < n / 2; i++) {
		for (int j = 0; j < m; j++) {
			swap(arr[i][j], arr[m - i - 1][j]);
		}
	}
}

/* 填充三角 */
template<typename T, int n, int m>
void fillTriangle(T(&arr)[n][m])
{
	int i = 0;
	for (; i < n / 2; ++i)
	{
		for (int j = 0; j < m; ++j)
		{
			if (j - m / 2 >= -i && j - m / 2 <= i)
				arr[i][j] = 1;
			else
				arr[i][j] = 0;
		}
	}
	for (; i < piexl_height; ++i)
	{
		for (int j = 0; j < m; ++j)
			arr[i][j] = 1;
	}
	left90angle_flip(arr);
}

int map_Array[Map_array_Height][Map_array_Width]{ 0 };
//墙与食物部分 16位    1111 0000  0000 0000 0000
const int  wallOffsetBit = 16; //0001 墙

//本身方向标识部分 12位  1111 0000 0000 0000
const int snakeOffsetBit = 12;//0001头 0010身子 0100 尾

//身体转弯下一个方向 8位   0000  1111 0000 0000
const int curDirectionOffsetBit = 8;

//校验下一个方向部分高位 4位    0000 0000 1111 0000
const int directionOffsetBit = 4;//保存1 2 4 8

typedef struct Snake {
	SDL_Point mPosXY;//位置
	int direction;//方向
}Snake;
typedef struct SnakeSprite {
	char mPosXY[piexl_height][piexl_width];
}SnakeSprite;


/* 眼睛部分 */
SnakeSprite snakeEye;
/* 蛇身 */
SnakeSprite snakeBody;
/* 尾 */
SnakeSprite snakeTailRect;
/* 转弯 */
SnakeSprite snakeTurn[4];
/* 蛇 */
vector<Snake> SNAKE;
/* 红心 */
SnakeSprite snakeHeart;
//蛋
SnakeSprite snakeEgg;
/* 初始化地图 */
void initMap() {
	for (int i = 0; i < Map_array_Height; ++i)
	{
		for (int j = 0; j < Map_array_Width; ++j)
		{
			if (i == 0 || i == Map_array_Height - 1 || j == 0 || j == Map_array_Width - 1)
				map_Array[i][j] = 1 << wallOffsetBit;
			else
				map_Array[i][j] = 0;
		}
	}
	//	getchar();
}

string GetHeartStr()
{
	string s;
	for (double y = 1.3f; y > -1.0f; y -= 0.1259f)
	{
		for (double x = -1.f; x < 1.5f; x += 0.1358f)
		{
			double a = x * x + y * y - 1;

			// printf("\e[38;5;%dm%c\e[0m",ii,
			// putchar(
			a *a * a - x * x * y * y * y <= 0.0f ? s += '\1' : s += ' ';	// ' ');
		//	fflush(stdout);
		}
		s += '\xff';
	}
	return s;
}

string initSnakeEgg() {
	string s;
	memset(snakeEgg.mPosXY, 0, piexl_height *piexl_width * sizeof(char));
	int k = 0, headHeight = piexl_height / 2 - 2, headWidth = piexl_width / 2 - 1;
	for (int i = -headHeight; i <= headHeight; ++i)
	{
		for (int j = -headWidth; j <= headWidth; ++j)
		{
			double dx = (double)j / (double)headHeight;
			double dy = (double)i / (double)headWidth;
			if (dx* dx + dy * dy <= 0.902&&i < piexl_height&&j < piexl_height)
				s += '\1';
			else
				s += ' ';
		}
		s += (char)0xff;
	}
	return s;
}

template<typename T, int n, int m, typename Func>
void initSnakeComP(T(&arr)[n][m], Func(*func)()) {
	string s = func();
	int k = 0;
	for (int i = 0; i < n; ++i)
	{
		for (int j = 0; j < m; ++j)
		{
			if (s.size() == k)
				return;
			if (s[k] == '\1')
				arr[i][j] = s[k];
			else if (s[k] == 0xff)
			{
				k++, i++;
				break;
			}
			k++;
		}
	}
}

/* 初始化蛇 */
void initSnake(int x, int y, char direction) {
	SNAKE.clear();
	SNAKE.push_back({ { x,y},{direction} });
	map_Array[SNAKE[0].mPosXY.y][SNAKE[0].mPosXY.x] = direction;
	SNAKE.push_back({ { x - 1,y} ,{direction} });
	map_Array[SNAKE[1].mPosXY.y][SNAKE[1].mPosXY.x] = direction;

	snakeTailRect = { 0 };
	fillTriangle(snakeTailRect.mPosXY);
	//头部
	initSnakeComP(snakeEgg.mPosXY, initSnakeEgg);
	//left90angle_flip(snakeHead.mPosXY);

	//眼睛部分
	snakeEye = { 0 };
	for (int i = 0; i < piexl_height; ++i)
	{
		for (int j = 0; j < piexl_width; ++j)
		{
			if (j > 14 && j < 19 && (i > 3 && i < 8 || i > 14 && i < 19))
				snakeEye.mPosXY[i][j] = 1;
			/* 身体随便填充一下 */
			snakeBody.mPosXY[i][j] = 1;
		}
	}


	//转弯部分
	int mPointX = 0, mPointY = 0, r = piexl_width;

	for (double angle = 0; angle < 90; angle += 1)
	{

		double targerX = mPointX + r * sin(angle*(3.14 / 180));
		double targerY = mPointY + r * cos(angle*(3.14 / 180));
		for (int i = 0; i < piexl_height; ++i)
		{
			for (int j = 0; j < piexl_width; ++j)
			{
				if (j <= targerX && i <= targerY)
					snakeTurn[0].mPosXY[i][j] = 1;
			}
		}
	}

	for (int i = 1; i < 4; ++i)
	{
		memset(snakeTurn[i].mPosXY, 0, piexl_width*piexl_height * sizeof(char));
		memcpy(snakeTurn[i].mPosXY, snakeTurn[0].mPosXY, piexl_width*piexl_height * sizeof(char));
		if (i == 1)
			left90angle_flip(snakeTurn[i].mPosXY);
		else if (i == 2)
		{
			horizontalFlip(snakeTurn[i].mPosXY);
			verticalFlip(snakeTurn[i].mPosXY);
		}
		else if (i == 3)
			horizontalFlip(snakeTurn[i].mPosXY);
	}
	//初始化红心;
	initSnakeComP(snakeHeart.mPosXY, GetHeartStr);
	//getchar();
}
template<typename T, int n, int m>
void DrawRect(const T(&arr)[n][m], int x, int y) {
	//手绘部分
	for (int i = 0; i < n; ++i)
	{
		for (int j = 0; j < m; ++j)
		{
			if (arr[i][j])
				SDL_RenderDrawPoint(gRenderer, x + j, y + i);
		}
	}
}

void FillRect(SDL_Color drawColor, SDL_Rect &renderQuad) {
	SDL_SetRenderDrawColor(gRenderer, drawColor.r, drawColor.g, drawColor.b, drawColor.a);
	SDL_RenderFillRect(gRenderer, &renderQuad);
}
void DrawRect(SDL_Color drawColor, SDL_Rect &renderQuad) {
	SDL_SetRenderDrawColor(gRenderer, drawColor.r, drawColor.g, drawColor.b, drawColor.a);
	SDL_RenderDrawRect(gRenderer, &renderQuad);
}
/* 上1，下2，左4，右8 */

// deque<pair<int, int>> direction_list1 = { {1,4}, {2,8},{4,2}, {8,1} };
// deque<pair<int, int>> direction_list2 = { {1,8}, {2,4},{4,1}, {8,2} };
// 
// int selectDirection(int a, int  b) {
// 	if (std::find_if(direction_list1.begin(), direction_list1.end(), 
// 		[&](auto i) {return i.first == a && i.second == b; }) != direction_list1.end())
// 		return 1;//左旋
// 	else if (std::find_if(direction_list2.begin(), direction_list2.end(),
// 		[&](auto i) {return i.first == a && i.second == b; }) != direction_list2.end())
// 		return 2;//右旋
// 	else
// 		return 0;//不旋转
// }
/* 上1，下2，左4，右8*/
int selectDirection(char a, char  b) {
	if (a == 8 && b == 1 || a == 1 && b == 4 || a == 4 && b == 2 || a == 2 && b == 8)
		return 1;//左旋
	else if (a == 8 && b == 2 || a == 2 && b == 4 || a == 4 && b == 1 || a == 1 && b == 8)
		return 2;//右旋
	return 0;
}
/* 转弯处调用 */
int selectTurnDirection(char a, char  b) {

	if (a == 1 && b == 4 || a == 8 && b == 2)
		return 1;
	else if (a == 1 && b == 8 || a == 4 && b == 2)
		return 2;
	else if (a == 2 && b == 8 || a == 4 && b == 1)
		return 3;
	else if (a == 2 && b == 4 || a == 8 && b == 1)
		return 0;//默认

	else
		return -1;
}
int checkCollision(stringstream &text,int x, int y, vector<SDL_Point>&random_food, char& food_attribute) {
	char ch = (map_Array[y][x] >> wallOffsetBit) & 0xff;
	if (map_Array[y][x] & 0xF)
	{
		text << "撞到身子了";
		return -1;
	}
	if (ch == 1 || (map_Array[y][x] & 0xF))
	{
		text << "撞到墙了";
		return -1;
	}
	else if (ch == 2 || ch == 4) {
		if (ch == 2)
			cout << "吃到鸡蛋" << endl;
		else if (ch == 4)
			cout << "吃到红心" << endl;

		food_attribute += ch >> 1;
		map_Array[y][x] &= ~(ch << wallOffsetBit);

		auto it = std::find_if(random_food.begin(), random_food.end(), [&](auto &ref) {return ref.y == y && ref.x == x; });
		if (it != random_food.end())
			it = random_food.erase(it);
		return 1;
	}
	return 0;
}
bool snakeMove(int &x, int&y, char curDirection) {
	if (curDirection == 1)
	{
		y--;
		return true;
	}
	else if (curDirection == 2)
	{
		y++;
		return true;
	}
	else if (curDirection == 4)
	{
		x--;
		return true;
	}
	else if (curDirection == 8)
	{
		x++;
		return true;
	}
	return false;//没有移动
}

//#define bit_size(x) sizeof(x)==4?0xFFFFFFFF0:0xFFFFFFFFFFFFFFF0
template<typename T, int n, int m>
void FlipSnakePiexlComp(T(&arr)[n][m], int a, int b) {
	int selectDirection_tmp = selectDirection(a, b);

	if (selectDirection_tmp == 1)
		left90angle_flip(arr);
	else if (selectDirection_tmp == 2)
		right90angle_flip(arr);
}
void SnakeHandle(int x, int y, char curDirection, char &preDriection, SDL_Point &GetswapLastMPos, int *&GetTailLastMap) {


	/* 绘制蛇 */

	int selectDirection_tmp = 0;
	char ch = '\0';

	for (size_t i = 0; i < SNAKE.size(); ++i)
	{

		if (i == 0) {
			/* 保存上一个坐标 */
			GetswapLastMPos = SNAKE[i].mPosXY;

			SNAKE[i].mPosXY.x = x, SNAKE[i].mPosXY.y = y;
			map_Array[y][x] = map_Array[y][x] | curDirection | (1 << snakeOffsetBit);

			/* 头与眼部处理 */

			if (SNAKE[i].direction != curDirection) {

				FlipSnakePiexlComp(snakeEye.mPosXY, SNAKE[i].direction, curDirection);

				/* 通过移位,保存障碍，身体部分标识、方向高低位比较值 */
				map_Array[GetswapLastMPos.y][GetswapLastMPos.x] = map_Array[GetswapLastMPos.y][GetswapLastMPos.x] |
					(1 << snakeOffsetBit) |
					(curDirection << directionOffsetBit) |
					SNAKE[i].direction;

				SNAKE[i].direction = curDirection;
			}

		}
		/* 尾部处理 */
		else if (i == SNAKE.size() - 1) {

			/* 获取高位比较 */
			ch = (map_Array[GetswapLastMPos.y][GetswapLastMPos.x] >> directionOffsetBit) & 0xF;

			map_Array[GetswapLastMPos.y][GetswapLastMPos.x] &= ~(0xF << snakeOffsetBit);
			//尾部
			map_Array[GetswapLastMPos.y][GetswapLastMPos.x] |= (3 << snakeOffsetBit);
			/* 保存当前 位置 */
			GetTailLastMap = &map_Array[GetswapLastMPos.y][GetswapLastMPos.x];
			/* 如果当前已经到达该转弯存盘点 */
			preDriection = SNAKE[i].direction;

			if (ch)
			{
				FlipSnakePiexlComp(snakeTailRect.mPosXY, SNAKE[i].direction, ch);

				SNAKE[i].direction = ch;
			}

			swap(GetswapLastMPos, SNAKE[i].mPosXY);
			//及时还原数组值
			map_Array[GetswapLastMPos.y][GetswapLastMPos.x] = map_Array[GetswapLastMPos.y][GetswapLastMPos.x] & (1 << wallOffsetBit);
		}
		else {

			/* 获取高位比较 */
			ch = (map_Array[GetswapLastMPos.y][GetswapLastMPos.x] >> directionOffsetBit) & 0xF;
			map_Array[GetswapLastMPos.y][GetswapLastMPos.x] &= ~(0xF << snakeOffsetBit);

			//身子部分
			map_Array[GetswapLastMPos.y][GetswapLastMPos.x] |= (2 << snakeOffsetBit);

			/* 如果当前已经到达该转弯存盘点 */

			if (ch)
			{
				/* 比较方向 */
				selectDirection_tmp = selectTurnDirection(SNAKE[i].direction, ch);
				if (selectDirection_tmp != -1)
				{
					map_Array[GetswapLastMPos.y][GetswapLastMPos.x] &= ~(0xF << curDirectionOffsetBit);
					/* 重点部分(selectDirection_tmp +1) << curDirectionOffsetBit)调用贴图 */
					map_Array[GetswapLastMPos.y][GetswapLastMPos.x] |= ((selectDirection_tmp + 1) << curDirectionOffsetBit);
					/* 更新它 */
					SNAKE[i].direction = ch;
				}


			}

			swap(GetswapLastMPos, SNAKE[i].mPosXY);
		}
	}
}


bool handleEvent(SDL_Event &event,int x,int y, char &direction_key) {
	if (event.type == SDL_KEYDOWN && !event.key.repeat) {

		switch (event.key.keysym.sym)
		{
		case SDLK_w://上
			if ((map_Array[y][x]&0xf) != 2)
			{
				direction_key = 1;
			}
			break;
		case SDLK_s://下
			if ((map_Array[y][x] & 0xf) != 1)
			{
				direction_key = 2;
			}
			break;
		case SDLK_a://左
			if ((map_Array[y][x] & 0xf) != 8)
				direction_key = 4;
			break;
		case SDLK_d://右
			if ((map_Array[y][x] & 0xf) != 4)
				direction_key = 8;
			break;
		default:
			break;
		}
		return 1;
	}
	return 0;
}

void CreateRandomFood(vector<SDL_Point> &random_food, std::default_random_engine& e,
	uniform_int_distribution<int> &u) {
	random_food.resize(random_food.size() + 1);

	/* 产生一个 1~ piexl_width-1 的数 */
	int x = random_food.back().x = u(e), y = random_food.back().y = u(e);
	bool ai_select_direction_x = x > Map_array_Width / 2, ai_select_direction_y = y > Map_array_Width / 2;

	while (1)
	{/* 当前位置不是身体或墙、食物 */
		if (!(map_Array[y][x] & 0xF) && (!(map_Array[y][x] & (0xF << wallOffsetBit)))) {
			/* 当没有东西的话,得到一个随机食物 2：egg, 4:love heart */
			map_Array[y][x] |= (((u(e) & 1) ? 2 : 4) << wallOffsetBit);
			break;
		}
		else
		{
			if (ai_select_direction_x)
				x--;//向左移动
			else
				x++;//右移动
			if (ai_select_direction_y)
				y--;//上移
			else
				y++;
		}
	}
}

/* 蛇运行类(随便封装一下) */
class SnakeRunHandle {
	std::default_random_engine e;
	uniform_int_distribution<int> u;
	vector<SDL_Point> random_food;

	int mPosX, mPosY;
	char curDirection, preDriection, GetsnakeDirection;
	SDL_Point GetswapLastMPos;
	char food_attribute;
	/* 获取最后一节更新的位置 */
	int *GetTailLastMap, curSnakeLength;
	/* 计数器 */
	int  mCounted = 0;
	double mStartTicks = 0, mStartAvgFps = 0, mSpeed = 0;
	int snakeState = 0;
	bool moveState = false;
	stringstream text;
	/*计分系统 */
	int score = 0, highscore = 0;
public:
	SnakeRunHandle() :e((unsigned)time(0)), u(1, Map_array_Width - 1) {
		init();
	}
	void init() {
		random_food.clear();
		mPosX = 10, mPosY = 10;
		curDirection = 8, preDriection = 8, GetsnakeDirection = 8;
		GetswapLastMPos = { 0 };
		food_attribute = 0;
		/* 获取最后一节更新的位置 */
		GetTailLastMap = NULL, curSnakeLength = 0;
		initMap();
		initSnake(mPosX, mPosY, curDirection);
		char key_press = 0;
		snakeState = 1;
		/* 计数器 */
		mStartAvgFps = 0, mCounted = 0;
		mStartTicks = 0;
		text.str("");
		moveState = false;
		/* 分值 */
		score = 0;
	}
	string GetErrorStr() {
		return text.str();
	}
	char &GetCurDirection() {
		return curDirection;
	}
	int GetScore() {
		return score;
	}
	int GetHighScore() {
		return highscore;
	}
	SnakeRunHandle& checkfoodLengthIncOrStop() {
		if (random_food.size() < 30 )
			CreateRandomFood(random_food, e, u);
		return *this;
	}
	SnakeRunHandle& checkSnakeLengthIncOrStop() {
		/* 添加长度 */
			if (GetTailLastMap && SNAKE.size() <= 30 && food_attribute != 0)
			{
				food_attribute =0;
				map_Array[GetswapLastMPos.y][GetswapLastMPos.x] |= *GetTailLastMap;
				map_Array[GetswapLastMPos.y][GetswapLastMPos.x] &= ~(0xF << snakeOffsetBit);
				map_Array[GetswapLastMPos.y][GetswapLastMPos.x] |= (2 << snakeOffsetBit);
				swap(*GetTailLastMap, map_Array[GetswapLastMPos.y][GetswapLastMPos.x]);
				/* 如果吃到东西，先翻转尾部 */
				FlipSnakePiexlComp(snakeTailRect.mPosXY, (map_Array[GetswapLastMPos.y][GetswapLastMPos.x] >> directionOffsetBit) & 0xF, map_Array[GetswapLastMPos.y][GetswapLastMPos.x] & 0xF);

				SNAKE.push_back({ { GetswapLastMPos}, {preDriection } });
				
			}
		return *this;
	}
	SnakeRunHandle& Move() {

		mCounted++;
		/* 每秒帧数 */
		mStartAvgFps = mCounted / ((SDL_GetTicks() - mStartTicks) / 1000.);
		cout << SNAKE.size() << ":" << SNAKE.size() / 1.2 << endl;
		double cnt = 30 - (signed)SNAKE.size() / 1.2;

		mSpeed++;//系统的不稳定啊.换一个吧
		if (mSpeed > cnt)
		{
			mSpeed = 0;
			moveState = snakeMove(mPosX, mPosY, curDirection);

			//if (moveState)
			{
				snakeState = checkCollision(text, mPosX, mPosY, random_food, food_attribute);
				if (snakeState == -1 || snakeState == 3)
				{
					snakeState = 2;
					if (score > highscore)
						highscore = score;
				}
				else
				{
					snakeState = 1;
					score += food_attribute;
				}


				checkSnakeLengthIncOrStop();
				SnakeHandle(mPosX, mPosY, curDirection, preDriection, GetswapLastMPos, GetTailLastMap);
			}
		}
		return *this;
	}

	SnakeRunHandle& SetsnakeState(int n) {
		snakeState = n;
		if (snakeState == 2) {
			mStartAvgFps = mSpeed = mCounted = 0;
		}
		else if (snakeState && !mStartTicks) {
			mStartTicks = SDL_GetTicks();
		}
		return *this;
	}
	int& GetsnakeState() {
		return snakeState;
	}
	double GetFps() {
		return mStartAvgFps;
	}

	int SDL_GetMposX() {
		return mPosX;
	}
	int SDL_GetMposY() {
		return mPosY;
	}
};

class Menu {
	SDL_Texture* gTextrue;
	vector<SDL_Rect> gTextPosition={
	{(screen_width-200) / 2,(screen_width - 200) / 2,200,200},{0,screen_height - Map_array_Width*2} ,
	{(screen_width - 200) / 2,(screen_width-200) / 2,200,200} };
	int n = 0;
public:
	Menu(string str,int n):n(n) {
		CallLoadText(str,n);

	}
	void Free() {
		
			SDL_DestroyTexture(gTextrue);
		
	}
	void CallLoadText(string s, int n) {
		if (this->n != n) {
			Free();
			this->n = n;
		}
		auto it = gTextPosition.begin()+n;

		loadText(gTextrue, s.c_str(), { 255,255,255 });
		if (n == 1)
			it->w = mWidth*2, it->h = mHeight*2;
	}
	SDL_Texture* GetTexture() {
		return gTextrue;
	}
	SDL_Rect GetTextPosition() {
		return gTextPosition[n];
	}
};

/* 绘制地图 */
void Draw(Menu &menu,int &RunState) {

	if (!RunState) {

		SDL_Rect renderQuad = menu.GetTextPosition();
		SDL_RenderCopy(gRenderer, menu.GetTexture(), 0, &renderQuad);
		SDL_RenderPresent(gRenderer);
		SDL_Delay(1000);
		RunState = 1;
		return;
	}
	
	/* 绘制背景与 地图 */
	SDL_SetRenderDrawColor(gRenderer, 0, 232, 207, 120);
	SDL_Rect renderQuad = { piexl_width,piexl_height,screen_width - piexl_width * 2,screen_width - piexl_height * 2 };
	SDL_RenderFillRect(gRenderer, &renderQuad);


	for (int col = 0; col < Map_array_Height; ++col) {
		for (int row = 0; row < Map_array_Width; ++row)
		{
			renderQuad = { row * piexl_width + piexl_width,col * piexl_width + piexl_width,piexl_width,piexl_height };

			if (((map_Array[col][row] >> wallOffsetBit) & 0xF) == 1)
				//绘制墙
				FillRect({ 50, 160, 200, 255 }, renderQuad);

			else
			{
				int row_x = row * piexl_width + piexl_width, col_y = col * piexl_width + piexl_width;
				//绘制地图
				DrawRect({ 0, 128, 128, 128 }, renderQuad);
				char curDirectionState = (map_Array[col][row] >> snakeOffsetBit) & 0xF;
				//判断是不是头
				if (curDirectionState == 1)
				{

					SDL_SetRenderDrawColor(gRenderer, 0, 255, 100, 120);
					DrawRect(snakeBody.mPosXY, row_x, col_y);
					/* 眼部处理 */
					SDL_SetRenderDrawColor(gRenderer, 0, 120, 120, 240);
					DrawRect(snakeEye.mPosXY, row_x, col_y);

				}
				/* 身子 */
				else if (curDirectionState == 2) {
					SDL_SetRenderDrawColor(gRenderer, 20, 200, 128, 180);

					int curTmp = (map_Array[col][row] >> curDirectionOffsetBit) & 0xF;
					if (curTmp) {
						DrawRect(snakeTurn[curTmp - 1].mPosXY, row_x, col_y);
					}
					else
						DrawRect(snakeBody.mPosXY, row_x, col_y);

				}
				/* 尾部 */
				else if (curDirectionState == 3)
				{
					SDL_SetRenderDrawColor(gRenderer, 0, 255, 100, 120);
					DrawRect(snakeTailRect.mPosXY, row_x, col_y);
				}
				curDirectionState = (map_Array[col][row] >> wallOffsetBit) & 0xF;
				if (curDirectionState == 2)
				{
					SDL_SetRenderDrawColor(gRenderer, 255, 100, 100, 255);
					DrawRect(snakeEgg.mPosXY, row_x + 2, col_y + 2);
				}
				else if (curDirectionState == 4)
				{
					SDL_SetRenderDrawColor(gRenderer, 255, 120, 128, 240);
					DrawRect(snakeHeart.mPosXY, row_x + 2, col_y + 2);
				}
			}
		}

	}
	if (RunState == 1) {

		SDL_Rect renderQuad = menu.GetTextPosition();
		SDL_RenderCopy(gRenderer, menu.GetTexture(), 0, &renderQuad);
		return;
	}
	else if (RunState == 2) {

		SDL_Rect renderQuad = menu.GetTextPosition();
		SDL_RenderCopy(gRenderer, menu.GetTexture(), 0, &renderQuad);
		SDL_RenderPresent(gRenderer);
		SDL_Delay(1500);
		RunState = 1;
		return;
	}
}

int main(int argc, char *argv[]) {
	initSDL();

	int runState = 0;
	SnakeRunHandle snakeRun;
	Menu menu(u8"贪吃蛇", runState);
	snakeRun.SetsnakeState(runState);

	bool quit = false;
	SDL_Event event;
	char key_press = 0;
	while (!quit)
	{
		int capTimer = SDL_GetTicks();

		while (SDL_PollEvent(&event)&& key_press == 0)
		{
			if (event.type == SDL_QUIT)
				quit = true;
			if (key_press == 0 && snakeRun.GetsnakeState() == 1)
			{
				key_press = 1;
				handleEvent(event, snakeRun.SDL_GetMposX(), snakeRun.SDL_GetMposY(), snakeRun.GetCurDirection());
				break;
			}

		}

		if (snakeRun.GetsnakeState() == 1)
		{
			snakeRun.Move().checkfoodLengthIncOrStop();
			stringstream text;
			text << u8"分数:" << snakeRun.GetScore() * 100 << u8"   最高分:" << snakeRun.GetHighScore() * 100 << u8"  平均帧数:" << snakeRun.GetFps();
			menu.CallLoadText(text.str(), snakeRun.GetsnakeState());

		}
		if (snakeRun.GetsnakeState() == 2) {
			runState = snakeRun.GetsnakeState();
			menu.CallLoadText("GAME OVER", runState);
			snakeRun.init();
			
		}

		ClearScreen();

		Draw(menu,runState);
		snakeRun.SetsnakeState(runState);
		
		SDL_RenderPresent(gRenderer);

		/* 帧速 */
		float timerFrmes = (float)SDL_GetTicks() - capTimer;
		if (timerFrmes < SCREEN_TICK_FPS_FRAMES)
			SDL_Delay(SCREEN_TICK_FPS_FRAMES - (int)timerFrmes);
		//cout << timerFrmes << ":"<< SCREEN_TICK_FPS_FRAMES<<"-"<<(timerFrmes < SCREEN_TICK_FPS_FRAMES)<<endl;
		key_press = 0;
	}

	close();
	return 0;
}