#define _CRT_SECURE_NO_WARNINGS
#include "routing.h"
#include <iostream>
#include <fstream>
#include <cmath>
#include <cstdio>
#include <vector>
#include <algorithm>
#include <QString>
#include <QTextStream>
#include <QTextCodec>
#include <QFile>
#include <QDebug>
using namespace std;

STOP* GIS::st = 0; //公交站
LINE* GIS::ls = 0; //公交线路
int GIS::ns = 0;   //公交站数
int GIS::nl = 0;   //公交线路数
int GIS::obs = 0;  // GIS对象数量
MAT GIS::raw;     //原始转移图
MAT GIS::tra;     //闭包转移图

//##################STOP类的实现########################

//构造函数
STOP::STOP(int n, int x, int y)
{
    this->numb = n;
    this->x = x;
    this->y = y;
}
//获取x坐标
int& STOP::X()
{
    return x;
}
//获取y坐标
int& STOP::Y()
{
    return y;
}
//获取公交站编号
int& STOP::N()
{
    return numb;
}


//#################LINE类的实现###################

//构造函数
LINE::LINE(int n, int c, int* stop) : numb(n), stop(c ? new int[c] : nullptr), nofs(stop ? c : 0)
{
    if (LINE::stop == nullptr && nofs != 0)
        throw "Memory allocation for bus line error!";
    for (int x = 0; x < nofs; x++)
        LINE::stop[x] = stop[x];
}
//深拷贝构造
LINE::LINE(const LINE& r) : numb(r.numb), stop(r.nofs ? new int[r.nofs] : nullptr), nofs(stop ? r.nofs : 0)
{
    if (stop == nullptr && r.nofs != 0)
        throw "Memory allocation for bus line error!";
    for (int x = 0; x < nofs; x++)
        stop[x] = r.stop[x];
}
//移动构造
LINE::LINE(LINE&& r) noexcept : numb(r.numb), stop(r.stop), nofs(r.nofs)
{
    (int&)(r.numb) = (int&)(r.nofs) = 0;
    (int*&)(r.stop) = nullptr;
}
//深拷贝赋值
LINE& LINE::operator=(const LINE& r)
{
    if (this == &r) //防止自拷贝
        return *this;
    if (stop) //如果存在先删除
    {
        delete[] stop;
    }
    (int&)numb = r.numb;
    (int&)nofs = r.nofs;
    (int*&)stop = new int[r.nofs];
    if (stop == 0)
        throw "memory allocation error";
    for (int i = 0; i < nofs; i++) //数组值拷贝
        this->stop[i] = r.stop[i];
    return *this;
}
//移动赋值
LINE& LINE::operator=(LINE&& r) noexcept
{
    if (this == &r) //防止自拷贝
        return *this;
    if (stop) //如果存在先删除
    {
        delete[] stop;
    }
    (int&)numb = r.numb;
    (int&)nofs = r.nofs;
    (int*&)stop = r.stop;
    (int&)r.numb = 0;
    (int&)r.nofs = 0;
    (int*&)r.stop = 0;
    return *this;
}
//是否包含站点编号s，包含则返回包含第几站次，-1代表未包含
int LINE::has(int s) const
{
    for (int x = 0; x < nofs; x++)
        if (stop[x] == s)
            return x;
    return -1;
}
//判断两条线路相交，相交返回1
int LINE::cross(const LINE& b) const
{ //线路相交则返回1
    if (this == &b)
        return 0;
    for (int x = 0; x < nofs; x++)
        if (b.has(stop[x]) != -1)
            return 1;
    return 0;
}

