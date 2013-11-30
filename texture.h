#ifndef _TEXTURE_H_
#define _TEXTURE_H_

#include <d3dx9.h>
#include <map>
#include <vector>
#include <set>
#include <wx/wx.h>
#include "manager.h"
#include "enums.h"

/* 这里先这么写 为了编译 */
#define GL_RED 0x1903
#define GL_GREEN 0x1904
#define GL_BLUE 0x1905
#define GL_ALPHA 0x1906
#define GL_RGB 0x1907
#define GL_RGBA 0x1908
#define GL_BGRA_EXT 0x80E1


class Texture : public ManagedItem {
public:
	int w,h;
	unsigned int id;
	bool compressed;
	wxString texture_name;
	LPDIRECT3DTEXTURE9 tex;

	Texture(wxString name):ManagedItem(name), texture_name(name),w(0), h(0), id(0), compressed(false) {}
	void getPixels(unsigned char *buff, unsigned int format);

};


class TextureManager : public Manager<unsigned int> {

public:
	TextureManager():id(0){}
	//GetTexture 得到紋理 如果這個紋理不存在 則返回NULL
	LPDIRECT3DTEXTURE9 GetTexture(int id);
	//这里我1月16日增加了一个通过WxString查找相应的纹理的 如果纹理没有被加载 则加载
	LPDIRECT3DTEXTURE9 GetTexture(const wxString& name);
	//
	bool Exists(const wxString& name){
		std::map<wxString,LPDIRECT3DTEXTURE9>::iterator ir = maps_string_.begin();
		bool find = false;
		while(ir != maps_string_.end() ){
			if(ir->first == name){
				find = true;
				break;
			}
			ir++;
		}
		return find;
	}

	//1月17日增加  这个不涉及到文件的打开 我默认这个纹理已经在内存中了
	unsigned int add(wxString name,LPDIRECT3DTEXTURE9 texture);


	virtual unsigned int add(wxString name);
	void doDelete(unsigned int id);

	void LoadBLP(unsigned int id, Texture *tex,const wxString& name);
private:
	std::map<int,LPDIRECT3DTEXTURE9> maps_;
	std::map<wxString,LPDIRECT3DTEXTURE9> maps_string_;
	unsigned int id; //用来计数
};


struct TextureGroup {
	static const size_t num = 3;
	int base, count;
	wxString tex[num];
	TextureGroup()
	{
		for (size_t i=0; i<num; i++) {
			tex[i] = wxT("");
		}
	}

	// default copy constr
	TextureGroup(const TextureGroup &grp)
	{
		for (size_t i=0; i<num; i++) {
			tex[i] = grp.tex[i];
		}
		base = grp.base;
		count = grp.count;
	}
	const bool operator<(const TextureGroup &grp) const
	{
		for (size_t i=0; i<num; i++) {
			if (tex[i]<grp.tex[i]) return true;
			if (tex[i]>grp.tex[i]) return false;
		}
		return false;
	}
};

typedef std::set<TextureGroup> TextureSet;

//1月16日增加的 人物的纹理 涉及到一些 纹理混合之类的问题
struct CharTextureComponent
{
	wxString name;
	int region;
	int layer;

	const bool operator<(const CharTextureComponent& c) const
	{
		return layer < c.layer;
	}
};

enum TextureFlags {
	TEXTURE_WRAPX=1,
	TEXTURE_WRAPY
};

//纹理的id
typedef unsigned int TextureID;
enum TextureTypes {
	TEXTURE_FILENAME=0,			// Texture given in filename
	TEXTURE_BODY,				// Body + clothes
	TEXTURE_CAPE,				// Item, Capes ("Item\ObjectComponents\Cape\*.blp")
	TEXTURE_ITEM=TEXTURE_CAPE,
	TEXTURE_ARMORREFLECT,		// 
	TEXTURE_HAIR=6,				// Hair, bear
	TEXTURE_FUR=8,				// Tauren fur
	TEXTURE_INVENTORY_ART1,		// Used on inventory art M2s (1): inventoryartgeometry.m2 and inventoryartgeometryold.m2
	TEXTURE_QUILL,				// Only used in quillboarpinata.m2. I can't even find something referencing that file. Oo Is it used?
	TEXTURE_GAMEOBJECT1,		// Skin for creatures or gameobjects #1
	TEXTURE_GAMEOBJECT2,		// Skin for creatures or gameobjects #2
	TEXTURE_GAMEOBJECT3,		// Skin for creatures or gameobjects #3
	TEXTURE_INVENTORY_ART2,		// Used on inventory art M2s (2): ui-buffon.m2 and forcedbackpackitem.m2 (LUA::Model:ReplaceIconTexture("texture"))
	TEXTURE_15,					// Patch 12857, Unknown
	TEXTURE_16,					//
	TEXTURE_17,					//
};

struct CharTexture
{
	std::vector<CharTextureComponent> components;
	void addLayer(wxString fn, int region, int layer)
	{
		if (!fn || fn.length()==0)
			return;

		CharTextureComponent ct;
		ct.name = fn;
		ct.region = region;
		ct.layer = layer;
		components.push_back(ct);
	}
	unsigned int compose(TextureID texID,wxString name);  //name 是给新的纹理命名 返回新的纹理的id
};

//这应该是 纹理贴图对应的部位 一个坐标的映射
struct CharRegionCoords {
	int xpos, ypos, xsize, ysize;
};


#define	REGION_FAC	2
#define	REGION_PX	(256*REGION_FAC)

const CharRegionCoords regions[NUM_REGIONS] =
{
	{0, 0, 256*REGION_FAC, 256*REGION_FAC},	// base
	{0, 0, 128*REGION_FAC, 64*REGION_FAC},	// arm upper
	{0, 64*REGION_FAC, 128*REGION_FAC, 64*REGION_FAC},	// arm lower
	{0, 128*REGION_FAC, 128*REGION_FAC, 32*REGION_FAC},	// hand
	{0, 160*REGION_FAC, 128*REGION_FAC, 32*REGION_FAC},	// face upper
	{0, 192*REGION_FAC, 128*REGION_FAC, 64*REGION_FAC},	// face lower
	{128*REGION_FAC, 0, 128*REGION_FAC, 64*REGION_FAC},	// torso upper
	{128*REGION_FAC, 64*REGION_FAC, 128*REGION_FAC, 32*REGION_FAC},	// torso lower
	{128*REGION_FAC, 96*REGION_FAC, 128*REGION_FAC, 64*REGION_FAC}, // pelvis upper
	{128*REGION_FAC, 160*REGION_FAC, 128*REGION_FAC, 64*REGION_FAC},// pelvis lower
	{128*REGION_FAC, 224*REGION_FAC, 128*REGION_FAC, 32*REGION_FAC}	// foot
};


#endif