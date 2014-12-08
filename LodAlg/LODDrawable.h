#pragma 
#include "stdafx.h"
#include "Matrix.h"
#include "LODTiles.h"
#include <osg/Drawable>
#include <osg/RenderInfo>
#include "LODTiles.h"
#include <vector>
#include <assert.h>
#include <boost/thread.hpp>
//class TileThread :
//	public OpenThreads::Thread
//{
//public:
//	TileThread() :
//		OpenThreads::Thread()
//	{
//			m_pTile = new LODTile();
//			Init();
//	}
//public:
//	virtual void run()
//	{
//	
//		m_pTile->BFSRender();
//		return;
//	
//	}
//	void init(BYTE* heightMat, const Range globalRange, const Range localRange)
//	{
//		m_pTile->init(heightMat, globalRange, localRange);
//	}
//	void updateCameraInfo(osg::Vec3d& eye)
//	{
//	
//		m_pTile->updateCameraInfo(eye);
//	}
//
//	LODTile* m_pTile;
//	
//
//};
class TileThread
{
public:
	TileThread()

	{
			m_pTile = new LODTile();
			
	}
public:
	void operator()()
	{
		m_pTile->BFSRender();
		return;
	
	}
	void init(BYTE* heightMat, const Range globalRange, const Range localRange)
	{
		m_pTile->init(heightMat, globalRange, localRange);
	}
	void updateCameraInfo(osg::Vec3d& eye)
	{
	
		m_pTile->updateCameraInfo(eye);
	}

	LODTile* m_pTile;
	

};

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

private:


	void drawImplementation(osg::RenderInfo& renderInfo) const;
	

	mutable std::vector<TileThread> m_vecTile;
	std::vector<Range> m_vecRange;
	Range m_rglobalPara;


	

};

