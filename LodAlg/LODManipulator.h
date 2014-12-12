#pragma once
#include <osg/Object>
#include <osgGA/CameraManipulator>
#include <osgGA/StandardManipulator>
#include <osgGA/GUIEventAdapter>
#include <osgGA/GUIActionAdapter>
#include <osg/Matrix>
#include "LODDrawable.h"
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

class LODManipulator :public osgGA::CameraManipulator
{
public:


	LODManipulator();
	LODManipulator(LODDrawable* lod);
	~LODManipulator();


public:

	LODManipulator(const LODManipulator& fpm,
		const osg::CopyOp& copyOp = osg::CopyOp::SHALLOW_COPY)
	{
	
	}
	virtual void setByMatrix(const osg::Matrixd& matrix);

	virtual void setByInverseMatrix(const osg::Matrixd& matrix);

	virtual osg::Matrixd getMatrix() const;

	virtual osg::Matrixd getInverseMatrix() const;

	virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us);

	bool handleMouseMove(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us);
	bool handleMouseDrag(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us);
	bool handleMousePush(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us);
	bool handleMouseRelease(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us);
	bool handleKeyDown(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us);
	bool handleKeyUp(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us);
private:
	void initPara();
	osg::Matrixd m_viewMatrix;
	osg::Matrixd m_projMatrix ;
	osg::Matrixd m_RotMatrix;
	osg::Matrixd m_TransformMatrix;
	
	int m_posX;
	int m_posY;
	int m_posZ;
	int m_velocityX;
	int m_velocityY;
	LODDrawable* m_LOD;
};

