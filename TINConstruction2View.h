
// TINConstruction2View.h : CTINConstruction2View ��Ľӿ�
//

#pragma once


class CTINConstruction2View : public CView
{
protected: // �������л�����
	CTINConstruction2View();
	DECLARE_DYNCREATE(CTINConstruction2View)

// ����
public:
	CTINConstruction2Doc* GetDocument() const;

// ����
public:

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
	PointSet *PointData; 
	int pointNumber;
// ���ɵ���Ϣӳ�亯��
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnReadShapefile();
	void CalcBoundPoint();
};

#ifndef _DEBUG  // TINConstruction2View.cpp �еĵ��԰汾
inline CTINConstruction2Doc* CTINConstruction2View::GetDocument() const
   { return reinterpret_cast<CTINConstruction2Doc*>(m_pDocument); }
#endif

