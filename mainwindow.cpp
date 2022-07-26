#include<QPainter>
#include<QTimer>
#include<QMouseEvent>
#include<QMessageBox>
#include<QMenuBar>
#include<QMenu>
#include<QAction>
#include<QDebug>
#include<math.h>
#include<vector>
#include<fstream>
#include"gamemodel.h"
#include"mainwindow.h"
using namespace std;

const int BoardMargin = 30; // 棋盘边缘空隙
const int Radius = 15; // 棋子半径
const int MarkSize = 8; // 落子标记边长
const int BlockSize = 40; // 格子的大小
const int PosDelta = 20; // 鼠标点击的模糊距离上限
const int AIDelay = 500; // AI下棋的思考时间
int ibefore, jbefore, ilast, jlast;//记录刚刚下棋的位置，用于悔棋操作
//AI默认初始化为攻守兼备
int KillTwo = 10, KillThreeAct = 40, KillThreeDead = 30, KillFourAct = 110, KillFourDead = 60, KillFive = 100000;//用于计算玩家棋子加权
int SetCommon = 5, SetTwo = 10, SetThreeAct = 40, SetThreeDead = 30, SetFourAct = 110, SetFourDead = 60, SetFive = 100000;//用于计算AI棋子加权

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    //设置窗口名称
    setWindowTitle(QStringLiteral("五子棋小游戏"));
    //设置棋盘大小
    setFixedSize(BoardMargin * 2 + BlockSize * BoardSizeNum, BoardMargin * 2 + BlockSize * BoardSizeNum + 25);

    setMouseTracking(true);
    // 添加菜单
    QMenu* gameMenu = menuBar()->addMenu(tr("模式选择"));
    QAction* actionPVP = new QAction("单机双人对战", this);
    connect(actionPVP, SIGNAL(triggered()), this, SLOT(initPVPGame()));
    gameMenu->addAction(actionPVP);

    QAction* actionPVE = new QAction("人机对战", this);
    connect(actionPVE, SIGNAL(triggered()), this, SLOT(initPVEGame()));
    gameMenu->addAction(actionPVE);

    QMenu* BoardMenu = menuBar()->addMenu(tr("棋局"));
    QAction* clear = new QAction("清空棋盘", this);
    connect(clear, SIGNAL(triggered()), this, SLOT(clearGame()));
    BoardMenu->addAction(clear);

    QAction* save = new QAction("保存当前棋局", this);
    connect(save, SIGNAL(triggered()), this, SLOT(saveGame()));
    BoardMenu->addAction(save);

    QAction* load = new QAction("加载上次保存的残局", this);
    connect(load, SIGNAL(triggered()), this, SLOT(loadGame()));
    BoardMenu->addAction(load);

    QAction* quit = new QAction("退出游戏", this);
    connect(quit, SIGNAL(triggered()), this, SLOT(quitGame()));
    BoardMenu->addAction(quit);

    QMenu* RegretMenu = menuBar()->addMenu(tr("悔棋"));
    QAction* regret = new QAction("悔棋", this);
    connect(regret, SIGNAL(triggered()), this, SLOT(Regret()));
    RegretMenu->addAction(regret);

    QMenu* SetAIMenu = menuBar()->addMenu(tr("AI风格选择"));
    QAction* Attack = new QAction("攻击风格", this);
    connect(Attack, SIGNAL(triggered()), this, SLOT(SetAttackAI()));
    SetAIMenu->addAction(Attack);

    QAction* Defend = new QAction("防守风格", this);
    connect(Defend, SIGNAL(triggered()), this, SLOT(SetDefendAI()));
    SetAIMenu->addAction(Defend);

    QAction* Neutral = new QAction("攻守兼备", this);
    connect(Neutral, SIGNAL(triggered()), this, SLOT(SetNeutralAI()));
    SetAIMenu->addAction(Neutral);

    // 开始游戏
    initGame();
}

MainWindow::~MainWindow()
{
    if (game)
    {
        delete game;
        game = NULL;
    }
}