//取线路编号
LINE::operator int() const
{
    return numb;
}
//取公交车站点数量
int LINE::NOFS() const
{
    return nofs;
}
//线路从站d到站a的距离
double LINE::dist(int b, int e) const
{
    double d;
    int bi, ei; //两个站点的开始及结束序号
    int x1, y1, x2, y2, w;
    if ((bi = has(b)) == -1)
        throw "Wrong stop number while calculating distance!";
    if ((ei = has(e)) == -1)
        throw "Wrong stop number while calculating distance!";
    if (bi > ei)
    {
        b = ei;
        ei = bi;
        bi = b;
    }
    d = 0;
    for (b = bi; b < ei; b++)
    {
        x1 = GIS::st[stop[b]].X();
        y1 = GIS::st[stop[b]].Y();
        x2 = GIS::st[stop[b + 1]].X();
        y2 = GIS::st[stop[b + 1]].Y();
        d += sqrt(1.0*((x2-x1) * (x2-x1) + (y2-y1) * (y2-y1)));
    }
    return d;
}
//取出第x站的站点编号
int& LINE::operator[](int x)
{
    if (x >= 0)
        return stop[x];
}
//析构函数
LINE::~LINE() noexcept
{
    if (stop != 0)
    {
        delete[] stop;
    }
    (int*&)stop = 0;
    (int&)numb = 0;
    (int&)nofs = 0;
}
//######################TRAN类的实现###########################

//构造函数
TRAN::TRAN(int from, int to, int stop) : from(from), to(to), stop(stop) 
{

}
//判断是否相等
int TRAN::operator==(const TRAN& t) const
{
    if(from == t.from && to == t.to && stop == t.stop)
        return 1;
    return 0;
}
int& TRAN::F() { return from; }
int& TRAN::T() { return to; }
int& TRAN::S() { return stop; }

//######################ROUTE类的实现###########################

//构造函数
ROUTE::ROUTE(TRAN* tran, int noft) : tran(noft ? new TRAN[noft] : nullptr), noft(noft)
{
    if (this->tran == 0 && noft != 0)
        throw "memory allocation error!";
    for (int i = 0; i < noft; i++)
        this->tran[i] = tran[i];
}
//用单次转乘构造ROUTE
ROUTE::ROUTE(const TRAN& t) : tran(new TRAN[1]), noft(0)
{
    if (tran == 0)
        throw "memory allocation error!";
    if (tran)
        *tran = t;
}
//深拷贝构造
ROUTE::ROUTE(const ROUTE& r) : tran(r.noft ? new TRAN[r.noft] : 0), noft(r.noft)
{
    if (tran == 0 && r.noft != 0)
        throw "memory allocation error!";
    for (int i = 0; i < noft; i++)
        tran[i] = r.tran[i];
}
//移动构造
ROUTE::ROUTE(ROUTE&& r) noexcept : tran(r.tran), noft(r.noft)
{
    (TRAN*&)(r.tran) = 0;
    (int&)(r.noft) = 0;
}
//析构函数
ROUTE::~ROUTE() noexcept
{
    if (tran)
    {
        delete[] tran;
        *(int*)&noft = 0;
        (TRAN*&)tran = 0;
    }
}
//打印转乘路径,返回转乘次数
int ROUTE::print() const
{
    for (int i = 0; i < noft; i++)
    {
        if (tran[i].S() == -1)
            cout << "乘坐 公交" << tran[i].F() + 1 << "路 直达" << tran[i].S() << endl;
        else
            cout << "乘坐 公交" << tran[i].F() + 1 << "路 在" << tran[i].S() << "换乘 " << tran[i].T() << "路 公交车" << endl;
    }
    return noft;
}
//转乘次数
ROUTE::operator int() const
{
    return noft;
}
//去除重复转乘
ROUTE ROUTE::operator*() const
{
    int nn = noft;
    TRAN* t = new TRAN[noft];//临时数组
    if (t == 0)
        throw "memory allocation error!";
    for (int i = 0; i < nn; i++)
        t[i] = tran[i];
    for (int i = 0; i < nn - 1; i++)
        for (int j = i + 1; j < nn; j++)
        {
            if (t[i].S() == t[j].S() && t[i].F() == t[j].T())//两个环路构成循环,回到之前做坐的车
            {
                for (int k = i, n = j + 1; n < nn; n++, k++)
                    t[k] = t[n];//全部前移
                nn -= (j + 1 - i);//两个线路之间的是环线的组成部分,全部删除
                j = i;//去掉后该轮需要重新开始防止连续构成的循环未被处理
            }
        }
    ROUTE r(t, nn);
    delete[] t;
    return r;
}

