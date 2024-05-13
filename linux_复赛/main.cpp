#include <bits/stdc++.h>
using namespace std;
// ‘.’ ： 空地
// ‘>’ ： 陆地主干道
// ‘*’ ： 海洋
// ‘~’ ： 海洋主航道
// ‘#’ ： 障碍
// ‘R’ ： 机器人购买地块，同时该地块也是主干道
// ‘S’ ： 船舶购买地块，同时该地块也是主航道
// ‘B’ ： 泊位
// ‘K’ ： 靠泊区
// ‘C’ ： 海陆立体交通地块
// ‘c’ ： 海陆立体交通地块，同时为主干道和主航道
// ‘T’ ： 交货点
const int N = 210;
const int n = 200;
int robot_num = 0;
int boat_num = 0;
int berth_num = 0;
int goods_num = 0;
int frame_id = 0;
int money, boat_capacity, boat_price = 8000, robot_price = 2000;
//参数
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
int out = 12500;
char ch[N][N]; // 地图
char ck_out[N][N];
int good_time[N][N]; // 货物出现时间
int vis[N][N];       // 是否访问过
int vis_rot[N][N][4];       // 是否访问过
// 0 right 1 left 2 up 3 down
int dx[] = {0, 0, -1, 1};
int dy[] = {1, -1, 0, 0};
int dx_rot[4][2] = {{0,1}, {0, -1}, {-2, -1}, {2, 1}};
int dy_rot[4][2] = {{2,1}, {-2, -1}, {0, 1}, {0, -1}};
int now_rot[4][2] = {{3, 2}, {2, 3}, {0, 1}, {1, 0}};

int fa[N][N][2];
int fa_rot[N][N][4][3];
int vis_good[N][N];   // 货物是否被锁定
int vis_ban[N][N];    // 该位置是否被锁定
int vis_berth[20];    // 泊位是否被锁定
int flag_berth[N][N]; // 泊位ID
int good_value[N][N];
int last_robot_vis[20][2];
int pre_dis_berth[20][N][N]; // 预处理泊位到所有点的最短距离
int pre_dis_boat[20][N][N];  // 预处理泊位到所有点的最短距离
int pre_dis_comit[20][N][N]; // 预处理交货点到所有点的最短距离

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
vector<pair<int, int>> robot_purchase_point;//机器人购买点
vector<pair<int, int>> boat_purchase_point;//船购买点
vector<pair<int, int>> delivery_point;//交货点
map<pair<int,int>,int> mp_T;//坐标转交货点id
std::ofstream logFile;
struct Robot
{
    int id, x, y;
    int status;
} robot[20];

struct Berth
{
    int x, y;
    int loading_speed;
    int transport_time;
    double near_val;
    int near_T_dis;
    int near_T;
    int size;
    Berth() {}
    Berth(int x, int y, int loading_speed)
    {
        this->x = x;
        this->y = y;
        this->loading_speed = loading_speed;
    }
} berth[10];

struct Boat
{
    int id, x, y, dir;
    int goods_num, status;
    int flag;
    int berth;
    Boat() {}
    Boat(int startX, int startY)
    {
        x = startX;
        y = startY;
    }
} boat[10];

struct Node
{
    int x, y;
    int gn, hn;

    bool operator>(const Node &W) const
    {
        return gn + hn > W.gn + W.hn;
    }
};

struct SEARCH
{
    int id;
    double x;

    friend bool operator<(SEARCH a, SEARCH b)
    {
        return a.x < b.x;
    }
} search_berth[20];

struct NodeBoat
{
    int x, y;
    int gn, hn;
    int dir;
    bool operator>(const NodeBoat &W) const
    {
        return gn + hn > W.gn + W.hn;
    }
};

void ProcessMap()
{
    for (int i = 1; i <= n; i++)
    {
        for (int j = 1; j <= n; j++)
        {
            if (ch[i][j] == 'R')
                robot_purchase_point.push_back(make_pair(i, j));
            else if (ch[i][j] == 'S')
                boat_purchase_point.push_back(make_pair(i, j));
            else if (ch[i][j] == 'T'){
                mp_T[make_pair(i, j)]=delivery_point.size();
                delivery_point.push_back(make_pair(i, j));
            }
                
            
        }
    }
}

int find_berth(int frame_id, int id)
{
    // 寻找最佳泊位
    double mx = -1e9;
    int pos = -1;
    for (int t = 0; t < berth_num; t++)
    {
        // if (14980 - frame_id - boat[id].goods_num / berth[t].loading_speed - 2 * berth[t].transport_time < 0)
        // {
        //     continue;
        // }
        if (vis_berth[t] == 0)
        {
            // 计算总性价比
            double w;
            w = berth[t].size * 1.0 / boat_capacity * w_boat_size +
                berth[t].loading_speed / 5.0 * w_boat_speed;
            if (w > mx)
            {
                mx = w;
                pos = t;
            }
        }
    }
    return pos;
}

int find_berth_2(int zhen, int id)
{
    // 寻找最佳泊位
    double mx = -1e9;
    int pos = -1;
    for (int t = 0; t < berth_num; t++)
    {
        if (14980 - zhen - boat[id].goods_num / berth[t].loading_speed - berth[t].transport_time - 500 < 0)
        {
            continue;
        }
        if (vis_berth[t] == 0)
        {
            // 计算总性价比
            double w;
            w = berth[t].size * 1.0 / boat_capacity * w_boat_size +
                berth[t].loading_speed / 5.0 * w_boat_speed -
                berth[t].transport_time / 1000.0 * w_boat_transport;

            if (w > mx)
            {
                mx = w;
                pos = t;
            }
        }
    }
    return pos;
}

