#define _CRT_SECURE_NO_WARNINGS
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <nlohmann/json.hpp>
#include <vector>
#include <iostream>
#include <fstream>
#include <deque>
#include <stack>
#include <algorithm>
#include <steam/steam_api.h> 
#define MAXLEVEL 40
#define SFML_STATIC
using json = nlohmann::json;

enum class GameState { TitleScreen, LevelSelect, inGame ,credit};
enum class textures { mainButton, backButton, clear, notClear, tile, wall, apple, spike, ice,iceApple,
    antiApple, redButton, redWall, redWallDown, blueButton, blueWall, blueWallDown,box,turn,passWall,
    cloneItem,cloneHead,cloneBody,gameOver, snakeShort1, snakeShort2, snakeShort3, snakeShort4,snakeHead1,snakeHead2, 
    snakeHead3, snakeHead4, snakeBody1, snakeBody2, snakeBody3, snakeBody4, snakeBody5, snakeBody6, snakeBody7,snakeBody8, 
    snakeTail1, snakeTail2, snakeTail3, snakeTail4,arrowDown,arrowRight,arrowUp,arrowLeft,keyZ,keyR,space,
    title,musicOn,musicOff,soundOn,soundOff,level,credit,deleteButton,deletePopup,
    yes,no,deleteCompletedPopup
};

enum class sounds { buttonCliked,item,button,move,gameOver,clear,eat,undo

};


enum class Objects {
    blank,wall,apple,spike,ice,iceApple,antiApple,redButton,redWall,RedWallDown,
    blueButton, blueWall, blueWallDown,box,turnItem,passWall,cloneItem,cloneHead,cloneBody
};

std::vector<int>clear = std::vector<int>(MAXLEVEL, 0);
bool allClear = false;
sf::Sprite inGameSprites[100];
sf::Texture textures[100];
sf::Sound sounds[20];
sf::SoundBuffer soundBuffers[20];
sf::Music bgm;
int resX = 1600;
int resY = 900;
std::string logs="log";

void saveClearData();
void unlockAchievement(const char* achievementID);

struct MapStatus
{
    int remainedFruits;
    int redPressed;
    int turnAble;
    int lastPressed;
    int gameMap[25][25] ;
    int isBox[25][25] ;
    std::deque<std::vector<int>> snake;
    std::deque<std::vector<int>> snakeClone ;
};

std::stack<struct MapStatus> mapStatusHistory = std::stack<struct MapStatus>();
struct Level
{
    int id;
    std::vector<std::vector<int>> ini;
    std::vector<std::vector<int>> snake;
    std::vector<std::vector<int>> box;
    std::vector<std::vector<int>> clone;
    int height;
    int width;
    int fruits;

};

std::vector<Level> levels;

class CustomSprite
{
public:
    CustomSprite(float x, float y, float width, float height, int textureNum)
    {
        x *= resX;
        y *= resY;
        width *= resX;
        height *= resY;
        this->textureNum = textureNum;
        sprite.setTexture(textures[textureNum]);
        sprite.setPosition(x, y);
        sprite.setScale(width / sprite.getLocalBounds().width, height / sprite.getLocalBounds().height);
    }

    ~CustomSprite()
    {
    }

    void draw(sf::RenderWindow& window)
    {
        sprite.setTexture(textures[textureNum]);

        window.draw(sprite);
    }

    void setTextureNum(int n)
    {
        textureNum = n;
    }

    int getTexutreNum()
    {
        return textureNum;
    }

private:
    sf::Sprite sprite;
    int textureNum;
};

class CustomText
{
public:
    CustomText(const std::string& text, float x, float y, float size)
    {
        if (!font.loadFromFile("arial.ttf"))
        {
            std::cerr << "Error loading font from file: " << "arial.ttf\n";
        }
        else
        {
            this->text.setFont(font);
            this->text.setString(text);
            this->text.setCharacterSize(size);
            this->text.setPosition(x * resX - this->text.getLocalBounds().width / 2, y * resY);
            this->text.setFillColor(sf::Color::White);
        }
    }

    ~CustomText()
    {

    }

    void setText(const std::string& newText)
    {
        text.setString(newText);
    }

    void setPosition(float x, float y)
    {
        text.setPosition(x * resX - text.getLocalBounds().width / 2, y * resY);
    }

    void setCharacterSize(float size)
    {
        text.setCharacterSize(size);
    }

    void setFillColor(const sf::Color& color)
    {
        text.setFillColor(color);
    }

    void draw(sf::RenderWindow& window)
    {
        this->text.setFont(font);
        window.draw(text);
    }

private:
    sf::Font font;
    sf::Text text;
};

class CustomButton
{
public:
    CustomButton(float x, float y, float width, float height, int textureNum, const std::string& text_str, const sf::IntRect& rect = sf::IntRect()) :
        text(text_str, x + width / 2, y, height* resY * 0.7)
    {
        x *= resX;
        y *= resY;
        width *= resX;
        height *= resY;
        this->textureNum = textureNum;
        sprite.setTexture(textures[textureNum]);
        if (rect != sf::IntRect())
        {
            sprite.setTextureRect(rect);
        }
        sprite.setPosition(x, y);
        sprite.setScale(width / sprite.getLocalBounds().width, height / sprite.getLocalBounds().height);

    }

    ~CustomButton()
    {
    }

    void draw(sf::RenderWindow& window)
    {
        sprite.setTexture(textures[textureNum]);

        window.draw(sprite);
        text.draw(window);
    }

    bool isClicked(float mouseX, float mouseY, const sf::RenderWindow& window)
    {
        sf::Vector2u windowSize = window.getSize();
        return sprite.getGlobalBounds().contains(mouseX / windowSize.x * resX, mouseY / windowSize.y * resY);
    }

    void setTextureNum(int n)
    {
        textureNum = n;
    }

    int getTexutreNum()
    {
        return textureNum;
    }


private:
    CustomText text;
    sf::Sprite sprite;
    int textureNum;

};

class Game
{
public:
    Game()
        :window(sf::VideoMode(resX, resY), "Gulping snake"), gameState(GameState::TitleScreen), titleText("Level", 0.5, 0.07, resY / 12)
    {
        titleText.setFillColor(sf::Color::Black);
        sf::VideoMode desktopMode = sf::VideoMode::getDesktopMode();
        window.setSize(sf::Vector2u( desktopMode.width, desktopMode.height*0.9));
        window.setPosition(sf::Vector2i(0, 0));
        window.setFramerateLimit(120);
        sf::Image icon;
        icon.loadFromFile("images/snakeHead1.png");
        window.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());
        makeTitleButtons();
    }
    ~Game()
    {
    }

    void run()
    {
        while (window.isOpen())
        {
            processEvents();
            render();
            SteamAPI_RunCallbacks();
        }
    }