//转乘路径相差的转乘次数
int ROUTE::operator-(const ROUTE& n) const
{
    return noft - n.noft;
}
//判断两个路径是否相同
bool ROUTE::operator==(const ROUTE& r)
{
    if (r.noft == noft)
    {
        for (int i = 0; i < noft; i++)
        {
            if (!(tran[i] == r.tran[i]))
            {
                return false;
            }
        }
        return true;
    }
    return false;
}
//一条路径上所有的换乘站点
TRAN& ROUTE::operator[](int x)
{
    if (x < 0 || x > noft)
        throw "out of index!";
    return tran[x];
}
//转乘路径连接
ROUTE ROUTE::operator+(const ROUTE& r) const
{
    if (noft == 0)
        return *this;
    if (r.noft == 0)
        return r;
    if (tran[noft - 1].T() != r.tran[0].F())//首尾不相连
        throw "can not be connected!";
    ROUTE s;
    (TRAN*&)(s.tran) = new TRAN[noft + r.noft];
    if(!s.tran)
        throw "memory allocation error!";
    (int&)(s.noft) = noft + r.noft;
    int x,y;
    for (x = 0; x < noft; x++)
        s.tran[x] = tran[x];
    for (y = 0; y < r.noft; y++)
        s.tran[x++] = r.tran[y];
    return *s;
}
//深拷贝赋值
ROUTE& ROUTE::operator=(const ROUTE& r)
{
    if (this == &r)
        return *this;
    if (tran)
        delete[] tran;
    (TRAN*&)tran = new TRAN[r.noft];
    if(!tran)
        throw "Memory allocation error!";
    (int&)noft = r.noft ;
    for (int i = 0; i < noft; i++)
        tran[i] = r.tran[i];
    return *this;
}
//移动赋值
ROUTE& ROUTE::operator=(ROUTE&& r) noexcept
{
    if (this == &r)
        return *this;
    if (tran)
        delete[] tran;
    (TRAN*&)tran = r.tran;
    (int&)noft = r.noft;
    (TRAN*&)(r.tran) = 0;
    (int&)(r.noft) = 0;
    return *this;
}
//路径连接
ROUTE& ROUTE::operator+=(const ROUTE& r)
{
    return *this = *this + r;
}


//######################ROUTE类的实现###########################

