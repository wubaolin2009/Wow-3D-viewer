#ifndef _RAW_MODEL_H_
#define _RAW_MODEL_H_
#include "vector3d.h"
#include "enums.h"
//1.16 这个文件主要是一些MPQ的文件头格式 比如顶点格式什么的
//Model文件的Header
//这里是 没有define wotLK的那一段  这个好象是用来用个球包围一个模型，然后
//用来进行 Ray检测？ 或者体积检测？
struct Sphere
{
	/*0x00*/ Vec3D min;
	/*0x0C*/ Vec3D max;
	/*0x18*/ float radius;
};


//這裡後來加入了 那段define WotLK的那個Header的格式 看起來應該是比較正確的
//这包含了一些 顶点在哪个文件偏移 纹理坐标等信息
struct ModelHeader {
	char id[4];
	uint8 version[4];
	uint32 nameLength;
	uint32 nameOfs;
	uint32 GlobalModelFlags; // 1: tilt x, 2: tilt y, 4:, 8: add another field in header, 16: ; (no other flags as of 3.1.1);

	uint32 nGlobalSequences; // AnimationRelated
	uint32 ofsGlobalSequences; // A list of timestamps.
	uint32 nAnimations; // AnimationRelated
	uint32 ofsAnimations; // Information about the animations in the model.
	uint32 nAnimationLookup; // AnimationRelated
	uint32 ofsAnimationLookup; // Mapping of global IDs to the entries in the Animation sequences block.
	//uint32 nD;
	//uint32 ofsD;
	uint32 nBones; // BonesAndLookups
	uint32 ofsBones; // Information about the bones in this model.
	uint32 nKeyBoneLookup; // BonesAndLookups
	uint32 ofsKeyBoneLookup; // Lookup table for key skeletal bones.

	uint32 nVertices; // GeometryAndRendering
	uint32 ofsVertices; // Vertices of the model.
	uint32 nViews; // GeometryAndRendering
	//uint32 ofsViews; // Views (LOD) are now in .skins.

	uint32 nColors; // ColorsAndTransparency
	uint32 ofsColors; // Color definitions.

	uint32 nTextures; // TextureAndTheifAnimation
	uint32 ofsTextures; // Textures of this model.

	uint32 nTransparency; // H,  ColorsAndTransparency
	uint32 ofsTransparency; // Transparency of textures.
	//uint32 nI;   // always unused ?
	//uint32 ofsI;
	uint32 nTexAnims;	// J, TextureAndTheifAnimation
	uint32 ofsTexAnims;
	uint32 nTexReplace; // TextureAndTheifAnimation
	uint32 ofsTexReplace; // Replaceable Textures.

	uint32 nTexFlags; // Render Flags
	uint32 ofsTexFlags; // Blending modes / render flags.
	uint32 nBoneLookup; // BonesAndLookups
	uint32 ofsBoneLookup; // A bone lookup table.

	uint32 nTexLookup; // TextureAndTheifAnimation
	uint32 ofsTexLookup; // The same for textures.

	uint32 nTexUnitLookup;		// L, TextureAndTheifAnimation, seems gone after Cataclysm
	uint32 ofsTexUnitLookup; // And texture units. Somewhere they have to be too.
	uint32 nTransparencyLookup; // M, ColorsAndTransparency
	uint32 ofsTransparencyLookup; // Everything needs its lookup. Here are the transparencies.
	uint32 nTexAnimLookup; // TextureAndTheifAnimation
	uint32 ofsTexAnimLookup; // Wait. Do we have animated Textures? Wasn't ofsTexAnims deleted? oO

	Sphere collisionSphere;
	Sphere boundSphere;

	uint32 nBoundingTriangles; // Miscellaneous
	uint32 ofsBoundingTriangles;
	uint32 nBoundingVertices; // Miscellaneous
	uint32 ofsBoundingVertices;
	uint32 nBoundingNormals; // Miscellaneous
	uint32 ofsBoundingNormals;

	uint32 nAttachments; // O, Miscellaneous
	uint32 ofsAttachments; // Attachments are for weapons etc.
	uint32 nAttachLookup; // P, Miscellaneous
	uint32 ofsAttachLookup; // Of course with a lookup.
	uint32 nEvents; // 
	uint32 ofsEvents; // Used for playing sounds when dying and a lot else.
	uint32 nLights; // R
	uint32 ofsLights; // Lights are mainly used in loginscreens but in wands and some doodads too.
	uint32 nCameras; // S, Miscellaneous
	uint32 ofsCameras; // The cameras are present in most models for having a model in the Character-Tab.
	uint32 nCameraLookup; // Miscellaneous
	uint32 ofsCameraLookup; // And lookup-time again, unit16
	uint32 nRibbonEmitters; // U, Effects
	uint32 ofsRibbonEmitters; // Things swirling around. See the CoT-entrance for light-trails.
	uint32 nParticleEmitters; // V, Effects
	uint32 ofsParticleEmitters; // Spells and weapons, doodads and loginscreens use them. Blood dripping of a blade? Particles.
};