private:
    sf::RenderWindow window;
    GameState gameState;
    std::vector<CustomButton> buttons;
    std::vector<CustomButton> stageButtons;
    std::vector<CustomButton> backButton;
    CustomText titleText;

    int level = 0;
    int remainedFruits;
    int redPressed = 0;
    int turnAble = 0;
    int die = 0;
    int lastPressed = 0;
    int gameMap[25][25] = {};
    int isBox[25][25] = {};
    int currentMapWidth = 0;
    int currentMapHeight = 0;
    std::vector<std::vector<int>> buttonState = std::vector<std::vector<int>>(0, std::vector<int>(0));
    std::deque<std::vector<int>> snake = std::deque<std::vector<int>>(0);
    std::deque<std::vector<int>> snakeClone = std::deque<std::vector<int>>(0);
    int lastRemoved[2] = { 0,0 };
    int preCheckOriSnakeDestination = 0;
    int levelCursor = 0;
    bool music = true;
    bool sound = true;
    int dataDeletePopUp = 0;
    int firstTurn=0;

    void titleToLevel()
    {
        if (level == 40)
            unlockAchievement("AllClear");
        buttons.clear();
        stageButtons.clear();
        backButton.clear();
        backButton.push_back(CustomButton(0.02, 0.02, 0.08, 0.1, static_cast<int>(textures::backButton), ""));
        buttonState.clear();
        loadMap();
    }

    void levelToTitle()
    {
        buttonState.clear();
        backButton.clear();
        stageButtons.clear();
        makeTitleButtons();
    }

    void loadMap()
    {
        lastPressed = 0;
        die = 0;
        firstTurn = 0;
        while (!mapStatusHistory.empty()) 
        {
            mapStatusHistory.pop();
        }
        currentMapHeight = levels[level].height;
        currentMapWidth = levels[level].width;
        //맵
        for (int i = 0; i < currentMapHeight; ++i)
        {
            for (int j = 0; j < currentMapWidth; ++j)
            {
                gameMap[i][j] = levels[level].ini[i][j];
                isBox[i][j] = 0;
            }
        }

        //상자
        for (int i = 0; i < levels[level].box.size(); ++i)
        {
            isBox[levels[level].box[i][0]][levels[level].box[i][1]] = 1;
        }
        
        //뱀
        snake.clear();
        for (int i = 0; i < levels[level].snake.size(); ++i)
        {
            std::vector<int> newVec = std::vector<int>(0);
            newVec.push_back(levels[level].snake[i][0]);
            newVec.push_back(levels[level].snake[i][1]);
            snake.push_back(newVec);
        }
        snakeClone.clear();

        remainedFruits = levels[level].fruits;
        redPressed = 0;
        turnAble = 0;
    }

    void makeTitleButtons()
    {
        backButton.push_back(CustomButton(0.02, 0.02, 0.08, 0.1, static_cast<int>(textures::backButton), ""));

        buttons.push_back(CustomButton(0.3, 0.4, 0.4, 0.12, static_cast<int>(textures::mainButton), "play"));
        buttons.push_back(CustomButton(0.3, 0.6, 0.4, 0.12, static_cast<int>(textures::mainButton), "credit"));
        buttons.push_back(CustomButton(0.3, 0.8, 0.4, 0.12, static_cast<int>(textures::mainButton), "exit"));
        buttons.push_back(CustomButton(0.04, 0.77, 0.10, 0.15, static_cast<int>(textures::deleteButton),""));
        if(music)
            buttons.push_back(CustomButton(0.85, 0.6, 0.15, 0.15, static_cast<int>(textures::musicOn), ""));
        else
            buttons.push_back(CustomButton(0.85, 0.6, 0.15, 0.15, static_cast<int>(textures::musicOff), ""));
        if(sound)
            buttons.push_back(CustomButton(0.85, 0.8, 0.15, 0.15, static_cast<int>(textures::soundOn), ""));
        else
            buttons.push_back(CustomButton(0.85, 0.8, 0.15, 0.15, static_cast<int>(textures::soundOff), ""));
        buttons.push_back(CustomButton(0.32, 0.6, 0.08, 0.1, static_cast<int>(textures::yes), ""));
        buttons.push_back(CustomButton(0.6, 0.6, 0.08, 0.1, static_cast<int>(textures::no), ""));
        for (int i = 0; i < 5; ++i)
        {
            for (int j = 0; j < 8; ++j)
            {
                std::string text_str = std::to_string(i * 8 + j + 1);
                stageButtons.push_back(CustomButton(0.2 + j * 0.08, 0.25 + i * 0.12, 0.06, 0.09, static_cast<int>(textures::notClear), text_str));
            }
        }
        if(allClear==true)
            stageButtons.push_back(CustomButton(0.2 + 3.5 * 0.08, 0.25 + 5 * 0.12, 0.06, 0.09, static_cast<int>(textures::clear), "TY"));
    }


    void drawTiles()
    {
        sf::Sprite sprite;
        sprite.setTexture(textures[static_cast<int>(textures::tile)]);
        sprite.setScale(resX * 0.8 / currentMapWidth / sprite.getLocalBounds().width, resY * 0.8 / currentMapHeight / sprite.getLocalBounds().height);
        for (int i = 0; i < currentMapHeight; i++)
        {
            for (int j = 0; j < currentMapWidth; j++)
            {
                if (gameMap[i][j] == -1)
                    continue;
                sprite.setPosition((0.1 + (j) * 0.8 / currentMapWidth) * resX, (0.1 + (i) * 0.8 / currentMapHeight) * resY);
                window.draw(sprite);
            }
        }
    }

    void drawMapElements()
    {
        for (int i = 0; i < currentMapHeight; i++)
        {
            for (int j = 0; j < currentMapWidth; j++)
            {
                int currentElement = gameMap[i][j];
                if (currentElement <= 0)
                    continue;
                if (currentElement == static_cast<int>(Objects::redWall))
                {
                    std::vector<int> newVec = std::vector<int>(2);
                    newVec[0] = i; newVec[1] = j;
                    auto it=std::find(snake.begin(), snake.end(), newVec);
                    if (it != snake.end())
                        currentElement = 9;
                    it = std::find(snakeClone.begin(), snakeClone.end(), newVec);
                    if (it != snakeClone.end())
                        currentElement = 9;
                    if (isBox[i][j] == 1)
                        currentElement = 9;
                }
                if (currentElement == static_cast<int>(Objects::blueWall))
                {
                    std::vector<int> newVec = std::vector<int>(2);
                    newVec[0] = i; newVec[1] = j;
                    auto it = std::find(snake.begin(), snake.end(), newVec);
                    if (it != snake.end())
                        currentElement = 12;
                    if (isBox[i][j] == 1)
                        currentElement = 12;
                }
                inGameSprites[currentElement].setScale(resX * 0.8 / currentMapWidth / inGameSprites[currentElement].getLocalBounds().width, resY * 0.8 / currentMapHeight / inGameSprites[currentElement].getLocalBounds().height);
                inGameSprites[currentElement].setPosition((0.1 + (j) * 0.8 / currentMapWidth) * resX, (0.1 + (i) * 0.8 / currentMapHeight) * resY);
                window.draw(inGameSprites[currentElement]);
            }
        }

        //상자
        for (int i = 0; i < currentMapHeight; i++)
        {
            for (int j = 0; j < currentMapWidth; j++)
            {
                if (isBox[i][j])
                {
                    inGameSprites[13].setScale(resX * 0.8 / currentMapWidth / inGameSprites[13].getLocalBounds().width, resY * 0.8 / currentMapHeight / inGameSprites[13].getLocalBounds().height);
                    inGameSprites[13].setPosition((0.1 + (j) * 0.8 / currentMapWidth) * resX, (0.1 + (i) * 0.8 / currentMapHeight) * resY);
                    window.draw(inGameSprites[13]);
                }
            }
        }

        //뱀
        if (snake.size() != 0)
        {
            drawSnake(0);
        }
        if (snakeClone.size() != 0)
        {
            drawSnake(1);
        }       
    }

    void drawSnake(int isClone)
    {
        std::deque<std::vector<int>>* currentSnake;
        if (isClone == 0)
            currentSnake = &snake;
        else
            currentSnake = &snakeClone;

        if ((*currentSnake).size() == 1)
        {
            inGameSprites[50 + lastPressed].setScale(resX * 0.8 / currentMapWidth / inGameSprites[50 + lastPressed].getLocalBounds().width, resY * 0.8 / currentMapHeight / inGameSprites[50 + lastPressed].getLocalBounds().height);
            inGameSprites[50 + lastPressed].setPosition((0.1 + ((*currentSnake)[0][1]) * 0.8 / currentMapWidth) * resX, (0.1 + ((*currentSnake)[0][0]) * 0.8 / currentMapHeight) * resY);
            window.draw(inGameSprites[50 + lastPressed]);
        }
        else
        {
            //머리
            int targetSprite = 54 + relationalPos((*currentSnake)[1][0], (*currentSnake)[1][1], (*currentSnake)[0][0], (*currentSnake)[0][1]);
            inGameSprites[targetSprite].setScale(resX * 0.8 / currentMapWidth / inGameSprites[targetSprite].getLocalBounds().width, resY * 0.8 / currentMapHeight / inGameSprites[targetSprite].getLocalBounds().height);
            inGameSprites[targetSprite].setPosition((0.1 + ((*currentSnake)[0][1]) * 0.8 / currentMapWidth) * resX, (0.1 + ((*currentSnake)[0][0]) * 0.8 / currentMapHeight) * resY);
            window.draw(inGameSprites[targetSprite]);

            //몸통
            for (int i = 1; i < (*currentSnake).size() - 1; i++)
            {
                targetSprite = 58 + relationalPos3((*currentSnake)[i - 1][0], (*currentSnake)[i - 1][1], (*currentSnake)[i][0], (*currentSnake)[i][1], (*currentSnake)[i + 1][0], (*currentSnake)[i + 1][1]);
                inGameSprites[targetSprite].setScale(resX * 0.8 / currentMapWidth / inGameSprites[targetSprite].getLocalBounds().width, resY * 0.8 / currentMapHeight / inGameSprites[targetSprite].getLocalBounds().height);
                inGameSprites[targetSprite].setPosition((0.1 + ((*currentSnake)[i][1]) * 0.8 / currentMapWidth) * resX, (0.1 + ((*currentSnake)[i][0]) * 0.8 / currentMapHeight) * resY);
                window.draw(inGameSprites[targetSprite]);
            }

            //꼬리
            targetSprite = 66 + relationalPos((*currentSnake)[(*currentSnake).size() - 2][0], (*currentSnake)[(*currentSnake).size() - 2][1], (*currentSnake)[(*currentSnake).size() - 1][0], (*currentSnake)[(*currentSnake).size() - 1][1]);
            inGameSprites[targetSprite].setScale(resX * 0.8 / currentMapWidth / inGameSprites[targetSprite].getLocalBounds().width, resY * 0.8 / currentMapHeight / inGameSprites[targetSprite].getLocalBounds().height);
            inGameSprites[targetSprite].setPosition((0.1 + ((*currentSnake)[(*currentSnake).size() - 1][1]) * 0.8 / currentMapWidth) * resX, (0.1 + ((*currentSnake)[(*currentSnake).size() - 1][0]) * 0.8 / currentMapHeight) * resY);
            window.draw(inGameSprites[targetSprite]);

        }
    }

    int relationalPos(int x1, int y1, int x2, int y2)//머리,꼬리,길이1 스프라이트 방향 결정에 사용, 점2의 상대적 위치
    {
        if (x2 - x1 == 1)//아래
            return 0;
        if (y2 - y1 == 1)//오른쪽
            return 1;
        if (x2 - x1 == -1)//위
            return 2;
        if (y2 - y1 == -1)//왼쪽
            return 3;
    }

    int relationalPos3(int x1, int y1,int x2,int y2,int x3,int y3)//몸통 스프라이트 결정에 사용,숫자가 낮을수록 머리에 가까움
    {
        if (x3 - x1 == 2)
            return 0;
        if (y3 - y1 == 2)
            return 1;
        if (x3 - x1 == -2)
            return 2;
        if (y3 - y1 == -2)
            return 3;
        if (x3 - x1 == 1)
        {
            if (y3 - y1 == 1)
            {
                if(x1==x2)
                    return 6;
                else
                    return 4;
            }
            else
            {
                if (x1 == x2)
                    return 7;
                else
                    return 5;
            }
        }
        else
        {    
            if (y3 - y1 == 1)
            {
                if (x1 == x2)
                    return 5;
                else
                    return 7;
            }
            else
            {
                if (x1 == x2)
                    return 4;
                else
                    return 6;
            }
        }
    }

    void drawImageAndText()
    {
        if (level == MAXLEVEL)
        {
            CustomText thankYouForPlaying = CustomText("Thank You For Playing!", 0.5, 0.05, 70);
            thankYouForPlaying.draw(window);
        }
        else
        {
            CustomText currentLevel = CustomText(std::to_string(level + 1), 0.95, 0.9, 50);
            currentLevel.draw(window);
        }
        
        if (level == 0)
        {
            drawLevel0ImageAndText();
        }
        if (turnAble == 1)
        {
            CustomSprite turn = CustomSprite(0, 0.76, 0.1, 0.15, static_cast<int>(textures::turn));
            turn.draw(window);
            CustomSprite space = CustomSprite(0, 0.875, 0.1, 0.1, static_cast<int>(textures::space));
            space.draw(window);
        }
    }

    void drawLevel0ImageAndText()
    {
        CustomSprite arrowLeft = CustomSprite(0.1, 0.875, 0.1, 0.12, static_cast<int>(textures::arrowLeft));
        arrowLeft.draw(window);
        CustomSprite arrowDown = CustomSprite(0.2, 0.875, 0.1, 0.12, static_cast<int>(textures::arrowDown));
        arrowDown.draw(window);
        CustomSprite arrowRight = CustomSprite(0.3, 0.875, 0.1, 0.12, static_cast<int>(textures::arrowRight));
        arrowRight.draw(window);
        CustomSprite arrowUp = CustomSprite(0.2, 0.76, 0.1, 0.12, static_cast<int>(textures::arrowUp));
        arrowUp.draw(window);
        CustomSprite keyZ = CustomSprite(0.5, 0.76, 0.1, 0.12, static_cast<int>(textures::keyZ));
        keyZ.draw(window);
        CustomSprite keyR = CustomSprite(0.5, 0.875, 0.1, 0.12, static_cast<int>(textures::keyR));
        keyR.draw(window);

        CustomText textUndo = CustomText(" - Undo   ", 0.68, 0.76, 70);
        textUndo.draw(window);
        CustomText textRestart = CustomText(" - Restart", 0.68, 0.875, 70);
        textRestart.draw(window);
    }

    void move(int derX, int derY,int isClone)
    {
        if (level == MAXLEVEL)
            return;
        std::deque<std::vector<int>>* currentSnake;
        if (isClone == 0)
            currentSnake=&snake;
        else
        {
            if (snakeClone.size() == 0)
                return ;
            currentSnake = &snakeClone;
        }

        std::vector<int> newVec = std::vector<int>(2);
        newVec[0] = (*currentSnake)[0][0] + derX;
        newVec[1] = (*currentSnake)[0][1] + derY;

        //뱀의 몸과 부딪치는지 확인
        for (int i = 0; i < snake.size(); i++)
        {
            if ((newVec[0] == snake[i][0]) && (newVec[1] == snake[i][1]))
                return ;
        }
        for (int i = 0; i < snakeClone.size(); i++)
        {
            if ((newVec[0] == snakeClone[i][0]) && (newVec[1] == snakeClone[i][1]))
                return ;
        }

        //뱀과 클론의 이동 순서 차이가 없도록 나중에 이동하는 뱀이 클론이 있던 자리에 가려고 하면 못 가게 함
        if ((newVec[0] == lastRemoved[0]) && (newVec[1] == lastRemoved[1])&& (isClone == 0))
            return ;


        if (isBox[newVec[0]][newVec[1]])//상자
        {
            if (isBox[newVec[0] + derX][newVec[1] + derY] == 0)
            {
                for (int i = 0; i < snake.size(); i++)
                {
                    if ((newVec[0] + derX == snake[i][0]) && (newVec[1] + derY == snake[i][1]))
                        return ;
                }
                for (int i = 0; i < snakeClone.size(); i++)
                {
                    if ((newVec[0] + derX == snakeClone[i][0]) && (newVec[1] + derY == snakeClone[i][1]))
                        return ;
                }

                switch (gameMap[newVec[0] + derX][newVec[1] + derY])
                {
                case static_cast<int>(Objects::redButton):
                {
                    if (sound)
                    {
                        sounds[static_cast<int>(sounds::button)].play();
                        sounds[static_cast<int>(sounds::move)].play();
                    }
                    isBox[newVec[0]][newVec[1]] = 0;
                    isBox[newVec[0] + derX][newVec[1] + derY] = 1;
                    if (redPressed == 0)
                    {
                        redPressed = 1;
                        redStateChange();
                    }
                    (*currentSnake).push_front(newVec);
                    removeTail(isClone);
                    return ;
                }
                case static_cast<int>(Objects::blueButton):
                {
                    if (sound)
                    {
                        sounds[static_cast<int>(sounds::button)].play();
                        sounds[static_cast<int>(sounds::move)].play();
                    }
                    blueStateChange();
                    isBox[newVec[0]][newVec[1]] = 0;
                    isBox[newVec[0] + derX][newVec[1] + derY]=1;
                    (*currentSnake).push_front(newVec);
                    removeTail(isClone);
                    return ;
                }
                case static_cast<int>(Objects::blank) : case static_cast<int>(Objects::ice):case static_cast<int>(Objects::RedWallDown):
                case static_cast<int>(Objects::blueWallDown): case static_cast<int>(Objects::passWall):
                case static_cast<int>(Objects::cloneHead):case static_cast<int>(Objects::cloneBody):
                {
                    if (sound)
                        sounds[static_cast<int>(sounds::move)].play();
                    isBox[newVec[0]][newVec[1]] = 0;
                    isBox[newVec[0] + derX][newVec[1] + derY] = 1;
                    (*currentSnake).push_front(newVec);
                    removeTail(isClone);
                    return ;
                }
                default:
                    return ;
                }
            }
            else
                return ;
        }

        switch (gameMap[newVec[0]][newVec[1]])
        {
        case static_cast<int>(Objects::blank): case static_cast<int>(Objects::RedWallDown):
        case static_cast<int>(Objects::blueWallDown): case static_cast<int>(Objects::passWall):
        case static_cast<int>(Objects::cloneHead):case static_cast<int>(Objects::cloneBody):
        {
            if ((isClone == 0) && ((preCheckOriSnakeDestination == 1) || (preCheckOriSnakeDestination == 8) || (preCheckOriSnakeDestination == 11)))
            {
                break;
            }
            if (sound)
                sounds[static_cast<int>(sounds::move)].play();
            (*currentSnake).push_front(newVec);
            removeTail(isClone);
            break;
        }
        case static_cast<int>(Objects::wall): case static_cast<int>(Objects::blueWall):case static_cast<int>(Objects::redWall): 
        {
            if ((isClone == 0) && ((preCheckOriSnakeDestination == 0) || (preCheckOriSnakeDestination == 9) || (preCheckOriSnakeDestination == 12)
                || (preCheckOriSnakeDestination == 15) || (preCheckOriSnakeDestination == 17) || (preCheckOriSnakeDestination == 18)))
            {
                (*currentSnake).push_front(newVec);
                removeTail(isClone);
            }
            break;
        }
        case static_cast<int>(Objects::apple):
        {
            (*currentSnake).push_front(newVec);
            gameMap[newVec[0]][newVec[1]] = 0;
            remainedFruits--;
            if (remainedFruits == 0)
            {
                if (sound)
                    sounds[static_cast<int>(sounds::clear)].play();
                clear[level] = 1;
                achievementCheck();
                levelCursor = level + 1;
                if (levelCursor == MAXLEVEL)
                    levelCursor = 0;
                saveClearData();
                for (int i = 0; i < MAXLEVEL; i++)
                {
                    if ((clear[i] == 0)||allClear)
                        break;
                    if (i == 39)
                    {
                        unlockAchievement("AllClear");
                        allClear = true;
                        levelToTitle();
                        level = MAXLEVEL;
                        titleToLevel();
                        return;
                    }
                }
                level = 0;
                levelToTitle();
                gameState = static_cast<GameState>(GameState::LevelSelect);
                break;
            }
            if (sound)
               sounds[static_cast<int>(sounds::eat)].play();
            break;
        }
        case static_cast<int>(Objects::spike):
        {
            die = 1;
            while ((*currentSnake).size() != 0)
                (*currentSnake).pop_back();
            return ;
        }
        case static_cast<int>(Objects::ice):
        {
            if (sound)
              sounds[static_cast<int>(sounds::move)].play();
            (*currentSnake).push_front(newVec);
            removeTail(isClone);
            if(!isBox[newVec[0]+derX][newVec[1] + derY])
                move(derX, derY,isClone);
            break;
        }
        case static_cast<int>(Objects::iceApple):
        {
            (*currentSnake).push_front(newVec);
            gameMap[newVec[0]][newVec[1]] = 4;
            remainedFruits--;
            if (remainedFruits == 0)
            {
                if (sound)
                    sounds[static_cast<int>(sounds::clear)].play();
                clear[level] = 1;
                achievementCheck();
                levelCursor = level + 1;
                if (levelCursor == MAXLEVEL)
                    levelCursor = 0;
                saveClearData();
                for (int i = 0; i < MAXLEVEL; i++)
                {
                    if ((clear[i] == 0) || allClear)
                        break;
                    if (i == 39)
                    {
                        unlockAchievement("AllClear");
                        allClear = true;
                        levelToTitle();
                        level = MAXLEVEL;
                        titleToLevel();
                        return;
                    }
                }
                level = 0;
                levelToTitle();
                gameState = static_cast<GameState>(GameState::LevelSelect);
                break;
            }
            if (sound)
                sounds[static_cast<int>(sounds::eat)].play();
            if (!isBox[newVec[0] + derX][newVec[1] + derY])
                move(derX, derY,isClone);
            break;
        }
        case static_cast<int>(Objects::antiApple):
        {
            gameMap[newVec[0]][newVec[1]] = 0;
            if ((*currentSnake).size() == 1)
            {
                die = 1;
                (*currentSnake).clear();
                return ;
            }
            if (sound)
                sounds[static_cast<int>(sounds::eat)].play();

            (*currentSnake).push_front(newVec);
            removeTail(isClone);
            removeTail(isClone);
            break;
        }
        case static_cast<int>(Objects::redButton):
        {
            if (sound)
                sounds[static_cast<int>(sounds::button)].play();
            (*currentSnake).push_front(newVec);
            removeTail(isClone);
            if (redPressed == 0)
            {
                redPressed = 1;
                redStateChange();
            }
            break;
        }
        case static_cast<int>(Objects::blueButton):
        {
            if (sound)
                sounds[static_cast<int>(sounds::button)].play();
            (*currentSnake).push_front(newVec);
            removeTail(isClone);
            blueStateChange();
            
            break;
        }
        case static_cast<int>(Objects::turnItem):
        {
            if (sound)
                sounds[static_cast<int>(sounds::item)].play();
            if (firstTurn == 0)
                firstTurn = newVec[1];
            (*currentSnake).push_front(newVec);
            removeTail(isClone);
            turnAble = 1;
            gameMap[newVec[0]][newVec[1]] = 0;
            break;
        }
        case static_cast<int>(Objects::cloneItem):
        {
            (*currentSnake).push_front(newVec);
            removeTail(isClone);
            if (isCloneAble())
            {
                if (sound)
                    sounds[static_cast<int>(sounds::item)].play();
                for (const auto& segment : levels[level].clone) 
                {
                    std::vector<int> cloneSegment = { segment[0], segment[1] };
                    snakeClone.push_back(cloneSegment);
                    gameMap[cloneSegment[0]][cloneSegment[1]] = 0;
                }
                gameMap[newVec[0]][newVec[1]] = 0;
            }
            else
            {
            }

            break;
        }

        }
        return ;
    }

    void removeTail(int isClone)
    {
        std::deque<std::vector<int>>* currentSnake;
        if (isClone == 0)      
            currentSnake = &snake;      
        else
        {
            currentSnake = &snakeClone;
            lastRemoved[0] = snakeClone[snakeClone.size() - 1][0];
            lastRemoved[1] = snakeClone[snakeClone.size() - 1][1];
        }
        
        int tailElement=gameMap[(*currentSnake)[(*currentSnake).size() - 1][0]][(*currentSnake)[(*currentSnake).size() - 1][1]];//꼬리에 있던 것
        if (tailElement == static_cast<int>(Objects::passWall))
        {
            gameMap[(*currentSnake)[(*currentSnake).size() - 1][0]][(*currentSnake)[(*currentSnake).size() - 1][1]] = 1;
        }
        else if((tailElement== static_cast<int>(Objects::redButton))&& (redPressed == 1))
        {
            for (int i = 0; i < (*currentSnake).size()-1; i++)
            {
                if (gameMap[(*currentSnake)[i][0]][(*currentSnake)[i][1]] == 7)
                    break;
                if (i == (*currentSnake).size() - 2)
                {
                    int pressedByBox = 0;
                    for (int k = 0; k < currentMapHeight; k++)
                    {
                        for (int l = 0; l < currentMapWidth; l++)
                        {
                            if ((isBox[k][l] == 1) && (gameMap[k][l] == 7))
                                pressedByBox = 1;
                        }
                    }
                    if (pressedByBox == 0)
                    {
                        redPressed = 0;
                        redStateChange();
                    }
                }
            }
        }
        (*currentSnake).pop_back();
    }

    void redStateChange()
    {
        for (int i = 0; i < currentMapHeight; i++)
        {
            for (int j = 0; j < currentMapWidth; j++)
            {
                if (gameMap[i][j] == 8)
                    gameMap[i][j] = 9;
                else if (gameMap[i][j] == 9)
                    gameMap[i][j] = 8;
            }
        }
    }

    void blueStateChange()
    {
        for (int i = 0; i < currentMapHeight; i++)
        {
            for (int j = 0; j < currentMapWidth; j++)
            {
                if (gameMap[i][j] == 11)
                    gameMap[i][j] = 12;
                else if (gameMap[i][j] == 12)
                    gameMap[i][j] = 11;
            }
        }
    }

    int isCloneAble()
    {
        for (const auto& segment : levels[level].clone)
        {
            //원래 뱀의 몸과 겹치는지 확인
            for (int i = 0; i < snake.size(); i++)
            {
                if ((segment[0] == snake[i][0]) && (segment[1] == snake[i][1]))
                    return 0;
            }
            //상자와 겹치는지 확인
            if (isBox[segment[0]][segment[1]] == 1)
                return 0;
        }
        return 1;
    }

    int sameSnake()
    {
        if (!mapStatusHistory.empty())
        {
            int same = 1;
            struct MapStatus preMap = mapStatusHistory.top();
            if ((snake.size() == preMap.snake.size()) && (snakeClone.size() == preMap.snakeClone.size()))
            {
                for (int i = 0; i < snake.size(); i++)
                {
                    if ((snake[i][0] != preMap.snake[i][0]) || (snake[i][1] != preMap.snake[i][1]))
                    {
                        same = 0;
                    }
                }
                for (int i = 0; i < snakeClone.size(); i++)
                {
                    if ((snakeClone[i][0] != preMap.snakeClone[i][0]) || (snakeClone[i][1] != preMap.snakeClone[i][1]))
                    {
                        same = 0;
                    }
                }
                if (same)
                    return 1;
            }
        }
        return 0;       
    }

    void pushMapStatus()
    {
        //이전 맵상태와 같은지 확인
        if (sameSnake())
            return;

        struct MapStatus currentMap;
        for (int i = 0; i < 25; i++)
        {
            for (int j = 0; j < 25; j++)
            {
                currentMap.gameMap[i][j] = gameMap[i][j];
                currentMap.isBox[i][j] = isBox[i][j];
            }
        }

        currentMap.snake.assign(snake.begin(), snake.end());
        currentMap.snakeClone.assign(snakeClone.begin(), snakeClone.end());

        currentMap.remainedFruits= remainedFruits ;
        currentMap.redPressed= redPressed ;
        currentMap.turnAble= turnAble;
        currentMap.lastPressed = lastPressed;

        mapStatusHistory.push(currentMap);
    }

    void popMapStatus()
    {
        //이전 맵상태와 현재와 같은지 확인 
        if (sameSnake())
            mapStatusHistory.pop();

        struct MapStatus currentMap;
        if (mapStatusHistory.empty())
            return;

        if (sound)
            sounds[static_cast<int>(sounds::undo)].play();
        if (die >= 1)
            die = 0;
        currentMap = mapStatusHistory.top();
        mapStatusHistory.pop();

        for (int i = 0; i < 25; i++)
        {
            for (int j = 0; j < 25; j++)
            {
                 gameMap[i][j]= currentMap.gameMap[i][j] ;
                 isBox[i][j]=currentMap.isBox[i][j] ;
            }
        }

        snake.assign(currentMap.snake.begin(), currentMap.snake.end());
        snakeClone.assign(currentMap.snakeClone.begin(), currentMap.snakeClone.end());
        
        remainedFruits = currentMap.remainedFruits;
        redPressed = currentMap.redPressed;
        turnAble = currentMap.turnAble;
        lastPressed = currentMap.lastPressed;
    }

    void drawGameOver()
    {
        sf::Sprite sprite;
        sprite.setTexture(textures[static_cast<int>(textures::gameOver)]);
        sprite.setScale(resX / sprite.getLocalBounds().width, resY / sprite.getLocalBounds().height);
        sprite.setPosition(0, 0);
        window.draw(sprite);
    }

    void achievementCheck()
    {
        unlockAchievement("Clear");
        if (level == 24)
        {
            if (isBox[1][14] == 1)
            {
                unlockAchievement("BoxOnIce");
            }
        }
        else if (level == 30)
        {
            if (firstTurn == 5)
            {
                unlockAchievement("AgainstTheFlow");
            }
        }
        else if (level == 31)
        {
            if (relationalPos(snake[0][0], snake[0][1], snake[1][0], snake[1][1]) != 
                relationalPos(snakeClone[0][0], snakeClone[0][1], snakeClone[1][0], snakeClone[1][1]))
            {
                unlockAchievement("DifferentView");
            }

        }
    }

    void processEvents()
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                SteamAPI_Shutdown();
                window.close();
            }
            if (event.type == sf::Event::MouseButtonPressed)
            {
                handleMouseClick(event.mouseButton.x, event.mouseButton.y);
            }

            if (event.type == sf::Event::KeyPressed)
            {
                if (gameState == GameState::LevelSelect)
                {
                    if ((event.key.code == sf::Keyboard::Space)|| (event.key.code == sf::Keyboard::Enter)|| (event.key.code == sf::Keyboard::Z))
                    {
                        if (sound)
                            sounds[static_cast<int>(sounds::buttonCliked)].play();
                        gameState = GameState::inGame;
                        level = levelCursor;
                        titleToLevel();
                    }
                    else if (event.key.code == sf::Keyboard::Right)
                    {
                        levelCursor ++;
                        if (levelCursor > 39)
                            levelCursor -= MAXLEVEL;
                    }
                    else if (event.key.code == sf::Keyboard::Up)
                    {
                        levelCursor-=8;
                        if (levelCursor < 0)
                            levelCursor += MAXLEVEL;
                    }
                    else if (event.key.code == sf::Keyboard::Left)
                    {
                        levelCursor--;
                        if (levelCursor < 0)
                            levelCursor += MAXLEVEL;
                    }
                    else if (event.key.code == sf::Keyboard::Down)
                    {
                        levelCursor+=8;
                        if (levelCursor > 39)
                            levelCursor -= MAXLEVEL;
                    }
                }
                else if (gameState == GameState::inGame)
                {
                    if (event.key.code == sf::Keyboard::Space)
                    {
                        if (!die)
                        {
                            pushMapStatus();
                            if (turnAble == 1)
                            {
                                if (sound)
                                    sounds[static_cast<int>(sounds::item)].play();
                                std::reverse(snake.begin(), snake.end());
                                std::reverse(snakeClone.begin(), snakeClone.end());
                                turnAble = 0;
                            }
                        }
                    }
                    else if (event.key.code == sf::Keyboard::Right)
                    {
                        arrowKeyPressed(0, 1);
                    }
                    else if (event.key.code == sf::Keyboard::Up)
                    {
                        arrowKeyPressed(-1, 0);
                    }
                    else if (event.key.code == sf::Keyboard::Left)
                    {
                        arrowKeyPressed(0, -1);
                    }
                    else if (event.key.code == sf::Keyboard::Down)
                    {
                        arrowKeyPressed(1, 0);
                    }
                    else if (event.key.code == sf::Keyboard::Z)
                    {
                        popMapStatus();
                    }
                    else if (event.key.code == sf::Keyboard::R)
                    {
                        if (sound)
                            sounds[static_cast<int>(sounds::undo)].play();
                        loadMap();
                    }
                    else if ((event.key.code == sf::Keyboard::Escape)|| (event.key.code == sf::Keyboard::BackSpace))
                    {
                            if (sound)
                                sounds[static_cast<int>(sounds::buttonCliked)].play();
                            level = 0;
                            levelToTitle();
                            gameState = GameState::LevelSelect;                  
                    }
                }
            }
        }
    }

    void arrowKeyPressed(int derX, int derY)
    {
        if (!die)
        {
            pushMapStatus();
            lastPressed = 1-derX-derY+derY*derY;
            preCheckOriSnakeDestination = gameMap[snake[0][0] + derX][snake[0][1]+derY];
            lastRemoved[0] = -1; lastRemoved[1] = -1;
            move(derX, derY, 1);
            move(derX, derY, 0);
        }
    }

    void render()
    {
        sf::Color mycolor(152, 222, 255);
        window.clear(mycolor);

        if (gameState == GameState::TitleScreen)
        {
            CustomSprite title = CustomSprite(0.325, 0.08, 0.35, 0.25, static_cast<int>(textures::title));
            title.draw(window);
            for (int i = 0; i < 6; i++)
            {
                buttons[i].draw(window);
            }
            if (dataDeletePopUp == 1)
            {
                CustomSprite deletePopUp = CustomSprite(0.25, 0.25, 0.5, 0.6, static_cast<int>(textures::deletePopup));
                deletePopUp.draw(window);
                buttons[6].draw(window);
                buttons[7].draw(window);
            }
            if (dataDeletePopUp == 2)
            {
                CustomSprite deletePopUp = CustomSprite(0.25, 0.25, 0.5, 0.6, static_cast<int>(textures::deleteCompletedPopup));
                deletePopUp.draw(window);
            }
        }
        else if (gameState == GameState::LevelSelect)
        {
            CustomSprite level = CustomSprite(0.4, 0.05, 0.2, 0.12, static_cast<int>(textures::level));
            level.draw(window);
            for (int i = 0; i < stageButtons.size(); i++)
            {
                if (i != MAXLEVEL)
                {
                    if (clear[i] == 0)
                        stageButtons[i].setTextureNum(static_cast<int>(textures::notClear));
                    else
                        stageButtons[i].setTextureNum(static_cast<int>(textures::clear));
                }
                stageButtons[i].draw(window);
            }
            backButton[0].draw(window); 

            CustomSprite lvCursor = CustomSprite(0.19 + levelCursor%8 * 0.08, 0.23 + levelCursor/8 * 0.12, 0.08, 0.12, static_cast<int>(textures::cloneBody));
            lvCursor.draw(window);
        }
        else if (gameState == GameState::inGame)
        {
            if (die>=1)
            {
                if (die == 1)
                {
                    if (sound)
                        sounds[static_cast<int>(sounds::gameOver)].play();
                    die++;
                }
                window.clear(sf::Color(55, 55, 55));
                drawTiles();
                drawMapElements();
                backButton[0].draw(window);
                drawGameOver();
                window.display();
                return;
            }
            window.clear(sf::Color(55, 55, 55));
            drawTiles();
            drawMapElements();
            drawImageAndText();
            backButton[0].draw(window);
        }
        else if (gameState == GameState::LevelSelect)
        {
            titleText.draw(window);
            for (int i = 0; i < stageButtons.size(); i++)
            {
                if (clear[i] == 0)
                    stageButtons[i].setTextureNum(static_cast<int>(textures::notClear));
                else
                    stageButtons[i].setTextureNum(static_cast<int>(textures::clear));
                stageButtons[i].draw(window);
            }
            backButton[0].draw(window);
        }
        else if (gameState == GameState::credit)
        {
            CustomSprite credit = CustomSprite(0, 0, 1, 1, static_cast<int>(textures::credit));
            credit.draw(window);
            backButton[0].draw(window);
        }
        window.display();
    }

    void handleMouseClick(float x, float y)
    {
        if (gameState == GameState::TitleScreen)
        {
            if (dataDeletePopUp == 1)
            {
                if (buttons[6].isClicked(x, y, window))
                {
                    if (sound)
                        sounds[static_cast<int>(sounds::buttonCliked)].play();
                    for (int i = 0; i < MAXLEVEL; i++)
                    {
                        clear[i] = 0;
                    }
                    saveClearData();
                    dataDeletePopUp = 2;
                }
                else if (buttons[7].isClicked(x, y, window))
                {
                    if (sound)
                        sounds[static_cast<int>(sounds::buttonCliked)].play();
                    dataDeletePopUp = 0;
                }
            }
            else if (dataDeletePopUp == 2)
            {
                if (sound)
                    sounds[static_cast<int>(sounds::buttonCliked)].play();
                dataDeletePopUp = 0;
            }
            else if (buttons[0].isClicked(x, y, window))
            {
                if (sound)
                    sounds[static_cast<int>(sounds::buttonCliked)].play();
                gameState = GameState::LevelSelect;
            }
            else if (buttons[1].isClicked(x, y, window))
            {
                if (sound)
                    sounds[static_cast<int>(sounds::buttonCliked)].play();
                gameState = GameState::credit;
            }
            else if (buttons[2].isClicked(x, y, window))
            {
                if (sound)
                    sounds[static_cast<int>(sounds::buttonCliked)].play();
                SteamAPI_Shutdown();
                window.close();
            }
            else if (buttons[3].isClicked(x, y, window))
            {
                if (sound)
                    sounds[static_cast<int>(sounds::buttonCliked)].play();
                dataDeletePopUp = 1;
            }
            else if (buttons[4].isClicked(x, y, window))
            {
                if (music)
                {
                    music = false;
                    buttons[4].setTextureNum(static_cast<int>(textures::musicOff));
                    bgm.stop();
                }
                else
                {
                    music = true;
                    buttons[4].setTextureNum(static_cast<int>(textures::musicOn));
                    bgm.setLoop(true);
                    bgm.play();
                }
            }
            else if (buttons[5].isClicked(x, y, window))
            {
                if (sound)
                {
                    sound = false;
                    buttons[5].setTextureNum(static_cast<int>(textures::soundOff));
                }
                else
                {
                    sound = true;
                    buttons[5].setTextureNum(static_cast<int>(textures::soundOn));
                }
            }
        }
        else if (gameState == GameState::LevelSelect)
        {
            for (int i = 0; i < stageButtons.size(); i++)
            {
                if (stageButtons[i].isClicked(x, y, window))
                {
                    if(sound)
                        sounds[static_cast<int>(sounds::buttonCliked)].play();
                    gameState = GameState::inGame;
                    level = i;
                    titleToLevel();
                    break;
                }
            }
            if (backButton[0].isClicked(x, y, window))
            {
                if (sound)
                    sounds[static_cast<int>(sounds::buttonCliked)].play();
                gameState = GameState::TitleScreen;
            }
        }
        else if (gameState == GameState::inGame)
        {
            if (backButton[0].isClicked(x, y, window))
            {
                if (sound)
                    sounds[static_cast<int>(sounds::buttonCliked)].play();
                levelToTitle();
                gameState = GameState::LevelSelect;
                level = 0;
            }
        }
        else if (gameState == GameState::credit)
        {
            if (backButton[0].isClicked(x, y, window))
            {
                if (sound)
                    sounds[static_cast<int>(sounds::buttonCliked)].play();
                gameState = GameState::TitleScreen;
            }

        }
    }
};

