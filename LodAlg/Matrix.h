#ifndef __MATRIX_H__
#define __MATRIX_H__

#include <memory>

template <class T>
class CMatrix
{

public:
	CMatrix() :_width(0), _height(0), data(NULL){}
    ~CMatrix(){if(data!=NULL) delete data; data = NULL;}
	
	T& operator()(int w, int h) const
	{ 
		return data[h*_width + w];
	
	};
	void SetData(T *pData)
	{	
		memcpy(data, pData, _width*_height*sizeof(T));
	}
	void SetData(T *pData,int srcOffsetX,int srcOffsetY,int srcWidth,int SrcHeight,int dstWidth,int dstHeight)
	{

		for (int y = 0; y < dstHeight; y++)
		{
			memcpy(data + y*dstWidth, pData + ((srcOffsetY + dstHeight - 1 - y)*srcWidth + srcOffsetX)*sizeof(T), dstWidth*sizeof(T));
		}
	}
	void Reset(int w, int h) const
	{

		if (data != NULL && (w != _width || h != _height))
		{
			
			delete data;
			data = NULL;
		}
		_width = w;
		_height = h;
		if (data == NULL)
			data = new T[_width*_height];
	
		memset(data, 0, _width*_height);
	}
private:
	mutable int _width, _height;
	mutable T * data;

};
#endif //__MATRIX_H__