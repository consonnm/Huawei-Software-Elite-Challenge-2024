#include <bits/stdc++.h>

#pragma GCC optimize(3)
using namespace std;
const int n = 200;
const int robot_num = 10;
const int berth_num = 10;
const int N = 210;

double w_good_val = 0.4863057189296891;
double w_good_dis = 0.8868439498723094;
double w_boat_speed = 0.024617235768745756;
double w_boat_size = 0.7832439756346595;
double w_boat_transport = 0.15525831625701295;
double w_berth_fill = 0.27078128246924754;
double w_berth_dis = 0.08038432245180017;
double w_berth_vis = 0.26606106612385333;
double w_near_dis = 0.8725311208246415;
double w_near_val = 0.006203446118222546;
double max_near_dis = 100;

int out;
int money, boat_capacity, id;
char ch[N][N];       // 地图
int good_time[N][N]; // 货物出现时间
int vis[N][N];       // 是否访问过
// 0 right 1 left 2 up 3 down
int dx[] = {0, 0, -1, 1};
int dy[] = {1, -1, 0, 0};
int fa[N][N][2];
int vis_good[N][N];       // 货物是否被锁定
int vis_ban[N][N];        // 该位置是否被锁定
int vis_berth[berth_num]; // 泊位是否被锁定
int flag_berth[N][N];     // 泊位ID
int good_value[N][N];
int vis_search_berth[10];
int last_robot_vis[20][2];
double pre_dis_berth[20][N][N]; // 预处理泊位到所有点的最短距离
int targer_good[20][2];
int vis_robot[N][N];
int search_size;
int ck_move;
int before[20][2];
int count_before[20];
int ship_run[20];
int max_dis;
int total_num;
int disapper_num;
int get_num;
int get_num_val;
std::ofstream logFile;

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

    double near_val;

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

struct SEARCH {
    int id;
    double x;

    friend bool operator<(SEARCH a, SEARCH b) {
        return a.x < b.x;
    }
} search_berth[10];

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
            if (nx < 1 || nx > n || ny < 1 || ny > n)
                continue;
            if (ch[nx][ny] == '*' || ch[nx][ny] == '#')
                continue;

            if (vis[nx][ny] == 1)
                continue;
            q.emplace(nx, ny);
            distance.push(dis + 1);
            vis[nx][ny] = 1;
        }
    }
}

int find_berth(int zhen, int id) {
    // 寻找最佳泊位
    double mx = -1e9;
    int pos = -1;
    for (int t = 0; t < berth_num; t++) {
        if (14980 - zhen - boat[id].num / berth[t].loading_speed - 2 * berth[t].transport_time < 0) {
            continue;
        }
        if (vis_berth[t] == 0) {
            // 计算总性价比
            double w;
            w = berth[t].size * 1.0 / boat_capacity * w_boat_size +
                berth[t].loading_speed / 5.0 * w_boat_speed -
                berth[t].transport_time / 1000.0 * w_boat_transport;
            if (w > mx) {
                mx = w;
                pos = t;
            }
        }
    }
    return pos;
}

int find_berth_2(int zhen, int id) {
    // 寻找最佳泊位
    double mx = -1e9;
    int pos = -1;
    for (int t = 0; t < berth_num; t++) {
        if (14980 - zhen - boat[id].num / berth[t].loading_speed - berth[t].transport_time - 500 < 0) {
            continue;
        }
        if (vis_berth[t] == 0) {
            // 计算总性价比
            double w;
            w = berth[t].size * 1.0 / boat_capacity * w_boat_size +
                berth[t].loading_speed / 5.0 * w_boat_speed -
                berth[t].transport_time / 1000.0 * w_boat_transport;

            if (w > mx) {
                mx = w;
                pos = t;
            }
        }
    }
    return pos;
}

