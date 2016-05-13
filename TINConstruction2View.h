
// TINConstruction2View.h : CTINConstruction2View 类的接口
//

#pragma once


class CTINConstruction2View : public CView
{
protected: // 仅从序列化创建
	CTINConstruction2View();
	DECLARE_DYNCREATE(CTINConstruction2View)

// 特性
public:
	CTINConstruction2Doc* GetDocument() const;

// 操作
public:

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
	PointSet *PointData; 
	int pointNumber;
// 生成的消息映射函数
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnReadShapefile();
	void CalcBoundPoint();
};

#ifndef _DEBUG  // TINConstruction2View.cpp 中的调试版本
inline CTINConstruction2Doc* CTINConstruction2View::GetDocument() const
   { return reinterpret_cast<CTINConstruction2Doc*>(m_pDocument); }
#endif

