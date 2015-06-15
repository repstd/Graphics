#include "stdafx.h"
#include "linput.h"
#include <windows.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include "ltiles.h"
#include <cstdint>
void dataImp::flipBuffer(BYTE* src, int width, int height)
{
	BYTE* temp = new unsigned char[width];
	int top = 0;
	int bottom = height - 1;
	while (bottom > top)
	{
		memcpy(temp, src + top*width, width);
		memcpy(src + (top++)*width, src + bottom*width, width);
		memcpy(src + (bottom--)*width, temp, width);
	}

}
rawData::rawData() :dataImp()
{
}
void rawData::load(const char* filename, const char* driverName)
{
	FILE* fRaw = fopen(filename, "r");
	assert(fRaw != NULL);
	long sz;
	fseek(fRaw, 0, std::ios::end);
	sz = ftell(fRaw);
	fseek(fRaw, 0, std::ios::beg);
	fclose(fRaw);
	m_iSize = sz;
	m_iWidth = sqrt(double(sz));
	m_iHeight = m_iWidth;
}
void rawData::getExtent(extent& extent)
{
	extent._width = m_iWidth;
	extent._height = m_iHeight;
	extent._channel = 1;
}
void rawData::getTile(BYTE* src, BYTE* dst, int row, int col, int N)
{
	assert(row < N && row>0);
	assert(col< N && col>0);
	assert(src != NULL&& dst != NULL);
	extent ext = getExtent();
	int channel = ext._channel;
	int srcWidth = ext._width;
	int tileWidth = (srcWidth - 1) / N + 1;
	int tileHeight = tileWidth;
	int tileWidthEven = srcWidth / N;
	int srcOffsetX = row*tileWidth;
	int srcOffsetY = (N - 1 - col)*tileHeight;
	for (int y = 0; y < tileHeight; y++)
		memcpy(dst + y*tileWidth, src + ((srcOffsetY + tileHeight - 1 - y)*srcWidth + srcOffsetX), tileWidth);
}
extent rawData::getExtent()
{
	extent ext;
	getExtent(ext);
	return ext;
}
void	rawDataProxy::load(const char* filename, const char* driverName)
{
	std::unique_ptr<rawData> temp(new rawData);
	m_rawData = std::move(temp);
	temp.release();
	getInputData()->load(filename);
	m_filename = filename;
}
void	rawDataProxy::getExtent(extent& extent)
{
	getInputData()->getExtent(extent);
}
void	rawDataProxy::getTile(BYTE* src, BYTE* dst, int row, int col, int N)
{
	getInputData()->getTile(src, dst, row, col, N);

}
void rawDataProxy::getTile(BYTE* dst, int row, int col, int N)
{
	FILE* fp = fopen(getFilename(), "r");
	extent ext = getExtent();
	BYTE* buf = new unsigned char[ext._width*ext._height*ext._channel];

	fread(buf, ext._width*ext._height*ext._channel, 1, fp);

	getTile(buf, dst, row, col, N);

	fclose(fp);
	delete[] buf;

}
extent  rawDataProxy::getExtent()
{
	return getInputData()->getExtent();
}
const char*   rawDataProxy::getFilename()
{
	return m_filename.c_str();
}
rawData* rawDataProxy::getInputData()
{
	return m_rawData.get();
}
rawDataProxy::rawDataProxy() :dataImp()
{

}
rawDataProxy::~rawDataProxy() {}