void readLevelData()
{
    std::ifstream inputFile("levels.json");
    if (!inputFile.is_open())
    {
        std::cerr << "Failed to open JSON file\n";
        return;
    }

    json jsonData;
    inputFile >> jsonData;
    inputFile.close();

    for (auto& levelData : jsonData["levels"])
    {
        Level level;
        level.id = levelData["id"];
        level.height = levelData["height"];
        level.width = levelData["width"];
        level.fruits = 0;
        for (auto& row : levelData["ini"])
        {
            std::vector<int> rowData;
            for (auto& value : row)
            {
                rowData.push_back(value);
                if ((value == 2)|| (value == 5))
                    level.fruits++;
            }
            level.ini.push_back(rowData);
        }
        for (const auto& segment : levelData["snake"])
        {
            std::vector<int> rowData;
            rowData.push_back(segment[0]);
            rowData.push_back(segment[1]);
            level.snake.push_back(rowData);
        }
        for (const auto& segment : levelData["box"])
        {
            std::vector<int> rowData;
            rowData.push_back(segment[0]);
            rowData.push_back(segment[1]);
            level.box.push_back(rowData);
        }
        for (const auto& segment : levelData["clone"])
        {
            std::vector<int> rowData;
            rowData.push_back(segment[0]);
            rowData.push_back(segment[1]);
            level.clone.push_back(rowData);
        }
        levels.push_back(level);
    }
}