// block X - render flags  一些渲染信息 没用到暂时
/* flags */
#define	RENDERFLAGS_UNLIT	1
#define	RENDERFLAGS_UNFOGGED	2
#define	RENDERFLAGS_TWOSIDED	4
#define	RENDERFLAGS_BILLBOARD	8
#define	RENDERFLAGS_ZBUFFERED	16
struct ModelRenderFlags {
	uint16 flags;
	//unsigned char f1;
	//unsigned char f2;
	uint16 blend; // see enums.h, enum BlendModes
};



#define	TEXTUREUNIT_STATIC	16
/// Lod part, A texture unit (sub of material)
//用来描述 纹理的 不清楚
struct ModelTexUnit{
	// probably the texture units
	// size always >=number of materials it seems
	uint16 flags;		// Usually 16 for static textures, and 0 for animated textures.
	uint16 shading;		// If set to 0x8000: shaders. Used in skyboxes to ditch the need for depth buffering. See below.
	uint16 op;			// Material this texture is part of (index into mat)
	uint16 op2;			// Always same as above?
	int16 colorIndex;	// A Color out of the Colors-Block or -1 if none.
	uint16 flagsIndex;	// RenderFlags (index into render flags, TexFlags)
	uint16 texunit;		// Index into the texture unit lookup table.
	uint16 mode;		// See below.
	uint16 textureid;	// Index into Texture lookup table
	uint16 texunit2;	// copy of texture unit value?
	uint16 transid;		// Index into transparency lookup table.
	uint16 texanimid;	// Index into uvanimation lookup table. 
};
class Model;
//每次渲染都是一个pass 每个pass代表一个部位
//因为不同部位有不同的纹理信息等  函数的实现在character.cpp 中
struct ModelRenderPass {
	uint32 indexStart, indexCount, vertexStart, vertexEnd;
	//TextureID texture, texture2;
	int tex;
	bool useTex2, useEnvMap, cull, trans, unlit, noZWrite, billboard;
	float p;
	int16 texanim, color, opacity, blendmode;

	// Geoset ID
	int geoset;

	// texture wrapping
	bool swrap, twrap;

	// colours
	Vec4D ocol, ecol;

	bool init(Model *m);
	void deinit();

	bool operator< (const ModelRenderPass &m) const
	{
		// This is the old sort order method which I'm pretty sure is wrong - need to try something else.
		// Althogh transparent part should be displayed later, but don't know how to sort it
		// And it will sort by geoset id now.
		return geoset < m.geoset;
	}
};


/// Lod part, One material + render operation
//GeoSet 还没弄明白是什么意思  大概是一个部位 比如说arm什么的
struct ModelGeoset {
	uint32 id;		// mesh part id?
	uint16 vstart;	// first vertex, Starting vertex number.
	uint16 vcount;	// num vertices, Number of vertices.
	uint16 istart;	// first index, Starting triangle index (that's 3* the number of triangles drawn so far).
	uint16 icount;	// num indices, Number of triangle indices.
	uint16 nSkinnedBones;	// number of bone indices, Number of elements in the bone lookup table.
	uint16 StartBones;		// ? always 1 to 4, Starting index in the bone lookup table.
	uint16 rootBone;		// root bone?
	uint16 nBones;		// 
	Vec3D BoundingBox[2];
	float radius;
};

//ModelView 中有 這個模型有幾個三角形什麼的等信息 模型的顶点信息就是通过这个读的
struct ModelView{
	char id[4];				 // Signature
	uint32 nIndex;
	uint32 ofsIndex; // int16, Vertices in this model (index into vertices[])
	uint32 nTris;
	uint32 ofsTris;	 // int16[3], indices
	uint32 nProps;
	uint32 ofsProps; // int32, additional vtx properties
	uint32 nSub;
	uint32 ofsSub;	 // ModelGeoset, materials/renderops/submeshes
	uint32 nTex;
	uint32 ofsTex;	 // ModelTexUnit, material properties/textures
	int lod;				 // LOD bias?
};

//这个是用来描述一个具体的纹理的吧?
struct ModelTextureDef {
	uint32 type;
	uint32 flags;
	uint32 nameLen;
	uint32 nameOfs;
};
//用来描述某个具体的顶点的 这个顶点属于某个bones bones参与动画
//动画就是对不同的bone应用不同的变换矩阵 
//MPQ文件中存储的顶点信息格式
struct ModelVertex {
	Vec3D pos;
	uint8 weights[4];
	uint8 bones[4];
	Vec3D normal;
	Vec2D texcoords;
	int unk1, unk2; // always 0,0 so this is probably unused
};

// sub-block in block E - animation data, size 28 bytes, WotLK 20 bytes
struct AnimationBlock {
	int16 type;		// interpolation type (0=none, 1=linear, 2=hermite)
	int16 seq;		// global sequence id or -1
	uint32 nTimes;
	uint32 ofsTimes;
	uint32 nKeys;
	uint32 ofsKeys;
};

// block G - color defs
// For some swirling portals and volumetric lights, these define vertex colors. 
// Referenced from the Texture Unit blocks in the LOD part. Contains a separate timeline for transparency values. 
// If no animation is used, the given value is constant.
struct ModelColorDef {
	AnimationBlock color; // (Vec3D) Three floats. One for each color.
	AnimationBlock opacity; // (UInt16) 0 - transparent, 0x7FFF - opaque.
};

// block H - transparency defs
struct ModelTransDef {
	AnimationBlock trans; // (UInt16)
};

#endif