void bfs_dis(int x, int y, int id)
{
    for (int i = 1; i <= n; i++)
        for (int j = 1; j <= n; j++)
            vis[i][j] = 0, pre_dis_berth[id][i][j] = 1e9;
    queue<pair<int, int>> q;
    queue<int> distance;
    q.emplace(x, y);
    distance.push(0);
    vis[x][y] = 1;
    while (!q.empty())
    {
        pair<int, int> now = q.front();
        q.pop();
        int dis = distance.front();
        distance.pop();
        if (ch[now.first][now.second] == '.' || ch[now.first][now.second] == '>' || ch[now.first][now.second] == 'B' || ch[now.first][now.second] == 'R' || ch[now.first][now.second] == 'C' || ch[now.first][now.second] == 'c')
        {
            pre_dis_berth[id][now.first][now.second] = dis;
        }
        for (int i = 0; i < 4; i++)
        {
            int nx = now.first + dx[i];
            int ny = now.second + dy[i];
            if (nx < 1 || nx > n || ny < 1 || ny > n)
                continue;
            if (ch[nx][ny] == '*' || ch[nx][ny] == '#' || ch[nx][ny] == 'K' || ch[nx][ny] == 'k' || ch[nx][ny] == '~')
                continue;
            if (vis[nx][ny] == 1)
                continue;
            q.emplace(nx, ny);
            distance.push(dis + 1);
            vis[nx][ny] = 1;
        }
    }
}

void bfs_dis_boat(int x, int y, int id)
{
    for (int i = 1; i <= n; i++)
        for (int j = 1; j <= n; j++)
            vis[i][j] = 0, pre_dis_boat[id][i][j] = 1e9;
    queue<pair<int, int>> q;
    queue<int> distance;
    q.emplace(x, y);
    distance.push(0);
    vis[x][y] = 1;
    while (!q.empty())
    {
        pair<int, int> now = q.front();
        q.pop();
        int dis = distance.front();
        distance.pop();
        if(ch[now.first][now.second] =='T'){
            if(berth[id].near_T_dis > dis){
                berth[id].near_T_dis = dis;
                berth[id].near_T = mp_T[now];
            }
        }
        if (ch[now.first][now.second] == '*' || ch[now.first][now.second] == 'B' || ch[now.first][now.second] == 'S' || ch[now.first][now.second] == 'K' || ch[now.first][now.second] == 'T' || ch[now.first][now.second] == '~' || ch[now.first][now.second] == 'C' || ch[now.first][now.second] == 'c')
        {
            pre_dis_boat[id][now.first][now.second] = dis;
        }
        for (int i = 0; i < 4; i++)
        {
            int nx = now.first + dx[i];
            int ny = now.second + dy[i];
            if (nx < 1 || nx > n || ny < 1 || ny > n)
                continue;
            if (ch[nx][ny] == '.' || ch[nx][ny] == '#' || ch[nx][ny] == '>' || ch[nx][ny] == 'R')
                continue;
            if (vis[nx][ny] == 1)
                continue;
            q.emplace(nx, ny);
            distance.push(dis + 1);
            vis[nx][ny] = 1;
        }
    }
}

void bfs_dis_comit(int x, int y, int id)
{
    for (int i = 1; i <= n; i++)
        for (int j = 1; j <= n; j++)
            vis[i][j] = 0, pre_dis_comit[id][i][j] = 1e9;

    queue<pair<int, int>> q;
    queue<int> distance;
    q.emplace(x, y);
    distance.push(0);
    vis[x][y] = 1;
    while (!q.empty())
    {
        pair<int, int> now = q.front();
        q.pop();
        int dis = distance.front();
        distance.pop();
         if (ch[now.first][now.second] == '*' || ch[now.first][now.second] == 'B' || ch[now.first][now.second] == 'S' || ch[now.first][now.second] == 'K' || ch[now.first][now.second] == 'T' || ch[now.first][now.second] == '~' || ch[now.first][now.second] == 'C' || ch[now.first][now.second] == 'c')
        {
            pre_dis_comit[id][now.first][now.second] = dis;
        }
        for (int i = 0; i < 4; i++)
        {
            int nx = now.first + dx[i];
            int ny = now.second + dy[i];
            if (nx < 1 || nx > n || ny < 1 || ny > n)
                continue;
            if (ch[nx][ny] == '.' || ch[nx][ny] == '#' || ch[nx][ny] == '>' || ch[nx][ny] == 'R')
                continue;
            if (vis[nx][ny] == 1)
                continue;
            q.emplace(nx, ny);
            distance.push(dis + 1);
            vis[nx][ny] = 1;
        }
    }
}

void ban_boat(int x, int y, int dir, int k)
{
    int flag;
    if (dir == 0 || dir == 1)
    {
        if (dir == 0)
            flag = 1;
        else
            flag = -1;
        for (int i = 0; i < 2; i++)
            for (int t = 0; t < 3; t++)
            {
                if(ch[x + i * flag][y + t * flag] == 'c' || ch[x + i * flag][y + t * flag] == '~'||ch[x + i * flag][y + t * flag] == 'S')
                    continue;
                vis_ban[x + i * flag][y + t * flag] += k;
            }
    }
    else
    {
        if (dir == 3)
            flag = 1;
        else
            flag = -1;
        for (int i = 0; i < 3; i++)
            for (int t = 0; t < 2; t++)
            {
                if(ch[x + i * flag][y + t * flag] == 'c' || ch[x + i * flag][y + t * flag] == '~'||ch[x + i * flag][y + t * flag] == 'S')
                    continue;
                vis_ban[x + i * flag][y + t * -flag] += k;
            }
    }
}

