#pragma once
#include <QString>
#include <vector>
class STOP {//公交车站
    int numb;//公交站编号
    int x;//x坐标
    int y;//y坐标
public:
    //构造函数
    STOP(int numb = 0, int x = 0, int y = 0);
    //数据接口函数
    virtual int& N();//公交站编号
    virtual int& X();//x坐标
    virtual int& Y();//y坐标
};


class LINE {//公交车线路
    const int numb;//公交车编号
    int* const stop;//数组记录该线路上的所有站点
    const int nofs;//总停靠站点数
public:
    LINE(int numb = 0, int nofs = 0, int* stop = 0);
    LINE(const LINE& r);//深拷贝构造
    LINE(LINE&& r) noexcept;//移动构造
    LINE& operator=(const LINE& r);//深拷贝赋值
    LINE& operator=(LINE&& r)noexcept;//移动赋值
    virtual int has(int s)const;//是否包含站点编号s，包含则返回包含第几站次，-1代表未包含
    virtual int cross(const LINE& b) const;//判断两条线路相交，相交返回1
    virtual operator int() const;//取线路编号
    virtual int NOFS() const;//取公交车站点数量
    virtual double dist(int d, int a) const;//线路从站d到站a的距离
    virtual int& operator[](int x);//取出某个站次的站点编号
    virtual ~LINE() noexcept;//析构函数
};

class TRAN {//描述一次转乘，从from经stop到达to
    int from;//现在乘坐的编号
    int to;  //需要转乘的编号
    int stop;//由stops.txt定义的站点编号
public:
    TRAN(int from = 0, int to = 0, int stop = 0);//构造函数
    int operator==(const TRAN& t)const;//判断是否相同
    virtual int& F();//现在乘坐的
    virtual int& T();//接下来需要乘坐的的
    virtual int& S();//站点编号
};

class ROUTE {//一个转乘路径
    TRAN* const tran;//该路径上的所有转乘站点
    const int noft; //该路径的转乘次数
public:
    ROUTE(TRAN* tran = 0, int noft = 0);//构造函数
    ROUTE(const TRAN& t);//用一次转乘构造
    ROUTE(const ROUTE& r);//用转乘路径深拷贝构造
    ROUTE(ROUTE&& r) noexcept;//移动构造

    virtual operator int() const;//转乘次数
    virtual int operator-(const ROUTE& n) const;//转乘路径相差的转乘次数
    virtual ROUTE operator*() const;//去掉重复的公交转乘方案
    virtual TRAN& operator[](int);//一条路径上所有的换乘站点
    virtual ROUTE operator+(const ROUTE& r) const;//转乘路径连接
    virtual ROUTE& operator=(const ROUTE& r);//深拷贝赋值
    virtual ROUTE& operator=(ROUTE&& r) noexcept;//移动赋值
    virtual ROUTE& operator+=(const ROUTE& r);//路径连接
    virtual bool operator==(const ROUTE& r);//判断两个路径是否相同
    virtual ~ROUTE() noexcept;//析构函数
    virtual int print() const;//打印转乘路径
};

class ELEM {        //闭包矩阵元素：记载的转乘次数及线路
    ROUTE* const p; //闭包矩阵r*c个元素记载的是转乘路径方案
    int n;          //闭包矩阵r*c个元素记载的是转乘路径方案数
public:
    ELEM(ROUTE* p, int n);//构造函数
    ELEM(int n = 0);//缺省转乘方案的构造
    ELEM(const ELEM& n);//深拷贝构造
    ELEM(ELEM&& n) noexcept;//移动构造
    virtual ELEM operator*() const;//去掉转乘中的环
    virtual ELEM operator+(const ROUTE& n) const;//元素增加路径
    virtual ELEM operator+(const ELEM& n) const;//元素路径增加
    virtual ELEM operator*(const ELEM& n) const;//元素路径转乘连接
    virtual ELEM& operator=(const ELEM& n);//深拷贝赋值
    virtual ELEM& operator+=(const ELEM& n);//元素路径相加
    virtual ELEM& operator+=(const ROUTE& n);//元素增加路径
    virtual ELEM& operator*=(const ELEM& n);//元素路径转乘连接
    virtual ELEM& operator=(ELEM&& n) noexcept;//移动赋值
    virtual ROUTE& operator[](int x);//获得第x个转乘路径
    virtual operator int& ();//返回可转乘路径数
    virtual ~ELEM() noexcept;//析构函数
    virtual void print() const;//打印矩阵元素
};

class MAT {//所有公交转乘元素的闭包矩阵
    ELEM* const p;//指向闭包矩阵的r*c个元素
    const int r, c;//闭包矩阵的行数和列数
public:
    MAT(int r = 0, int c = 0);//构造函数
    MAT(const MAT& a);//深拷贝构造
    MAT(MAT&& a) noexcept;//移动构造
    virtual ~MAT();//析构函数
    virtual bool notZero() const;//不可达点返回false
    //起点b终点e返回最少转乘的路径数
    virtual int miniTran(int b, int e, int& noft, ROUTE(&r)[100]) const;
    //起点b终点e返回最短距离的路径数
    virtual int miniDist(int b, int e, double& dist, ROUTE(&r)[100]) const;
    static double getDist(int b, int e, ROUTE& r);//从b到e基于路径r的距离
    virtual ELEM* operator[](int r);//取矩阵r行的首地址
    virtual int& operator()(int r, int c);//从r到c可转乘路径数目
    virtual MAT operator*(const MAT& a) const;//闭包乘法
    virtual MAT operator+(const MAT& a) const;//闭包加法
    virtual MAT& operator=(const MAT& a);//深拷贝赋值
    virtual MAT& operator=(MAT&& a);//移动赋值
    virtual MAT& operator+=(const MAT& a);//闭包加法
    virtual MAT& operator*=(const MAT& a);//闭包乘法
    virtual MAT& operator()(int r, int c, const ROUTE& a);//将路径a加入矩阵元素
    virtual void print() const;//输出转乘矩阵
};

class Organization {//描述一个组织
private:
    QString orgname;//组织名		
    int X;//X坐标
    int Y;//Y坐标
public:
    Organization(QString& org1, int x1, int y1);
    Organization() = default;//使用默认无参构造函数和析构函数
    ~Organization() = default;
    const QString& org_name();		//返回组织名
    int& x();		//返回X	
    int& y();		//返回Y
};

struct GIS {//描述地理信息系统
//默认public
    static STOP* st;//所有公交站点
    static LINE* ls;//所有公交线路
    static int ns;//公交站数
    static int nl;//公交线路数
    static MAT raw;//原始转乘矩阵raw，
    static MAT tra;//闭包转乘矩阵tra
    static int obs;//GIS的对象数量
    int org_num=0;//地名数量
    std::vector<Organization>orgs;//记录地名
    GIS();//构造函数
    GIS(const char* flstop, const char* flline, const char* florg);//用站点及线路文件加载地图
    int miniTran(int fx, int fy, int tx, int ty, int& f, int& t, int& n, ROUTE(&r)[100]);//最少转乘
    int miniDist(int fx, int fy, int tx, int ty, int& f, int& t, double& d, ROUTE(&r)[100]);//最短距离
    ~GIS();//析构函数
};
extern GIS* gis;
