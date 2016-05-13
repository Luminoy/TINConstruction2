
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
	HCURSOR  m_hZoomIn;	//�Ŵ�ʱ�Ĺ��
	HCURSOR  m_hZoomOut;	//��Сʱ�Ĺ��
	HCURSOR  m_hPan;
	HCURSOR  m_hSelect;
	PNT  ptMouse, ptMouseTmp;
	BOOL   Captured;
	//-----------------------------------------------
	double ClientWidth;    //�ͻ�����͸�
	double ClientHeight;
	double zoomratio, xcenter, ycenter; //ͼ�ε����ġ��Ŵ�ϵ��
	double xmin, ymin, xmax, ymax, dx, dy; //ͼ����ʾʱ����С�������(xmin,ymin)��(xmax,ymax)

	bool nFlagPoint, nFlagArc;
	double xminPoint, yminPoint, xmaxPoint, ymaxPoint;
	double xminArc, yminArc, xmaxArc, ymaxArc;
	//-----------------------------------------------   
	//2.���ݳ�Ա����
	long pointNumber;  //��ɢ��ĸ���
	PointSet *PointData;
	PointSet *CopyData;
	int arcNumber;    //��������
	ArcSet *Arc;
	HANDLE* m_hMutex;   //�����߳�
	double runTime[6];    //����ʱ��
	int iBlock;        //�ֿ��
	BlockTin *blockTin;
	TRIANGLE **TinSave;
	int BlockTotal;
	CThreadParam* param;
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
	void DrawPoint(CDC* pDC, PointSet *Data, int size, bool bFlag = true);
	void RefreshPoint(CDC *pDC, double x, double y, bool bFlag);
	void DrawArc(CDC* pDC);
	void RefreshARC(CDC *pDC, ArcSet arc);
	void RefreshLine(CDC *pDC, PNT pt1, PNT pt2);
	bool isIn_Qujian(double x, double x1, double x2);
	void DeleteVertex();
	void LOPScan_BlockTIN(TRIANGLE * p, TRIANGLE ** tinfirst, PointSet * OriginalData);
	void GetBlockGridInfo();
	int OnLeft(double x, double y, PointSet P1, PointSet P2);
	void EdgeChange(int a, int d, int b, int c, TRIANGLE **t, TRIANGLE **t1);
	void AddNewTin(int a, int b, int c, TRIANGLE *p, TRIANGLE **tinlast, PointSet *original);
	int TheOtherPoint(int p1, int p2, int a, int b, int c);
	void SetTriInfor(TRIANGLE* tri, TRIANGLE *blockTri, int e1, int e2);
	void RefreshBlockInfor(BlockTin* block, TRIANGLE tri);
	void SetBlockTri(BlockTin* block, TRIANGLE* blockTri, TRIANGLE tri, int no, int* found);

	// Operations