int check_ban_boat(int x, int y, int dir, int k)
{
    int flag;
    if (dir == 0 || dir == 1)
    {
        if (dir == 0)
            flag = 1;
        else
            flag = -1;
        for (int i = 0; i < 2; i++)
            for (int t = 0; t < 3; t++)
            {
                if(ch[x + i * flag][y + t * flag] == 'c' || ch[x + i * flag][y + t * flag] == '~'||ch[x + i * flag][y + t * flag] == 'S')
                    continue;
                if (vis_ban[x + i * flag][y + t * flag] >= k)
                    return 1;
            }
    }
    else
    {
        if (dir == 3)
            flag = 1;
        else
            flag = -1;
        for (int i = 0; i < 3; i++)
            for (int t = 0; t < 2; t++)
            {
                if(ch[x + i * flag][y + t * flag] == 'c' || ch[x + i * flag][y + t * flag] == '~'||ch[x + i * flag][y + t * flag] == 'S')
                    continue;
                if (vis_ban[x + i * flag][y + t * -flag] >= k)
                    return 1;
            }
    }
    return 0;
}

int check_ban_boat_out(int x, int y, int dir)
{
    int flag;
    if (dir == 0 || dir == 1)
    {
        if (dir == 0)
            flag = 1;
        else
            flag = -1;
        for (int i = 0; i < 2; i++)
            for (int t = 0; t < 3; t++)
            {
                if(x+i*flag>n||y+t*flag>n||x+i*flag<1||y+t*flag<1)
                    return 1;
            }
    }
    else
    {
        if (dir == 3)
            flag = 1;
        else
            flag = -1;
        for (int i = 0; i < 3; i++)
            for (int t = 0; t < 2; t++)
            {
                if(x + i * flag>n||y + t * -flag>n||x + i * flag<1||y + t * -flag<1)
                    return 1;
            }
    }
    return 0;
}

int check_ch_boat(int x, int y, int dir, char k)
{
    int flag;
    if (dir == 0 || dir == 1)
    {
        if (dir == 0)
            flag = 1;
        else
            flag = -1;
        for (int i = 0; i < 2; i++)
            for (int t = 0; t < 3; t++)
            {
                if (ch[x + i * flag][y + t * flag] == k)
                    return 1;
            }
    }
    else
    {
        if (dir == 3)
            flag = 1;
        else
            flag = -1;
        for (int i = 0; i < 3; i++)
            for (int t = 0; t < 2; t++)
            {
                if (ch[x + i * flag][y + t * -flag] == k)
                    return 1;
            }
    }
    return 0;
}

int check_move_boat(int nx, int ny, int i, int dis, int id)
{
    if (check_ban_boat_out(nx, ny, i)==1)
        return 1;
    if (check_ch_boat(nx, ny, i, '.') || check_ch_boat(nx, ny, i, '#') || check_ch_boat(nx, ny, i, '>') || check_ch_boat(nx, ny, i, 'S'))
        return 1;
    if (vis_rot[nx][ny][i] == 1)
        return 1;
    // if ((check_ban_boat(nx, ny, i, 1) && dis == 0) || check_ban_boat(nx, ny, i, 2))
    //     return 1;
    if (check_ban_boat(nx, ny, i, 1))
        return 1;
    // if ((vis_ban[nx][ny] >= 1 && dis == 0) || vis_ban[nx][ny] >= 2)
    //     continue;
    // if (vis_robot[nx][ny] > 0 && (vis_robot[nx][ny] - 1) < id && dis <= 2)
    //     return 1;
    return 0;
}

