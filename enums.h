#ifndef _ENUMS_H_
#define _ENUMS_H_
#include <d3dx9.h>
#include <wx/wx.h>
//1.16 主要是一些类型定义和 枚举
typedef unsigned int uint32;
typedef int int32;
//這裡有很大的錯誤 1月20日糾正
//typedef char uint8;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef short int16;
typedef unsigned int GLuint;
typedef char int8;

#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 768

//这个是D3D的顶点格式
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_NORMAL |D3DFVF_DIFFUSE| D3DFVF_TEX1)
//最多一个模型32个纹理
#define	TEXTURE_MAX	32
struct CHARACTERVERTEX
{
	FLOAT x, y, z;      // The untransformed, 3D position for the vertex
	FLOAT nx,ny,nz;
	DWORD color;        // The vertex color
	FLOAT u,v;
};

enum ModelType {
	MT_NORMAL,
	MT_CHAR,  //这个应该是人物模型吧 还没见到这个怎么用
	MT_WMO,
	MT_NPC
};

enum Gender
{
	GENDER_MALE                        = 0,
	GENDER_FEMALE                      = 1,
	GENDER_NONE                        = 2
};
// Race value is index in ChrRaces.dbc
enum Races
{
	RACE_HUMAN              = 1,
	RACE_ORC                = 2,
	RACE_DWARF              = 3,
	RACE_NIGHTELF           = 4,
	RACE_UNDEAD             = 5,
	RACE_TAUREN             = 6,
	RACE_GNOME              = 7,
	RACE_TROLL              = 8,
	RACE_GOBLIN             = 9,
	RACE_BLOODELF           = 10,
	RACE_DRAENEI            = 11,
	RACE_FEL_ORC            = 12,
	RACE_NAGA               = 13,
	RACE_BROKEN             = 14,
	RACE_SKELETON           = 15,
	RACE_VRYKUL             = 16,
	RACE_TUSKARR            = 17,
	RACE_FOREST_TROLL       = 18,
	RACE_TAUNKA             = 19,
	RACE_NORTHREND_SKELETON = 20,
	RACE_ICE_TROLL          = 21,
	RACE_WORGEN             = 22
};

//动画的插值计算方式
enum Interpolations {
	INTERPOLATION_NONE,
	INTERPOLATION_LINEAR,
	INTERPOLATION_HERMITE,
	INTERPOLATION_BEZIER
};

//人物的不同部位
enum CharRegions {
	CR_BASE = 0,
	CR_ARM_UPPER,
	CR_ARM_LOWER,
	CR_HAND,
	CR_FACE_UPPER,
	CR_FACE_LOWER,
	CR_TORSO_UPPER,
	CR_TORSO_LOWER,
	CR_PELVIS_UPPER,
	CR_PELVIS_LOWER,
	CR_FOOT,
	NUM_REGIONS,

	CR_LEG_UPPER = CR_PELVIS_UPPER,
	CR_LEG_LOWER = CR_PELVIS_LOWER
};
//每個部位的紋理地址
const wxString regionPaths[NUM_REGIONS] =
{
	wxEmptyString,
	wxT("Item\\TextureComponents\\ArmUpperTexture\\"),
	wxT("Item\\TextureComponents\\ArmLowerTexture\\"),
	wxT("Item\\TextureComponents\\HandTexture\\"),
	wxEmptyString,
	wxEmptyString,
	wxT("Item\\TextureComponents\\TorsoUpperTexture\\"),
	wxT("Item\\TextureComponents\\TorsoLowerTexture\\"),
	wxT("Item\\TextureComponents\\LegUpperTexture\\"),
	wxT("Item\\TextureComponents\\LegLowerTexture\\"),
	wxT("Item\\TextureComponents\\FootTexture\\")
};

// copied from the .mdl docs? this might be completely wrong
/*
Blending mode
Value	 Mapped to	 Meaning
0	 0	 Combiners_Opaque
1	 1	 Combiners_Mod
2	 1	 Combiners_Decal
3	 1	 Combiners_Add
4	 1	 Combiners_Mod2x
5	 4	 Combiners_Fade
6	 4	 Used in the Deeprun Tram subway glass, supposedly (src=dest_color, dest=src_color) (?)
*/
enum BlendModes {
	BM_OPAQUE,
	BM_TRANSPARENT,
	BM_ALPHA_BLEND,
	BM_ADDITIVE,
	BM_ADDITIVE_ALPHA,
	BM_MODULATE,
	BM_MODULATEX2
};

#endif