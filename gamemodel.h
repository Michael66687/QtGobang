#ifndef GAMEMODEL_H
#define GAMEMODEL_H

#include <vector>
#include<iostream>
using namespace std;

// 游戏类型，单机双人还是AI（固定玩家先手黑子，AI后手白子）
enum GameType
{
    PERSON,
    BOT
};

// 游戏状态
enum GameStatus
{
    PLAYING,
    WIN,
    DEAD
};

enum AIStlye
{
    ATTACK,
    DEFEND,
    NEUTRAL
};

// 棋盘尺寸
const int BoardSizeNum = 18;

class GameModel
{
public:
    GameModel();

public:
    std::vector<std::vector<int>> gameMapVec; // 存储当前游戏棋盘和棋子的情况,空白为0，白子1，黑子-1
    std::vector<std::vector<int>> scoreMapVec; // 存储各个点位的加权情况，作为AI下棋依据
    bool playerFlag; // 表示下棋方
    GameType gameType; // 游戏模式
    GameStatus gameStatus; // 游戏状态
    AIStlye aistlye; // AI风格

    void startGame(GameType type); // 开始游戏
    void calculateScore(int KillTwo, int KillThreeAct, int KillThreeDead, int KillFourAct, int KillFourDead, int KillFive,
        int SetCommon, int SetTwo, int SetThreeAct, int SetThreeDead, int SetFourAct, int SetFourDead, int SetFive); // 计算评分
    void actionByAI(int& clickRow, int& clickCol, 
        int KillTwo, int KillThreeAct, int KillThreeDead, int KillFourAct, int KillFourDead, int KillFive,
        int SetCommon, int SetTwo, int SetThreeAct, int SetThreeDead, int SetFourAct, int SetFourDead, int SetFive); // AI下棋
    void actionByPerson(int row, int col); // 玩家下棋
    void updateGameMap(int row, int col); // 每次落子后更新游戏棋盘
    bool isWin(int row, int col); // 判断游戏是否胜利
    bool isDeadGame(); // 判断是否和棋
};
#endif 