void bfs_berth_boat(int x, int y, int frame_id, int id, int berth_id, int dir)
{
    for (int i = 1; i <= n; i++)
        for (int j = 1; j <= n; j++)
            for(int l=0;l<4;l++)
                vis_rot[i][j][l] = 0;
    priority_queue<NodeBoat, vector<NodeBoat>, greater<NodeBoat>> q;
    // logFile << x << " " << y << endl;
    q.push({x, y, 0, pre_dis_boat[berth_id][x][y], dir}); // 核心点初始位置
    pair<int, int> pos;
    pos = {-1, -1};
    int pos_dir;
    vis[x][y] = 1;
    int re = 0;
    while (!q.empty())
    {
        re++;
        NodeBoat node = q.top();
        pair<int, int> now = {node.x, node.y};
        q.pop();
        int dis = node.gn;
        int dir_now = node.dir;
        if (dis > 30)
        {
            pos = now;
            pos_dir = dir_now;
            break;
        }
        if (ch[now.first][now.second] == 'K'&&flag_berth[now.first][now.second] == berth_id)
        {
            pos = now;
            pos_dir = dir_now;
            break;
        }
        // 前进
        int nx = now.first + dx[dir_now];
        int ny = now.second + dy[dir_now];
        if (check_move_boat(nx, ny, dir_now, dis, id) == 0)
        {
            fa_rot[nx][ny][dir_now][0] = now.first;
            fa_rot[nx][ny][dir_now][1] = now.second;
            fa_rot[nx][ny][dir_now][2] = dir_now;
            int next_dis = dis + 1;
            if (ch[nx][ny] == '~' || ch[nx][ny] == 'c')
            {
                next_dis += 1;
            }
            q.push({nx, ny, next_dis, pre_dis_boat[berth_id][nx][ny], dir_now});
            vis_rot[nx][ny][dir_now] = 1;
        }
        // 旋转
        for (int i = 0; i < 2; i++)
        {
            int nx = now.first + dx_rot[dir_now][i];
            int ny = now.second + dy_rot[dir_now][i];
            if (check_move_boat(nx, ny, now_rot[dir_now][i], dis, id) == 0)
            {
                fa_rot[nx][ny][now_rot[dir_now][i]][0] = now.first;
                fa_rot[nx][ny][now_rot[dir_now][i]][1] = now.second;
                fa_rot[nx][ny][now_rot[dir_now][i]][2] = dir_now;
                int next_dis = dis + 1;
                if (ch[nx][ny] == '~' || ch[nx][ny] == 'c')
                {
                    next_dis += 1;
                }
                q.push({nx, ny, next_dis, pre_dis_boat[berth_id][nx][ny], now_rot[dir_now][i]});
                vis_rot[nx][ny][now_rot[dir_now][i]] = 1;
            }
        }
    }
    if (pos.first == -1)
    {
        return;
    }
    int temp_x = pos.first;
    int temp_y = pos.second;
    int temp_dir = pos_dir;
    int ck = 0;
    int bk = 0;
    pair<int, int> temp;
    
    while (fa_rot[temp_x][temp_y][temp_dir][0] != x || fa_rot[temp_x][temp_y][temp_dir][1] != y)
    {
        int x1 = fa_rot[temp_x][temp_y][temp_dir][0];
        int y1 = fa_rot[temp_x][temp_y][temp_dir][1];
        temp_dir = fa_rot[temp_x][temp_y][temp_dir][2];
        temp_x = x1;
        temp_y = y1;
    }

    
    ;
    int di_x = temp_x - x;
    int di_y = temp_y - y;
    if (abs(di_x) + abs(di_y) == 1)
    {
        printf("ship %d\n", id);
        ban_boat(x, y, dir, -1);
        ban_boat(temp_x, temp_y, dir, 1);
    }
    else
    {
        for (int i = 0; i < 2; i++)
        {
            int nx = x + dx_rot[dir][i];
            int ny = y + dy_rot[dir][i];
            if (nx == temp_x && ny == temp_y)
            {
                printf("rot %d %d\n", id, i);
                ban_boat(x, y, dir, -1);
                ban_boat(temp_x, temp_y, now_rot[dir][i], 1);
                break;
            }
        }
    }
}

void bfs_comit_boat(int x, int y, int frame_id, int id, int comit_id, int dir)
{
    for (int i = 1; i <= n; i++)
        for (int j = 1; j <= n; j++)
            for(int l=0;l<4;l++)
                vis_rot[i][j][l] = 0;
    priority_queue<NodeBoat, vector<NodeBoat>, greater<NodeBoat>> q;
    // logFile << x << " " << y << endl;
    q.push({x, y, 0, pre_dis_comit[comit_id][x][y], dir}); // 核心点初始位置
    pair<int, int> pos;
    pos = {-1, -1};
    int pos_dir;
    vis[x][y] = 1;
    int re = 0;
    while (!q.empty())
    {
        re++;
        NodeBoat node = q.top();
        pair<int, int> now = {node.x, node.y};
        q.pop();
        int dis = node.gn;
        int dir_now = node.dir;
        if (dis > 30)
        {
            pos = now;
            pos_dir = dir_now;
            break;
        }
        if (ch[now.first][now.second] == 'T')
        {
            pos = now;
            pos_dir = dir_now;
            break;
        }
        // 前进
        int nx = now.first + dx[dir_now];
        int ny = now.second + dy[dir_now];
        if (check_move_boat(nx, ny, dir_now, dis, id) == 0)
        {
            fa_rot[nx][ny][dir_now][0] = now.first;
            fa_rot[nx][ny][dir_now][1] = now.second;
            fa_rot[nx][ny][dir_now][2] = dir_now;
            int next_dis = dis + 1;
            if (ch[nx][ny] == '~' || ch[nx][ny] == 'c')
            {
                next_dis += 1;
            }
            q.push({nx, ny, next_dis, pre_dis_comit[comit_id][nx][ny], dir_now});
            vis_rot[nx][ny][dir_now] = 1;
        }
        // 旋转
        for (int i = 0; i < 2; i++)
        {
            int nx = now.first + dx_rot[dir_now][i];
            int ny = now.second + dy_rot[dir_now][i];
            if (check_move_boat(nx, ny, now_rot[dir_now][i], dis, id) == 0)
            {
                fa_rot[nx][ny][now_rot[dir_now][i]][0] = now.first;
                fa_rot[nx][ny][now_rot[dir_now][i]][1] = now.second;
                fa_rot[nx][ny][now_rot[dir_now][i]][2] = dir_now;
                int next_dis = dis + 1;
                if (ch[nx][ny] == '~' || ch[nx][ny] == 'c')
                {
                    next_dis += 1;
                }
                q.push({nx, ny, next_dis, pre_dis_comit[comit_id][nx][ny], now_rot[dir_now][i]});
                vis_rot[nx][ny][now_rot[dir_now][i]] = 1;
            }
        }
    }
    if (pos.first == -1)
    {
        return;
    }
    int temp_x = pos.first;
    int temp_y = pos.second;
    int temp_dir = pos_dir;
    int ck = 0;
    int bk = 0;
    pair<int, int> temp;
    while (fa_rot[temp_x][temp_y][temp_dir][0] != x || fa_rot[temp_x][temp_y][temp_dir][1] != y)
    {
        int x1 = fa_rot[temp_x][temp_y][temp_dir][0];
        int y1 = fa_rot[temp_x][temp_y][temp_dir][1];
        temp_dir = fa_rot[temp_x][temp_y][temp_dir][2];
        temp_x = x1;
        temp_y = y1;
    }
    int di_x = temp_x - x;
    int di_y = temp_y - y;
    if (abs(di_x) + abs(di_y) == 1)
    {
        logFile<<x<<" "<<y<<" "<<dir<<endl;
        printf("ship %d\n", id);
        ban_boat(x, y, dir, -1);
        ban_boat(temp_x, temp_y, dir, 1);
    }
    else
    {
        for (int i = 0; i < 2; i++)
        {
            int nx = x + dx_rot[dir][i];
            int ny = y + dy_rot[dir][i];
            if (nx == temp_x && ny == temp_y)
            {
                printf("rot %d %d\n", id, i);
                ban_boat(x, y, dir, -1);
                ban_boat(temp_x, temp_y, now_rot[dir][i], 1);
                break;
            }
        }
    }
}

