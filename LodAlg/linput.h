#pragma once
#include <gdal.h>
#include <gdal_alg.h>
#include <gdal_priv.h>
#include <osgDB/Registry>
#include <osgDB/WriteFile>

typedef struct _Range Range;

typedef struct _extent
{
	int _width;
	int _height;
	int _channel;
} extent;

class texImp
{
public:
	virtual void load(const char* imgName)= 0;
	virtual void bind() = 0;
	virtual void unbind() = 0;
};
class dataImp
{
public:
	virtual void load(const char* filename,const char* driverName=NULL) = 0;
	virtual void getExtent(extent&) = 0;
	virtual extent getExtent() = 0;
	virtual void getTile(BYTE*, BYTE*, int, int, int) = 0;
	virtual void getTile(BYTE*, int, int, int){ return; }

	void flipBuffer(BYTE* src,int width,int height);

protected:
	dataImp() { return; }
};

class rawData : public dataImp
{
public:
	rawData();
	virtual void load(const char* filenam,const char* driverName=NULL);
	virtual void getExtent(extent& extent);
	//Generate a tile of index(i,j) from a  N by N grid.
	virtual void getTile(BYTE* src, BYTE* dst, int row, int col, int N);
	virtual extent getExtent();
private:
	LONG m_iSize;
	int m_iWidth, m_iHeight;
};

class rawDataProxy :public dataImp
{
public:
	virtual void load(const char* filenam,const char* driverName=NULL);
	virtual void getExtent(extent& extent);
	//Generate a tile of index(i,j) from a  N by N grid.
	virtual void getTile(BYTE* src, BYTE* dst, int row, int col, int N);
	virtual void getTile(BYTE* dst, int row, int col, int N);
	extent getExtent();
	const char* getFilename();
	rawData* getInputData();
	virtual ~rawDataProxy();
protected:
	rawDataProxy();
private:
	std::unique_ptr<rawData> m_rawData;
	std::string m_filename;
	friend class dataImpFactory;
};
class gdalData :public dataImp
{
public:
	gdalData();
	virtual void load(const char* filename,const char* driverName);
	virtual void getExtent(extent& extent);
	//Generate a tile of index(i,j) from a  N by N grid.
	virtual void getTile(BYTE* src, BYTE* dst, int row, int col, int N);
	virtual extent getExtent();
	void clip();
	void setSize(int width,int height);
private:
	LONG m_iSize;
	int m_iRasterWidth, m_iRasterHeight;
	int m_iRasterCnts;
};

class gdalDataProxy :public dataImp
{

public:
	virtual void load(const char* filename,const char* driverName);
	virtual void getExtent(extent& extent);
	//Generate a tile of index(i,j) from a  N by N grid.
	virtual void getTile(BYTE* src, BYTE* dst, int row, int col, int N);
	virtual void getTile(BYTE* dst, int row, int col, int N);
	extent getExtent();
	const char* getFilename();
	const char* getDrivername();
	gdalData* getInputData();
	void clip();
	void setSize(int width,int height);
protected:
	gdalDataProxy();
private:
	std::unique_ptr<gdalData> m_gdalData;
	std::unique_ptr<GDALDriver> m_driver;
	std::string m_filename;
	std::string m_driverName;
	friend class dataImpFactory;
};

class glTexImp :public texImp
{
public:
	glTexImp();
	glTexImp(const char* imgName);
	~glTexImp();
	virtual void load(const char* imgName);
	virtual void bind();
	virtual void unbind();
	void loadBmp(const char* filename);
	int getWidth();
	int getHeight();
	GLuint getTexId();
private:
	GLuint m_texId;
	int m_width;
	int m_height;
};
/*combine the mesh and texture
*/
class terrainImp:public rawDataProxy
{
public:
	texImp* getTexture();
	virtual ~terrainImp();
protected:
	terrainImp(texImp* imp);
private:
	std::unique_ptr<texImp> m_texture;
	friend class dataImpFactory;
};
class dataImpFactory
{
public:
	enum _data_type
	{
		_RAW,
		_GDAL_SUPPORTED,
		_RAW_BMP
	};
	static dataImpFactory* instance();
	dataImp* create(int type, const char* fileneme, const char* driverName = NULL);
	dataImp* createRawImp(const char* filename);
	dataImp* createGDALImp(const char* filename, const char* driverName);
	dataImp* createBmpTerrainImp(const char* meshFilename, const char* bmpFilename);
protected:
	dataImpFactory(){ return; }
};

class heightField
{
public:
	heightField(dataImp* input);
	void setImp(dataImp* imp);
	dataImp* getImp();
	int getWidth();
	int getHeight();
	int getChannel();
	int getCenterX();
	int getCenterY();
	int getTileWidth(int i, int j, int N);
	int getTileHeight(int i, int j, int N);
	int getTileCenterX(int i, int j, int N);
	int getTileCenterY(int i, int j, int N);
	void generateTile(int i, int j, int N, BYTE* dst, Range& tileRange);
	void generateTile(int i, int j, int N, BYTE* dst, Range& tileRange,Range& globalRange);
private:
	std::unique_ptr<dataImp> m_dataInputImp;
};