gdalData::gdalData() :dataImp()
{
}
void gdalData::load(const char* filename, const char* driverName)
{
	GDALDriver* driver = GetGDALDriverManager()->GetDriverByName(driverName);
	assert(driver != NULL);
	GDALDataset* src = (GDALDataset*)GDALOpen(filename, GA_ReadOnly);
	m_iRasterCnts = src->GetRasterCount();
	m_iRasterWidth = src->GetRasterXSize();
	m_iRasterHeight = src->GetRasterYSize();
	m_iSize = m_iRasterHeight*m_iRasterWidth*m_iRasterCnts;
	GDALDestroyDriver(driver);

}
void gdalData::getExtent(extent& extent)
{
	extent._width = m_iRasterWidth;
	extent._height = m_iRasterHeight;
	extent._channel = m_iRasterCnts;
}
void gdalData::getTile(BYTE* src, BYTE* dst, int row, int col, int N)
{
	assert(row < N && row>0);
	assert(col< N && col>0);
	assert(src != NULL&& dst != NULL);
	extent ext = getExtent();
	int channel = ext._channel;
	int srcWidth = ext._width;
	int tileWidth = (srcWidth - 1) / N + 1;
	memcpy(dst, src, tileWidth*tileWidth);

}
extent gdalData::getExtent()
{
	extent ext;
	getExtent(ext);
	return ext;
}

void gdalData::clip()
{
	//We only need a region whose size is (2^n+1)*(2^n+1)
	extent ext;
	getExtent(ext);
	int originalWidth = ext._width;
	int originalHeight = ext._height;
	float	m = log((float)originalWidth) / log(2.0f);
	float	n = log((float)originalHeight) / log(2.0f);

	int exp = ([](int a, int b){return a > b ? b : a; })(m, n);

	int cliped = pow(2.0, exp) + 1;

	setSize(cliped, cliped);

}
void gdalData::setSize(int width, int height)
{
	m_iRasterWidth = width;
	m_iRasterHeight = height;
}
void gdalDataProxy::load(const char* filename, const char* driverName)
{
	m_driverName = driverName;
	m_filename = filename;
	std::unique_ptr<GDALDriver> tempDriver(GetGDALDriverManager()->GetDriverByName(driverName));
	m_driver = std::move(tempDriver);
	tempDriver.release();
	std::unique_ptr<gdalData> tempGdalData(new gdalData);
	m_gdalData = std::move(tempGdalData);
	tempGdalData.release();
	getInputData()->load(filename, driverName);
	getInputData()->clip();
}
void gdalDataProxy::getExtent(extent& extent)
{
	getInputData()->getExtent(extent);
}
void gdalDataProxy::getTile(BYTE* src, BYTE* dst, int row, int col, int N)
{
	getInputData()->getTile(src, dst, row, col, N);
}

void gdalDataProxy::getTile(BYTE* dst, int row, int col, int N)
{

	GDALDataset* src = (GDALDataset*)GDALOpen(getFilename(), GA_ReadOnly);
	assert(src != NULL);
	extent ext = getExtent();
	int tileWidth = (ext._width - 1) / N + 1;
	int tileHeight = (ext._height - 1) / N + 1;

	int srcOffsetX = row*(tileWidth - 1);
	int srcOffsetY = (N - 1 - col)*(tileHeight - 1);

	BYTE* buf = (BYTE*)CPLCalloc(tileWidth*tileHeight * 1 * sizeof(BYTE), 1);

	CPLErr error = src->RasterIO(GF_Read, srcOffsetX, srcOffsetY, tileWidth, tileHeight, buf, tileWidth, tileHeight, GDT_Byte, 1, NULL, 1, 0, 1);

	flipBuffer(buf, tileWidth, tileHeight);

	getTile(buf, dst, row, col, N);

	GDALClose(src);
	CPLFree(buf);
}
extent gdalDataProxy::getExtent()
{
	return getInputData()->getExtent();
}
const char* gdalDataProxy::getFilename()
{
	return m_filename.c_str();
}
const char* gdalDataProxy::getDrivername()
{
	return m_driverName.c_str();
}

