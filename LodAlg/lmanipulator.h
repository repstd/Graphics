#pragma once
#include <osgGA/FirstPersonManipulator>
#include "ldrawable.h"
#define _DEBUG_LOG_FILENAME_MANI "./data/log_manipulator.txt"
#define _DEBUG_LOG_INIT_MANI _DEBUG_LOG_INIT(_DEBUG_LOG_FILENAME_MANI)
#define _DEBUG_ENCODE_MSG_MANI(format,data,...)  _DEBUG_ENCODE_MSG(_DEBUG_LOG_FILENAME_MANI,format,data) 

#define _MAT_FMT "%f,\t%f,\t%f,\t%f\n"
#define _MAT_ROW(mat,i) mat.ptr()[4*i],mat.ptr()[4*i+1],mat.ptr()[4*i+2],mat.ptr()[4*i+3]
#define _LOG_MATRIX(mat,desc)\
{\
	_DEBUG_ENCODE_MSG_MANI("%s\n", desc); \
	_DEBUG_ENCODE_MSG_MANI(_MAT_FMT, _MAT_ROW(mat, 0)); \
	_DEBUG_ENCODE_MSG_MANI(_MAT_FMT, _MAT_ROW(mat, 1)); \
	_DEBUG_ENCODE_MSG_MANI(_MAT_FMT, _MAT_ROW(mat, 2)); \
	_DEBUG_ENCODE_MSG_MANI(_MAT_FMT, _MAT_ROW(mat, 3)); \
}

class Manipulator : 
	public osgGA::FirstPersonManipulator
{
public:
	Manipulator(LODDrawable* lod);
	~Manipulator();
protected:
	virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us);

private:	
	LODDrawable* m_LOD;
	int m_velocityX;
	int m_velocityY;
	int m_posX;
	int m_posY;
	int m_posZ;
};

