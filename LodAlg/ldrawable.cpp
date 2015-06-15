#include "StdAfx.h"
#include "ldrawable.h"
#include <vector>
#include <queue>
#include <list>
#include <osg/Matrix>
#include <osgDB/ReaderWriter>
#include <stdlib.h>
#include <stdio.h>
#include <gdal.h>
#include <gdal_alg.h>
#include <gdal_priv.h>
#include <osgDB/Registry>
#include <osgDB/WriteFile>
#include "lmanipulator.h"
#include <typeinfo>
#define MAX_DIS 700.0
void saveBMP(int width, int height, int channel, BYTE* data, char* filename, int pixelFormat = GL_BGR)
{
	osg::ref_ptr<osgDB::ReaderWriter> m_bmpWirter;

	m_bmpWirter = osgDB::Registry::instance()->getReaderWriterForExtension("mp4");


	osg::ref_ptr<osg::Image> img = new osg::Image();
	img->allocateImage(width, height, channel, pixelFormat, GL_BYTE);
	BYTE* p = img->data(0, 0);
	memcpy(p, data, width*height * channel);

	m_bmpWirter->writeImage(*img, filename);
}
lodImp::lodImp()
{
}
Range lodImp::getLODRange()
{
	return m_rglobalPara;
}
void lodImp::setGlobalRange(Range range)
{
	m_rglobalPara = range;
}
lodImpFactory::lodImpFactory()
{
	return;
}

lodImpFactory* lodImpFactory::instance()
{
	static lodImpFactory inst;
	return &inst;
}
lodImp* lodImpFactory::createQuadTreeImp()
{
	std::unique_ptr<quadTreeImp> qtreeAlg(new quadTreeImp);
	return qtreeAlg.release();
}

quadTreeImp::quadTreeImp() :lodImp()
{

}
void quadTreeImp::init(heightField* input)
{ 

	Range global;
	global._width = input->getWidth();
	global._height = input->getHeight();
	global._centerX = input->getCenterX();
	global._centerY = input->getCenterY();

	setGlobalRange(global);
	int index;
	int N = 1;

	for (int i = 0; i < N; i++)
	{
		for (int j = 0;j < N; j++)
		{
			PTileThread	pTile = new TileThread();
			pTile->init(input, i, j, N);
			m_vecTile.push_back(std::auto_ptr<TileThread>(pTile));
			m_vecRange.push_back(pTile->getLocalRange());
		}
	}
	return;
	//delete[] tempHeightMap;

}
void quadTreeImp::drawImplementation(osg::RenderInfo& renderInfo) const
{

	osg::GLBeginEndAdapter& gl = renderInfo.getState()->getGLBeginEndAdapter();

	osg::ref_ptr<osg::Camera> camera = renderInfo.getCurrentCamera();

	osg::Vec3d eye, at, up;
	camera->getViewMatrixAsLookAt(eye, at, up, 2.0);
	_LOG_MATRIX(camera->getViewMatrix(), "drawImplementation");

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);

	gluLookAt(eye.x(), eye.y(), eye.z(), at.x(), at.y(), at.z(), up.x(), up.y(), up.z());

	int sz = m_vecTile.size();
	osg::ref_ptr<osg::DisplaySettings> ds = camera->getDisplaySettings();
	for (int i = 0; i < sz; i++)
	{
		m_vecTile[i]->updateCameraInfo(eye, gl, renderInfo.getState());
#ifdef _GL_MT
		m_vecTile[i]->run();
#else
		m_vecTile[i]->BFSRender();
#endif
		//m_vecTile[i]->start();
	}
#ifdef _GL_MT
	for (int i = 0; i < sz; i++)
	{
		m_vecTile[i]->DrawIndexedPrimitive();
	}
	int isStopped = false;
	while (1)
	{
		isStopped = true;
		for (int i = 0; i < sz; i++)
		{
			if (m_vecTile[i]->isRunning())
			{
				isStopped = false;
				break;
			}
		}
		if (isStopped)
			break;
	}
#endif

}
PTileThread quadTreeImp::getTile(int index)
{
	if (index < 0 || index >= m_vecTile.size())
		index = 0;
	return m_vecTile[index].get();
}
LODDrawable::LODDrawable()

{
	setSupportsDisplayList(false);
	glewInit();

}
LODDrawable::LODDrawable(lodImp* imp)
{
	setSupportsDisplayList(false);
	glewInit();

	setImp(imp);
}
LODDrawable::~LODDrawable()
{

}

void LODDrawable::setImp(lodImp* imp)
{
	std::unique_ptr<lodImp> temp(imp);
	m_implementation = std::move(temp);
	temp.release();
}
lodImp* LODDrawable::getImp() const
{
	return m_implementation.get();
}
void LODDrawable::init(heightField* input)
{
	getImp()->init(input);
}


void LODDrawable::init(const char* filename)
{
	getImp()->init(filename);
}


int LODDrawable::getFieldHeight(int index, int x, int y)
{
	Range region = getImp()->getTile(index)->getLocalRange();
	int rx = x%region._width;
	int ry = y%region._height;

	return getImp()->getTile(index)->getHeight(rx, ry);
}

Range LODDrawable::getLODRange()
{
	return getImp()->getLODRange();
}
void LODDrawable::drawImplementation(osg::RenderInfo& renderInfo) const
{

	getImp()->drawImplementation(renderInfo);

}