void gdalDataProxy::setSize(int width, int height)
{

	getInputData()->setSize(width, height);

}
void gdalDataProxy::clip()
{
	getInputData()->clip();
}
gdalData* gdalDataProxy::getInputData()
{
	return m_gdalData.get();
}
gdalDataProxy::gdalDataProxy() :dataImp()
{

}
dataImpFactory* dataImpFactory::instance()
{
	static dataImpFactory inst;
	return &inst;
}
dataImp* dataImpFactory::create(int type, const char* filename, const char* driverName)
{
	switch (type)
	{
	case _RAW:
		return createRawImp(filename);
	case _GDAL_SUPPORTED:
		return createGDALImp(filename, driverName);
	default:
		exit(0);
		break;
	}
}
dataImp* dataImpFactory::createRawImp(const char* filename)
{
	std::unique_ptr<rawDataProxy> raw(new rawDataProxy);
	raw->load(filename);
	return raw.release();
}
dataImp* dataImpFactory::createGDALImp(const char* filename, const char* driverName)
{
	assert(driverName != NULL);
	GDALAllRegister();
	std::unique_ptr<gdalDataProxy> gdal(new gdalDataProxy);
	gdal->load(filename, driverName);
	return gdal.release();
}
dataImp* dataImpFactory::createBmpTerrainImp(const char* meshFilename, const char* bmpFilename)
{
	std::unique_ptr<terrainImp> texturedData(new terrainImp(new glTexImp(bmpFilename)));
	texturedData->load(meshFilename);
	return texturedData.release();
}
heightField::heightField(dataImp* input)
{
	setImp(input);
}
dataImp* heightField::getImp()
{
	return m_dataInputImp.get();
}

void heightField::setImp(dataImp* imp)
{
	std::unique_ptr<dataImp> temp(imp);

	m_dataInputImp = std::move(temp);

	temp.release();
}

int heightField::getWidth()
{
	extent ext = m_dataInputImp->getExtent();

	return ext._width;
}
int heightField::getHeight()
{
	extent ext = m_dataInputImp->getExtent();

	return ext._height;
}
int heightField::getChannel()
{
	extent ext = m_dataInputImp->getExtent();
	return ext._channel;
}
int heightField::getCenterX()
{
	int width = getWidth();
	return width / 2;
}
int heightField::getCenterY()
{
	int height = getHeight();
	return height / 2;
}

int heightField::getTileWidth(int i, int j, int N)
{
	int srcWidth = getWidth();
	int tileWidth = (srcWidth - 1) / N + 1;
	return tileWidth;
}
int heightField::getTileHeight(int i, int j, int N)
{
	int srcHeight = getHeight();
	int tileHeight = (srcHeight - 1) / N + 1;
	return tileHeight;
}

int heightField::getTileCenterX(int i, int j, int N)
{
	int tileWidth = getTileWidth(i, j, N);
	int tileWidthEven = tileWidth - 1;
	return j*tileWidth + tileWidthEven / 2;

}
int heightField::getTileCenterY(int i, int j, int N)
{
	int tileHeight = getTileHeight(i, j, N);
	int tileHeightEven = tileHeight - 1;
	return i*tileHeight + tileHeightEven / 2;
}
void heightField::generateTile(int i, int j, int N, BYTE* dst, Range& tileRange)
{

	dataImp* imp = getImp();
	tileRange._centerX = getTileCenterY(i, j, N);
	tileRange._centerY = getTileCenterX(i, j, N);
	tileRange._width = getTileWidth(i, j, N);
	tileRange._height = getTileHeight(i, j, N);
	tileRange._index_i = i;
	tileRange._index_j = j;
	tileRange._N = N;

	if (dst == NULL)
		dst = new unsigned char[tileRange._width*tileRange._height];

	imp->getTile(dst, i, j, N);

}

void heightField::generateTile(int i, int j, int N, BYTE* dst, Range& tileRange, Range& globalRange)
{
	generateTile(i, j, N, dst, tileRange);
	globalRange._centerX = getCenterX();
	globalRange._centerY = getCenterY();
	globalRange._width = getWidth();
	globalRange._height = getHeight();
	globalRange._index_i = 0;
	globalRange._index_j = 0;
	globalRange._N = N;
}

