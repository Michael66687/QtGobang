#include <utility>
#include <stdlib.h>
#include <time.h>
#include<qmessagebox.h>
#include<iostream>
#include"gamemodel.h"
using namespace std;

GameModel::GameModel()
{
    playerFlag = true;
    gameType = PERSON;
    gameStatus = PLAYING;
    aistlye = DEFEND;
}

void GameModel::startGame(GameType type)
{
    gameType = type;
    //初始化空棋盘
    if (gameType == PERSON)
    {
        gameMapVec.clear();
        for (int i = 0; i < BoardSizeNum; i++)
        {
            std::vector<int> lineBoard;
            for (int j = 0; j < BoardSizeNum; j++)
                lineBoard.push_back(0);
            gameMapVec.push_back(lineBoard);
        }
    }

    //如果是AI模式，需要初始化评分数组
    if (gameType == BOT)
    {
        scoreMapVec.clear();
        for (int i = 0; i < BoardSizeNum; i++)
        {
            std::vector<int> lineScores;
            for (int j = 0; j < BoardSizeNum; j++)
                lineScores.push_back(0);
            scoreMapVec.push_back(lineScores);
        }
        gameMapVec.clear();
        for (int i = 0; i < BoardSizeNum; i++)
        {
            std::vector<int> lineBoard;
            for (int j = 0; j < BoardSizeNum; j++)
                lineBoard.push_back(0);
            gameMapVec.push_back(lineBoard);
        }
    }
    //玩家黑子先行
    playerFlag = true;
}

void GameModel::updateGameMap(int row, int col)
{
    if (playerFlag)
    {
        gameMapVec[row][col] = -1;
    }
    else
    {
        gameMapVec[row][col] = 1;
    }
    // 换手
    playerFlag = !playerFlag;
}

void GameModel::actionByPerson(int row, int col)
{
    if (row < BoardSizeNum && col < BoardSizeNum)
    {
        updateGameMap(row, col);
    }
}

void GameModel::actionByAI(int& clickRow, int& clickCol, 
    int KillTwo, int KillThreeAct, int KillThreeDead, int KillFourAct, int KillFourDead, int KillFive,
    int SetCommon, int SetTwo, int SetThreeAct, int SetThreeDead, int SetFourAct, int SetFourDead, int SetFive)
{
    //对每个点进行加权
    calculateScore(KillTwo, KillThreeAct, KillThreeDead, KillFourAct, KillFourDead, KillFive,
        SetCommon, SetTwo, SetThreeAct, SetThreeDead, SetFourAct, SetFourDead, SetFive);

    //在加权数组中找出权重最大的点
    int maxScore = 0;
    std::vector<std::pair<int, int>> maxPoints;

    for (int row = 1; row < BoardSizeNum; row++)
    {
        for (int col = 1; col < BoardSizeNum; col++)
        {
            //只对空点进行查找
            if (gameMapVec[row][col] == 0)
            {
                if (scoreMapVec[row][col] > maxScore)
                {
                    maxPoints.clear();
                    maxScore = scoreMapVec[row][col];
                    maxPoints.push_back(std::make_pair(row, col));
                }
                else if (scoreMapVec[row][col] == maxScore)
                {
                    maxPoints.push_back(std::make_pair(row, col));//将多个最大权重的点都保存起来
                }
            }
        }
    }

    //如果棋盘为空，则在棋盘中央落子
    for (int row = 1, col = 1; row < BoardSizeNum; row++)
    {
        for (; col < BoardSizeNum; col++)//col在第一层循环里已经定义过
        {
            if (scoreMapVec[row][col] != scoreMapVec[1][1])
            {
                break;
            }
        }
        if (row == BoardSizeNum - 1 && col == BoardSizeNum - 1)
        {
            updateGameMap(BoardSizeNum / 2, BoardSizeNum / 2);
        }
    }

    //如果有多个最大权重的点，就在其中随机选择一个落子
    srand((unsigned)time(0));
    int index = rand() % maxPoints.size();

    std::pair<int, int> pointPair = maxPoints.at(index);
    clickRow = pointPair.first;
    clickCol = pointPair.second;
    updateGameMap(clickRow, clickCol);
}

