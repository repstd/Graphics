#pragma 
#include "stdafx.h"
#include "Matrix.h"
#include <osg/Drawable>
#include <osg/RenderInfo>
#include "LODTiles.h"
#include <vector>
#include <assert.h>


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

	mutable std::vector<LODTile> m_vecTile;
	std::vector<Range> m_vecRange;
	Range m_rglobalPara;


	

};