//构造函数
ELEM::ELEM(ROUTE* p, int n) : p(n > 0 ? new ROUTE[n] : 0), n(n)
{
    if (this->p == 0 && n != 0)
        throw "memory allocation error!";
    for (int i = 0; i < n; i++)
        this->p[i] = p[i];
}
//缺省转乘方案的构造
ELEM::ELEM(int n) : p(n>0 ? new ROUTE[n] : 0), n(n)
{
    if (p == 0 && n != 0)
        throw "memory allocation error!";
}
//深拷贝构造
ELEM::ELEM(const ELEM& n) : p(n.n > 0 ? new ROUTE[n.n] : 0), n(n.n)
{
    if (p == 0 && n.n != 0)
        throw "memory allocation error!";
    for (int i = 0; i < this->n; i++)
        p[i] = n.p[i];
}
//移动构造
ELEM::ELEM(ELEM&& n) noexcept : p(n.p), n(n.n)
{
    (ROUTE*&)(n.p) = 0;
    (int&)n.n = 0;
}
//去掉转乘中的环
ELEM ELEM::operator*() const
{
    int nn = this->n;
    if (nn == 0)
        return *this;
    ROUTE* t = new ROUTE[nn];
    if (t == 0)
        throw "memory allocation error!";
    for (int x = 0; x < nn; x++)
        t[x] = p[x];
    for (int x = 0; x < nn - 1; x++)
        for (int y = x + 1; y < nn; y++)
        {
            if (t[x] == t[y])//存在环
            {
                for (int k = x + 1, j = y + 1; j < nn - 1; j++, k++)
                    t[k] = t[j];
                nn -= (y - x);//将环上的所有元素去除
                y = x;
            }
        }
    ELEM r(t, nn);
    if (t != 0)//删除临时变量
        delete[] t;
    t = 0;
    return r;
}
//元素增加路径
ELEM ELEM::operator+(const ROUTE& n) const
{
    ELEM r(this->n + 1);
    for (int i = 0; i < this->n; i++)
        r.p[i] = *p[i];
    r.p[this->n] = n;
    return *r;
}
//元素路径增加
ELEM ELEM::operator+(const ELEM& n) const
{
    if (this->n == 0)
        return n;
    if (n.n == 0)
        return *this;
    ELEM r(this->n + n.n);//创建新的NODE,路径为两者相加
    for (int i = 0; i < this->n; i++)
        r.p[i] = *p[i];//将本身深拷贝
    for (int i = 0; i < n.n; i++)
        r.p[i + this->n] = *n.p[i];//将n中的元素拷贝
    return *r;
}
//元素路径转乘连接
ELEM ELEM::operator*(const ELEM& n) const
{
    if (this->n == 0)
        return *this;
    if (n.n == 0)
        return n;
    ELEM r(this->n * n.n);
    int cnt = 0;
    for (int i = 0; i < this->n; i++)
        for (int j = 0; j < n.n; j++)
        {
            if (p[i][-1 + p[i]].T() != n.p[j][0].F())
                throw "be a circle!";
            try
            {
                r.p[cnt] = p[i] + n.p[j]; //路径相连
            }
            catch (const char* e)
            {
                const char* p = e;
            }
            cnt++;
        }
    return *r;
}
//深拷贝赋值
ELEM& ELEM::operator=(const ELEM& n)
{
    if (this == &n)//防止自赋值
        return *this;
    if (p)
        delete[] p;
    (ROUTE*&)p = new ROUTE[n.n];
    if (p == 0)
        throw "memory allocation error!";
    (int&)(this->n) = p ? n.n : 0;
    for (int i = 0; i < n.n; i++)//值拷贝
        p[i] = n.p[i];
    return *this;
}
//移动赋值
ELEM& ELEM::operator=(ELEM&& n) noexcept
{
    if (this == &n)//防止自赋值
        return *this;
    if (p)
        delete[] p;
    (ROUTE*&)p = n.p;
    (int&)(this->n) = n.n;
    (ROUTE*&)(n.p) = 0;
    (int&)(n.n) = 0;
    return *this;
}
//元素增加路径
ELEM& ELEM::operator+=(const ROUTE& n)
{
    return *this = *this + n;
}
//元素路径转乘连接
ELEM& ELEM::operator+=(const ELEM& n)
{
    return *this = *this + n;
}
//移动赋值
ELEM& ELEM::operator*=(const ELEM& n)
{
    return *this = *this * n;
}
//获得第x个转乘路径
ROUTE& ELEM::operator[](int x)
{
    if (x < 0 || x >= n)
        throw "error input of index!";
    return p[x];
}
//取出方案数量
ELEM::operator int& ()
{
    return n;
}
//析构函数
ELEM::~ELEM() noexcept
{
    if (p)
    {
        delete[] p;
        (ROUTE*&)p = 0;
        (int&)n = 0;
    }
}
//输出函数
void ELEM::print() const
{
    for (int i = 0; i < n; i++)
    {
        p[i].print();
    }
}


//######################TMAP类的实现###########################

