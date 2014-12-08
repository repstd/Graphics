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
    ~CMatrix(){if(data!=NULL) delete data; data = NULL;}
	
	T& operator()(int i, int j) const
	{ 
		return data[i*col + j];
	
	};
	void SetData(T *pData)
	{	
		memcpy(data, pData, row*col*sizeof(T));
	}
	void SetData(T *pData,int srcOffsetX,int srcOffsetY,int srcWidth,int SrcHeight,int dstWidth,int dstHeight)
	{

		for (int y = 0; y < dstHeight; y++)
		{

			memcpy(data+srcOffsetY*dstWidth,pData + (srcOffsetY + y)*srcWidth+srcOffsetX,dstHeight)
		}
	}
	void Reset(int r, int c) const
	{

		if (data != NULL && (r != row || c != col) )
		{
			
			delete data;
			data = NULL;
		}
		row = r;
		col = c;
		if (data == NULL)
			data = new T[row*col];
	
		memset(data, 0, row*col);
	}
};
#endif //__MATRIX_H__