private:
	//1.���帨������
	double dXmin, dYmin, dXmax, dYmax;
	int nLineSize, nColSize;  //������
	double dXdelt, dYdelt; //�ֱ���x��y����Ŀ�Ⱦ���
	double dDx, dDy;       //ÿһС��ĳ�������
						   //2.����Ӧ�ֿ�
	Queue *queuehead;
	//3.��ȡ���ĵ�
	PointSet *PointCenter;
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
	GridField** CraeteGridField(PointSet *Data, int num, int Line, int Col, double dxmin, double dymin, double ddx, double ddy);
	//����Ӧ����
	Queue* CreateQueue(GridField **GridArray, int Line, int Col, int ndivision, int num);
	BinaryTree* GreateTreeNode(int startLine, int endLine, int startCol, int endCol, int pNum, int level);
	Queue* GreateQueueElement(BinaryTree *element);
	void AdaptiveDivision(Queue *&head, Queue *&rear, int ndivision, GridField **GridArray, Queue *List);
	//��ȡÿ������ĵ�
	void AbstractCenterPoint(GridField **GridArray, Queue *head, PointSet *&Data, int &num, PointSet *OriginalData);
	void InsertElementForCenter(QueueForCenter *&rear, QueueForCenter *insertelement, GridField **GridArray);
	QueueForCenter* GreateElementForCenter(int L, int C);
	//ɾ���ڴ�
	void ClearGridFieldMemory(GridField **GridArray, int Line, int Col);
	void ClearQueueMemory(Queue *head, PointSet *Data);
	//////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////
	//2.��������������
	void DrawTin(CDC *pDC, PointSet *OriginalData);
	void swap(PointSet r[], int i, int j);
	void qs_x(PointSet *point, int left, int right);
	int Partition(PointSet r[], int first, int end);
	void QuickSort(PointSet r[], int first, int end);
	int TheNumber(int m, int n, int a, int b, int c);
	void SetInitialScan(PointSet *Point, int pNum, TRIANGLE **tinfirst, TRIANGLE **tinlast, int &g_pointmark, TREENODE **root, TREENODE **frontf, TREENODE **frontl, FB **basef, FB **basel);
	void AddFirstTin(TRIANGLE **tinfirst, TRIANGLE **tinlast, int a, int b, int c);
	void FirAddTin(int a, int b, int c, TRIANGLE **p, TRIANGLE **tinlast);
	void AddNewTin(int a, int b, int c, TRIANGLE *p, TRIANGLE **tinlast);
	void AddNewPointer(TRIANGLE *tinNode1, TRIANGLE *tinNode2);
	void CreateTinBy_Scan(PointSet *Point, int pNum, TRIANGLE **tinlast, int &g_pointmark, TREENODE **root, TREENODE **frontf, TREENODE **frontl, FB **basef, FB **basel);
	void Circs2(int i, PointSet *Point, TRIANGLE **tinlast, TREENODE **Root, TREENODE **frontf, TREENODE **frontl, FB **basef, FB **basel);
	void Circs1(int i, PointSet *Point, TREENODE *p, TRIANGLE **tinlast, TREENODE **Root, FB **basel, FB **baser);
	void Circs1(int i, PointSet *Point, TREENODE *p, TRIANGLE **tinlast, TREENODE **Root);
	void TreeSearch(PointSet *Point, TREENODE **root, TREENODE **f, int no);
	void TreeDelete(TREENODE *p, int no, TREENODE **Root);
	void TreeInsert(PointSet *Point, TREENODE *p, TRIANGLENODE *tin, int no, int flag);    //������root
	int HaveSameEdgeScan(int P1, int P2, int P3, int PA, int PB, int PC, int *Q1, int *Q2);
	int SameEdge(int P1, int P2, int PA, int PB);
	int PointUpLine(PointSet *Point, int a, int b, int c);  //��c��ֱ��ab�Ϸ��ͷ���
	void EndFrontLine(PointSet *Point, TRIANGLE **tinlast, TREENODE **frontf, TREENODE **Root);
	void LOPScan_Nonrecursion(TRIANGLE *p, TRIANGLE **tinfirst, PointSet *OriginalData);
	void Lop_all(TRIANGLE **t, LIST **rear1, LIST *rear2, PointSet *OriginalData);
	int CorrespondingTriangle(int a, int b, int c, int *d, TRIANGLE *t, TRIANGLE **t2);
	int PointInCircle(PointSet *OriginalData, int P0, int P1, int P2, int P3);
	void SetTinScan_all(int a, int d, int b, int c, TRIANGLE **t, TRIANGLE **t1, LIST **rear1, LIST *rear2);
	void CircleBy3Points(PointSet P1, PointSet P2, PointSet P3, double * ox, double * oy, double * RR);
	void GeneDelaunay(PointSet *Point, int pNum, TRIANGLE **tinfirst, TRIANGLE **tinlast, TREENODE **root, TREENODE **frontf, TREENODE **frontl, FB **basef, FB **basel);
	void InitialCenterPointset(PointSet *Data, int &DataSize, PointSet *OriginalData, long &OriginalSize);
	void GetCurrentBlockInfor(BlockTin &blocktin, TRIANGLE *TinSave, PointSet *Point, TREENODE *frontf, FB *basef);
	void ClearPartionMemory(TREENODE *treefirst, FB *fbfirst);
	void ClearTinMemory(TRIANGLE **tin);
	int OnLeft(PointSet P, PointSet P1, PointSet P2);
	void SetBlockPointer(TRIANGLE *t, BlockTin *block);
	void DrawBlockTin(TRIANGLE *t, PointSet *OriginalData);

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
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
};

#ifndef _DEBUG  // TINConstruction2View.cpp �еĵ��԰汾
inline CTINConstruction2Doc* CTINConstruction2View::GetDocument() const
   { return reinterpret_cast<CTINConstruction2Doc*>(m_pDocument); }
#endif