void MainWindow::initGame()
{
    //默认初始化为双人玩家对局
    game = new GameModel;
    initPVPGame();
}

void MainWindow::initPVPGame()
{
    game_type = PERSON;
    game->gameStatus = PLAYING;
    game->startGame(game_type);
    update();
}

void MainWindow::initPVEGame()
{
    SetNeutralAI();//AI默认初始化为攻防兼备风格
    game_type = BOT;
    game->gameStatus = PLAYING;
    game->startGame(game_type);
    update();
}

void MainWindow::SetAttackAI()
{
    KillTwo = 10;
    KillThreeAct = 50;
    KillThreeDead = 20;
    KillFourAct = 120;
    KillFourDead = 50;
    KillFive = 100000;
    SetCommon = 5;
    SetTwo = 10;
    SetThreeAct = 60;
    SetThreeDead = 30;
    SetFourAct = 150;
    SetFourDead = 70;
    SetFive = 100000;
    game->aistlye = ATTACK;
}

void MainWindow::SetDefendAI()
{
    KillTwo = 15;
    KillThreeAct = 60;
    KillThreeDead = 30;
    KillFourAct = 150;
    KillFourDead = 70;
    KillFive = 100000;
    SetCommon = 5;
    SetTwo = 10;
    SetThreeAct = 50;
    SetThreeDead = 20;
    SetFourAct = 120;
    SetFourDead = 50;
    SetFive = 100000;
    game->aistlye = DEFEND;
}

void MainWindow::SetNeutralAI()
{
    KillTwo = 10;
    KillThreeAct = 40;
    KillThreeDead = 30;
    KillFourAct = 110;
    KillFourDead = 60;
    KillFive = 100000;
    SetCommon = 5;
    SetTwo = 10;
    SetThreeAct = 40;
    SetThreeDead = 30;
    SetFourAct = 110;
    SetFourDead = 60;
    SetFive = 100000;
    game->aistlye = NEUTRAL;
}

