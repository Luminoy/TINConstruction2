
// TINConstruction2View.h : CTINConstruction2View 类的接口
//

#pragma once

#include "TINHeader.h"

#include <vector>
#include <queue>
#include <map>
#include <algorithm>
#include <iostream>
using namespace std;


class CTINConstruction2View;

class CThreadParam
{
public:
	HANDLE hHandle;
	CTINConstruction2View* pTin;
	int iBlock;
	CThreadParam()
	{
	}
	CThreadParam(CTINConstruction2View* tin)
	{
		pTin = tin;
	}
	virtual ~CThreadParam() {}

};

class CTINConstruction2View : public CView
{
protected:
	// 仅从序列化创建
	CTINConstruction2View();
	DECLARE_DYNCREATE(CTINConstruction2View)

// 特性
public:
	CTINConstruction2Doc* GetDocument() const;

// 操作
public:
	//1.数据成员定义(窗口操作)
	//-----------------------------------------------
	int      Type;
	int      OperateID;
	HCURSOR  m_hZoomIn;	    //放大时的光标
	HCURSOR  m_hZoomOut;	//缩小时的光标
	HCURSOR  m_hPan;
	HCURSOR  m_hSelect;
	HCURSOR  m_StartPNT;
	HCURSOR  m_EndPNT;
	PNT      ptMouse, ptMouseTmp;
	BOOL     Captured;
	//-----------------------------------------------
	double ClientWidth;    //客户区宽和高
	double ClientHeight;
	double zoomratio, xcenter, ycenter; //图形的中心、放大系数
	double xmin, ymin, xmax, ymax, dx, dy; //图形显示时的最小最大坐标(xmin,ymin)、(xmax,ymax)

	bool   nFlagPoint, nFlagArc;
	double xminPoint, yminPoint, xmaxPoint, ymaxPoint;
	double xminArc, yminArc, xmaxArc, ymaxArc;
	//-----------------------------------------------   
	//2.数据成员定义
	long pointNumber;  //离散点的个数
	MyPoint *PointData;
	MyPoint *CopyData;
	int arcNumber;    //弧段条数
	ArcSet *Arc;
	HANDLE* m_hMutex;   //互斥线程
	double runTime[6];    //运行时间
	int iBlock;        //分块号
	BlockTin *blockTin;
	TRIANGLE **TinSave;
	int BlockTotal;
	CThreadParam* param;

	PNT *pStartPoint, *pEndPoint;
	long nStartTri, nEndTri;
	long nStartPointID, nEndPointID;
	TRIANGLE *pStartTri, *pEndTri;
	LineSet m_LineSet;            //
	TopoPoint *m_TopoPoint;       //
	MyPoint *pPathPoints;           // 存放结果数据点
	long nPathPointNum;            // 路径点个数
	/////////////////////////辅助栅格场，简化点在三角形中定位/////////////////////////////////////////////////
	GroupGrid blockGrid[BlockGridSize][BlockGridSize];
public:
	//1.函数成员定义(窗口操作)
	void LoadFile(int Type);
	void RefreshScreen();
	void OnDelaunayTri();
	void ZoomFull();
	void GetScreenPoint(PNT *pt);
	void GetMapPoint(PNT * pt);
	void CalcBoundPoint();
	void CalcBoundArc();
	void CalcBoundGraph();
	void DrawGraph(CDC*pDC);
	void DrawPoint(CDC* pDC, MyPoint *Data, int size, COLOR PRGB = BLACK, COLOR BRGB = WHITE, int radius = 2);
	void DrawResultPath(CDC* pDC, MyPoint* pPathPoints, int count);
	void RefreshPoint(CDC *pDC, bool IsScreenPoint, double x, double y,COLOR PRGB, COLOR BRGB, int radius);
	void DrawArc(CDC* pDC);
	void RefreshARC(CDC *pDC, ArcSet arc);
	void RefreshLine(CDC *pDC, PNT pt1, PNT pt2);
	bool isIn_Qujian(double x, double x1, double x2);
	void DeleteVertex();
	void LOPScan_BlockTIN(TRIANGLE * p, TRIANGLE ** tinfirst, MyPoint * OriginalData);
	void GetBlockGridInfo();
	int  OnLeft(double x, double y, MyPoint P1, MyPoint P2);
	void EdgeChange(int a, int d, int b, int c, TRIANGLE **t, TRIANGLE **t1);
	void AddNewTin(int a, int b, int c, TRIANGLE *p, TRIANGLE **tinlast, MyPoint *original);
	int  TheOtherPoint(int p1, int p2, int a, int b, int c);
	void SetTriInfor(TRIANGLE* tri, TRIANGLE *blockTri, int e1, int e2);
	void RefreshBlockInfor(BlockTin* block, TRIANGLE tri);
	void SetBlockTri(BlockTin* block, TRIANGLE* blockTri, TRIANGLE tri, int no, int* found);