//构造函数
MAT::MAT(int r, int c) : p((r > 0 && c > 0) ? new ELEM[r * c] : 0), r( r), c(c)
{
    if (this->p == 0 && r > 0 && c > 0)
        throw "memory allocation error!";
}
//深拷贝构造
MAT::MAT(const MAT& a) : p((a.r > 0 && a.c > 0) ? new ELEM[a.r * a.c] : 0), r(a.r), c(a.c)
{
    if (p == 0 && a.r > 0 && a.c > 0)
        throw "memory allocation error!";
    for (int i = r * c - 1; i >= 0; i--)
        p[i] = a.p[i];
}
//移动构造
MAT::MAT(MAT&& a) noexcept : p(a.p), r(a.r), c(a.c)
{
    (ELEM*&)(a.p) = 0;
    (int&)(a.r) = (int&)(a.c) = 0;
}
//析构函数
MAT::~MAT()
{
    if (p)
    {
        delete[] p;
        (ELEM*&)p = 0;
        (int&)r = (int&)c = 0;
    }
}
//不可达点返回false
bool MAT::notZero() const
{
    for (int i = r * c - 1; i >= 0; i--)
        if (p[i].operator int& () == 0)
            return 0;
    return 1;
}
//从r到c可转乘路径数目
int& MAT::operator()(int x, int y)
{
    if (x < 0 || x >= r||y<0||y>=c)
        throw "out of index!";
    return p[x * c + y];
}
//取矩阵r行的首地址
ELEM* MAT::operator[](int r)
{
    if (r < 0 || r >= this->r)
        throw "out of index!";
    return p + r * c;
}
//深拷贝赋值
MAT& MAT::operator=(const MAT& a)
{
    if (this == &a)//防止自赋值
        return *this;
    if (p)
        delete[] p;
    (ELEM*&)p = new ELEM[a.r * a.c];
    if (p == 0)
        throw "memory allocation error!";
    (int&)r = a.r;
    (int&)c = a.c;
    for (int i = r * c - 1; i >= 0; i--)
        p[i] = a.p[i];
    return *this;
}
//移动赋值
MAT& MAT::operator=(MAT&& a)
{
    if (this == &a)//防止自赋值
        return *this;
    if (p)//存在先删除
        delete[] p;
    (ELEM*&)p = a.p;
    (int&)r = a.r;
    (int&)c = a.c;
    (ELEM*&)(a.p) = 0;
    (int&)(a.r) = 0;
    (int&)(a.c) = 0;
    return *this;
}
//闭包乘法
MAT MAT::operator*(const MAT& a) const
{
    if (c != a.r)
        throw "can not mutiply!";
    int t=0;
    MAT s(r, a.c);//需要返回的变量,由乘法填入相应的值
    for (int h = 0; h < r; h++)//仿照矩阵乘法
    {
        for (int j = a.c - 1; j >= 0; j--)
        {
            if (h == j) 
                continue;
            t = h * s.c + j;
            for (int k = 0; k < c; k++)
                if (k != h && k != j)
                    s.p[t] += p[h * c + k] * a.p[k * a.c + j];
        }
    }
    return s;
}
//闭包加法
MAT MAT::operator+(const MAT& a) const
{
    if (r != a.r && c != a.c)
        throw "can not not add!";
    MAT s(*this);
    for (int i = r * c - 1; i >= 0; i--)
        s.p[i] += a.p[i];
    return s;
}
//加法
MAT& MAT::operator+=(const MAT& a)
{ 
    return *this = *this + a;
}
//乘法
MAT& MAT::operator*=(const MAT& a)
{ 
    return *this = *this * a;
}
//将路径a加入矩阵元素
MAT& MAT::operator()(int ro, int co, const ROUTE& a)
{
    p[ro * c + co] += a;
    return *this;
}
//起点b终点e返回最少转乘的路径数
int MAT::miniTran(int s, int t, int& notf, ROUTE(&r)[100]) const
{
    int b = 0, e = 0;//起点和终点
    int num_tran[100];//对应的转乘路线数
    int bls[20], els[20];//bls为包含起点的路线数,els为包含终点的路线数
    ELEM rou;
    for (int i = 0; i < GIS::nl; i++)//寻找包含起点或终点相关公交线路下标
    {
        if (GIS::ls[i].has(s) != -1)//存在
            if (b < 20)
                bls[b++] = i;
        if (GIS::ls[i].has(t) != -1)
            if (e < 20)
                els[e++] = i;
    }
    int z = 0,u,v,w;
    for (int i = 0; i < b; i++)
        for (int j = 0; j < e; j++)
        {
            rou = GIS::tra[bls[i]][els[j]];//得到两线路的所有转乘线路
            w = GIS::tra(bls[i], els[j]);
            if (w == 0)//转乘线路数量为0
                continue;
            for (v = 0; v < w; v++)
            {
                u = rou[v]; //得到转乘次数
                if (z == 0 || u < num_tran[0])//若nott为空，或转乘次数比nott[0]小
                {
                    num_tran[0] = u;
                    r[0] = rou[v];
                    z = 1;
                }
                if (u == num_tran[0])//和已有线路转乘次数相同时，插入
                {
                    if (z == 100)
                        return z;
                    num_tran[z] = u;
                    r[z] = rou[v];
                    z++;
                }
            }
        }
    notf = num_tran[0];
    return z;
}
//从b到e基于路径r的距离
double MAT::getDist(int b, int e, ROUTE& r)
{
    double d = 0;
    if ((int)r==1 && r[0].F() == r[0].T())//一次转乘或者现在乘坐的是需要转乘的,不需要换乘
    {
        d = GIS::ls[r[0].F()].dist(b, e);//一条线上直接计算距离
        return d;
    }
    d = GIS::ls[r[0].F()].dist(b, r[0].S());
    int y = (int)r - 1;//记录转乘的次数
    for (int i = 0; i < y; i++)
        d += GIS::ls[r[i].T()].dist(r[i].S(), r[i + 1].S());//多条线的分段距离之和
    d += GIS::ls[r[y].T()].dist(r[y].S(), e);
    return d;
}
//起点b终点e返回最短距离的路径数
int MAT::miniDist(int s, int t, double& dist, ROUTE(&r)[100]) const
{
    int b = 0, e = 0;//起点和终点
    int dis[100];//对应的转乘距离
    int bls[20], els[20];//bls为包含起点的路线数,els为包含终点的路线数
    ELEM rou;
    for (int i = 0; i < GIS::nl; i++)//寻找包含起点或终点相关公交线路下标
    {
        if (GIS::ls[i].has(s) != -1)//存在
            if (b < 20)
                bls[b++] = i;
        if (GIS::ls[i].has(t) != -1)
            if (e < 20)
                els[e++] = i;
    }
    int z = 0, u, w;
    for (int i = 0; i < b; i++)
        for (int j = 0; j < e; j++)
        {
            rou = GIS::tra[bls[i]][els[j]];//得到两线路的所有转乘线路
            w = GIS::tra(bls[i], els[j]);
            if (w == 0)//转乘线路数量为0
                continue;
            for (int i = 0; i < w; i++)
            {
                u = MAT::getDist(s, t, rou[i]);//转乘距离
                if (z == 0 || u < dis[0])//若nott为空，或转乘次数比nott[0]小
                {
                    dis[0] = u;
                    r[0] = rou[i];
                    z = 1;
                }
                if (u == dis[0])//和已有线路转乘次数相同时，插入
                {
                    if (z == 100)//满了
                        return z;
                    dis[z] = u;
                    r[z] = rou[i];
                    z++;
                }
            }
        }
    dist = dis[0];
    return z;
}
//输出转乘矩阵
void MAT::print() const
{
    for (int i = 0; i < r; i++)
    {
        for (int j = 0; j < c; j++)
        {
            cout << "从" << i << "到" << j << "有" << (int)p[i * c + j] << "条路径" << endl;
        }
    }
}

