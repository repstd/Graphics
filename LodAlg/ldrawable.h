#pragma once
#include "stdafx.h"
#include "Matrix.h"
#include "ltiles.h"
#include <osg/Drawable>
#include <osg/GLBeginEndAdapter>
#include <osg/RenderInfo>
#include <OpenThreads/Thread>
#include "ltiles.h"
#include "lthreads.h"
#include <vector>
#include <assert.h>


typedef TileThreadW TileThread;
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
	void initRaw(const char* heightFiledMap);
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
	std::vector<std::auto_ptr<TileThread>> m_vecTile;
	std::vector<Range> m_vecRange;
	Range m_rglobalPara;
};