	// Operations
private:
	//1.定义辅助格网
	double dXmin, dYmin, dXmax, dYmax;
	int    nLineSize, nColSize;  //行列数
	double dXdelt, dYdelt; //分别在x、y方向的跨度距离
	double dDx, dDy;       //每一小格的长度与宽度
						   //2.自适应分块
	Queue *queuehead;
	//3.提取中心点
	MyPoint *PointCenter;
	int CenterSize;
	//图层显示控制
	bool m_displayGrid;

	//-------------------------------
	TRIANGLENODE *tinHead, *tinEnd, *tin;
	COLORREF colors[14];

public:
	///////////////////////自适应分组//////////////////////////////////////////////
	void DrawGrid(CDC* pDC);
	void DrawBinaryLeaf(CDC* pDC);
	//初始化栅格场
	void InitialGrid(int &Line, int &Col, int &ndivision);
	//创建栅格场
	GridField** CraeteGridField(MyPoint *Data, int num, int Line, int Col, double dxmin, double dymin, double ddx, double ddy);
	//自适应分组
	Queue* CreateQueue(GridField **GridArray, int Line, int Col, int ndivision, int num);
	BinaryTree* GreateTreeNode(int startLine, int endLine, int startCol, int endCol, int pNum, int level);
	Queue* GreateQueueElement(BinaryTree *element);
	void AdaptiveDivision(Queue *&head, Queue *&rear, int ndivision, GridField **GridArray, Queue *List);
	//提取每组的中心点
	void AbstractCenterPoint(GridField **GridArray, Queue *head, MyPoint *&Data, int &num, MyPoint *OriginalData);
	void InsertElementForCenter(QueueForCenter *&rear, QueueForCenter *insertelement, GridField **GridArray);
	QueueForCenter* GreateElementForCenter(int L, int C);
	//删除内存
	void ClearGridFieldMemory(GridField **GridArray, int Line, int Col);
	void ClearQueueMemory(Queue *head, MyPoint *Data);
	//////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////
	//2.三角网函数定义
	void DrawTin(CDC * pDC, MyPoint * OriginalData, COLOR PRGB = BLUE);
	void DrawTriangle(CDC * pDC, TRIANGLE * tri);
	void swap(MyPoint r[], int i, int j);
	void qs_x(MyPoint *point, int left, int right);
	int Partition(MyPoint r[], int first, int end);
	void QuickSort(MyPoint r[], int first, int end);
	int TheNumber(int m, int n, int a, int b, int c);
	void SetInitialScan(MyPoint *Point, int pNum, TRIANGLE **tinfirst, TRIANGLE **tinlast, int &g_pointmark, TREENODE **root, TREENODE **frontf, TREENODE **frontl, FB **basef, FB **basel);
	void AddFirstTin(TRIANGLE **tinfirst, TRIANGLE **tinlast, int a, int b, int c);
	void FirAddTin(int a, int b, int c, TRIANGLE **p, TRIANGLE **tinlast);
	void AddNewTin(int a, int b, int c, TRIANGLE *p, TRIANGLE **tinlast);
	void AddNewPointer(TRIANGLE *tinNode1, TRIANGLE *tinNode2);
	void CreateTinBy_Scan(MyPoint *Point, int pNum, TRIANGLE **tinlast, int &g_pointmark, TREENODE **root, TREENODE **frontf, TREENODE **frontl, FB **basef, FB **basel);
	void Circs2(int i, MyPoint *Point, TRIANGLE **tinlast, TREENODE **Root, TREENODE **frontf, TREENODE **frontl, FB **basef, FB **basel);
	void Circs1(int i, MyPoint *Point, TREENODE *p, TRIANGLE **tinlast, TREENODE **Root, FB **basel, FB **baser);
	void Circs1(int i, MyPoint *Point, TREENODE *p, TRIANGLE **tinlast, TREENODE **Root);
	void TreeSearch(MyPoint *Point, TREENODE **root, TREENODE **f, int no);
	void TreeDelete(TREENODE *p, int no, TREENODE **Root);
	void TreeInsert(MyPoint *Point, TREENODE *p, TRIANGLENODE *tin, int no, int flag);    //不包括root
	int HaveSameEdgeScan(int P1, int P2, int P3, int PA, int PB, int PC, int *Q1, int *Q2);
	int SameEdge(int P1, int P2, int PA, int PB);
	int PointUpLine(MyPoint *Point, int a, int b, int c);  //点c在直线ab上方就返回
	void EndFrontLine(MyPoint *Point, TRIANGLE **tinlast, TREENODE **frontf, TREENODE **Root);
	void LOPScan_Nonrecursion(TRIANGLE *p, TRIANGLE **tinfirst, MyPoint *OriginalData);
	void Lop_all(TRIANGLE **t, LIST **rear1, LIST *rear2, MyPoint *OriginalData);
	int CorrespondingTriangle(int a, int b, int c, int *d, TRIANGLE *t, TRIANGLE **t2);
	int PointInCircle(MyPoint *OriginalData, int P0, int P1, int P2, int P3);
	void SetTinScan_all(int a, int d, int b, int c, TRIANGLE **t, TRIANGLE **t1, LIST **rear1, LIST *rear2);
	void CircleBy3Points(MyPoint P1, MyPoint P2, MyPoint P3, double * ox, double * oy, double * RR);
	void GeneDelaunay(MyPoint *Point, int pNum, TRIANGLE **tinfirst, TRIANGLE **tinlast, TREENODE **root, TREENODE **frontf, TREENODE **frontl, FB **basef, FB **basel);
	void InitialCenterMyPoint(MyPoint *Data, int &DataSize, MyPoint *OriginalData, long &OriginalSize);
	void GetCurrentBlockInfor(BlockTin &blocktin, TRIANGLE *TinSave, MyPoint *Point, TREENODE *frontf, FB *basef);
	void ClearPartionMemory(TREENODE *treefirst, FB *fbfirst);
	void ClearTinMemory(TRIANGLE **tin);
	int OnLeft(MyPoint P, MyPoint P1, MyPoint P2);
	void SetBlockPointer(TRIANGLE *t, BlockTin *block);
	void DrawBlockTin(TRIANGLE *t, MyPoint *OriginalData);

// 重写
public:
	virtual void OnDraw(CDC* pDC);  // 重写以绘制该视图
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// 实现
public:
	virtual ~CTINConstruction2View();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 生成的消息映射函数
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg
		void CalPointDistance(vector<PNT>& PNTSet);
	void OnReadShapefile();
	void OnTINNoGroup();
	afx_msg void DeleteTin(TRIANGLE * pTin);
	void OnTinGenerate();
	afx_msg void OnGlobe();
	afx_msg void OnSelect();
	afx_msg void OnZoomIn();
	afx_msg void OnZoomOut();
	afx_msg void OnPan();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	bool SameSide(PNT & A, PNT & B, PNT & C, PNT & P);
	bool IsPointInTriangle(TRIANGLE * T, PNT & P);
	bool IsPointInTriangle(TRIANGLE * T, double xp, double yp);
	void OnMouseMove(UINT nFlags, CPoint point);
//	afx_msg void OnNetConstruction();
	afx_msg void OnStartPNT();
	bool IsLineExist(int PID1, int PID2);
	void LineTopologyConstruct();
	void PointTopologyConstruct();
	afx_msg void CreateTriPath();
	int ModifyPointData(int PID, PNT *pData = NULL);
	void CreateLinePath();
	void AccuSort(vector<long>& vec, long left, long right);
	void OnPathConstruction();
	afx_msg void OnEndPNT();
	afx_msg void OnTinDensify();
	afx_msg void OnSaveShapefile();
};

#ifndef _DEBUG  // TINConstruction2View.cpp 中的调试版本
inline CTINConstruction2Doc* CTINConstruction2View::GetDocument() const
   { return reinterpret_cast<CTINConstruction2Doc*>(m_pDocument); }
#endif

