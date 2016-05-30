
// TINConstruction2View.h : CTINConstruction2View ��Ľӿ�
//

#pragma once

#include "TINHeader.h"


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
	// �������л�����
	CTINConstruction2View();
	DECLARE_DYNCREATE(CTINConstruction2View)

// ����
public:
	CTINConstruction2Doc* GetDocument() const;

// ����
public:
	//1.���ݳ�Ա����(���ڲ���)
	//-----------------------------------------------
	int      Type;
	int      OperateID;
	HCURSOR  m_hZoomIn;	    //�Ŵ�ʱ�Ĺ��
	HCURSOR  m_hZoomOut;	//��Сʱ�Ĺ��
	HCURSOR  m_hPan;
	HCURSOR  m_hSelect;
	HCURSOR  m_StartPNT;
	HCURSOR  m_EndPNT;
	PNT      ptMouse, ptMouseTmp;
	BOOL     Captured;
	//-----------------------------------------------
	double ClientWidth;    //�ͻ�����͸�
	double ClientHeight;
	double zoomratio, xcenter, ycenter; //ͼ�ε����ġ��Ŵ�ϵ��
	double xmin, ymin, xmax, ymax, dx, dy; //ͼ����ʾʱ����С�������(xmin,ymin)��(xmax,ymax)

	bool   nFlagPoint, nFlagArc;
	double xminPoint, yminPoint, xmaxPoint, ymaxPoint;
	double xminArc, yminArc, xmaxArc, ymaxArc;
	//-----------------------------------------------   
	//2.���ݳ�Ա����
	long pointNumber;  //��ɢ��ĸ���
	MyPoint *PointData;
	MyPoint *CopyData;
	int arcNumber;    //��������
	ArcSet *Arc;
	HANDLE* m_hMutex;   //�����߳�
	double runTime[6];    //����ʱ��
	int iBlock;        //�ֿ��
	BlockTin *blockTin;
	TRIANGLE **TinSave;
	int BlockTotal;
	CThreadParam* param;

	PNT *pStartPoint, *pEndPoint;
	long nStartTri, nEndTri;
	TRIANGLE *pStartTri, *pEndTri;
	LineSet m_LineSet;  //
	/////////////////////////����դ�񳡣��򻯵����������ж�λ/////////////////////////////////////////////////
	GroupGrid blockGrid[BlockGridSize][BlockGridSize];
public:
	//1.������Ա����(���ڲ���)
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
	//1.���帨������
	double dXmin, dYmin, dXmax, dYmax;
	int    nLineSize, nColSize;  //������
	double dXdelt, dYdelt; //�ֱ���x��y����Ŀ�Ⱦ���
	double dDx, dDy;       //ÿһС��ĳ�������
						   //2.����Ӧ�ֿ�
	Queue *queuehead;
	//3.��ȡ���ĵ�
	MyPoint *PointCenter;
	int CenterSize;
	//ͼ����ʾ����
	bool m_displayGrid;

	//-------------------------------
	TRIANGLENODE *tinHead, *tinEnd, *tin;
	COLORREF colors[14];

public:
	///////////////////////����Ӧ����//////////////////////////////////////////////
	void DrawGrid(CDC* pDC);
	void DrawBinaryLeaf(CDC* pDC);
	//��ʼ��դ��
	void InitialGrid(int &Line, int &Col, int &ndivision);
	//����դ��
	GridField** CraeteGridField(MyPoint *Data, int num, int Line, int Col, double dxmin, double dymin, double ddx, double ddy);
	//����Ӧ����
	Queue* CreateQueue(GridField **GridArray, int Line, int Col, int ndivision, int num);
	BinaryTree* GreateTreeNode(int startLine, int endLine, int startCol, int endCol, int pNum, int level);
	Queue* GreateQueueElement(BinaryTree *element);
	void AdaptiveDivision(Queue *&head, Queue *&rear, int ndivision, GridField **GridArray, Queue *List);
	//��ȡÿ������ĵ�
	void AbstractCenterPoint(GridField **GridArray, Queue *head, MyPoint *&Data, int &num, MyPoint *OriginalData);
	void InsertElementForCenter(QueueForCenter *&rear, QueueForCenter *insertelement, GridField **GridArray);
	QueueForCenter* GreateElementForCenter(int L, int C);
	//ɾ���ڴ�
	void ClearGridFieldMemory(GridField **GridArray, int Line, int Col);
	void ClearQueueMemory(Queue *head, MyPoint *Data);
	//////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////
	//2.��������������
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
	void TreeInsert(MyPoint *Point, TREENODE *p, TRIANGLENODE *tin, int no, int flag);    //������root
	int HaveSameEdgeScan(int P1, int P2, int P3, int PA, int PB, int PC, int *Q1, int *Q2);
	int SameEdge(int P1, int P2, int PA, int PB);
	int PointUpLine(MyPoint *Point, int a, int b, int c);  //��c��ֱ��ab�Ϸ��ͷ���
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

// ��д
public:
	virtual void OnDraw(CDC* pDC);  // ��д�Ի��Ƹ���ͼ
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// ʵ��
public:
	virtual ~CTINConstruction2View();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// ���ɵ���Ϣӳ�亯��
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnReadShapefile();
	void OnTINNoGroup();
	afx_msg void OnTinGenerate();
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
	void TopologyConstruct();
	afx_msg void CreateTriPath();
	void CreateLinePath();
	void OnPathConstruction();
	afx_msg void OnEndPNT();
	afx_msg void OnTinDensify();
};

#ifndef _DEBUG  // TINConstruction2View.cpp �еĵ��԰汾
inline CTINConstruction2Doc* CTINConstruction2View::GetDocument() const
   { return reinterpret_cast<CTINConstruction2Doc*>(m_pDocument); }
#endif