int hashing()
{
    long long result = 1;
    for (int i = 0; i < MAXLEVEL; i++)
    {
        result *= i + 2;
        if (clear[i])
            result *=result;
        else
            result *= 720;

        result %= 100000009;
    }
    return result;
}

void readClearData()
{
    std::ifstream inputFile("clearData.json");
    if (!inputFile.is_open())
    {
        std::cerr << "Failed to open JSON file\n";
        return;
    }

    json jsonData;
    inputFile >> jsonData;
    inputFile.close();

    auto& clearArray = jsonData["clear"];
    for (int i = 0; i < MAXLEVEL ; i++) 
    {
        clear[i] = clearArray[i];
    }
    int hash = jsonData["hash"];
    if (hash != hashing())
    {
        std::cerr << "Incorrect Data\n";
        for (int i = 0; i < MAXLEVEL; i++)
        {
            clear[i] = 0;
        }
        saveClearData();
    }
    for (int i = 0; i < MAXLEVEL; i++)
    {
        if (clear[i] == 0)
            break;
        if (i == 39)
        {
            allClear = true;
        }
    }
}

void saveClearData() 
{

    json jsonData;
    jsonData["clear"] = clear;
    jsonData["hash"]=hashing();
    std::ofstream file("clearData.json");
    if (file.is_open()) 
    {
        file << jsonData.dump(4);  
        file.close();
    }
}


