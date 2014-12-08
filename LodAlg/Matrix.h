#ifndef __MATRIX_H__
#define __MATRIX_H__

#include <memory>

template <class T>
class CMatrix
{
private:
	mutable int row, col;
	mutable T * data;
	
public:
	CMatrix():row(0),col(0),data(NULL){}
	//CMatrix(int r, int c):row(r),col(c),data(new T[r*c]){}
	//CMatrix(int r, int c, T* pData):row(r),col(c),data(new T[r*c])
	//{
	//	memcpy(data, pData, row*col*sizeof(T));
	//}
    ~CMatrix(){if(data!=NULL) delete data; data = NULL;}
	
	T& operator()(int i, int j) const
	{ 
		return data[i*col + j];
	
	};
	void SetData(T *pData){	memcpy(data, pData, row*col*sizeof(T)); }
	void Reset(int r, int c) const
	{
		if (data == NULL)
		{
			row = r;
			col = c;
			data = new T[row*col];

		}
		memset(data, 0, row*col);
	}
	void Reset(int r, int c, T* pData){Reset(r,c); SetData(pData);}
};
#endif //__MATRIX_H__