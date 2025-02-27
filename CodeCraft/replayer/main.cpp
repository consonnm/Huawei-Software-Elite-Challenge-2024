#include <bits/stdc++.h>

#pragma GCC optimize(3)
using namespace std;
const int n = 200;
const int robot_num = 10;
const int berth_num = 10;
const int N = 210;
//int ck = 0;
// double w_berth_1 = 10000, w_berth_2 = -10, w_berth_3 = 0.5, w_berth_4 = 0.5, w_berth_5 = 100;
////w_berth_1:该泊位是否停靠船只 w_berth_2:该泊位货物量减船容量 w_berth_3:该泊位性价比 w_berth_4:该泊位货物量 w_berth_5:到达泊位的距离
// double w_good_1 = 1000, w_good_2 = 1000, w_good_3 = 0.5;

//double w_berth_size = 0.5, w_berth_speed = 0.5, w_berth_dis = 0.5; // w_berth_size > w_berth_dis > w_berth_speed
double w_good_val = 0.5, w_good_dis = 0.5;
double w_boat_speed = 4, w_boat_transport = 2;

// w_good_1:货物价值 w_good_2:货物距离
struct Robot {
    int x, y, goods;
    int status;
    int mbx, mby;

    Robot() {}

    Robot(int startX, int startY) {
        x = startX;
        y = startY;
    }
} robot[robot_num + 10];

struct Berth {
    int x;
    int y;
    int transport_time;
    int loading_speed;
    int size;
    int load;
    int arrive;

    int near_good_num;

    Berth() {}

    Berth(int x, int y, int transport_time, int loading_speed) {
        this->x = x;
        this->y = y;
        this->transport_time = transport_time;
        this->loading_speed = loading_speed;
    }
} berth[berth_num + 10];

struct Boat {
    int num, pos, status, berth, flag;
} boat[10];
struct Goods {
    int x, y, val, time;
} good;

struct Node {
    int x, y;
    double gn, hn;

    bool operator>(const Node &W) const {
        return gn + hn > W.gn + W.hn;
    }
};

//int goods_num;
int money, boat_capacity, id;
char ch[N][N];         // 地图
int good_time[N][N]; // 货物出现时间
int vis[N][N];         // 是否访问过
// 0 right 1 left 2 up 3 down
int dx[] = {0, 0, -1, 1};
int dy[] = {1, -1, 0, 0};
int fa[N][N][2];
//int robot_move[robot_num + 10];
int vis_good[N][N];          // 货物是否被锁定
int vis_ban[N][N];          // 该位置是否被锁定
int vis_berth[berth_num]; // 泊位是否被锁定
int flag_berth[N][N];      // 泊位ID
int good_value[N][N];
int vis_search_berth[10];
//int latest_near_good_time[N][N]; // 附近最近刷新货物时间
//int highest_near_good_val[N][N]; // 附近货物最高价值
//int virtual_berth[N][N];         // 虚拟泊位
//int id_virtual_berth[N][N];         // 虚拟泊位ID
int ban_boot[10][2];             // 禁止机器人走的位置
double pre_dis_berth[10][N][N];     // 预处理泊位到所有点的最短距离
//int vis_add[10];
int targer_good[10][2];

struct SEARCH {
    int x, id;

    friend bool operator<(SEARCH a, SEARCH b) {
        return a.x < b.x;
    }
} search_berth[10];

int search_size;

void bfs_dis(int x, int y, int id) {
    for (int i = 1; i <= n; i++)
        for (int j = 1; j <= n; j++)
            vis[i][j] = 0, pre_dis_berth[id][i][j] = 1e9;
    queue<pair<int, int>> q;
    queue<int> distance;
    q.emplace(x, y);
    distance.push(0);
    vis[x][y] = 1;
    while (!q.empty()) {
        pair<int, int> now = q.front();
        q.pop();
        int dis = distance.front();
        distance.pop();
        if (ch[now.first][now.second] == '.' || ch[now.first][now.second] == 'A' || ch[now.first][now.second] == 'B') {
            pre_dis_berth[id][now.first][now.second] = dis;
        }
        for (int i = 0; i < 4; i++) {
            int nx = now.first + dx[i];
            int ny = now.second + dy[i];
            if (ch[nx][ny] == '*' || ch[nx][ny] == '#')
                continue;
            if (nx < 1 || nx > n || ny < 1 || ny > n)
                continue;
            if (vis[nx][ny] == 1)
                continue;
            q.emplace(nx, ny);
            distance.push(dis + 1);
            vis[nx][ny] = 1;
        }
    }
}