//AI的核心部分，用于计算每个点的权重
void GameModel::calculateScore(int KillTwo, int KillThreeAct, int KillThreeDead, int KillFourAct, int KillFourDead, int KillFive,
    int SetCommon, int SetTwo, int SetThreeAct, int SetThreeDead, int SetFourAct, int SetFourDead, int SetFive)
{
    int personNum = 0; //玩家连子的个数
    int botNum = 0; //AI连子的个数
    int emptyNum = 0; //各方向空点的个数，用于判断棋子的“死”“活”情况，1个空位为死，2个空位为活

    //先清除上次计算保留的数据，重新初始化加权数组
    scoreMapVec.clear();
    for (int i = 0; i < BoardSizeNum; i++)
    {
        std::vector<int> lineScores;
        for (int j = 0; j < BoardSizeNum; j++)
        {
            lineScores.push_back(0);
        }
        scoreMapVec.push_back(lineScores);
    }

    //计分模块,通过调整权重值可以调整AI智能程度以及攻守风格
    for (int row = 0; row < BoardSizeNum; row++)
    {
        for (int col = 0; col < BoardSizeNum; col++)
        {
            //只计算空白点
            if (gameMapVec[row][col] == 0)
            {
                //2*3=6种组合，（0，0）和（-1，0）方向除外，共四个方向，每个方向上向正反两个方向延伸，从而遍历该点周围八个方向
                for (int y = 0; y <= 1; y++)
                {
                    for (int x = -1; x <= 1; x++)
                    {
                        //每次计算前需要将数据重置为0
                        personNum = 0;
                        botNum = 0;
                        emptyNum = 0;

                        if (!(y == 0 && x == 0) && !(y == 0 && x == -1))//不包含（0，0）和（-1，0）
                        {
                            //只需要向每个方向延伸4个子，若已经四连则必须堵，五连则游戏已经结束

                            //对玩家黑子评分（正反两个方向）
                            for (int i = 1; i <= 4; i++)
                            {
                                if (row + i * y > 0 && row + i * y < BoardSizeNum &&
                                    col + i * x > 0 && col + i * x < BoardSizeNum &&
                                    gameMapVec[row + i * y][col + i * x] == -1)
                                {
                                    personNum++;//增加personNum加权
                                }
                                else if (row + i * y > 0 && row + i * y < BoardSizeNum &&
                                    col + i * x > 0 && col + i * x < BoardSizeNum &&
                                    gameMapVec[row + i * y][col + i * x] == 0)
                                {
                                    emptyNum++;//增加emptyNum加权，有一个空位后就break
                                    break;
                                }
                                else//出边界的情况
                                {
                                    break;
                                }
                            }
                            for (int i = 1; i <= 4; i++)//另一个方向
                            {
                                if (row - i * y > 0 && row - i * y < BoardSizeNum &&
                                    col - i * x > 0 && col - i * x < BoardSizeNum &&
                                    gameMapVec[row - i * y][col - i * x] == -1)
                                {
                                    personNum++;
                                }
                                else if (row - i * y > 0 && row - i * y < BoardSizeNum &&
                                    col - i * x > 0 && col - i * x < BoardSizeNum &&
                                    gameMapVec[row - i * y][col - i * x] == 0)
                                {
                                    emptyNum++;
                                    break;
                                }
                                else
                                {
                                    break;
                                }
                            }

                            if (personNum == 1)//堵玩家可能二连的棋
                            {
                                scoreMapVec[row][col] += KillTwo;// 10
                            }
                            else if (personNum == 2)//堵玩家可能三连的棋
                            {
                                //空位数不同，加权也不同
                                if (emptyNum == 1)
                                {
                                    scoreMapVec[row][col] += KillThreeDead;//30
                                }
                                else if (emptyNum == 2)
                                {
                                    scoreMapVec[row][col] += KillThreeAct;//40
                                }
                            }
                            else if (personNum == 3)//堵玩家可能四连的棋
                            {
                                if (emptyNum == 1)
                                {
                                    scoreMapVec[row][col] += KillFourDead;//60
                                }
                                else if (emptyNum == 2)
                                {
                                    scoreMapVec[row][col] += KillFourAct;//110
                                }
                            }
                            else if (personNum == 4)//堵玩家可能五连的棋
                            {
                                scoreMapVec[row][col] += KillFive;//10100
                            }

                            emptyNum = 0;//重置空位数，用于统计AI的情况

                            //对AI的白子加权，同样也是两个方向
                            for (int i = 1; i <= 4; i++)
                            {
                                if (row + i * y > 0 && row + i * y < BoardSizeNum &&
                                    col + i * x > 0 && col + i * x < BoardSizeNum &&
                                    gameMapVec[row + i * y][col + i * x] == 1)
                                {
                                    botNum++;
                                }
                                else if (row + i * y > 0 && row + i * y < BoardSizeNum &&
                                    col + i * x > 0 && col + i * x < BoardSizeNum &&
                                    gameMapVec[row + i * y][col + i * x] == 0)//空白位
                                {
                                    emptyNum++;
                                    break;
                                }
                                else//出边界
                                {
                                    break;
                                }
                            }
                            for (int i = 1; i <= 4; i++)//另一个方向
                            {
                                if (row - i * y > 0 && row - i * y < BoardSizeNum &&
                                    col - i * x > 0 && col - i * x < BoardSizeNum &&
                                    gameMapVec[row - i * y][col - i * x] == 1)
                                {
                                    botNum++;
                                }
                                else if (row - i * y > 0 && row - i * y < BoardSizeNum &&
                                    col - i * x > 0 && col - i * x < BoardSizeNum &&
                                    gameMapVec[row - i * y][col - i * x] == 0)
                                {
                                    emptyNum++;
                                    break;
                                }
                                else//出边界
                                {
                                    break;
                                }
                            }

                            if (botNum == 0)//普通下子
                            {
                                scoreMapVec[row][col] += SetCommon;//5
                            }
                            else if (botNum == 1)//活二
                            {
                                scoreMapVec[row][col] += SetTwo;//10
                            }
                            else if (botNum == 2)
                            {
                                if (emptyNum == 1)//死三
                                {
                                    scoreMapVec[row][col] += SetThreeDead;//25
                                }
                                else if (emptyNum == 2)// 活三
                                {
                                    scoreMapVec[row][col] += SetThreeAct;//50
                                }
                            }
                            else if (botNum == 3)
                            {
                                if (emptyNum == 1)//死四
                                {
                                    scoreMapVec[row][col] += SetFourDead;//55
                                }
                                else if (emptyNum == 2)//活四
                                {
                                    scoreMapVec[row][col] += SetFourAct;//100
                                }
                            }
                            else if (botNum >= 4)//五子连珠，直接获得胜利
                            {
                                scoreMapVec[row][col] += SetFive;//10000
                            }
                        }
                    }
                }
            }
        }
    }
}