//######################Organization类的实现###########################
//构造函数
Organization::Organization(QString& loc1, int x1, int y1):orgname(loc1), X(x1), Y(y1)
{
    //空的
}
//取出名称
const QString& Organization::org_name()
{
    return orgname;
}
//取x坐标
int& Organization::x()
{ 
    return X;
}
//取y坐标
int& Organization::y()
{
    return Y;
}


//######################GIS类的实现###########################

//构造函数
GIS::GIS()
{ 
    obs++; 
}
//用站点及线路文件加载地图
GIS::GIS(const char* flstop, const char* flline, const char* florg)
{
    FILE* fs, * fl;
    fs = fopen(flstop, "r");
    fl = fopen(flline, "r");
    if (fs == 0 || fl == 0)
        throw "File open error!";
    fscanf(fs, "%d", &ns);
    st = new STOP[ns];
    int m, n, p, q, r;
    int* s, * t;
    for (m = 0; m < ns; m++)
    {
        fscanf(fs, "%d%d", &st[m].X(), &st[m].Y());
        st[m].N() = m + 1;//公交线路的编号从1开始
    }
    fclose(fs);
    fscanf(fl, "%d", &nl);
    s = new int[nl];//每条线路的站点数
    t = new int[100];//每条线路的站点数，上限为100
    for (m = 0; m < nl; m++)//读入站点
    {
        fscanf(fl, "%d", &s[m]);
    }
    *(LINE**)&ls = new LINE[nl];
    for (m = 0; m < nl; m++)//读入站点数
    {
        for (n = 0; n < s[m]; n++)
        {
            fscanf(fl, "%d", &t[n]);
            t[n]--;
        }
        ls[m] = LINE(m + 1, s[m], t);
    }
    fclose(fl);
    //读取组织
    QFile f(florg);
    if (!f.open(QIODevice::ReadOnly))  //打开文件失败
        throw "error";
    QTextCodec* code = QTextCodec::codecForName("utf8");//防止乱码
    QTextStream fino(&f);
    fino.setCodec(code);
    while (!fino.atEnd()) {
        QString s, t;
        int x, y;
        fino >> s >> x >> t >> y;
        if (s.isEmpty()) { org_num = orgs.size(); break; }
        orgs.push_back({ s,x,y });
    }
    f.close();
    for (m = 0; m < nl; m++)//构造raw
    {
        for (p = 0,n = 0; n < nl; n++)
            if (m != n)
                p += GIS::ls[m].cross(GIS::ls[n]);
        if (p == 0)
        {
            throw "there is an independent line";
        }
    }
    MAT ra(nl, nl);
    ROUTE a;
    TRAN* u = new TRAN[100];//临时数组
    for (m = 0; m < nl; m++)
        for (n = 0; n < nl; n++)
        {
            if (m == n)
            {
                u[0] = TRAN(m, n, -1);//自身可在任意一点转移
                a = ROUTE(&u[0], 1);//只有一次转乘
                ra(m, n, a);
                continue;
            }
            p = 0; //公交线路交点个数
            for (q = GIS::ls[m].NOFS() - 1; q >= 0; q--)
            {
                r = GIS::ls[m][q];
                if (GIS::ls[n].has(r) != -1)//每个交点产生一个新路线即转乘
                {
                    u[p] = TRAN(m, n, r);
                    a = ROUTE(&u[p++], 1);//只有一次转乘
                    ra(m, n, a);
                }
            }
        }
    tra = raw = ra;
    for (n = 2; n < nl; n++)//构造矩阵闭包
    {
        raw *= ra;
        tra += raw;
    }
    raw = ra;
    delete[]s;
    delete[]t;
    delete[] u;
    obs++;
}