void Init() {
    for (int i = 1; i <= n; i++)
        scanf("%s", ch[i] + 1); // 地图
    for (int i = 0; i < berth_num; i++) {
        int id;
        scanf("%d", &id); // 泊位ID
        scanf("%d%d%d%d", &berth[id].x, &berth[id].y, &berth[id].transport_time, &berth[id].loading_speed);
        berth[id].x++;
        berth[id].y++;
        // 坐标，运输时间，装卸速度
        for (int t = 0; t <= 3; t++)
            for (int j = 0; j <= 3; j++)
                flag_berth[berth[id].x + t][berth[id].y + j] = id;
    }
    for (int i = 0; i < berth_num; i++) {
        bfs_dis(berth[i].x, berth[i].y, i);
    }
//    std::ofstream file("output.txt");
//
//    for (int i = 1; i <= n; i++) {
//        for (int j = 1; j <= n; j++) {
//            file << pre_dis_berth[7][i][j] << ' ';
//        }
//        file << '\n';
//    }
//
//    file.close();
    scanf("%d", &boat_capacity); // 船的容量
    char okk[100];
    scanf("%s", okk);
    printf("OK\n");
    fflush(stdout);
}

int Input(int zhen) {
    scanf("%d%d", &id, &money); // 帧序号和金钱
    int num;
    // 物品1000帧消失
    scanf("%d", &num); // 场上新增货物数量
    for (int i = 1; i <= num; i++) {
        int x, y, val;
        scanf("%d%d%d", &x, &y, &val); // 货物坐标和价值<100
        x++, y++;
        if (ch[x][y] != 'B' && ch[x][y] != '*' && ch[x][y] != '#') {
            good_time[x][y] = zhen;
            good_value[x][y] = val;

            for (int j = 0; j < berth_num; j++) {
                // (x, y) 在 berth 附件的范围内
                if (abs(x - berth[j].x) <= 10 && abs(y - berth[j].y) <= 10) {
                    berth[j].near_good_num++;
                }
            }
        }
    }
    for (int i = 0; i < robot_num; i++) {
        int sts;
        scanf("%d%d%d%d", &robot[i].goods, &robot[i].x, &robot[i].y, &sts);
        robot[i].x++;
        robot[i].y++;
        // 是否携带物品，坐标，状态
    }
    for (int i = 0; i < 5; i++) {
        scanf("%d%d\n", &boat[i].status, &boat[i].pos);
        // 船的状态和ID
        // 如果处于空闲状态，前往泊位
        if (boat[i].status == 1 && boat[i].flag == 0) {
            // 寻找最佳泊位
            double mx = -1e9;
            int pos;
            for (int t = 0; t < berth_num; t++) {
                if (vis_berth[t] == 0) {
                    // 计算总性价比
                    double w;
                    if (15000 - zhen <= 2100) {
                        // +
                        w = 100 * (w_boat_speed - berth[t].transport_time / 1000.0 * w_boat_transport);
                    } else {
                        // 1 <= boat_capacity <= 1000
                        // 1 <= loading_speed <= 5
                        // 1 <= transport_time <= 1000
                        // 0.001 <= loading_speed / transport_time <= 5
                        w = berth[t].size * 1.0 / boat_capacity *
                            (berth[t].loading_speed / 5.0 * w_boat_speed -
                             berth[t].transport_time / 1000.0 * w_boat_transport);
//
//                        double tt = berth[t].size * 1.0 / boat_capacity;
//                        w = tt * berth[t].loading_speed
//                        w = berth[t].size * 1.0 / (boat_capacity + 20) * w_boat_size +
//                            berth[t].loading_speed * 1.0 / berth[t].transport_time / 5 * w_boat_speed;

//                        w = berth[t].size * 1.0 / (boat_capacity + 20) * w_boat_size +
//                            berth[t].loading_speed * 1.0 / berth[t].transport_time * w_boat_speed;
                        // 248215  double w_boat_speed = 0.5, w_boat_size = 0.5;

//                        w = (berth[t].size / (boat_capacity + 20)) * berth[t].transport_time +
//                            berth[t].loading_speed * (1 - berth[t].size / (boat_capacity + 20));  // 246367
                    }

                    if (w > mx) {
                        mx = w;
                        pos = t;
                    }
                }
            }
            vis_berth[pos] = 1;
            printf("ship %d %d\n", i, pos);
            boat[i].num = boat_capacity;
            boat[i].berth = pos;
            boat[i].flag = 1;
        }
            // 如果处于装货状态，装货
        else if (boat[i].status == 1 && boat[i].flag == 1) {
            // 如果时间不够
            if (14980 - berth[boat[i].berth].transport_time - zhen < 0) {
                printf("go %d\n", i);
                boat[i].num = boat_capacity;
                vis_berth[boat[i].berth] = 0;
                boat[i].flag = 0;
                berth[boat[i].berth].load = 0;
                berth[boat[i].berth].arrive = 0;
                continue;
            }
            if (boat[i].num == 0) {
                printf("go %d\n", i);
                boat[i].num = boat_capacity;
                vis_berth[boat[i].berth] = 0;
                boat[i].flag = 0;
                berth[boat[i].berth].load = 0;
                berth[boat[i].berth].arrive = 0;
                continue;
            }
            int temp = min(berth[boat[i].berth].size, berth[boat[i].berth].loading_speed);
            temp = min(temp, boat[i].num);
            boat[i].num -= temp;
            berth[boat[i].berth].size -= temp;
            if (berth[boat[i].berth].load == 0) {
                berth[boat[i].berth].arrive = 1;
            }
            berth[boat[i].berth].load += temp;
        }
    }

    char okk[100];
    scanf("%s", okk);
    return id;
}