void MainWindow::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    // 绘制棋盘
    painter.setRenderHint(QPainter::Antialiasing, true);//抗锯齿
    for (int i = 0; i < BoardSizeNum + 1; i++)
    {
        painter.drawLine(BoardMargin + BlockSize * i, BoardMargin, BoardMargin + BlockSize * i, BoardMargin + BoardSizeNum * BlockSize);
        painter.drawLine(BoardMargin, BoardMargin + BlockSize * i, size().width() - BoardMargin, BoardMargin + BlockSize * i);
    }

    //绘制棋盘上的点
    QBrush brush;
    brush.setStyle(Qt::SolidPattern);
    brush.setColor(Qt::black);
    painter.setBrush(brush);
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            painter.drawEllipse(BoardMargin + BlockSize * ((2 * i + 1) * BoardSizeNum / 6) - MarkSize / 2, BoardMargin + BlockSize * ((2 * j + 1) * BoardSizeNum / 6) - MarkSize / 2, MarkSize, MarkSize);
        }
    }

    //绘制游戏模式显示
    if (game_type == PERSON)
    {
        painter.drawText(size().width() - 200, BlockSize * BoardSizeNum + 2 * BoardMargin, tr("当前游戏模式：单机双人对战"));
    }
    else
    {
        painter.drawText(size().width() - 200, BlockSize * BoardSizeNum + 2 * BoardMargin, tr("当前游戏模式：人机对战"));
        if (game->aistlye == ATTACK)
        {
            painter.drawText(size().width() - 200, BlockSize * BoardSizeNum + 2 * BoardMargin + 15, tr("当前AI风格：攻击风格"));
        }
        else if(game->aistlye == DEFEND)
        {
            painter.drawText(size().width() - 200, BlockSize * BoardSizeNum + 2 * BoardMargin + 15, tr("当前AI风格：防守风格"));
        }
        else
        {
            painter.drawText(size().width() - 200, BlockSize * BoardSizeNum + 2 * BoardMargin + 15, tr("当前AI风格：攻守兼备"));
        }
    }

    // 绘制落子标记(防止鼠标出框越界)    
    if (clickPosRow > 0 && clickPosRow < BoardSizeNum &&
        clickPosCol > 0 && clickPosCol < BoardSizeNum &&
        game->gameMapVec[clickPosRow][clickPosCol] == 0)
    {
        if (game->playerFlag)
            brush.setColor(Qt::black);
        else
            brush.setColor(Qt::white);
        painter.setBrush(brush);
        painter.drawRect(BoardMargin + BlockSize * clickPosCol - MarkSize / 2, BoardMargin + BlockSize * clickPosRow - MarkSize / 2, MarkSize, MarkSize);
    }

    // 绘制棋子 
    for (int i = 1; i < BoardSizeNum; i++)
        for (int j = 1; j < BoardSizeNum; j++)
        {
            if (game->gameMapVec[i][j] == 1)
            {
                brush.setColor(Qt::white);
                painter.setBrush(brush);
                painter.drawEllipse(BoardMargin + BlockSize * j - Radius, BoardMargin + BlockSize * i - Radius, Radius * 2, Radius * 2);
            }
            else if (game->gameMapVec[i][j] == -1)
            {
                brush.setColor(Qt::black);
                painter.setBrush(brush);
                painter.drawEllipse(BoardMargin + BlockSize * j - Radius, BoardMargin + BlockSize * i - Radius, Radius * 2, Radius * 2);
            }
        }

    // 判断输赢
    if (clickPosRow > 0 && clickPosRow < BoardSizeNum &&
        clickPosCol > 0 && clickPosCol < BoardSizeNum &&
        (game->gameMapVec[clickPosRow][clickPosCol] == 1 ||
            game->gameMapVec[clickPosRow][clickPosCol] == -1))
    {
        if (game->isWin(clickPosRow, clickPosCol) && game->gameStatus == PLAYING)
        {
            qDebug() << "win";
            game->gameStatus = WIN;
            QString str;
            if (game->gameMapVec[clickPosRow][clickPosCol] == 1)
                str = "白棋";
            else if (game->gameMapVec[clickPosRow][clickPosCol] == -1)
                str = "黑棋";
            QMessageBox::StandardButton btnValue = QMessageBox::information(this, "游戏结束", str + "胜出!");
            QMessageBox::StandardButton result;
            result = QMessageBox::question(this, QString::fromStdString("对局已结束"),
                QString::fromStdString("继续游戏？"),
                QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
            if (QMessageBox::No == result)
            {
                game->gameStatus = WIN;
                exit(0);
            }
            if (QMessageBox::Yes == result)
            {
                game->startGame(game_type);
                game->gameStatus = PLAYING;
            }
        }
    }

    // 判断平局
    if (game->isDeadGame())
    {
        QMessageBox::StandardButton btnValue = QMessageBox::information(this, "游戏结束", "平局!");
        QMessageBox::StandardButton result;
        result = QMessageBox::question(this, QString::fromStdString("对局已结束"),
            QString::fromStdString("继续游戏？"),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        if (QMessageBox::No == result)
        {
            game->gameStatus = DEAD;
            exit(0);
        }
        if (QMessageBox::Yes == result)
        {
            game->startGame(game_type);
            game->gameStatus = PLAYING;
        }
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent* event)
{
    int x = event->x();
    int y = event->y();

    //五子棋规定棋盘边缘不能落子
    if (x >= BoardMargin + BlockSize / 2 &&
        x < size().width() - BoardMargin &&
        y >= BoardMargin + BlockSize / 2 &&
        y < size().height() - BoardMargin - 40)
    {
        //获取最近的左上角的点
        int col = x / BlockSize;
        if (col > BoardSizeNum)
        {
            col = BoardSizeNum - 1;
        }
        int row = y / BlockSize;
        if (row > BoardSizeNum)
        {
            row = BoardSizeNum - 1;
        }

        int leftX = BoardMargin + BlockSize * col;
        int leftY = BoardMargin + BlockSize * row;

        //根据距离算出合适的点击位置，由几何关系可知这样只能找出至多一个点
        clickPosRow = -1;
        clickPosCol = -1;
        int len = 0;

        len = sqrt((x - leftX) * (x - leftX) + (y - leftY) * (y - leftY));
        if (len < PosDelta)
        {
            clickPosRow = row;
            clickPosCol = col;
        }
        len = sqrt((x - leftX - BlockSize) * (x - leftX - BlockSize) + (y - leftY) * (y - leftY));
        if (len < PosDelta)
        {
            clickPosRow = row;
            clickPosCol = col+ 1;
        }
        len = sqrt((x - leftX) * (x - leftX) + (y - leftY - BlockSize) * (y - leftY - BlockSize));
        if (len < PosDelta)
        {
            clickPosRow = row +1;
            clickPosCol = col;
        }
        len = sqrt((x - leftX - BlockSize) * (x - leftX - BlockSize) + (y - leftY - BlockSize) * (y - leftY - BlockSize));
        if (len < PosDelta)
        {
            clickPosRow = row +1;
            clickPosCol = col +1;
        }
    }

    //重绘棋盘，以更新落子标记
    update();
}

void MainWindow::mouseReleaseEvent(QMouseEvent* event)
{
    if (!(game_type == BOT && !game->playerFlag))
    {
        chessOneByPerson();
        if (game->gameType == BOT && !game->playerFlag)
        {
            //使得AI下棋时间稍有延迟，时间可通过修改AIDelay的值进行修改
            QTimer::singleShot(AIDelay, this, SLOT(chessOneByAI()));
        }
    }

}

void MainWindow::chessOneByPerson()
{
    // 根据当前存储的坐标下子
    // 有效点击且该处没有子时才能下子
    if (clickPosRow != -1 && clickPosCol != -1 && game->gameMapVec[clickPosRow][clickPosCol] == 0)
    {
        game->actionByPerson(clickPosRow, clickPosCol);
        //thinkTime = 90;//重置思考时长
        ilast = clickPosRow;
        jlast = clickPosCol;

        // 重绘
        update();
    }
}

void MainWindow::chessOneByAI()
{
    game->actionByAI(clickPosRow, clickPosCol,
       KillTwo, KillThreeAct, KillThreeDead, KillFourAct, KillFourDead, KillFive,
       SetCommon, SetTwo, SetThreeAct, SetThreeDead, SetFourAct, SetFourDead, SetFive);
    ibefore = clickPosRow;
    jbefore = clickPosCol;
    update();
}

void MainWindow::clearGame()
{
    //这里不能用vector自带的clear函数，那样会把分配的内存也清除掉
    //这里只需要清空棋子，即将所有点的值置为0
    for (int i = 0; i < BoardSizeNum; i++)
    {
        for (int j = 0; j < BoardSizeNum; j++)
        {
            game->gameMapVec[i][j] = 0;
        }
    }
    QMessageBox::StandardButton btnValue = QMessageBox::information(this, "清空棋盘", "棋盘已清空!");
}

void MainWindow::saveGame()
{
    const string& filename = "Board.txt";
    ofstream out(filename.c_str(), ios::out | ios::trunc);
    if (out)
    {
        for (int i = 0; i < BoardSizeNum; i++)
        {
            for (int j = 0; j < BoardSizeNum; j++)
            {
                out << ' ';
                out << game->gameMapVec[i][j];
            }
        }
        out << endl;
    }
    else
    {
        QMessageBox::StandardButton btnValue = QMessageBox::information(this, "异常", "文件打开失败!");
        return;
    }
    char T;
    if (game_type == PERSON)
    {
        T = 'p';
    }
    else
    {
        T = 'b';
    }
    out << T;
    out << endl;
    if (game->playerFlag == true)
    {
        out << 1;
    }
    else
    {
        out << 0;
    }
    out << endl;
    QMessageBox::StandardButton btnValue = QMessageBox::information(this, "保存残局", "保存残局成功!");
    out.close();
}

void MainWindow::loadGame()
{
    const string& filename = "Board.txt";
    ifstream in(filename.c_str(), ios::in);
    std::vector<std::vector<int>> ChessBoard;
    std::vector<int> lineBoard;
    for (int i = 0; i < BoardSizeNum; i++)
    {
        for (int j = 0; j < BoardSizeNum; j++)
        {
            lineBoard.push_back(0);
        }
        ChessBoard.push_back(lineBoard);
    }
    char type;
    int flag;
    if (in)
    {
        for (int i = 0; i < BoardSizeNum; i++)
        {
            for (int j = 0; j < BoardSizeNum; j++)
            {
                in >> ChessBoard[i][j];
            }
        }
    }
    else
    {
        QMessageBox::StandardButton btnValue = QMessageBox::information(this, "异常", "打开文件失败!");
        return;
    }
    in >> type;
    in >> flag;
    in.close();
    if (type == 'p')
    {
        initPVPGame();
        for (int i = 0; i < BoardSizeNum; i++)
        {
            for (int j = 0; j < BoardSizeNum; j++)
            {
                game->gameMapVec[i][j] = ChessBoard[i][j];
            }
        }
        if (flag == 1)
        {
            game->playerFlag = true;
        }
        if (flag == 0)
        {
            game->playerFlag = false;
        }
        QMessageBox::StandardButton btnValue = QMessageBox::information(this, "加载残局", "加载残局成功!");
    }
    else if (type == 'b')
    {
        initPVEGame();
        for (int i = 0; i < BoardSizeNum; i++)
        {
            for (int j = 0; j < BoardSizeNum; j++)
            {
                game->gameMapVec[i][j] = ChessBoard[i][j];
            }
        }
        if (flag == 1)
        {
            game->playerFlag = true;
        }
        if (flag == 0)
        {
            game->playerFlag = false;
        }
        QMessageBox::StandardButton btnValue = QMessageBox::information(this, "加载残局", "加载残局成功!");
    }
    else
    {
        QMessageBox::StandardButton btnValue = QMessageBox::information(this, "异常", "加载游戏类型失败!");
    }
}

void MainWindow::quitGame()
{
   QMessageBox::StandardButton result;
   result = QMessageBox::question(this, QString::fromStdString("退出游戏"),
            QString::fromStdString("您是否确认退出？"),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (QMessageBox::No == result)
    {
        return;
    }
    if (QMessageBox::Yes == result) 
    {
        QMessageBox::StandardButton saveresult;
        saveresult = QMessageBox::question(this, QString::fromStdString("保存当前对局"),
            QString::fromStdString("您是否要保存当前对局？"),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        if (QMessageBox::No == saveresult)
        {
            ;
        }
        if (QMessageBox::Yes == saveresult)
        {
            saveGame();
        }
        exit(0);
    }
}

void MainWindow::Regret()
{
    string str1, str2;
    if (game->playerFlag)
    {
        str1 = "白方";
        str2 = "黑方";
    }
    if (!game->playerFlag)
    {
        str1 = "黑方";
        str2 = "白方";
    }
    //悔棋时不能连续悔棋，PVP模式下只能从棋盘上去掉一个棋子，PVE模式下可去除两个棋子
    if (game_type == PERSON)
    {
        //PVP模式下悔棋时要得到对方同意
        QMessageBox::StandardButton result;
        result = QMessageBox::question(this, QString::fromStdString(str1 + "请求悔棋"),
            QString::fromStdString(str2 + "是否允许对方悔棋？"),
            QMessageBox::Yes | QMessageBox::No);
       
        if (QMessageBox::Yes == result)
        {
            if (game->playerFlag)
            {
                game->gameMapVec[ilast][jlast] = 0;
                update();
                game->playerFlag = !game->playerFlag;//换手
                return;
            }
            if (!game->playerFlag)
            {
                game->gameMapVec[ilast][jlast] = 0;
                update();
                game->playerFlag = !game->playerFlag;//换手
                return;
            }
        }
        if (QMessageBox::No == result)
        {
            return;
        }
    }
    if (game_type == BOT);
    {
        game->gameMapVec[ibefore][jbefore] = 0;
        game->gameMapVec[ilast][jlast] = 0;
        update();
        return;
    }
}