void loadTextures()
{
    textures[0].loadFromFile("images/mainButton.png");
    textures[1].loadFromFile("images/backButton.png");
    textures[2].loadFromFile("images/clear.png");
    textures[3].loadFromFile("images/notClear.png");
    textures[4].loadFromFile("images/tile.png");
    textures[5].loadFromFile("images/wall.png");
    textures[6].loadFromFile("images/apple.png");
    textures[7].loadFromFile("images/spike.png");
    textures[8].loadFromFile("images/ice.png");
    textures[9].loadFromFile("images/iceApple.png");
    textures[10].loadFromFile("images/antiApple.png");
    textures[11].loadFromFile("images/redButton.png");
    textures[12].loadFromFile("images/redWall.png");
    textures[13].loadFromFile("images/redWallDown.png");
    textures[14].loadFromFile("images/blueButton.png");
    textures[15].loadFromFile("images/blueWall.png");
    textures[16].loadFromFile("images/blueWallDown.png");
    textures[17].loadFromFile("images/box.png");
    textures[18].loadFromFile("images/turn.png");
    textures[19].loadFromFile("images/passWall.png");
    textures[20].loadFromFile("images/cloneItem.png");
    textures[21].loadFromFile("images/cloneHead.png");
    textures[22].loadFromFile("images/cloneBody.png");
    textures[23].loadFromFile("images/gameOver.png");

    textures[24].loadFromFile("images/snakeShort1.png");
    textures[25].loadFromFile("images/snakeShort2.png");
    textures[26].loadFromFile("images/snakeShort3.png");
    textures[27].loadFromFile("images/snakeShort4.png");
    textures[28].loadFromFile("images/snakeHead1.png");
    textures[29].loadFromFile("images/snakeHead2.png");
    textures[30].loadFromFile("images/snakeHead3.png");
    textures[31].loadFromFile("images/snakeHead4.png");
    textures[32].loadFromFile("images/snakeBody1.png");
    textures[33].loadFromFile("images/snakeBody2.png");
    textures[34].loadFromFile("images/snakeBody3.png");
    textures[35].loadFromFile("images/snakeBody4.png");
    textures[36].loadFromFile("images/snakeBody5.png");
    textures[37].loadFromFile("images/snakeBody6.png");
    textures[38].loadFromFile("images/snakeBody7.png");
    textures[39].loadFromFile("images/snakeBody8.png");
    textures[40].loadFromFile("images/snakeTail1.png");
    textures[41].loadFromFile("images/snakeTail2.png");
    textures[42].loadFromFile("images/snakeTail3.png");
    textures[43].loadFromFile("images/snakeTail4.png");
    textures[44].loadFromFile("images/arrowDown.png");
    textures[45].loadFromFile("images/arrowRight.png");
    textures[46].loadFromFile("images/arrowUp.png");
    textures[47].loadFromFile("images/arrowLeft.png");
    textures[48].loadFromFile("images/keyZ.png");
    textures[49].loadFromFile("images/keyR.png");
    textures[50].loadFromFile("images/space.png");
    textures[51].loadFromFile("images/title.png");
    textures[52].loadFromFile("images/musicOn.png");
    textures[53].loadFromFile("images/musicOff.png");
    textures[54].loadFromFile("images/soundOn.png");
    textures[55].loadFromFile("images/soundOff.png");
    textures[56].loadFromFile("images/level.png");
    textures[57].loadFromFile("images/credit.png");
    textures[58].loadFromFile("images/deleteButton.png");
    textures[59].loadFromFile("images/deletePopup.png");
    textures[60].loadFromFile("images/yes.png");
    textures[61].loadFromFile("images/no.png");
    textures[62].loadFromFile("images/deleteCompletedPopup.png");
}

