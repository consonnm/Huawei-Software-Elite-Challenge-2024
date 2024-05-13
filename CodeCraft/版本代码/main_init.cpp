//#include <bits/stdc++.h>
//using namespace std;
//
//const int n = 200;
//const int robot_num = 10;
//const int berth_num = 10;
//const int N = 210;
//
//struct Robot
//{
//    int x, y, goods;
//    int status;
//    int mbx, mby;
//    Robot() {}
//    Robot(int startX, int startY) {
//        x = startX;
//        y = startY;
//    }
//}robot[robot_num + 10];
//
//struct Berth
//{
//    int x;
//    int y;
//    int transport_time;
//    int loading_speed;
//    Berth(){}
//    Berth(int x, int y, int transport_time, int loading_speed) {
//        this -> x = x;
//        this -> y = y;
//        this -> transport_time = transport_time;
//        this -> loading_speed = loading_speed;
//    }
//}berth[berth_num + 10];
//
//struct Boat
//{
//    int capacity, pos, status;
//}boat[10];
//
//int money, boat_capacity, id;
//char ch[N][N];
//int gds[N][N];
//void Init()
//{
//    for(int i = 1; i <= n; i ++)
//        scanf("%s", ch[i] + 1);
//    for(int i = 0; i < berth_num; i ++)
//    {
//        int id;
//        scanf("%d", &id);
//        scanf("%d%d%d%d", &berth[id].x, &berth[id].y, &berth[id].transport_time, &berth[id].loading_speed);
//    }
//    scanf("%d", &boat_capacity);
//    char okk[100];
//    scanf("%s", okk);
//    printf("OK\n");
//    fflush(stdout);
//}
//
//int Input()
//{
//    scanf("%d%d", &id, &money); // 帧序号 金钱数
//    int capacity;
//    scanf("%d", &capacity); // 新增货物的数量
//    for(int i = 1; i <= capacity; i ++)
//    {
//        int x, y, val;
//        scanf("%d%d%d", &x, &y, &val); // 货物的坐标 & 货物的金额
//    }
//    for(int i = 0; i < robot_num; i ++)
//    {
//        int sts;
//        scanf("%d%d%d%d", &robot[i].goods, &robot[i].x, &robot[i].y, &sts); // 是否携带货物  机器人的坐标  机器人的状态
//    }
//    for(int i = 0; i < 5; i ++)
//        scanf("%d%d\n", &boat[i].status, &boat[i].pos); // 船的状态  泊位ID
//    char okk[100];
//    scanf("%s", okk);
//    return id;
//}
//
//int main_init()
//{
//
//    Init();
//    for(int zhen = 1; zhen <= 15000; zhen++)
//    {
//        int id = Input();
//        for(int i = 0; i < robot_num; i ++)
//            printf("move %d %d\n", i, rand() % 4);
//        puts("OK");
//        fflush(stdout);
//    }
//
//    return 0;
//}
//
////  .  空地
////  *  海洋
////  #  障碍
////  A  机器人起始位置，总共 10 个。
////  B  大小为 4*4，表示泊位的位置,泊位标号在后泊位处初始化。
