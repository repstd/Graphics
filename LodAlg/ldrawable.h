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
class lodImp
{
public:	
	lodImp();
	virtual void init(heightField* input){ return; }
	virtual void init(const char* heightField){ return; }
	virtual void drawImplementation(osg::RenderInfo& renderInfo) const = 0;;
	virtual PTileThread getTile(int index){ return NULL; }
	Range getLODRange();
	void setGlobalRange(Range range);
private:
	Range m_rglobalPara;
};

class quadTreeImp :public lodImp
{
public:
	virtual void init(heightField* input);
	virtual void drawImplementation(osg::RenderInfo& renderInfo) const;
	virtual PTileThread getTile(int index);
protected:
	quadTreeImp();
	std::vector<std::auto_ptr<TileThread>> m_vecTile;
	std::vector<Range> m_vecRange;
	friend class lodImpFactory;
};

class lodImpFactory
{
public:
	static lodImpFactory* instance();
	lodImp* createQuadTreeImp();
protected:
	lodImpFactory();
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
	LODDrawable(lodImp*);
	~LODDrawable();
	void init(const char* heightFiledMap);
	void init(heightField* input);
	void setImp(lodImp* imp);
	lodImp* getImp() const;
	int getFieldHeight(int index, int x, int y);
	Range getLODRange();
private:
	void drawImplementation(osg::RenderInfo& renderInfo) const;
	std::unique_ptr<lodImp> m_implementation;
};
