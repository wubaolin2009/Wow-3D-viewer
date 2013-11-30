#ifndef _WORLD_H_
#define _WORLD_H_
#include <vector>
#include <map>
#include <wx/wx.h>
#include <d3d9.h>
#include <d3dx9shader.h>
#include "vector3d.h"

#define  UNITSIZE (1.0f)

//1月31日 Test用 
//渲染魔獸中的地形 不知道能不能出結果 
//2月3日 加入 主要是用於地形渲染的 shader 
//資料相對來說比較匱乏
class World
{
public:
	//构造函数 指出文件名
	//x_ y_ 是 相对顺序
	//
	//例如:
	// World("world\\maps\\Kalimdor\\Kalimdor_35_50",0,0)
	//
	World(char* name,int x,int y){
		start_x_ = x; 
		start_y_ = y;
		char buffer_name[1024];
		sprintf(buffer_name,"%s.adt",name);
		map_name_ = buffer_name;
		sprintf(buffer_name,"%s_tex0.adt",name);
		tex_0_name_ = buffer_name;
	}
	void InitShader();
	void Draw();
	void InitADT();
private:
	
	void PrepareAlphaMap();
	wxString map_name_;  //地图的名字
	wxString tex_0_name_; //纹理tex0.adt中的名字
	//x_ y_用来确定地图tile的起始坐标
	int start_x_,start_y_;
	

	//一些存储顶点的变量
	std::vector<Vec3D> terrain_;
	std::vector<Vec3D> terrain_normal_;
	std::vector<Vec2D> terrain_uv_;
	LPDIRECT3DTEXTURE9      pTexture0_; // Our texture
	LPDIRECT3DTEXTURE9      pTexture1_; // Our texture
	LPDIRECT3DTEXTURE9      pTexture2_;
	//最高位 為 tex1 的透明度 次之為 tex2 次之為 tex3 第一位不用
	//LPDIRECT3DTEXTURE9 alpha_texs_[64][64];

	//這個是 第一個Chunck texture id 和 texture的內存中的id的對應表
	std::map<int,int> tex_id_map;

	LPDIRECT3DVERTEXBUFFER9 terrain_buffer_pass0;  //这个暂时保存此Tile所有的顶点

	//下边是Shader信息 都是Static 变量
	static IDirect3DPixelShader9*  Terrain_shader;  //P shader
	static  IDirect3DVertexShader9*  Terrain_shader_vetex;  //V Shader

	static  D3DXHANDLE Samp0Handle;
	static  D3DXHANDLE Samp1Handle;
	static  D3DXHANDLE Samp2Handle;
	static  D3DXHANDLE Samp3Handle;
	static  D3DXHANDLE SampAlphaHandle;  //isolated
	//static  D3DXHANDLE AlphamapHandle;
	static  D3DXHANDLE SampShadowHandle;

	static  D3DXCONSTANT_DESC Samp0Desc;
	static  D3DXCONSTANT_DESC Samp1Desc;
	static  D3DXCONSTANT_DESC Samp2Desc;
	static  D3DXCONSTANT_DESC Samp3Desc;
	static  D3DXCONSTANT_DESC SampAlphaDesc;
	static  D3DXCONSTANT_DESC SampShadowDesc;

	//V Shader 的参数
	static  D3DXHANDLE Vetex_matWorldViewProj;
	static  D3DXHANDLE Vetex_matInverseWorld;
	static  D3DXHANDLE Vetex_vLightDirection;

	static  LPD3DXCONSTANTTABLE TransformConstantTableVetex;
	//这些是Alpha Blend 相关的
	std::vector<int> maps_;  //这个是 16×16个 Chunck 按照顺序对应的纹理id
	//下边是 纹理混合有关的
	//这里先假定 alpha map 是 32*64个字节的 每个字节有2个alpha 对应着64*64个像素 应该对应纹理的256×256 的每4*4格子 的一个alpha  0是0 1111是1.0 1111是15
	FLOAT alpha_maps_[256][3][64][64];
 
	//這個記錄了 每個alpha map  offset ==> 第幾個alphamap的一一對應
	std::map<int,int> alpha_offset_to_index_[256];
	//決定了每次渲染 渲染哪些紋理 有幾個紋理等 用之前最好都清0 这里效率有纹理 等以后处理 可能需要用到Vertex Shader
	struct _RenderTexture{
		int tex_layers;   //有几层纹理
		bool use_alpha_map; //是否使用alpha
		LPDIRECT3DTEXTURE9 textures[4];  //最多四层纹理 //没有就是NULL
		int map_to_alpha[4];  //一个映射 这一层对应第几个alpha_texture	 根據 g_alpha_offset_to_index 計算
		LPDIRECT3DTEXTURE9 alpha_texture; //Alpha 层 这个暂时改动为 D3DXVECTOR4
		//是否使用 shadow
		bool use_shadow;
		//shadow map 参照 WowMapview 如果相应位为1的话 shadow_map对应值为85 并在AlphaMap中 创建对应的纹理
		unsigned  char shadow_map[64 * 64];
		LPDIRECT3DTEXTURE9 shadow_texture;

		//D3DXVECTOR4 alpha_maps[64*64]; 
	};

	_RenderTexture passes_[256];
	//读取 AlphaMap的函数
	int ReadAlphaMapCompressed(char* buffer,size_t start_offset,int start,int which_map);
	int ReadAlphaMapUnCompressed(char* buffer,int start_offset,size_t alpha_map_size_byte,int start,int which_map);


public:
	static void InitPShader();
	static void InitVShader();
};




#endif

