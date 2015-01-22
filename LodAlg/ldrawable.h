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
#include "linput.h"

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
	void init(heightField* input);
	int getFieldHeight(int index, int x, int y);
	Range getLODRange();
private:
	void drawImplementation(osg::RenderInfo& renderInfo) const;
	std::vector<std::auto_ptr<TileThread>> m_vecTile;
	std::vector<Range> m_vecRange;
	Range m_rglobalPara;
};
