#pragma once
#include "stdafx.h"
#include "Matrix.h"
#include "LODTiles.h"
#include <osg/Drawable>
#include <osg/GLBeginEndAdapter>
#include <osg/RenderInfo>
#include <OpenThreads/Thread>
#include "LODTiles.h"
#include <vector>
#include <assert.h>

class TileThread : 
	public OpenThreads::Thread,
	private LODTile
{
public:
	TileThread():
		OpenThreads::Thread(),
		LODTile()
	{
			
	}
public:
	virtual void run()
	{
		m_pTile.BFSRender();
		return;
	
	}
	void init(BYTE* heightMat, const Range globalRange, const Range localRange)
	{
		m_pTile.init(heightMat, globalRange, localRange);
	}
	void updateCameraInfo(osg::Vec3d& eye) const
	{
	
		m_pTile.updateCameraInfo(eye);
	}

	void updateCameraInfo(osg::Vec3d& eye,osg::GLBeginEndAdapter& gl,osg::State* stat) const
	{
	
		m_pTile.updateCameraInfo(eye,gl,stat);
	}
	void BFSRender() const
	{
		m_pTile.BFSRender();
	}

	int getHeight(int x, int y)
	{
	
		return m_pTile.GetAveHeight(x, y);
	}
private:
	mutable LODTile m_pTile;
	

};

typedef TileThread* PTileThread;
class LODDrawable :public osg::Drawable
{
public:

	LODDrawable(const LODDrawable& drawable, const osg::CopyOp    &copyop = osg::CopyOp::SHALLOW_COPY) 
		:Drawable(drawable, copyop)
	{
	}
	META_Object(osg, LODDrawable);

	LODDrawable();
	~LODDrawable();

	void init(const char* heightFiledMap);
	int getFieldHeight(int index,int x, int y)
	{
		if (index >= m_vecTile.size() || index < 0)
			return _LOD_ERROR;
		if (x < 0)
			x = 0;
		if (y < 0)
			y = 0;
		x %= m_vecRange[index]._width;
		y %= m_vecRange[index]._height;
		return m_vecTile[index]->getHeight(x, y);
	}
	Range getLODRange()
	{
	
		return m_rglobalPara;
	}
private:


	void drawImplementation(osg::RenderInfo& renderInfo) const;
	std::vector<PTileThread> m_vecTile;
	std::vector<Range> m_vecRange;
	Range m_rglobalPara;
	



};

