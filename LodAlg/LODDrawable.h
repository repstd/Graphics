#pragma 
#include "stdafx.h"
#include "Matrix.h"
#include "LODTiles.h"
#include <osg/Drawable>
#include <osg/RenderInfo>
#include "LODTiles.h"
#include <vector>
#include <assert.h>

class TileThread :
	public OpenThreads::Thread
{
public:
	TileThread() :
		OpenThreads::Thread()
	{
			m_pTile = new LODTile();
			Init();
	}
public:
	virtual void run()
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
	void BFSRender()
	{
		m_pTile->BFSRender();
	}

	LODTile* m_pTile;
	

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

private:


	void drawImplementation(osg::RenderInfo& renderInfo) const;


	std::vector<PTileThread> m_vecTile;
	std::vector<Range> m_vecRange;
	Range m_rglobalPara;


	

};