void loadInGameSprites()
{
    inGameSprites[1].setTexture(textures[static_cast<int>(textures::wall)]);
    inGameSprites[2].setTexture(textures[static_cast<int>(textures::apple)]);
    inGameSprites[3].setTexture(textures[static_cast<int>(textures::spike)]);
    inGameSprites[4].setTexture(textures[static_cast<int>(textures::ice)]);
    inGameSprites[5].setTexture(textures[static_cast<int>(textures::iceApple)]);
    inGameSprites[6].setTexture(textures[static_cast<int>(textures::antiApple)]);
    inGameSprites[7].setTexture(textures[static_cast<int>(textures::redButton)]);
    inGameSprites[8].setTexture(textures[static_cast<int>(textures::redWall)]);
    inGameSprites[9].setTexture(textures[static_cast<int>(textures::redWallDown)]);
    inGameSprites[10].setTexture(textures[static_cast<int>(textures::blueButton)]);
    inGameSprites[11].setTexture(textures[static_cast<int>(textures::blueWall)]);
    inGameSprites[12].setTexture(textures[static_cast<int>(textures::blueWallDown)]);
    inGameSprites[13].setTexture(textures[static_cast<int>(textures::box)]);
    inGameSprites[14].setTexture(textures[static_cast<int>(textures::turn)]);
    inGameSprites[15].setTexture(textures[static_cast<int>(textures::passWall)]);
    inGameSprites[16].setTexture(textures[static_cast<int>(textures::cloneItem)]);
    inGameSprites[17].setTexture(textures[static_cast<int>(textures::cloneHead)]);
    inGameSprites[18].setTexture(textures[static_cast<int>(textures::cloneBody)]);

    inGameSprites[50].setTexture(textures[static_cast<int>(textures::snakeShort1)]);
    inGameSprites[51].setTexture(textures[static_cast<int>(textures::snakeShort2)]);
    inGameSprites[52].setTexture(textures[static_cast<int>(textures::snakeShort3)]);
    inGameSprites[53].setTexture(textures[static_cast<int>(textures::snakeShort4)]);
    inGameSprites[54].setTexture(textures[static_cast<int>(textures::snakeHead1)]);
    inGameSprites[55].setTexture(textures[static_cast<int>(textures::snakeHead2)]);
    inGameSprites[56].setTexture(textures[static_cast<int>(textures::snakeHead3)]);
    inGameSprites[57].setTexture(textures[static_cast<int>(textures::snakeHead4)]);
    inGameSprites[58].setTexture(textures[static_cast<int>(textures::snakeBody1)]);
    inGameSprites[59].setTexture(textures[static_cast<int>(textures::snakeBody2)]);
    inGameSprites[60].setTexture(textures[static_cast<int>(textures::snakeBody3)]);
    inGameSprites[61].setTexture(textures[static_cast<int>(textures::snakeBody4)]);
    inGameSprites[62].setTexture(textures[static_cast<int>(textures::snakeBody5)]);
    inGameSprites[63].setTexture(textures[static_cast<int>(textures::snakeBody6)]);
    inGameSprites[64].setTexture(textures[static_cast<int>(textures::snakeBody7)]);
    inGameSprites[65].setTexture(textures[static_cast<int>(textures::snakeBody8)]);
    inGameSprites[66].setTexture(textures[static_cast<int>(textures::snakeTail1)]);
    inGameSprites[67].setTexture(textures[static_cast<int>(textures::snakeTail2)]);
    inGameSprites[68].setTexture(textures[static_cast<int>(textures::snakeTail3)]);
    inGameSprites[69].setTexture(textures[static_cast<int>(textures::snakeTail4)]);
}