//最少转乘
int GIS::miniTran(int fx, int fy, int tx, int ty, int& f, int& t, int& n, ROUTE(&r)[100])
{
    int m;
    f = 0; //设离起点最近的站点为f
    double df = sqrt((st[0].X() - fx) * (st[0].X() - fx) + (st[0].Y() - fy) * (st[0].Y() - fy));
    t = 0; //设离终点最近的站点为t
    double dt = sqrt((st[0].X() - tx) * (st[0].X() - tx) + (st[0].Y() - ty) * (st[0].Y() - ty));
    double tf, tt;
    for (m = 1; m < GIS::ns; m++)//搜索离起点或终点最近的站点
    {
        tf = sqrt((st[m].X() - fx) * (st[m].X() - fx) + (st[m].Y() - fy) * (st[m].Y() - fy));
        if (df > tf)//步行起点距离
        {
            df = tf;
            f = m;
        }
        tt = sqrt((st[m].X() - tx) * (st[m].X() - tx) + (st[m].Y() - ty) * (st[m].Y() - ty));
        if (dt > tt)
        {
            dt = tt;
            t = m;
        }
    }
    if (f == t)//起点和终点相同，不乘车
        return 0;
    return GIS::tra.miniTran(f, t, n, r);
}
//最短距离
int GIS::miniDist(int fx, int fy, int tx, int ty, int& f, int& t, double& d, ROUTE(&r)[100])
{
    int m;
    double df, tf, dt, tt;
    f = 0;
    df = sqrt((st[0].X() - fx) * (st[0].X() - fx) + (st[0].Y() - fy) * (st[0].Y() - fy));
    t = 0;
    dt = sqrt((st[0].X() - tx) * (st[0].X() - tx) + (st[0].Y() - ty) * (st[0].Y() - ty));
    for (m = 1; m < GIS::ns; m++)
    {
        tf = sqrt((st[m].X() - fx) * (st[m].X() - fx) + (st[m].Y() - fy) * (st[m].Y() - fy));
        if (df > tf)
        {
            df = tf;
            f = m;
        }
        tt = sqrt((st[m].X() - tx) * (st[m].X() - tx) + (st[m].Y() - ty) * (st[m].Y() - ty));
        if (dt > tt)
        {
            dt = tt;
            t = m;
        }
    }
    if (f == t)//起点和终点相同，不乘车
        return 0;
    return GIS::tra.miniDist(f, t, d, r);
}
//析构函数
GIS::~GIS()
{
    obs--;
    if (obs)//已经没有对象了
        return;
    if (st)
    {
        delete[] st;
        *(STOP**)&st = 0;
    }
    if (ls)
    {
        delete[] ls;
        *(LINE**)&ls = 0;
    }
}