void bfs_good(int x, int y, int zhen, int id) {
    int check_move = 0;
    for (int i = 1; i <= n; i++)
        for (int j = 1; j <= n; j++)
            vis[i][j] = 0;
    queue<pair<int, int>> q;
    queue<int> distance;
    q.emplace(x, y);
    distance.push(0);
    vis[x][y] = 1;
    double mx = -1e9;
    pair<int, int> pos;
    pos = {-1, -1};
    int re = 0;
    while (!q.empty()) {
        re++;
        if (pos.first != -1 && re > 1e3)
            break;
        pair<int, int> now = q.front();
        q.pop();
        int dis = distance.front();
        distance.pop();
        if (good_time[now.first][now.second] > 0 && zhen - good_time[now.first][now.second] < 1000 &&
            vis_good[now.first][now.second] == 0) {
            double w = good_value[now.first][now.second] * w_good_val - w_good_dis * dis;
            if (w > mx) {
                mx = w;
                pos = now;
            }
        }
        for (int i = 0; i < 4; i++) {
            int nx = now.first + dx[i];
            int ny = now.second + dy[i];
            if (ch[nx][ny] == '*' || ch[nx][ny] == '#')
                continue;
            if (nx < 1 || nx > n || ny < 1 || ny > n)
                continue;
            if (vis[nx][ny] == 1)
                continue;
            if ((vis_ban[nx][ny] == 1 && (abs(nx - x) + abs(ny - y)) < 3) || vis_ban[nx][ny] >= 2)
                continue;
            if (nx == ban_boot[id][0] && ny == ban_boot[id][1])
                continue;
            fa[nx][ny][0] = now.first;
            fa[nx][ny][1] = now.second;
            q.emplace(nx, ny);
            distance.push(dis + 1);
            vis[nx][ny] = 1;
        }
    }
    if (pos.first == -1)
        return;
    vis_good[pos.first][pos.second] = 1;
    targer_good[id][0] = pos.first;
    targer_good[id][1] = pos.second;
    int temp_x = pos.first;
    int temp_y = pos.second;
//    int h = 0;
//    int ck_x = temp_x;
//    int ck_y = temp_y;
    while (fa[temp_x][temp_y][0] != x || fa[temp_x][temp_y][1] != y) {
        if (vis_ban[temp_x][temp_y] == 1) {
            for (int i = 0; i < berth_num; i++) {
                if (temp_x == berth[i].x && temp_y == berth[i].y) {
                    ban_boot[i][0] = temp_x;
                    ban_boot[i][1] = temp_y;
                    break;
                }
            }
            vis_ban[temp_x][temp_y]++;
        }
        int x1 = fa[temp_x][temp_y][0];
        int y1 = fa[temp_x][temp_y][1];
        if (fa[x1][y1][0] == x && fa[x1][y1][1] == y) {
            vis_ban[temp_x][temp_y] = 1;
        }
        temp_x = x1;
        temp_y = y1;
    }
    int di_x = temp_x - x;
    int di_y = temp_y - y;
    for (int i = 0; i < 4; i++) {
        if (di_x == dx[i] && di_y == dy[i]) {
            check_move = 1;
            printf("move %d %d\n", id, i);
            vis_ban[temp_x][temp_y] = 1;
            break;
        }
    }
    if (check_move == 0) {
        vis_ban[x][y] = 1;
    }
}

