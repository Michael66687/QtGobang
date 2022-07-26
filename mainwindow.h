#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "gamemodel.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = 0);
    ~MainWindow();

protected:
    // 绘制
    void paintEvent(QPaintEvent* event);
    // 监听鼠标移动情况，方便落子
    void mouseMoveEvent(QMouseEvent* event);
    // 实际落子
    void mouseReleaseEvent(QMouseEvent* event);

private:
    GameModel* game; // 游戏指针
    GameType game_type; // 存储游戏类型
    int clickPosRow, clickPosCol; // 存储将点击的位置
    void initGame();//初始化

private slots:
    void chessOneByPerson(); // 人执行
    void chessOneByAI(); // AI攻击风格

    void SetAttackAI();
    void SetDefendAI();
    void SetNeutralAI();
    
    void initPVPGame();
    void initPVEGame();

    void clearGame();//清空棋盘
    void saveGame();//保存棋局
    void loadGame();//加载上次保存的残局
    void quitGame();//退出游戏

    void Regret();//悔棋
};

#endif