void Init() {
    for (int i = 1; i <= n; i++)
        scanf("%s", ch[i] + 1); // 地图
    for (int i = 1; i <= n; i++)
        for (int t = 1; t <= n; t++)
            flag_berth[i][t] = -1;
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
    total_num += num;
    for (int i = 1; i <= num; i++) {
        int x, y, val;
        scanf("%d%d%d", &x, &y, &val); // 货物坐标和价值<100
        x++, y++;
        if (ch[x][y] != 'B' && ch[x][y] != '*' && ch[x][y] != '#') {
            good_time[x][y] = zhen;
            good_value[x][y] = val;

            for (int j = 0; j < berth_num; j++) {
                // (x, y) 在 berth 附件的范围内
                if (pre_dis_berth[j][x][y] <= max_near_dis) {
                    berth[j].near_val++;
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
            int pos = find_berth(zhen, i);
            if (pos == -1)
                continue;
            vis_berth[pos] = i + 1;
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
                // logFile << zhen << " go " << i << " " << boat[i].num << endl;
                boat[i].num = boat_capacity;
                vis_berth[boat[i].berth] = 0;
                boat[i].flag = 0;
                continue;
            }
            if (boat[i].num == 0) {
                printf("go %d\n", i);
                boat[i].num = boat_capacity;
                vis_berth[boat[i].berth] = 0;
                boat[i].flag = 0;
                continue;
            }
            if (berth[boat[i].berth].size == 0) {
                int pos = find_berth_2(zhen, i);
                if (pos != -1) {
                    printf("ship %d %d\n", i, pos);
                    vis_berth[boat[i].berth] = 0;

                    vis_berth[pos] = i + 1;
                    boat[i].berth = pos;
                    boat[i].flag = 1;
                    continue;
                }
            }
            int temp = min(berth[boat[i].berth].size, berth[boat[i].berth].loading_speed);
            temp = min(temp, boat[i].num);
            boat[i].num -= temp;
            berth[boat[i].berth].size -= temp;
        }
    }

    char okk[100];
    scanf("%s", okk);
    return id;
}

void bfs_noway(int x, int y, int zhen, int id) {
    for (int i = 0; i < 4; i++) {
        int nx = x + dx[i];
        int ny = y + dy[i];
        if (nx < 1 || nx > n || ny < 1 || ny > n)
            continue;
        if (vis_ban[nx][ny] >= 1)
            continue;
        if (ch[nx][ny] == '#' || ch[nx][ny] == '*') {
            continue;
        }
        if (ch[nx][ny] == 'A' || ch[nx][ny] == '.' || ch[nx][ny] == 'B') {
            printf("move %d %d\n", id, i);
            vis_ban[x][y]--;
            vis_ban[nx][ny]++;
        }
    }
}

void bfs_good(int x, int y, int zhen, int id) {
    for (int i = 1; i <= n; i++)
        for (int j = 1; j <= n; j++)
            vis[i][j] = 0;
    queue<pair<int, int>> q;
    queue<int> distance;
    distance.push(0);
    q.push({x, y});

    vis[x][y] = 1;
    double mx = -1e9;
    pair<int, int> pos;
    pos = {-1, -1};
    int re = 0;
    while (!q.empty()) {
        re++;
        if (pos.first != -1 && re > 4e3)
            break;
        pair<int, int> now = q.front();
        q.pop();
        int dis = distance.front();
        distance.pop();
        if (good_time[now.first][now.second] > 0 && zhen - good_time[now.first][now.second] < 1000 &&
            vis_good[now.first][now.second] == 0) {
            double w;
            if (zhen >= out) {
                w = good_value[now.first][now.second] / 200.0 * (w_good_val + 1) - dis / 400.0 * w_good_dis;
            } else {
                w = good_value[now.first][now.second] / 200.0 * w_good_val - dis / 400.0 * w_good_dis;
            }

            if (w > mx) {
                mx = w;
                pos = now;
            }
        }
        for (int i = 0; i < 4; i++) {
            int nx = now.first + dx[i];
            int ny = now.second + dy[i];
            if (nx < 1 || nx > n || ny < 1 || ny > n)
                continue;
            if (ch[nx][ny] == '*' || ch[nx][ny] == '#')
                continue;

            if (vis[nx][ny] == 1)
                continue;
            if ((vis_ban[nx][ny] >= 1 && dis == 0) || vis_ban[nx][ny] >= 2)
                continue;
            if (vis_robot[nx][ny] > 0 && (vis_robot[nx][ny] - 1) < id && dis <= 2) {
                continue;
            }
            fa[nx][ny][0] = now.first;
            fa[nx][ny][1] = now.second;
            q.push({nx, ny});
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
    pair<int, int> temp;
    int ck = 0;
    int cycle = 0;
    while (fa[temp_x][temp_y][0] != x || fa[temp_x][temp_y][1] != y) {
        cycle++;
        if (cycle > 10000)
            exit(0);
        ck++;
        if (vis_robot[temp_x][temp_y] > 0) {
            temp = {temp_x, temp_y};
            ck = 0;
        }
        int x1 = fa[temp_x][temp_y][0];
        int y1 = fa[temp_x][temp_y][1];
        temp_x = x1;
        temp_y = y1;
    }
    if (temp_x == before[id][0] && temp_y == before[id][1]) {
        if (count_before[id] < 2) {
            count_before[id]++;
        } else {
            count_before[id] = 0;
            int l = rand() % 4;
            for (int i = l; i < l + 4; i++) {
                int nx = x + dx[i % 4];
                int ny = y + dy[i % 4];
                if (nx < 1 || nx > n || ny < 1 || ny > n)
                    continue;
                if (ch[nx][ny] == '*' || ch[nx][ny] == '#' || vis_ban[nx][ny] >= 1)
                    continue;
                if (nx == before[id][0] && ny == before[id][1])
                    continue;
                printf("move %d %d\n", id, i % 4);
                vis_ban[x][y]--;
                vis_ban[nx][ny]++;
                ck_move = 1;
                before[id][0] = x;
                before[id][1] = y;
                return;
            }
        }
    } else
        count_before[id] = 0;
    before[id][0] = x;
    before[id][1] = y;
    int di_x = temp_x - x;
    int di_y = temp_y - y;
    for (int i = 0; i < 4; i++) {
        if (di_x == dx[i] && di_y == dy[i]) {
            printf("move %d %d\n", id, i);
            vis_ban[x][y]--;
            vis_ban[temp_x][temp_y] += 1;
            ck_move = 1;
            if (ck == 0 || ck == 1) {
                vis_ban[temp.first][temp.second] += 10;
            }
            break;
        }
    }
}

void bfs_berth(int x, int y, int zhen, int id) {
    search_size = -1;
    double mx_pre_dis = -1e9, mx_near_val = -1e9;
    for (int i = 0; i < berth_num; i++) {
        mx_pre_dis = max(mx_pre_dis, 1.0 * pre_dis_berth[i][x][y]);
        mx_near_val = max(mx_near_val, 1.0 * berth[i].near_val);
    }
    for (int i = 0; i < berth_num; i++) {
        if (abs(1e9 - pre_dis_berth[i][x][y]) < 0.0001)
            continue;
        if (zhen >= out) {
            if (vis_berth[i] == 0)
                continue;
            if (berth[i].size > boat_capacity)
                continue;
            if (berth[i].size > boat[(vis_berth[i] - 1)].num)
                continue;
            search_berth[++search_size].x =
                    pre_dis_berth[i][x][y] * w_berth_dis - berth[i].near_val / (mx_near_val + 1) * w_near_val;
            search_berth[search_size].id = i;
        } else {
            search_berth[++search_size].x = -(berth[i].size < boat_capacity) * 100000.0 +
                                            pre_dis_berth[i][x][y] / mx_pre_dis * w_berth_dis -
                                            berth[i].near_val / (mx_near_val + 1) * w_near_val;
            search_berth[search_size].id = i;
        }
    }
    sort(search_berth, search_berth + search_size + 1);
    // for(int i=0;i<=search_size;i++){
    //     // 将消息写入文件
    //     logFile << zhen << " " << id << " " << search_berth[i].id << " " << search_berth[i].x <<endl;
    // }
    pair<int, int> pos;
    pos = {-1, -1};
    for (int l = 0; l <= search_size; l++) {

        int search_id = search_berth[l].id;
        for (int i = 1; i <= n; i++)
            for (int j = 1; j <= n; j++)
                vis[i][j] = 0;
        priority_queue<Node, vector<Node>, greater<Node>> q;
        q.push({x, y, 0, pre_dis_berth[search_id][x][y]}); // 机器人初始位置
        if (flag_berth[x][y] == search_id) {
            ck_move = 1;
            return;
        }
        vis[x][y] = 1;
        while (!q.empty()) {

            Node node = q.top();
            pair<int, int> now = {node.x, node.y};
            q.pop();
            int dis = node.gn;
            if (dis > 100) {
                pos = now;

                break;
            }
            if (ch[now.first][now.second] == 'B' &&
                flag_berth[now.first][now.second] == search_id) {

                pos = now;
                break;
            }
            if (ch[now.first][now.second] == 'B') {

                continue;
            }

            for (int i = 0; i < 4; i++) {
                int nx = now.first + dx[i];
                int ny = now.second + dy[i];
                if (nx < 1 || nx > n || ny < 1 || ny > n) {
                    continue;
                }
                if (ch[nx][ny] == '*' || ch[nx][ny] == '#') {
                    continue;
                }

                if (vis[nx][ny] == 1) {
                    continue;
                }

                if ((vis_ban[nx][ny] >= 1 && dis == 0) || vis_ban[nx][ny] >= 2) {
                    continue;
                }
                if (vis_robot[nx][ny] > 0 && (vis_robot[nx][ny] - 1) < id && dis <= 2) {
                    continue;
                }
                fa[nx][ny][0] = now.first;
                fa[nx][ny][1] = now.second;
                q.push({nx, ny, dis + 1.0, pre_dis_berth[search_id][nx][ny]});
                vis[nx][ny] = 1;
            }
        }
        if (pos.first != -1)
            break;
    }
    if (pos.first == -1)
        return;
    int temp_x = pos.first;
    int temp_y = pos.second;
    pair<int, int> temp;
    int ck = 0;
    int cycle = 0;
    while (fa[temp_x][temp_y][0] != x || fa[temp_x][temp_y][1] != y) {
        cycle++;
        if (cycle > 10000)
            exit(0);
        ck++;
        if (vis_robot[temp_x][temp_y] > 0) {
            ck = 0;
            temp = {temp_x, temp_y};
        }
        int x1 = fa[temp_x][temp_y][0];
        int y1 = fa[temp_x][temp_y][1];
        temp_x = x1;
        temp_y = y1;
    }

    if (temp_x == before[id][0] && temp_y == before[id][1]) {
        if (count_before[id] < 2) {
            count_before[id]++;
        } else {
            count_before[id] = 0;
            int l = rand() % 4;
            for (int i = l; i < l + 4; i++) {
                int nx = x + dx[i % 4];
                int ny = y + dy[i % 4];
                if (nx < 1 || nx > n || ny < 1 || ny > n)
                    continue;
                if (ch[nx][ny] == '*' || ch[nx][ny] == '#' || vis_ban[nx][ny] >= 1)
                    continue;
                if (nx == before[id][0] && ny == before[id][1])
                    continue;
                printf("move %d %d\n", id, i % 4);
                vis_ban[x][y]--;
                vis_ban[nx][ny]++;
                ck_move = 1;
                before[id][0] = x;
                before[id][1] = y;
                return;
            }
        }
    } else
        count_before[id] = 0;
    before[id][0] = x;
    before[id][1] = y;
    int di_x = temp_x - x;
    int di_y = temp_y - y;
    for (int i = 0; i < 4; i++) {
        if (di_x == dx[i] && di_y == dy[i]) {
            printf("move %d %d\n", id, i);
            vis_ban[x][y]--;
            ck_move = 1;
            vis_ban[temp_x][temp_y] += 1;
            if (ck == 0 || ck == 1) {
                vis_ban[temp.first][temp.second] += 10;
            }

            break;
        }
    }
}

int main(int argc, char *argv[]) {
    // out=std::atof(argv[1]);
    out = 12500;
    //    w_good_val = std::atof(argv[1]);
    //    w_good_dis = std::atof(argv[2]);
    //    w_boat_speed = std::atof(argv[3]);
    //    w_boat_size = std::atof(argv[4]);
    //    w_boat_transport = std::atof(argv[5]);
    //    w_berth_fill = std::atof(argv[6]);
    //    w_berth_dis = std::atof(argv[7]);
    //    w_berth_vis = std::atof(argv[8]);
    //    w_near_dis = std::atof(argv[9]);
    //    w_near_val = std::atof(argv[10]);

    // 打开文件，如果文件不存在则创建它
    //    logFile.open("log.txt");

    max_near_dis = max_near_dis * 1.0 * w_near_dis;
    Init();
    for (int zhen = 1; zhen <= 15000; zhen++) {
        // if (zhen == 2000)
        // {
        //     printf("move %d %d\n", 10, 1);
        //     puts("OK");
        //     fflush(stdout);
        //     continue;
        // }
        int id = Input(zhen);
        //        if (zhen >= 14995) {
        //            logFile << "total_num: " << total_num << " disapper_num: " << disapper_num << " get_num: " << get_num
        //                    << " get_num_val: " << get_num_val << endl;
        //        }
        //        if (zhen >= 14995) {
        //            for (int i = 0; i < berth_num; i++) {
        //                logFile << zhen << " " << i << " " << berth[i].size << endl;
        //            }
        //        }
        //        if (zhen == 1) {
        //            for (int i = 0; i < berth_num; i++) {
        //                logFile << i << " " << berth[i].x << " " << berth[i].y << " " << berth[i].transport_time << " "
        //                        << berth[i].loading_speed << " " << berth[i].size << " " << berth[i].load << " "
        //                        << berth[i].arrive << " " << berth[i].near_val << endl;
        //            }
        //        }
        for (int i = 1; i <= n; i++) {
            for (int j = 1; j <= n; j++) {
                vis_good[i][j] = 0;
                vis_ban[i][j] = 0;
                vis_robot[i][j] = 0;
                if (zhen - good_time[i][j] == 1000 && good_time[i][j] > 0)
                    disapper_num++;
                for (int k = 0; k < berth_num; ++k) {
                    if (pre_dis_berth[k][i][j] <= max_near_dis && zhen - good_time[i][j] == 1000 &&
                        good_time[i][j] > 0) {
                        berth[k].near_val--;
                    }
                }
            }
        }

        for (int i = 0; i < robot_num; i++) {
            vis_ban[robot[i].x][robot[i].y] = 1;
            vis_robot[robot[i].x][robot[i].y] = i + 1;
            // 没拿着货物 而且在货物上
            if (robot[i].goods == 0) {
                if (good_time[robot[i].x][robot[i].y] > 0 && zhen - good_time[robot[i].x][robot[i].y] < 1000 &&
                    targer_good[i][0] == robot[i].x && targer_good[i][1] == robot[i].y) {
                    printf("get %d\n", i);
                    get_num++;
                    get_num_val += good_value[robot[i].x][robot[i].y];
                    for (int j = 0; j < berth_num; j++) {
                        // (x, y) 在 berth 附件的范围内
                        if (pre_dis_berth[j][robot[i].x][robot[i].y] <= max_near_dis) {
                            berth[j].near_val--;
                        }
                    }
                } // 拿着货物 而且在码头
            } else if (robot[i].goods == 1) {
                if (ch[robot[i].x][robot[i].y] == 'B') {
                    printf("pull %d\n", i);
                    berth[flag_berth[robot[i].x][robot[i].y]].size++;
                }
            }
        }
        for (int i = 0; i < robot_num; i++) {
            ck_move = 0;
            if (robot[i].goods == 0) {
                if (good_time[robot[i].x][robot[i].y] > 0 && zhen - good_time[robot[i].x][robot[i].y] < 1000 &&
                    targer_good[i][0] == robot[i].x && targer_good[i][1] == robot[i].y) {
                    good_time[robot[i].x][robot[i].y] = 0;
                    ck_move = 1;
                } else {
                    if (zhen <= 50) {
                        bfs_berth(robot[i].x, robot[i].y, zhen, i);
                        if (ck_move == 0) {
                            bfs_good(robot[i].x, robot[i].y, zhen, i);
                        }
                    } else
                        bfs_good(robot[i].x, robot[i].y, zhen, i);
                }
            } else if (robot[i].goods == 1) {
                if (ch[robot[i].x][robot[i].y] == 'B') {
                    ck_move = 1;
                } else {
                    bfs_berth(robot[i].x, robot[i].y, zhen, i);
                }
            }
            if (ck_move == 0) {
                // printf("move %d %d\n", 10+i, 1);
                bfs_noway(robot[i].x, robot[i].y, zhen, i);
            } else {
                last_robot_vis[i][0] = -1;
                last_robot_vis[i][1] = -1;
            }
        }

        puts("OK");
        fflush(stdout);
    }
    return 0;
}