void bfs_berth(int x, int y, int id) {
    search_size = -1;
    for (int i = 0; i < berth_num; i++) {
        if (abs(1e9 - pre_dis_berth[i][x][y]) < 0.0001) continue;
        search_berth[++search_size].x =
                -vis_berth[i] * 1000 - (berth[i].load < boat_capacity) * 100 - berth[i].arrive * 10 +
                pre_dis_berth[i][x][y] - berth[i].near_good_num * (i % 2);

//        search_berth[++search_size].x =
//                -vis_berth[i] * 1000 - (berth[i].load < boat_capacity) * 10 - berth[i].arrive * 10 +
//                pre_dis_berth[i][x][y] - berth[i].near_good_num * (i % 3) * berth[i].arrive * 1000;
        search_berth[search_size].id = i;
    }
    sort(search_berth, search_berth + search_size + 1);
//    double mx = -1e9;
    pair<int, int> pos;
    pos = {-1, -1};
    for (int l = 0; l <= search_size; l++) {
        // if(pos.first!=-1) break;
        int search_id = search_berth[l].id;
//        int check_move = 0;
        for (int i = 1; i <= n; i++)
            for (int j = 1; j <= n; j++)
                vis[i][j] = 0;
        for (int &i: vis_search_berth) {
            i = 0;
        }
        priority_queue<Node, vector<Node>, greater<Node>> q;
        q.push({x, y, 0, pre_dis_berth[search_id][x][y]}); // 机器人初始位置
//        int ck = 0;
        vis[x][y] = 1;
        int re = 0;
//        int ck_berth = 0;
        while (!q.empty()) {
            re++;
            Node node = q.top();
            pair<int, int> now = {node.x, node.y};
            q.pop();
            int dis = node.gn;
            if (ch[now.first][now.second] == 'B' && vis_search_berth[flag_berth[now.first][now.second]] == 0 &&
                flag_berth[now.first][now.second] == search_id) {
                int id = flag_berth[now.first][now.second];
//                ck_berth = 1;
                vis_search_berth[id] = 1;
//                mx = 2;
                pos = now;
                break;
            }
            if (ch[now.first][now.second] == 'B')
                continue;
            for (int i = 0; i < 4; i++) {
                int nx = now.first + dx[i];
                int ny = now.second + dy[i];
                if (ch[nx][ny] == '*' || ch[nx][ny] == '#')
                    continue;
                if (nx < 1 || nx > n || ny < 1 || ny > n)
                    continue;
                if (vis[nx][ny] == 1)
                    continue;
                if ((vis_ban[nx][ny] == 1 && (abs(nx - x) + abs(ny - y)) < 3) || vis_ban[nx][ny] >= 2)
                    continue;
                if (nx == ban_boot[id][0] && ny == ban_boot[id][1])
                    continue;
                fa[nx][ny][0] = now.first;
                fa[nx][ny][1] = now.second;
                q.push({nx, ny, dis + 1.0, pre_dis_berth[search_id][nx][ny]});
                vis[nx][ny] = 1;
            }
        }
        if (pos.first != -1)
            break;
    }
    if (pos.first == -1) {
        vis_ban[x][y] = 1;
        return;
    }
    int temp_x = pos.first;
    int temp_y = pos.second;
    int h = 0;
//    int ck_x = temp_x;
//    int ck_y = temp_y;
    while (fa[temp_x][temp_y][0] != x || fa[temp_x][temp_y][1] != y) {
        h++;
        if (vis_ban[temp_x][temp_y] == 1) {
            for (int i = 0; i < berth_num; i++) {
                if (temp_x == berth[i].x && temp_y == berth[i].y) {
                    ban_boot[i][0] = temp_x;
                    ban_boot[i][1] = temp_y;
                    break;
                }
            }
            vis_ban[temp_x][temp_y]++;
        }
        int x1 = fa[temp_x][temp_y][0];
        int y1 = fa[temp_x][temp_y][1];
        if (fa[x1][y1][0] == x && fa[x1][y1][1] == y) {
            vis_ban[temp_x][temp_y] = 1;
        }
        temp_x = x1;
        temp_y = y1;
    }
    int di_x = temp_x - x;
    int di_y = temp_y - y;
    for (int i = 0; i < 4; i++) {
        if (di_x == dx[i] && di_y == dy[i]) {
            printf("move %d %d\n", id, i);
            vis_ban[temp_x][temp_y] = 1;
            break;
        }
    }
}