void setSound(int soundNum, std::string soundFileName)
{
    if (!soundBuffers[soundNum].loadFromFile(soundFileName))
    {
        std::cerr << "sound file load fail" << soundFileName << "\n";
        return;
    }
    sounds[soundNum].setBuffer(soundBuffers[soundNum]);
}

void loadSounds()
{
    setSound(static_cast<int>(sounds::item), "sounds/item.wav");
    setSound(static_cast<int>(sounds::gameOver), "sounds/gameOver.wav");
    setSound(static_cast<int>(sounds::clear), "sounds/clear.wav");
    setSound(static_cast<int>(sounds::move), "sounds/move.wav");
    setSound(static_cast<int>(sounds::button), "sounds/button.wav");
    setSound(static_cast<int>(sounds::buttonCliked), "sounds/buttonCliked.wav");
    setSound(static_cast<int>(sounds::eat), "sounds/eat.wav");
    setSound(static_cast<int>(sounds::undo), "sounds/undo.wav");

    if (!bgm.openFromFile("sounds/teaOrCoffee.wav")) 
    {
        std::cerr << "BGM file load fail\n" ;
    }
    bgm.setLoop(true);
    bgm.play();
}

void unlockAchievement(const char* achievementID)
{
    SteamUserStats()->SetAchievement(achievementID);
    SteamUserStats()->StoreStats();
      
}

int main()
{
    SteamAPI_Init();
    readLevelData();
    readClearData();
    loadTextures();
    loadInGameSprites();
    loadSounds();
    Game game;
    game.run();
    return 0;
}