void Init()
{
    for (int i = 1; i <= n; i++)
        for (int t = 1; t <= n; t++)
            flag_berth[i][t] = -1;
    for (int i = 1; i <= n; i++)
        scanf("%s", ch[i] + 1); // 地图
    for (int i = 1; i <= n; i++)
        for (int t = 1; t <= n; t++)
            ck_out[i][t] = ch[i][t];
    ProcessMap();
    for (int i = 0; i < 20; i++)
        boat[i].berth = -1;
    scanf("%d", &berth_num);
    for (int i = 0; i < berth_num; i++)
    {
        int id;
        scanf("%d", &id);
        scanf("%d%d%d", &berth[id].x, &berth[id].y, &berth[id].loading_speed);
        berth[id].near_T_dis=1e9;
        berth[id].x++;
        berth[id].y++;
        queue<pair<int, int>> q;
        map<pair<int, int>, int> mp;
        q.push({berth[id].x, berth[id].y});
        while (!q.empty())
        {
            pair<int, int> now = q.front();
            q.pop();
            mp[now] = 1;
            flag_berth[now.first][now.second] = id;
            for (int i = 0; i < 4; i++)
            {
                int nx = now.first + dx[i];
                int ny = now.second + dy[i];
                if (nx < 1 || nx > n || ny < 1 || ny > n)
                    continue;
                if (mp[{nx, ny}] == 0 && (ch[nx][ny] == 'B'||ch[nx][ny] == 'K'))
                {
                    q.push({nx, ny});
                }
            }
        }
    }
    for (int i = 0; i < berth_num; i++)
    {
        bfs_dis(berth[i].x, berth[i].y, i);
        bfs_dis_boat(berth[i].x, berth[i].y, i);
    }

    for (int i = 0; i < delivery_point.size(); i++)
    {
        bfs_dis_comit(delivery_point[i].first, delivery_point[i].second, i);
    }
    for (int i = 0; i < berth_num; i++)
    {
        for (int t = 0; t < delivery_point.size(); t++)
        {
            berth[i].transport_time = min(berth[i].transport_time, pre_dis_comit[t][berth[i].x][berth[i].y] + 100);
        }
    }
    scanf("%d", &boat_capacity);
    char okk[100];
    scanf("%s", okk);
    printf("OK\n");
    fflush(stdout);
}