glTexImp::glTexImp() :texImp(), m_width(-1), m_height(-1)
{
	
}
glTexImp::~glTexImp()
{
}
void glTexImp::load(const char* imgName)
{
	if (imgName&&strstr(imgName, ".bmp") != NULL)
		loadBmp(imgName);
}
glTexImp::glTexImp(const char* imgName):texImp(), m_width(-1), m_height(-1)
{
	load(imgName);
}
bool is_big_endian()
{
	int i = 1;
	return !((char*)&i)[0];
}
int32_t to_int32(char* buffer, int length)
{
	int32_t i = 0;
	if (!is_big_endian()) {
		for (int j = 0; j < length; j++)
			((char*)&i)[j] = buffer[j];
	}
	else {
		for (int j = 0; j<length; j++)
			((char*)&i)[sizeof(int)-1 - j] = buffer[j];
	}
	return i;
}
#define BITMAP_ID 0x4D42
void glTexImp::loadBmp(const char* imgName)
{
	//ref:https://github.com/confuzedskull/bmpLoader/blob/master/main.cpp
	FILE *pFile = 0;
	unsigned char* image;
	BITMAPINFOHEADER bitmapInfoHeader;
	BITMAPFILEHEADER header;
	unsigned char textureColors = 0;

	pFile = fopen(imgName, "rb");
	if (pFile == 0) 
		return;
	fread(&header, sizeof(BITMAPFILEHEADER), 1, pFile);
	if (header.bfType != BITMAP_ID)
	{
		fclose(pFile);             
		return;
	}
	fread(&bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, pFile);
	m_width= bitmapInfoHeader.biWidth;
	m_height= bitmapInfoHeader.biHeight;
	if (bitmapInfoHeader.biSizeImage == 0)
		bitmapInfoHeader.biSizeImage = bitmapInfoHeader.biWidth *
		bitmapInfoHeader.biHeight * 3;
	fseek(pFile, header.bfOffBits, SEEK_SET);
	image = new unsigned char[bitmapInfoHeader.biSizeImage];
	if (!image)                       
	{
		delete[] image;
		fclose(pFile);
		return;
	}
	fread(image, 1, bitmapInfoHeader.biSizeImage, pFile);
	for (int index = 0; index < (int)bitmapInfoHeader.biSizeImage; index += 3)
	{
		textureColors = image[index];
		image[index] = image[index + 2];
		image[index + 2] = textureColors;
	}

	fclose(pFile); 
#if 0
	char* data;
	char file_info[4];
	std::ifstream image_file;
	image_file.open(imgName, std::ios::binary);
	if (image_file.bad())
	{
		std::cerr << "error loading file.\n";
		return;
	}
	image_file.seekg(18, image_file.cur);//skip beginning of header
	image_file.read(file_info, 4);//width
	m_width = to_int32(file_info, 4);//convert raw data to integer
	image_file.read(file_info, 4);//height
	m_height= to_int32(file_info, 4);//convert raw data to integer
	image_file.seekg(28, image_file.cur);//skip rest of header
	int image_area = m_width*m_height;//calculate area now since it'll be used 3 times
	data = new char[image_area * 3];//set the buffer size
	image_file.read(data, image_area * 3);//get the pixel matrix
	image_file.close();
	//next we need to rearrange the color values from BGR to RGB
	for (int i = 0; i < image_area; ++i)//iterate through each cell of the matrix
	{
		int index = i * 3;
		unsigned char B, R;
		B = data[index];
		R = data[index + 2];
		data[index] = R;
		data[index + 2] = B;
	}
	delete[] data;
#endif
	glGenTextures(1, &m_texId);

	glBindTexture(GL_TEXTURE_2D, m_texId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, m_width, m_height, GL_RGB, GL_UNSIGNED_BYTE, image); 

	delete image;
}
int glTexImp::getWidth()
{
	return m_width;
}
int glTexImp::getHeight()
{
	return m_height;
}
GLuint glTexImp::getTexId()
{
	return m_texId;
}
void glTexImp::bind()
{
	//glEnable(GL_TEXTURE_2D);
	//glBindTexture(GL_TEXTURE_2D, m_texId);
}
void glTexImp::unbind()
{
	//glDisable(GL_TEXTURE_2D);
}
terrainImp::terrainImp(texImp* imp):rawDataProxy()
{
	m_texture.reset(imp);
}
terrainImp::~terrainImp()
{
	m_texture.reset();
}
texImp* terrainImp::getTexture()
{
	return m_texture.get();
}
