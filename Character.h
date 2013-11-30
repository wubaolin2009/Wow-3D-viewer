#ifndef _CHARACTER_H_
#define _CHARACTER_H_
//1月16日 主要用来加载人物模型
//ToDO： 提出基类 供NPC等模型共享一些方法
#include "mpq.h"
#include "raw_model.h"
#include "vector3d.h"
#include "wx/wx.h"
#include "animated.h"
#include "enums.h"
#include "texture.h"
#include "bone.h"

//通過一個部位 和名字 合成一個新的紋理文件名的信息
wxString makeItemTexture(int region, const wxString name);
class ParticleSystem;
class RibbonEmitter;
class TextureAnim;
class CharControl;
class CharacterModel
{
public:
	//这个Modelname 应该是 Character\\Bloodelf\\female\\bloodelffemale.m2 这种
	CharacterModel(const wxString& model_name,LPDIRECT3DDEVICE9 device):model_name_(model_name),d3d_device_(device){
		vetex_buffer_ = NULL;index_buffer_ = NULL;hasParticles_ = false;bounding_buffer_ = NULL;bounding_index_ = NULL;bounds = NULL;boundTris = NULL;
		current_anim_frame_ = 0;
	}
	//初始化模型 现在只有静态的 动画信息什么的以后静态模型搞定再说
	//包括初始化顶点 index 纹理坐标 纹理等
	void Init(); 
	//这里初始化动画 这个函数不应该在这里 但是为了方便 暂时放到这里 这里先做一个protype
	void InitAnimation();
	//Animate 以後會移出這個類中 調用渲染下一幀 //這裡會涉及到頂點位置的變化
	//frame 指定要画第几帧 由caller保证frame不会越界
	void Animate(int anim_id,int frame); 
	//这里初始化粒子和ribbon系统 ribbon现在不明白
	void InitParticle();
	//更新粒子的状态  参数 为 增加多少 输入的参数应该为上一帧的时间间隔（ms） /1000.0
	void UpdateParticle(FLOAT dt);
	//這個在animate()中被調用 重算所有的Bones 1月29 ItemModel重写此函数
	virtual void calcBones(ssize_t anim, size_t time);
	//畫出模型 現在只有靜態模型 所以只有這一個函數暫時 TODO 增加動畫信息
	virtual void Draw();
	//1月24日 加入 用来体积碰撞检测的Bounding Triangles的画出
	void DrawBoundingVolume();

	//1月25日
	//检测是否相交 并打印一些信息
	//在鼠标左键单击的时候被调用
	//这里是的屏幕上的点通过GetCurrentPos 然后ScreenToClient得到
	//1月28日 原形改變
	//如果相交 返回distance 和世界坐標的交點 distance 和cords_world可以為空
	//x y 為屏幕的坐標(窗口）
	bool InterSect(int x,int y,FLOAT* distance,Vec3D* cords_world,Vec3D* char_pos,Vec3D* char_rot);

	//1月27日加入 返回anim的id 一共有几帧数
	uint32 FrameCount(int anim_id);
	

protected:
	//这里可能以后是继承自Model类 所以下边函数可能是继承来的 
	//先写成protected
	//初始化顶点
	//f是模型的MPQ文件
	void InitVertex(MPQFile& f);
	//初始化索引
	//f是模型的MPQ文件 g应该是模型的一些LOD信息 具体什么的纹理，索引应该都在这个文件中
	void InitIndex(MPQFile& f,MPQFile& g);
	//初始化纹理
	void InitTexture(MPQFile& f,MPQFile& g);
	//初始化GeoSet 和 Passes
	void InitGeoSet(MPQFile& f,MPQFile& g);
	//初始化 Body的纹理 在InitTexture中调用 这个涉及到纹理的Blend
	virtual void InitBodyTexture(MPQFile& f,MPQFile& g);
	//初始化 Hair的纹理 原理类似于Body
	virtual void InitHairTexture(MPQFile& f,MPQFile& g);
	//初始化 Capde的纹理 类似于Body
	virtual void InitCapeTexture(MPQFile& f,MPQFile& g);
	//初始化Bounding的信息 在Init中被调用
	virtual void InitBounding(MPQFile& f,MPQFile& g);

	//将顶点坐标 纹理坐标 颜色 法向量等 写入 渲染设备中
	void FillVetex(MPQFile& f);
	//寫入Index
	void FillIndex(MPQFile& f,MPQFile& g);
	//主要是用于纹理的创建
	void FillTexture(MPQFile& f,MPQFile& g);
protected:  
	//模型名  就是构造函数中的 带有Character 。。。。.m2的那个
	wxString model_name_;
	//保存的用来描述头的信息
	ModelHeader model_header_;
	Vec3D* vertices_;  //顶点坐标
	Vec2D* texCoords;  //頂點紋理坐標
	Vec3D* normals_;  //法向量坐标
	ModelColor* colors_;  //颜色
	ModelTransparency* trans_; //透明信息
	uint32			*globalSequences_; //不知道干什么的
	int count_vetices_; //頂點的數量
	uint16* indices_ ;  //索引
	int count_indices_; //index number
	float rad; //半径
	unsigned int * textures_; //纹理的id数组
	wxArrayString TextureList; //保存texture名字的？debug就知道了
	int specialTextures[TEXTURE_MAX];  //特殊的texture 只用于人物的？不清楚
	unsigned int replaceTextures[TEXTURE_MAX];
	bool useReplaceTextures[TEXTURE_MAX];
	//模型类型
	ModelType modelType;

	// 因為每次要渲染的地方不同 於是有RenderPass這個類
	std::vector<ModelRenderPass> passes_;
	std::vector<ModelGeoset> geosets_;

	//这里保存的是 Animation信息 以后要放到AnimModel这个基类中
	ModelAnimation *anims; //不清楚
	int16 *animLookups;
	Bone *bones;
	MPQFile *animfiles;
	uint32			*globalSequences;  //這個不明白是什麼
	int16 keyBoneLookup[BONE_MAX];  //一個 部位-》 骨頭id的映射應該是
	//Particle And Ribbon 信息
	ParticleSystem* particleSystems_;
	bool hasParticles_;
	RibbonEmitter	*ribbons_;
	//体积检测相关
	uint16 *boundTris; //这个是bounding的索引集
	Vec3D *bounds;   //这个是顶点集合

	//texanmiation 1月26日
	TextureAnim		*texAnims;
	//1月29日 主要是用于transparency的动画过程
	int current_anim_frame_;


protected:
	//这些是和D3D有关的
	LPDIRECT3DVERTEXBUFFER9 vetex_buffer_;
	LPDIRECT3DINDEXBUFFER9 index_buffer_;
	LPDIRECT3DDEVICE9      d3d_device_;

	//碰撞检测相关的
	LPDIRECT3DVERTEXBUFFER9 bounding_buffer_;
	LPDIRECT3DINDEXBUFFER9 bounding_index_;

	friend class ParticleSystem;
	friend class RibbonEmitter;
	friend class CharControl;
};



#endif