void Input(int frame_id){
    scanf("%d", &money);
    scanf("%d", &goods_num);
    for (int i = 0; i < goods_num; i++)
    {
        int x, y, val;
        scanf("%d%d%d", &x, &y, &val);
        x++, y++;
        good_time[x][y] = frame_id;
        good_value[x][y] = val;

        for (int j = 0; j < berth_num; j++)
        {
            // (x, y) 在 berth 附件的范围内
            if (pre_dis_berth[j][x][y] <= max_near_dis)
            {
                berth[j].near_val++;
            }
        }
    }

    scanf("%d", &robot_num);

    for (int i = 0; i < robot_num; i++)
    {
        scanf("%d%d%d%d", &robot[i].id, &robot[i].status, &robot[i].x, &robot[i].y);
        robot[robot[i].id].x++;
        robot[robot[i].id].y++;
    }

    scanf("%d", &boat_num);
    for (int i = 0; i < boat_num; i++)
    {
        scanf("%d%d%d%d%d%d\n", &boat[i].id, &boat[i].goods_num, &boat[i].x, &boat[i].y, &boat[i].dir, &boat[i].status);
        boat[boat[i].id].x++;
        boat[boat[i].id].y++;
        // logFile << boat[i].goods_num<<endl;
        if(ch[boat[boat[i].id].x][boat[boat[i].id].y] == 'T'){
            boat[boat[i].id].flag = 0;
        }
        if (boat[boat[i].id].status == 1)
        {
            continue;
        }
        // 如果处于装货状态，装货
        else if (boat[boat[i].id].status == 2)
        {
            // 如果时间不够
            if (14980 - berth[boat[i].berth].transport_time - frame_id < 0 || boat[i].goods_num == boat_capacity)
            {
                bfs_comit_boat(boat[i].x, boat[i].y, frame_id, i, berth[boat[i].berth].near_T, boat[i].dir);
                vis_berth[boat[i].berth] = 0;
                boat[i].berth = -1;
                boat[i].flag = 1;//前往交货点标志
                continue;
            }
            else if(berth[boat[i].berth].size == 0){
                vis_berth[boat[i].berth] = 0;
                boat[i].berth = -1;
                int pos;
                //  寻路最佳泊位
                pos = find_berth(frame_id, i);
                bfs_berth_boat(boat[i].x, boat[i].y, frame_id, i, pos, boat[i].dir);
                vis_berth[pos] = i + 1;
                boat[i].berth = pos;
                continue;
            }
            int temp = min(berth[boat[i].berth].size, berth[boat[i].berth].loading_speed);
            temp = min(temp, boat_capacity - boat[i].goods_num);
            berth[boat[i].berth].size -= temp;
        }
        // 行驶
        else 
        {
            // 到达靠泊点
            if (ch[boat[i].x][boat[i].y] == 'K'&&boat[i].flag != 1&&flag_berth[boat[i].x][boat[i].y]==boat[i].berth)
            {
                // 靠泊
                printf("berth %d\n", i);
            }
            else
            {
                if (boat[i].flag == 1)
                {
                    bfs_comit_boat(boat[i].x, boat[i].y, frame_id, i, berth[boat[i].berth].near_T, boat[i].dir);
                }
                else
                {
                    
                    int pos;
                    //  寻路最佳泊位
                    if (boat[i].berth == -1)
                    {
                        pos = find_berth(frame_id, i);
                    }
                    else
                        pos = boat[i].berth;
                    if (pos == -1)
                        continue;
                    bfs_berth_boat(boat[i].x, boat[i].y, frame_id, i, pos, boat[i].dir);
                    vis_berth[pos] = i + 1;
                    boat[i].berth = pos;
                }
            }
        }
    }

    char okk[100];
    scanf("%s", okk);
}

void bfs_noway(int x, int y, int frame_id, int id)
{
    for (int i = 0; i < 4; i++)
    {
        int nx = x + dx[i];
        int ny = y + dy[i];
        if (nx < 1 || nx > n || ny < 1 || ny > n)
            continue;
        if (vis_ban[nx][ny] >= 1)
            continue;
        if (ch[nx][ny] == '*' || ch[nx][ny] == '#' || ch[nx][ny] == 'K' || ch[nx][ny] == '~' || ch[nx][ny] == 'S')
        {
            continue;
        }
        if (ch[nx][ny] == '.' || ch[nx][ny] == 'B' || ch[nx][ny] == '>' || ch[nx][ny] == 'C' || ch[nx][ny] == 'c' || ch[nx][ny] == 'R')
        {
            printf("move %d %d\n", id, i);
            vis_ban[x][y]--;
            vis_ban[nx][ny]++;
        }
    }
}