int main(int argc, char *argv[]) {
    Init();
    for (int zhen = 1; zhen <= 15000; zhen++) {
//		if (zhen == 200)
//		{
//			printf("move %d %d\n", 10, 1);
//			puts("OK");
//			fflush(stdout);
//			continue;
//		}
        for (int i = 0; i < robot_num; i++) {
            ban_boot[i][0] = ban_boot[i][1] = 0;
        }
        int id = Input(zhen);
        for (int i = 1; i <= n; i++)
            for (int t = 1; t <= n; t++) {
                if (zhen - good_time[i][t] == 1000) {
                    for (int j = 0; j < berth_num; j++) {
                        if (pre_dis_berth[j][i][t] <= 20) {
                            berth[j].near_good_num--;
                        }
                    }
                }
            }
//        for (int i = 0; i < robot_num; i++) {
//            robot_move[i] = -1;
//        }
        for (int i = 1; i <= n; i++) {
            for (int j = 1; j <= n; j++) {
                vis_good[i][j] = 0;
                vis_ban[i][j] = 0;
            }
        }

        for (int i = 0; i < robot_num; i++) {
            vis_ban[robot[i].x][robot[i].y] = 1;
            // 没拿着货物 而且在货物上
            if (robot[i].goods == 0) {
                if (good_time[robot[i].x][robot[i].y] > 0 && zhen - good_time[robot[i].x][robot[i].y] < 1000 &&
                    targer_good[i][0] == robot[i].x && targer_good[i][1] == robot[i].y) {
                    printf("get %d\n", i);
                    vis_ban[robot[i].x][robot[i].y] = 1;
                    for (int j = 0; j < berth_num; j++) {
                        // (x, y) 在 berth 附件的范围内
                        if (pre_dis_berth[j][robot[i].x][robot[i].y] <= 20) {
                            berth[j].near_good_num--;
                        }
                    }
                } // 拿着货物 而且在码头
            } else if (robot[i].goods == 1) {
                if (ch[robot[i].x][robot[i].y] == 'B') {
                    printf("pull %d\n", i);
                    vis_ban[robot[i].x][robot[i].y] = 1;
                    berth[flag_berth[robot[i].x][robot[i].y]].size++;
                }
            }
        }
        for (int i = 0; i < robot_num; i++) {
            if (robot[i].goods == 0) {
                if (good_time[robot[i].x][robot[i].y] > 0 && zhen - good_time[robot[i].x][robot[i].y] < 1000 &&
                    targer_good[i][0] == robot[i].x && targer_good[i][1] == robot[i].y) {
                    good_time[robot[i].x][robot[i].y] = 0;
                } else {
                    bfs_good(robot[i].x, robot[i].y, zhen, i);
                }
            } else if (robot[i].goods == 1) {
                if (ch[robot[i].x][robot[i].y] == 'B')
                    continue;
                else
                    bfs_berth(robot[i].x, robot[i].y, i);
            }
        }
        puts("OK");
        fflush(stdout);
    }
    return 0;
}