bool GameModel::isWin(int row, int col)
{
    // 横竖斜四种大情况，每种情况都根据当前落子往后遍历5个棋子，有一种符合就算赢
    // 水平方向
    for (int i = 0; i < 5; i++)
    {
        // 往左5个，往右匹配4个子，20种情况
        if (col - i > 0 &&
            col - i + 4 < BoardSizeNum &&
            gameMapVec[row][col - i] == gameMapVec[row][col - i + 1] &&
            gameMapVec[row][col - i] == gameMapVec[row][col - i + 2] &&
            gameMapVec[row][col - i] == gameMapVec[row][col - i + 3] &&
            gameMapVec[row][col - i] == gameMapVec[row][col - i + 4])
            return true;
    }

    // 竖直方向(上下延伸4个)
    for (int i = 0; i < 5; i++)
    {
        if (row - i > 0 &&
            row - i + 4 < BoardSizeNum &&
            gameMapVec[row - i][col] == gameMapVec[row - i + 1][col] &&
            gameMapVec[row - i][col] == gameMapVec[row - i + 2][col] &&
            gameMapVec[row - i][col] == gameMapVec[row - i + 3][col] &&
            gameMapVec[row - i][col] == gameMapVec[row - i + 4][col])
            return true;
    }

    // 左斜方向
    for (int i = 0; i < 5; i++)
    {
        if (row + i < BoardSizeNum &&
            row + i - 4 > 0 &&
            col - i > 0 &&
            col - i + 4 < BoardSizeNum &&
            gameMapVec[row + i][col - i] == gameMapVec[row + i - 1][col - i + 1] &&
            gameMapVec[row + i][col - i] == gameMapVec[row + i - 2][col - i + 2] &&
            gameMapVec[row + i][col - i] == gameMapVec[row + i - 3][col - i + 3] &&
            gameMapVec[row + i][col - i] == gameMapVec[row + i - 4][col - i + 4])
            return true;
    }

    // 右斜方向
    for (int i = 0; i < 5; i++)
    {
        if (row - i > 0 &&
            row - i + 4 < BoardSizeNum &&
            col - i > 0 &&
            col - i + 4 < BoardSizeNum &&
            gameMapVec[row - i][col - i] == gameMapVec[row - i + 1][col - i + 1] &&
            gameMapVec[row - i][col - i] == gameMapVec[row - i + 2][col - i + 2] &&
            gameMapVec[row - i][col - i] == gameMapVec[row - i + 3][col - i + 3] &&
            gameMapVec[row - i][col - i] == gameMapVec[row - i + 4][col - i + 4])
            return true;
    }

    return false;
}

bool GameModel::isDeadGame()
{
    // 所有空格全部填满
    for (int i = 1; i < BoardSizeNum; i++)
        for (int j = 1; j < BoardSizeNum; j++)
        {
            if (!(gameMapVec[i][j] == 1 || gameMapVec[i][j] == -1))
                return false;
        }
    return true;
}