void bfs_good(int x, int y, int frame_id, int id)
{

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
    while (!q.empty())
    {
        re++; //&&pos.first != -1
        if (re > 4e3)
            break;
        pair<int, int> now = q.front();
        q.pop();
        int dis = distance.front();
        distance.pop();
        if (good_time[now.first][now.second] > 0 && frame_id - good_time[now.first][now.second] < 1000 &&
            vis_good[now.first][now.second] == 0)
        {

            double w;
            if (frame_id >= out)
            {
                w = good_value[now.first][now.second] / 200.0 * (w_good_val + 1) - dis / 400.0 * w_good_dis;
            }
            else
            {
                w = good_value[now.first][now.second] / 200.0 * w_good_val - dis / 400.0 * w_good_dis;
            }

            if (w > mx)
            {
                mx = w;
                pos = now;
            }
        }
        for (int i = 0; i < 4; i++)
        {
            int nx = now.first + dx[i];
            int ny = now.second + dy[i];
            if (nx < 1 || nx > n || ny < 1 || ny > n)
                continue;
            if (ch[nx][ny] == '*' || ch[nx][ny] == '#' || ch[nx][ny] == 'K' || ch[nx][ny] == '~' || ch[nx][ny] == 'S')
                continue;
            if (vis[nx][ny] == 1)
                continue;
            if ((vis_ban[nx][ny] >= 1 && dis == 0) || vis_ban[nx][ny] >= 2)
                continue;
            if (vis_robot[nx][ny] > 0 && (vis_robot[nx][ny] - 1) < id && dis <= 2)
            {
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
    while (fa[temp_x][temp_y][0] != x || fa[temp_x][temp_y][1] != y)
    {
        cycle++;
        if (cycle > 10000)
            exit(0);
        ck++;
        if (vis_robot[temp_x][temp_y] > 0)
        {
            temp = {temp_x, temp_y};
            ck = 0;
        }
        int x1 = fa[temp_x][temp_y][0];
        int y1 = fa[temp_x][temp_y][1];
        temp_x = x1;
        temp_y = y1;
    }
    if (temp_x == before[id][0] && temp_y == before[id][1])
    {
        if (count_before[id] < 2)
        {
            count_before[id]++;
        }
        else
        {
            count_before[id] = 0;
            int l = rand() % 4;
            for (int i = l; i < l + 4; i++)
            {
                int nx = x + dx[i % 4];
                int ny = y + dy[i % 4];
                if (nx < 1 || nx > n || ny < 1 || ny > n)
                    continue;
                if (ch[nx][ny] == '*' || ch[nx][ny] == '#' || ch[nx][ny] == 'K' || ch[nx][ny] == '~' || ch[nx][ny] == 'S')
                    continue;
                if (vis_ban[nx][ny] >= 1)
                    continue;
                if (nx == before[id][0] && ny == before[id][1])
                    continue;
                if (ch[nx][ny] == '#')
                {
                    exit(0);
                }
                printf("move %d %d\n", id, i % 4);
                vis_ban[x][y]--;
                vis_ban[nx][ny]++;
                ck_move = 1;
                before[id][0] = x;
                before[id][1] = y;
                return;
            }
        }
    }
    else
        count_before[id] = 0;
    before[id][0] = x;
    before[id][1] = y;
    int di_x = temp_x - x;
    int di_y = temp_y - y;
    for (int i = 0; i < 4; i++)
    {
        if (di_x == dx[i] && di_y == dy[i])
        {
            printf("move %d %d\n", id, i);
            vis_ban[x][y]--;
            vis_ban[temp_x][temp_y] += 1;
            ck_move = 1;
            if (ck == 0 || ck == 1)
            {
                vis_ban[temp.first][temp.second] += 10;
            }
            break;
        }
    }
}

void bfs_berth(int x, int y, int frame_id, int id)
{
    search_size = -1;
    double mx_pre_dis = -1e9, mx_near_val = -1e9;
    for (int i = 0; i < berth_num; i++)
    {
        mx_pre_dis = max(mx_pre_dis, 1.0 * pre_dis_berth[i][x][y]);
        mx_near_val = max(mx_near_val, 1.0 * berth[i].near_val);
    }
    for (int i = 0; i < berth_num; i++)
    {
        if (abs(1e9 - pre_dis_berth[i][x][y]) < 0.0001)
        {
            continue;
        }
        if (frame_id >= 150000)
        {
            if (vis_berth[i] == 0)
                continue;
            if (berth[i].size > boat_capacity)
                continue;
            if (berth[i].size > boat[(vis_berth[i] - 1)].goods_num)
                continue;
            search_berth[++search_size].x =
                pre_dis_berth[i][x][y] * w_berth_dis - berth[i].near_val / (mx_near_val + 1) * w_near_val;
            search_berth[search_size].id = i;
        }
        else
        {
            search_berth[++search_size].x = -(berth[i].size < boat_capacity) * 100000.0 +
                                            pre_dis_berth[i][x][y] / mx_pre_dis * w_berth_dis -
                                            berth[i].near_val / (mx_near_val + 1) * w_near_val;
            search_berth[search_size].id = i;
        }
    }
    sort(search_berth, search_berth + search_size + 1);
    pair<int, int> pos;
    pos = {-1, -1};
    for (int l = 0; l <= search_size; l++)
    {

        int search_id = search_berth[l].id;
        for (int i = 1; i <= n; i++)
            for (int j = 1; j <= n; j++)
                vis[i][j] = 0;
        priority_queue<Node, vector<Node>, greater<Node>> q;
        q.push({x, y, 0, pre_dis_berth[search_id][x][y]}); // 机器人初始位置
        if (flag_berth[x][y] == search_id)
        {
            ck_move = 1;
            return;
        }
        vis[x][y] = 1;
        while (!q.empty())
        {

            Node node = q.top();
            pair<int, int> now = {node.x, node.y};
            // logFile << now.first << " " << now.second << " " << node.gn<<" "<<node.hn << endl;
            q.pop();
            int dis = node.gn;
            if (dis > 100)
            {
                pos = now;

                break;
            }
            if (ch[now.first][now.second] == 'B' &&
                flag_berth[now.first][now.second] == search_id)
            {

                pos = now;
                break;
            }
            if (ch[now.first][now.second] == 'B')
            {

                continue;
            }

            for (int i = 0; i < 4; i++)
            {
                int nx = now.first + dx[i];
                int ny = now.second + dy[i];
                if (nx < 1 || nx > n || ny < 1 || ny > n)
                {
                    continue;
                }
                if (ch[nx][ny] == '*' || ch[nx][ny] == '#' || ch[nx][ny] == 'K' || ch[nx][ny] == '~' || ch[nx][ny] == 'S')
                {
                    continue;
                }

                if (vis[nx][ny] == 1)
                {
                    continue;
                }

                if ((vis_ban[nx][ny] >= 1 && dis == 0) || vis_ban[nx][ny] >= 2)
                {
                    continue;
                }
                if (vis_robot[nx][ny] > 0 && (vis_robot[nx][ny] - 1) < id && dis <= 2)
                {
                    continue;
                }
                fa[nx][ny][0] = now.first;
                fa[nx][ny][1] = now.second;
                q.push({nx, ny, dis + 1, pre_dis_berth[search_id][nx][ny]});
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
    while (fa[temp_x][temp_y][0] != x || fa[temp_x][temp_y][1] != y)
    {
        cycle++;
        if (cycle > 10000)
            exit(0);
        ck++;
        if (vis_robot[temp_x][temp_y] > 0)
        {
            ck = 0;
            temp = {temp_x, temp_y};
        }
        int x1 = fa[temp_x][temp_y][0];
        int y1 = fa[temp_x][temp_y][1];
        temp_x = x1;
        temp_y = y1;
    }

    if (temp_x == before[id][0] && temp_y == before[id][1])
    {
        if (count_before[id] < 2)
        {
            count_before[id]++;
        }
        else
        {
            count_before[id] = 0;
            int l = rand() % 4;
            for (int i = l; i < l + 4; i++)
            {
                int nx = x + dx[i % 4];
                int ny = y + dy[i % 4];
                if (nx < 1 || nx > n || ny < 1 || ny > n)
                    continue;
                if (ch[nx][ny] == '*' || ch[nx][ny] == '#' || ch[nx][ny] == 'K' || ch[nx][ny] == '~' || ch[nx][ny] == 'S')
                    continue;
                if (vis_ban[nx][ny] >= 1)
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
    }
    else
        count_before[id] = 0;
    before[id][0] = x;
    before[id][1] = y;
    int di_x = temp_x - x;
    int di_y = temp_y - y;
    for (int i = 0; i < 4; i++)
    {
        if (di_x == dx[i] && di_y == dy[i])
        {
            printf("move %d %d\n", id, i);
            vis_ban[x][y]--;
            ck_move = 1;
            vis_ban[temp_x][temp_y] += 1;
            if (ck == 0 || ck == 1)
            {
                vis_ban[temp.first][temp.second] += 10;
            }

            break;
        }
    }
}

void buy()
{
    if (money >= robot_price && robot_num <= 10)
    {
        printf("lbot %d %d\n", robot_purchase_point[robot_num % robot_purchase_point.size()].first - 1, robot_purchase_point[robot_num % robot_purchase_point.size()].second - 1);
    }
    if (money >= boat_price && boat_num <= 0)
    {
        printf("lboat %d %d\n", boat_purchase_point[boat_num % boat_purchase_point.size()].first - 1, boat_purchase_point[boat_num % boat_purchase_point.size()].second - 1);
    }
}

// -----------------------------------------------start----------------------------------------
int main()
{
    logFile.open("log.txt");

    // 初始化
    Init();
    while (scanf("%d", &frame_id) != EOF)
    {
        // 读入
        Input(frame_id);

        // 购买逻辑
        buy();
        for (int i = 1; i <= n; i++)
        {
            for (int j = 1; j <= n; j++)
            {
                vis_good[i][j] = 0;
                vis_ban[i][j] = 0;
                vis_robot[i][j] = 0;
                if (frame_id - good_time[i][j] == 1000 && good_time[i][j] > 0)
                    disapper_num++;
                for (int k = 0; k < berth_num; ++k)
                {
                    if (pre_dis_berth[k][i][j] <= max_near_dis && frame_id - good_time[i][j] == 1000 &&
                        good_time[i][j] > 0)
                    {
                        berth[k].near_val--;
                    }
                }
            }
        }
        for (int i = 0; i < robot_num; i++)
        {
            vis_ban[robot[i].x][robot[i].y] = 1;
            vis_robot[robot[i].x][robot[i].y] = i + 1;
            // 没拿着货物 而且在货物上
            if (robot[i].status == 0)
            {
                if (good_time[robot[i].x][robot[i].y] > 0 && frame_id - good_time[robot[i].x][robot[i].y] < 1000 &&
                    targer_good[i][0] == robot[i].x && targer_good[i][1] == robot[i].y)
                {
                    printf("get %d\n", i);

                    for (int j = 0; j < berth_num; j++)
                    {
                        // (x, y) 在 berth 附件的范围内
                        if (pre_dis_berth[j][robot[i].x][robot[i].y] <= max_near_dis)
                        {
                            berth[j].near_val--;
                        }
                    }
                }
            }
            // 拿着货物 而且在码头
            else if (robot[i].status == 1)
            {
                if (ch[robot[i].x][robot[i].y] == 'B')
                {
                    printf("pull %d\n", i);
                    berth[flag_berth[robot[i].x][robot[i].y]].size++;
                }
            }
        }
        for (int i = 0; i < robot_num; i++)
        {
            ck_move = 0;
            if (robot[i].status == 0)
            {
                if (good_time[robot[i].x][robot[i].y] > 0 && frame_id - good_time[robot[i].x][robot[i].y] < 1000 &&
                    targer_good[i][0] == robot[i].x && targer_good[i][1] == robot[i].y)
                {
                    good_time[robot[i].x][robot[i].y] = 0;
                    ck_move = 1;
                }
                else
                {
                    if (frame_id <= 50)
                    {
                        bfs_berth(robot[i].x, robot[i].y, frame_id, i);
                        if (ck_move == 0)
                        {
                            bfs_good(robot[i].x, robot[i].y, frame_id, i);
                        }
                    }
                    else
                        bfs_good(robot[i].x, robot[i].y, frame_id, robot[i].id);
                }
            }
            else if (robot[i].status == 1)
            {
                if (ch[robot[i].x][robot[i].y] == 'B')
                {
                    ck_move = 1;
                }
                else
                {
                    bfs_berth(robot[i].x, robot[i].y, frame_id, i);
                }
            }
            if (ck_move == 0)
            {
                bfs_noway(robot[i].x, robot[i].y, frame_id, i);
            }
            else
            {
                last_robot_vis[i][0] = -1;
                last_robot_vis[i][1] = -1;
            }
        }

        puts("OK");
        fflush(stdout);
    }
    return 0;
}