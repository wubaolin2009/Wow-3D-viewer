#include "Character.h"
#include "manager.h"
#include "texture.h"
#include "DB.h"
#include "particle.h"
#include "Ray.h"
extern TextureManager texturemanager;
extern ItemDisplayDB		itemdisplaydb;
extern CharSectionsDB      chardb;
extern HWND g_hwnd;
//这个是Bounding Volumn的格式
#define D3DFVF_BOUNDINGVERTEX (D3DFVF_XYZ)
struct BOUNDINGVERTEX
{
	FLOAT x, y, z;      // The untransformed, 3D position for the vertex
};


wxString makeItemTexture(int region, const wxString name)
{
	//這裡我們先默認 gender都是female
	bool gender1 = true;
	// just return an empty filename
	if (name.Length() < 3) 
		return wxEmptyString;

	wxChar leggings = name[name.Length() - 2];

	// try prefered version first
	wxString fn = regionPaths[region];
	fn += name;
	fn += wxT("_");

	if (leggings == 'l' || leggings == 'L')
		fn += wxT("U");
	else
		fn += gender1 ? wxT("F") : wxT("M");

	fn += wxT(".blp");
	if (MPQFile::getSize(fn) > 0)  //MPQFile::exists(fn.c_str()) && 
		return fn;

	if (fn.Length() < 5)
		return wxEmptyString;

	// if that failed try alternate texture version
	if (leggings == 'l' || leggings == 'L')
		fn[fn.Length()-5] = gender1 ? 'F' : 'M';
	else
		fn[fn.Length()-5] = 'U';

	if (MPQFile::getSize(fn) > 0) //MPQFile::exists(fn.c_str()) && 
		return fn;

	fn = regionPaths[region];
	fn += name;	
	fn += wxT(".blp");

	// return the default name, nothing else could be found.
	return fn;
}


void CharacterModel::Init()
{
	//打开模型的文件
	MPQFile f(model_name_);
	memcpy(&model_header_, f.getBuffer(), sizeof(ModelHeader));
	//检测文件格式是否正确
	if (model_header_.id[0] != 'M' && model_header_.id[1] != 'D' && model_header_.id[2] != '2' && model_header_.id[3] != '0') {
		wxLogMessage(wxT("Error:\t\tInvalid model!  May be corrupted."));
		f.close();
		return;
	}
	if (model_header_.nameOfs != 304 && model_header_.nameOfs != 320) {
		wxLogMessage(wxT("Error:\t\tInvalid model nameOfs=%d/%d!  May be corrupted."), model_header_.nameOfs, sizeof(ModelHeader));
		//ok = false;
		//f.close();
		//return;
	}
	//1月20日加入globalSequences 的初始化
	globalSequences = NULL;
	if (model_header_.nGlobalSequences) {
		globalSequences = new uint32[model_header_.nGlobalSequences];
		memcpy(globalSequences, (f.getBuffer() + model_header_.ofsGlobalSequences), model_header_.nGlobalSequences * sizeof(uint32));
	}
	//初始化下边几个纹理数组 具体用途还没弄懂这里以后要改 这种纹理的使用并不清晰 
	for (size_t i=0; i<TEXTURE_MAX; i++) {
		specialTextures[i] = -1;
		replaceTextures[i] = 0;
		useReplaceTextures[i] = false;
	}
	//打开 LOD文件
	//这里Lods 先取00  010203不知道什么效果
	int index = 0;
	wxString lodname = model_name_.BeforeLast(wxT('.')) + wxString::Format(wxT("%02d.skin"), index); // Lods: 00, 01, 02, 03
	MPQFile g(lodname);
	//检测LOD文件是否正确
	if (g.isEof()) {
		wxLogMessage(wxT("Error: Unable to load Lods: [%s]"), lodname.c_str());
		g.close();
		return ; 
	}

	ModelView *view = (ModelView*)(g.getBuffer());

	if (view->id[0] != 'S' || view->id[1] != 'K' || view->id[2] != 'I' || view->id[3] != 'N') {
		wxLogMessage(wxT("Error: Unable to load Lods: [%s]"), lodname.c_str());
		g.close();
		return ;
	}
	modelType = MT_NORMAL;
	//初始化GlobalSequence 不知道干什么的
	if (model_header_.nGlobalSequences) {
		globalSequences_ = new uint32[model_header_.nGlobalSequences];
		memcpy(globalSequences_, (f.getBuffer() + model_header_.ofsGlobalSequences), model_header_.nGlobalSequences * sizeof(uint32));
	}

	InitVertex(f);
	InitIndex(f,g);
	InitTexture(f,g);
	InitGeoSet(f,g);
	//初始化BoundingVolumn
	InitBounding(f,g);

	//为渲染做准备
	FillVetex(f);
	FillIndex(f,g);	
	FillTexture(f,g);

}

void CharacterModel::InitAnimation()
{
	//打开模型的文件
	MPQFile f(model_name_);
	ModelVertex * origVertices = (ModelVertex *)(f.getBuffer() + model_header_.ofsVertices );

	if (model_header_.nAnimations > 0) {
		anims = new ModelAnimation[model_header_.nAnimations];

		// or load anim files ondemand?
		ModelAnimationWotLK animsWotLK;
		wxString tempname;
		animfiles = new MPQFile[model_header_.nAnimations];
		for(size_t i=0; i<model_header_.nAnimations; i++) {
			memcpy(&animsWotLK, f.getBuffer() + model_header_.ofsAnimations + i*sizeof(ModelAnimationWotLK), sizeof(ModelAnimationWotLK));
			anims[i].animID = animsWotLK.animID;
			anims[i].timeStart = 0;
			anims[i].timeEnd = animsWotLK.length;
			anims[i].moveSpeed = animsWotLK.moveSpeed;
			anims[i].flags = animsWotLK.flags;
			anims[i].probability = animsWotLK.probability;
			anims[i].d1 = animsWotLK.d1;
			anims[i].d2 = animsWotLK.d2;
			anims[i].playSpeed = animsWotLK.playSpeed;
			anims[i].boundSphere.min = animsWotLK.boundSphere.min;
			anims[i].boundSphere.max = animsWotLK.boundSphere.max;
			anims[i].boundSphere.radius = animsWotLK.boundSphere.radius;
			anims[i].NextAnimation = animsWotLK.NextAnimation;
			anims[i].Index = animsWotLK.Index;

			tempname = wxString::Format(wxT("%s%04d-%02d.anim"), (char *)model_name_.BeforeLast(wxT('.')).c_str(), anims[i].animID, animsWotLK.subAnimID);
			if (MPQFile::getSize(tempname) > 0) {
				animfiles[i].openFile(tempname);
				//g_modelViewer->modelOpened->Add(tempname);
			}
		}

		//animManager = new AnimManager(anims);
	}

	//if (animBones) { 我这里暂时认为animBones始终为true
	if(true){
		// init bones...
		bones = new Bone[model_header_.nBones];
		ModelBoneDef *mb = (ModelBoneDef*)(f.getBuffer() + model_header_.ofsBones);
		for (size_t i=0; i<model_header_.nBones; i++) {
			bones[i].model = this;
			bones[i].initV3(f, mb[i], globalSequences, animfiles);
		}

		// Block keyBoneLookup is a lookup table for Key Skeletal Bones, hands, arms, legs, etc.
		if (model_header_.nKeyBoneLookup < BONE_MAX) {
			memcpy(keyBoneLookup, f.getBuffer() + model_header_.ofsKeyBoneLookup, sizeof(int16)*model_header_.nKeyBoneLookup);
		} else {
			memcpy(keyBoneLookup, f.getBuffer() + model_header_.ofsKeyBoneLookup, sizeof(int16)*BONE_MAX);
			wxLogMessage(wxT("Error: keyBone number [%d] over [%d]"), model_header_.nKeyBoneLookup, BONE_MAX);
		}
	}

	// Index at ofsAnimations which represents the animation in AnimationData.dbc. -1 if none.
	if (model_header_.nAnimationLookup > 0) {
		animLookups = new int16[model_header_.nAnimationLookup];
		memcpy(animLookups, f.getBuffer() + model_header_.ofsAnimationLookup, sizeof(int16)*model_header_.nAnimationLookup);
	}
	//这里是 TexAnim 在1月26日加入
	if (model_header_.nTexAnims > 0) {
		texAnims = new TextureAnim[model_header_.nTexAnims];
		ModelTexAnimDef *ta = (ModelTexAnimDef*)(f.getBuffer() + model_header_.ofsTexAnims);
		for (size_t i=0; i<model_header_.nTexAnims; i++)
			texAnims[i].init(f, ta[i], globalSequences);
	}


	
}

//這個是一個全局變量 用來指示當前渲染的是第幾幀 變量的定義在
//main中 沒按一次空格 就+一定的數值
extern int g_current_frame;

//1月20日 加入 1月27日 完善1 加入参数frame 指定要画第几帧，实时的
//time_diff 属于[0,max]之间 由调用者进行判断 anim_id 为动画名称
void CharacterModel::Animate(int anim_id,int frame)
{
	size_t t=frame;  //第幾幀
	current_anim_frame_ = frame;
	 
	///這裡我讓anim 也就是動畫的index為15 也就是run這個動畫
	size_t anim = anim_id; 

	ModelAnimation &a = anims[anim];
	int tmax = (a.timeEnd-a.timeStart);
	if (tmax==0) 
		tmax = 1;

	/*if (isWMO == true) {
		t = globalTime;
		t %= tmax;
		t += a.timeStart;
	} else*/
	///t = animManager->GetFrame();
	///t = g_current_frame; //這個先hack下 以後改進 t的計算方式

	//this->animtime = t;
	//this->anim = anim;
	bool animBones = true;
	bool animGeometry = true;
	if (animBones) // && (!animManager->IsPaused() || !animManager->IsParticlePaused()))
		calcBones(anim, t);

	if (animGeometry) {
		CHARACTERVERTEX* vertices = NULL; //得到vetex_buffer_中的頂點
		if( FAILED( vetex_buffer_->Lock( 0, count_vetices_, ( void** )&vertices, 0 ) ) ){
			printf("Error in Fill Vertex Buffer\n");		
			return ;
		}
		//對其坐標進行改寫
		MPQFile f(model_name_);
		ModelVertex * origVertices = (ModelVertex *)(f.getBuffer() + model_header_.ofsVertices );
		ModelVertex *ov = origVertices;
		for(size_t i = 0;i < model_header_.nVertices;++i,++ov){
			Vec3D v(0,0,0);
			Vec3D n(0,0,0);
			for (size_t b=0; b<4; b++) {
				if (ov->weights[b]>0) {
					Vec3D vv = fixCoordSystem(ov->pos);
					Vec3D tv = bones[ov->bones[b]].mat * vv;
					Vec3D tn = bones[ov->bones[b]].mrot * ov->normal;
					v += tv * ((float)ov->weights[b] / 255.0f);
					n += tn * ((float)ov->weights[b] / 255.0f);
				}
			}
			//v = fixCoordSystem(v);

			
			vertices[i].x = v.x;
			vertices[i].y = v.y;
			vertices[i].z = v.z;
			vertices[i].nx = n.x;
			vertices[i].ny = n.y;
			vertices[i].nz = n.z;
			Vec3D new_vec = fixCoordSystem(ov->pos);

			/*vertices[i].x = new_vec.x;
			vertices[i].y = new_vec.y;
			vertices[i].z = new_vec.z;*/
		}				 
		vetex_buffer_->Unlock();
		/*if (video.supportVBO)	{
			glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbuf);
			glBufferDataARB(GL_ARRAY_BUFFER_ARB, 2*vbufsize, NULL, GL_STREAM_DRAW_ARB);
			vertices = (Vec3D*)glMapBufferARB(GL_ARRAY_BUFFER_ARB, GL_WRITE_ONLY);

			// Something has been changed in the past couple of days that is causing nasty bugs
			// this is an extra error check to prevent the program from crashing.
			if (!vertices) {
				wxLogMessage(wxT("Critical Error: void Model::animate(int anim), Vertex Buffer is null"));
				return;
			}
		}
		// transform vertices
		ModelVertex *ov = origVertices;
		for (size_t i=0; i<header.nVertices; ++i,++ov) { //,k=0
			Vec3D v(0,0,0), n(0,0,0);

			for (size_t b=0; b<4; b++) {
				if (ov->weights[b]>0) {
					Vec3D tv = bones[ov->bones[b]].mat * ov->pos;
					Vec3D tn = bones[ov->bones[b]].mrot * ov->normal;
					v += tv * ((float)ov->weights[b] / 255.0f);
					n += tn * ((float)ov->weights[b] / 255.0f);
				}
			}

			vertices[i] = v;
			if (video.supportVBO)
				vertices[header.nVertices + i] = n.normalize(); // shouldn't these be normal by default?
			else
				normals[i] = n;
		}
		// clear bind
		if (video.supportVBO) {
			glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);
		}*/
	}

	//1月23 加入
	for (size_t i=0; i<model_header_.nParticleEmitters; i++) {
		// random time distribution for teh win ..?
		//int pt = a.timeStart + (t + (int)(tmax*particleSystems[i].tofs)) % tmax;
		particleSystems_[i].setup(anim, t);
	}
	//1月29日加入 texture animation
	if (model_header_.nTexAnims > 0) {
		for (size_t i=0; i<model_header_.nTexAnims; i++) {
			texAnims[i].calc(anim, t);
		}
	}

}

void CharacterModel::calcBones(ssize_t anim, size_t time)
{
	// Reset all bones to 'false' which means they haven't been animated yet.
	for (size_t i=0; i<model_header_.nBones; i++) {
		bones[i].calc = false;
	}
	//我們這裡先略過了 人物的動畫 先做 Creature的動畫
	for (ssize_t i=0; i<keyBoneLookup[BONE_ROOT]; i++) {
		bones[i].calcMatrix(bones, anim, time);
	}

	// The following line fixes 'mounts' in that the character doesn't get rotated, but it also screws up the rotation for the entire model :(
	//bones[18].calcMatrix(bones, anim, time, false);

	// Animate key skeletal bones except the fingers which we do later.
	// -----
	size_t a, t;

	// if we have a "secondary animation" selected,  animate upper body using that.
	/*if (animManager->GetSecondaryID() > -1) {
		a = animManager->GetSecondaryID();
		t = animManager->GetSecondaryFrame();
	} else {*/
	a = anim;
	t = time;
	//}

	//我們根據程序的運行情況 將這裡的i設置為 i< 5 
	//for (size_t i=0; i<animManager->GetSecondaryCount(); i++) { // only goto 5, otherwise it affects the hip/waist rotation for the lower-body.
	for (size_t i=0; i<5; i++) {
		if (keyBoneLookup[i] > -1){
			bones[keyBoneLookup[i]].calcMatrix(bones, a, t);
		}
		
	}

	/*if (animManager->GetMouthID() > -1) {
		// Animate the head and jaw
		if (keyBoneLookup[BONE_HEAD] > -1)
			bones[keyBoneLookup[BONE_HEAD]].calcMatrix(bones, animManager->GetMouthID(), animManager->GetMouthFrame());
		if (keyBoneLookup[BONE_JAW] > -1)
			bones[keyBoneLookup[BONE_JAW]].calcMatrix(bones, animManager->GetMouthID(), animManager->GetMouthFrame());
	} else {*/
		// Animate the head and jaw
	if (keyBoneLookup[BONE_HEAD] > -1)
		bones[keyBoneLookup[BONE_HEAD]].calcMatrix(bones, a, t);
	if (keyBoneLookup[BONE_JAW] > -1)
		bones[keyBoneLookup[BONE_JAW]].calcMatrix(bones, a, t);
	//}

	// still not sure what 18-26 bone lookups are but I think its more for things like wrist, etc which are not as visually obvious.
	for (size_t i=BONE_ROOT; i<BONE_MAX; i++) {
		if (keyBoneLookup[i] > -1)
			bones[keyBoneLookup[i]].calcMatrix(bones, a, t);
	}
	// Animate everything thats left with the 'default' animation
	for (size_t i=0; i<model_header_.nBones; i++) {
		bones[i].calcMatrix(bones, anim, time);
	}

}

void CharacterModel::InitParticle()
{
	MPQFile f(model_name_);
	// particle systems
	if (model_header_.nParticleEmitters) {
		if (model_header_.version[0] >= 0x10) {
			ModelParticleEmitterDefV10 *pdefsV10 = (ModelParticleEmitterDefV10 *)(f.getBuffer() + model_header_.ofsParticleEmitters);
			ModelParticleEmitterDef *pdefs;
			particleSystems_ = new ParticleSystem[model_header_.nParticleEmitters];
			hasParticles_ = true;
			for (size_t i=0; i<model_header_.nParticleEmitters; i++) {
				pdefs = (ModelParticleEmitterDef *) &pdefsV10[i];
				particleSystems_[i].model = this;
				particleSystems_[i].init(f, *pdefs, globalSequences);
			}
		} else {
			ModelParticleEmitterDef *pdefs = (ModelParticleEmitterDef *)(f.getBuffer() + model_header_.ofsParticleEmitters);
			particleSystems_ = new ParticleSystem[model_header_.nParticleEmitters];
			hasParticles_ = true;
			for (size_t i=0; i<model_header_.nParticleEmitters; i++) {
				particleSystems_[i].model = this;
				particleSystems_[i].init(f, pdefs[i], globalSequences);
			}
		}
	}

	if (model_header_.nRibbonEmitters) {
		ModelRibbonEmitterDef *rdefs = (ModelRibbonEmitterDef *)(f.getBuffer() + model_header_.ofsRibbonEmitters);
		ribbons_ = new RibbonEmitter[model_header_.nRibbonEmitters];
		for (size_t i=0; i<model_header_.nRibbonEmitters; i++) {
			ribbons_[i].model = this;
			ribbons_[i].init(f, rdefs[i], globalSequences);
		}
	}
}
//1月26日加入坐标颜色的改动
void CharacterModel::Draw()
{
	//对于每个pass 进行渲染 48是 下身的袍子好像
	for(int i =0;i <passes_.size() ;i++){
		ModelRenderPass& pa = passes_[i];
		int tex = pa.tex;
		bool  render_it = true;

		unsigned int bindtex = 0;
		if (specialTextures[tex]==-1) 
			bindtex = textures_[tex];
		else {
			bindtex = replaceTextures[specialTextures[tex]];
			if(bindtex == 0)
				render_it  = false;
		}
		//bindtex = 10;

		//1月18日加入 cull的使用 TODO: 使用到noZwrte參數 是否使用
		//还要用到z缓冲 因为我以前camera调的可视范围太大 所以Z精度不够 导致显示的乱七八糟的
		//后来跳近了摄像头 就基本ok了 卡了近5个小时
		if(pa.cull){ //剔除表面 剔除CCW
			d3d_device_->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW);
		}
		else{
			d3d_device_->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE);
		}
		if(pa.noZWrite){ //关闭Depth 检测试试
			d3d_device_->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);

		}
		else{ 
			d3d_device_->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
		}
		//1月26日加入的 alpha blend
		// ALPHA BLENDING
		// blend mode 下边的两行代码会影响到 法术效果的渲染 暂时
		////d3d_device_->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);  
		/////d3d_device_->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		switch (pa.blendmode) {
			case BM_OPAQUE:	// 0
				break;
			case BM_TRANSPARENT: // 1
				///glEnable(GL_ALPHA_TEST);
				 d3d_device_->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE); 
				///glAlphaFunc(GL_GEQUAL,0.7f);
				  d3d_device_->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);

				break;
			case BM_ALPHA_BLEND: // 2
				///glEnable(GL_BLEND);
				d3d_device_->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
				///glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // default blend func
				d3d_device_->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);  
				d3d_device_->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
				break;
			case BM_ADDITIVE: // 3 
				///glEnable(GL_BLEND);
				d3d_device_->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
				///glBlendFunc(GL_SRC_COLOR, GL_ONE);
				d3d_device_->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);  
				break;
			case BM_ADDITIVE_ALPHA: // 4 这里有问题  glBlendFunc(GL_SRC_COLOR, GL_ONE);在D3D中的对应为 新的 1月29日改动 改正了很多问题 同理在粒子系统中也有改动
				///glEnable(GL_BLEND);
				d3d_device_->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
				///glBlendFunc(GL_SRC_ALPHA, GL_ONE);
				///这个是以前的错误的d3d_device_->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
				//这个可能是正确的
				d3d_device_->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA); 
				d3d_device_->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE); 
				break;
			case BM_MODULATE:	// 5
				///glEnable(GL_BLEND);
				d3d_device_->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
				///glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);
				d3d_device_->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR);
					break;
			case BM_MODULATEX2:	// 6, not sure if this is right
				///glEnable(GL_BLEND);
				d3d_device_->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
				///glBlendFunc(GL_DST_COLOR,GL_SRC_COLOR);	
				d3d_device_->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR);

				break;
			default:
				wxLogMessage(wxT("[Error] Unknown blendmode: %d\n"), pa.blendmode);
				///glEnable(GL_BLEND);
				d3d_device_->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
				///glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);
				d3d_device_->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR);

		}
		//这里对此renderpass 涉及到的顶点进行颜色的修正 1月29日基本完成
		Vec3D true_color = Vec3D(1.0,1.0,1.0);  //渲染的颜色
		FLOAT opa1 = 1.0;
		//Color
		if(pa.color != -1 && colors_ && colors_[pa.color].color.uses(0) ){
			Vec3D c;
			c = colors_[pa.color].color.getValue(0,current_anim_frame_);
			if(colors_[pa.color].opacity.uses(0)) {
				opa1 = colors_[pa.color].opacity.getValue(0,current_anim_frame_);
			}
			//这里忽略了opacity的信息 1月29日应该加入opacity的处理 使得颜色看起来不那么强 这个也是的渲染效果正确
			if(pa.unlit){
				true_color = c;						
			}
		}
		//Opacity
		FLOAT opa = 1.0;
		if(pa.opacity != -1){
			opa = trans_[pa.opacity].trans.getValue(0, current_anim_frame_);
			//这里认为小于0的 就是不显示 可能有问题 但是现在是一个很正确的方法 
			if(opa < 0)
				opa = 0.0;
		}
		//printf("Frame:%d opa %f\n",current_anim_frame_,opa);
		int r = true_color.x * 255.0; int g = true_color.y*255.0; int b = true_color.z*255.0;
		r*= opa*opa1; g*=opa*opa1; b*=opa*opa1;
		
		DWORD color_new = RGB(b,g,r);	
		//Lock The Buffer 重新画颜色
		//color_new = 0;
		VOID* pVertices;
		if( FAILED( vetex_buffer_->Lock( 0, 0, ( void** )&pVertices, 0 ) ) ){
			printf("Error in Fill Vertex Buffer\n");		
			return ;
		}
		CHARACTERVERTEX* vertex_start = (CHARACTERVERTEX*)pVertices;
		for(int ci = pa.vertexStart;ci < pa.vertexEnd;ci++){
			CHARACTERVERTEX* vetex_to_change = &vertex_start[ci];
			vetex_to_change->color = color_new;
		}
		vetex_buffer_->Unlock();

		//1月29日加入对于TextureAnimation的处理
		//原代码中是这么写的 m->texAnims[texanim].setup(texanim);
		//其主要是对于Texture的矩阵的一个变换 
		if(pa.texanim != -1){
			texAnims[pa.texanim].setup(pa.texanim,d3d_device_);
		}
		else{
			D3DXMATRIX MatTexture;
			D3DXMatrixIdentity(&MatTexture);
			d3d_device_->SetTransform(D3DTS_TEXTURE0, &MatTexture); // 无效 
		}
		

		// Render the vertex buffer contents
		d3d_device_->SetStreamSource( 0, vetex_buffer_, 0, sizeof( CHARACTERVERTEX ) );
		d3d_device_->SetIndices(index_buffer_);
		//d3d_device_->SetTexture(0,texturemanager.GetTexture(bindtex));
		//得到这个纹理id 
		Texture& c_tex = *((Texture*)texturemanager.items[bindtex] );
		
		d3d_device_->SetTexture(0,c_tex.tex);
		//1月29日加入的纹理过滤方式 可能会对程序的效果产生重要影响 线性纹理过滤？
		d3d_device_->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		d3d_device_->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);

		//d3d_device_->SetTexture(0,0);

		d3d_device_->SetFVF( D3DFVF_CUSTOMVERTEX );
		//g_pd3dDevice->DrawPrimitive( D3DPT_POINTLIST, 0, gp_current_model->GetVeticeCount() );
		//if(pa.tex == 1 || i == 4)
		if(render_it)
		{	
			//printf("pass %d use Texture id %d name %s Addr %x\n",i,bindtex,c_tex.texture_name,c_tex.tex,c_tex.tex);
			if(i!= 48 && i!= 48)
				d3d_device_->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,0,0,count_vetices_,pa.indexStart,pa.indexCount/3);				
		}
		else{
			//printf("pass %d use Texture id -1\n",i);
		}
		//这里对于之前对d3ddevice设置进行复原 防止影响到其他pass 1月26日加入
		switch (pa.blendmode) {
			case BM_OPAQUE:
				break;
			case BM_TRANSPARENT:
				d3d_device_->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE); 
				//glAlphaFunc(GL_GEQUAL,0.04f);
				break;
			case BM_ALPHA_BLEND:
				d3d_device_->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
				break;
			case BM_ADDITIVE: // 3
				d3d_device_->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
				break;
			case BM_ADDITIVE_ALPHA: // 4
				//glDisable(GL_ALPHA_TEST);
				d3d_device_->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
				break;

			case BM_MODULATE: // 5
				d3d_device_->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
				break;

			case BM_MODULATEX2: // 6
				d3d_device_->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
				break;
			default:
				d3d_device_->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
				d3d_device_->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);  
				d3d_device_->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		}
		if (pa.noZWrite)
			d3d_device_->SetRenderState(D3DRS_ZWRITEENABLE, TRUE); 
		if (pa.cull)
			d3d_device_->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);

	}
	//Particle Systems
	d3d_device_->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
	d3d_device_->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE);
	//for(int i = 0;i < model_header_.nParticleEmitters;i++){
	for(int i = 0;i < model_header_.nParticleEmitters;i++){
		particleSystems_[i].draw();
	}
}

//1月24日 加入 用来体积碰撞检测的Bounding Triangles的画出
void CharacterModel::DrawBoundingVolume()
{
	//Particle Systems
	d3d_device_->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
	d3d_device_->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE);

	if(bounding_buffer_ == NULL || bounding_index_ == NULL)
		return ;
	d3d_device_->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME);
	d3d_device_->SetStreamSource( 0, bounding_buffer_, 0, sizeof( BOUNDINGVERTEX ) );
	d3d_device_->SetFVF( D3DFVF_BOUNDINGVERTEX);
	d3d_device_->SetIndices(bounding_index_);
	d3d_device_->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,0,0,model_header_.nBoundingVertices,0,model_header_.nBoundingTriangles/3);

	d3d_device_->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID);

	//Particle Systems
	d3d_device_->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
	d3d_device_->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW);
}

bool CharacterModel::InterSect(int x,int y,FLOAT* distance,Vec3D* cords_world,Vec3D* char_pos,Vec3D* char_rot)
{
	POINT point;
	//::GetCursorPos(&point);
	//::ScreenToClient(g_hwnd,&point);
	point.x = x;
	point.y = y;
	bool is_intersect = false;

	RayDetect ray(point.x,point.y);
	Vec3D world; //交點的世界坐標
	FLOAT dis = 0.0f;
	if(ray.Intersect(bounds,model_header_.nBoundingVertices,boundTris,model_header_.nBoundingTriangles,&dis,&world,char_pos,char_rot)  ){
		//printf("Intersect distance is  %f\n",distance);
		is_intersect = true;
		if(distance)
			*distance = dis;
		if(cords_world)
			*cords_world = world;
	}
	return is_intersect;
}

uint32 CharacterModel::FrameCount(int anim_id)
{
	ModelAnimation &a = anims[anim_id];
	return a.timeEnd - a.timeStart;
}

void CharacterModel::UpdateParticle(FLOAT dt)
{
	//这里我一直设置dtime = 0.1
	for(int i = 0;i < model_header_.nParticleEmitters;i++){
		particleSystems_[i].update(dt);
	}
}

void CharacterModel::InitVertex(MPQFile& f)
{
	vertices_ = new Vec3D[model_header_.nVertices];
	normals_ = new Vec3D[model_header_.nVertices];
	//多少顶点
	count_vetices_ = model_header_.nVertices;
	//指向文件中顶点的位移
	ModelVertex * origVertices = (ModelVertex *)(f.getBuffer() + model_header_.ofsVertices );
	//顶点集
	// Correct the data from the model, so that its using the Y-Up axis mode.
	float len = 0;  //计算半径 现在猜就是模型的体积的一个近似 暂时没用到
	for (size_t i=0; i<model_header_.nVertices; i++) {
		//这里的fixCoordSystem应该是原代码中OpenGL的右手坐标系的关系 到了D3D应该不需要的 
		vertices_[i] = fixCoordSystem(origVertices[i].pos );
		normals_[i]  = fixCoordSystem(origVertices[i].normal.normalize() );

		float len = origVertices[i].pos.lengthSquared();
		if (len > rad){ 
			rad = len;
		}
	}
	rad = sqrt((double)rad);
	//color和透明信息 透明信息不知道干什么用的
	// init colors
	if (model_header_.nColors) {
		colors_ = new ModelColor[model_header_.nColors];
		ModelColorDef *colorDefs = (ModelColorDef*)(f.getBuffer() + model_header_.ofsColors);
		for (size_t i=0; i<model_header_.nColors; i++) 
			colors_[i].init(f, colorDefs[i], globalSequences_);
	}

	// init transparency
	if (model_header_.nTransparency) {
		trans_ = new ModelTransparency[model_header_.nTransparency];
		ModelTransDef *trDefs = (ModelTransDef*)(f.getBuffer() + model_header_.ofsTransparency);
		for (size_t i=0; i<model_header_.nTransparency; i++) 
			trans_[i].init(f, trDefs[i], globalSequences_);
	}
}

void CharacterModel::InitIndex(MPQFile& f,MPQFile& g)
{
	ModelView *view = (ModelView*)(g.getBuffer());
	uint16 *indexLookup = (uint16*)(g.getBuffer() + view->ofsIndex);
	uint16 *triangles = (uint16*)(g.getBuffer() + view->ofsTris);
	int nIndices = view->nTris;
	printf("This model is %d vetices and %d indices\n",count_vetices_,nIndices);
	count_indices_ = nIndices;
	//wxDELETEA(indices);
	indices_ = new uint16[nIndices];
	for (size_t i = 0; i<nIndices; i++) {
		indices_[i] = indexLookup[triangles[i]];
	}
}

void CharacterModel::InitTexture(MPQFile& f,MPQFile& g)
{
	ModelTextureDef *texdef = (ModelTextureDef*)(f.getBuffer() + model_header_.ofsTextures);
	if (model_header_.nTextures) {
		textures_ = new TextureID[model_header_.nTextures];
		for (size_t i=0; i<model_header_.nTextures; i++) {
			// Error check
			if (i > TEXTURE_MAX-1) {
				wxLogMessage(wxT("Critical Error: Model Texture %d over %d"), model_header_.nTextures, TEXTURE_MAX);
				break;
			}
			if (texdef[i].type == TEXTURE_FILENAME) {
				wxString texname((char*)(f.getBuffer()+texdef[i].nameOfs), wxConvUTF8);
				textures_[i] = texturemanager.add(texname);
				TextureList.push_back(texname);
				wxLogMessage(wxT("Info: Added %s to the TextureList[%i]."), texname.c_str(), TextureList.size());
			} else {
				// special texture - only on characters and such...
				textures_[i] = 0;
				//while (texdef[i].type < TEXTURE_MAX && specialTextures[texdef[i].type]!=-1) texdef[i].type++;
				//if (texdef[i].type < TEXTURE_MAX)specialTextures[texdef[i].type] = (int)i;
				specialTextures[i] = texdef[i].type;


				wxString tex = wxT("Special_");
				tex << texdef[i].type;
				//tex = model_name_.BeforeLast('.').AfterLast(wxT('/')) + wxT("_");
				//tex += wxT("Cape.blp");
				///textures_[i] = texturemanager.add(tex);

				if (modelType == MT_NORMAL){
					if (texdef[i].type == TEXTURE_HAIR)
						tex = wxT("Hair.blp");
					else if(texdef[i].type == TEXTURE_BODY)
						tex = wxT("Body.blp");
					else if(texdef[i].type == TEXTURE_FUR)
						tex = wxT("Fur.blp");
				}
				wxLogMessage(wxT("Info: Added %s to the TextureList[%i] via specialTextures. Type: %i"), tex.c_str(), TextureList.size(), texdef[i].type);
				TextureList.push_back(tex);

				if (texdef[i].type < TEXTURE_MAX)
					useReplaceTextures[texdef[i].type] = true;

				if (texdef[i].type == TEXTURE_ARMORREFLECT) {
					// a fix for weapons with type-3 textures.
					replaceTextures[texdef[i].type] = texturemanager.add(wxT("Item\\ObjectComponents\\Weapon\\ArmorReflect4.BLP"));
				}
			}
		}
	}
	//这之后的主要任务就是将 Special_02 这种纹理 以及 Hair.blp这种纹理 转换成对应的真实的文件名
	//1.16日只做了 Body 的处理
	InitBodyTexture(f,g);
	//1.17 Hair 的处理
	InitHairTexture(f,g);
	InitCapeTexture(f,g);

}

void CharacterModel::InitGeoSet(MPQFile& f,MPQFile& g)
{
	
	ModelTextureDef *texdef = (ModelTextureDef*)(f.getBuffer() + model_header_.ofsTextures);
	ModelView *view = (ModelView*)(g.getBuffer());
	//GeoSet應該是某個部位 和TexUnit應該是特定部位的紋理
	// render ops
	ModelGeoset *ops = (ModelGeoset*)(g.getBuffer() + view->ofsSub);
	ModelTexUnit *tex = (ModelTexUnit*)(g.getBuffer() + view->ofsTex);

	ModelRenderFlags *renderFlags = (ModelRenderFlags*)(f.getBuffer() + model_header_.ofsTexFlags);
	uint16 *texlookup = (uint16*)(f.getBuffer() + model_header_.ofsTexLookup);
	uint16 *texanimlookup = (uint16*)(f.getBuffer() + model_header_.ofsTexAnimLookup);
	int16 *texunitlookup = (int16*)(f.getBuffer() + model_header_.ofsTexUnitLookup);
	// Transparency
	int16 *transLookup = (int16*)(f.getBuffer() + model_header_.ofsTransparencyLookup);

	for (size_t i=0; i<view->nSub; i++) {
		geosets_.push_back(ops[i]);
		//showGeosets[i] = true;
	}

	passes_.clear();
	for (size_t j = 0; j<view->nTex; j++) {
		ModelRenderPass pass;

		pass.useTex2 = false;
		pass.useEnvMap = false;
		pass.cull = false;
		pass.trans = false;
		pass.unlit = false;
		pass.noZWrite = false;
		pass.billboard = false;
		pass.texanim = -1; // no texture animation

		//pass.texture2 = 0;
		size_t geoset = tex[j].op;

		pass.geoset = (int)geoset;

		pass.indexStart = ops[geoset].istart;
		pass.indexCount = ops[geoset].icount;
		pass.vertexStart = ops[geoset].vstart;
		pass.vertexEnd = pass.vertexStart + ops[geoset].vcount;

		//TextureID texid = textures[texlookup[tex[j].textureid]];
		//pass.texture = texid;
		pass.tex = texlookup[tex[j].textureid];

		// TODO: figure out these flags properly -_-
		ModelRenderFlags &rf = renderFlags[tex[j].flagsIndex];

		pass.blendmode = rf.blend;
		//if (rf.blend == 0) // Test to disable/hide different blend types
		//	continue;

		pass.color = tex[j].colorIndex;
		//printf("pass %d index %d\n",j,pass.color);
		pass.opacity = transLookup[tex[j].transid];

		pass.unlit = (rf.flags & RENDERFLAGS_UNLIT) != 0;

		pass.cull = (rf.flags & RENDERFLAGS_TWOSIDED) == 0;

		pass.billboard = (rf.flags & RENDERFLAGS_BILLBOARD) != 0;

		// Use environmental reflection effects?
		pass.useEnvMap = (texunitlookup[tex[j].texunit] == -1) && pass.billboard && rf.blend>2; //&& rf.blend<5;

		// Disable environmental mapping if its been unchecked.
		////if (pass.useEnvMap && !video.useEnvMapping)
		pass.useEnvMap = false;

		pass.noZWrite = (rf.flags & RENDERFLAGS_ZBUFFERED) != 0;

		// ToDo: Work out the correct way to get the true/false of transparency
		pass.trans = (pass.blendmode>0) && (pass.opacity>0);	// Transparency - not the correct way to get transparency

		pass.p = ops[geoset].BoundingBox[0].z;

		// Texture flags
		pass.swrap = (texdef[pass.tex].flags & TEXTURE_WRAPX) != 0; // Texture wrap X
		pass.twrap = (texdef[pass.tex].flags & TEXTURE_WRAPY) != 0; // Texture wrap Y

		// tex[j].flags: Usually 16 for static textures, and 0 for animated textures.	
		////1月29日加入
		if (model_header_.nTexAnims > 0 && (tex[j].flags & TEXTUREUNIT_STATIC) == 0) {
			pass.texanim = texanimlookup[tex[j].texanimid];
		}

		passes_.push_back(pass);
	}
}

//这里的10是血精灵 1是女 3是skin color 0是useNPC
Races race = RACE_BLOODELF;
Gender gender = GENDER_FEMALE;
int skincolor = 1;

void CharacterModel::InitBodyTexture(MPQFile& f,MPQFile& g)
{
	///1月16日 下边的代码摘自 CharControl.cpp 中的RefreshModel
	// 主要是如何将 Special_2这个纹理替换成相对应的真正的纹理 可能涉及到人物数据库
	CharSectionsDB::Record rec = chardb.getRecord(0);
	//然后是body的纹理 这个比较麻烦 涉及到纹理的compose
	//应该是涉及到一些位图blend的操作 我先把代码写死 这些代码来自wmv的运行情况
	
	CharTexture c_tex;
	rec = chardb.getRecord(0);
	try {
		rec = chardb.getByParams(race, gender, CharSectionsDB::SkinType, 0, skincolor, 0);
		wxString baseTexName = rec.getString(CharSectionsDB::Tex1);
		c_tex.addLayer(baseTexName, CR_BASE, 0);
		//UpdateTextureList(baseTexName, TEXTURE_BODY);

		// Tauren fur
		wxString furTexName = rec.getString(CharSectionsDB::Tex2);
		if (!furTexName.IsEmpty())
		{
			///furTex = texturemanager.add(furTexName); 毛的纹理暂时没考虑
			///UpdateTextureList(furTexName, TEXTURE_FUR);
		}

	} catch (CharSectionsDB::NotFound) {
		wxLogMessage(wxT("Assertion base character Error: %s : line #%i : %s"), __FILE__, __LINE__, __FUNCTION__);
	}
	//哥布林那部分我没考虑 
	bool showUnderwear = false;
	if (showUnderwear) {  //显示内衣
		try {
			rec = chardb.getByParams(race, gender, CharSectionsDB::UnderwearType, 0, skincolor, 0);
			c_tex.addLayer(rec.getString(CharSectionsDB::Tex1), CR_PELVIS_UPPER, 1); // pants
			c_tex.addLayer(rec.getString(CharSectionsDB::Tex2), CR_TORSO_UPPER, 1); // top
		} catch (CharSectionsDB::NotFound) {
			wxLogMessage(wxT("DBC underwear Error: %s : line #%i : %s"), __FILE__, __LINE__, __FUNCTION__);
		}
	}
	// face
	try {
		int face_type = 1;
		rec = chardb.getByParams(race, gender, CharSectionsDB::FaceType, face_type, skincolor, 0);
		c_tex.addLayer(rec.getString(CharSectionsDB::Tex1), CR_FACE_LOWER, 1);
		c_tex.addLayer(rec.getString(CharSectionsDB::Tex2), CR_FACE_UPPER, 1);
	} catch (CharSectionsDB::NotFound) {
		wxLogMessage(wxT("DBC face Error: %s : line #%i : %s"), __FILE__, __LINE__, __FUNCTION__);
	}
	//脸的geoset没弄因为不知道是什么东西 暂时
	//脸部细节的纹理
	int facialHair = 2;
	int facialColor = 2;
	try {
		rec = chardb.getByParams(race, gender, CharSectionsDB::FacialHairType, facialHair, facialColor, 0);
		c_tex.addLayer(rec.getString(CharSectionsDB::Tex1), CR_FACE_LOWER, 2);
		c_tex.addLayer(rec.getString(CharSectionsDB::Tex2), CR_FACE_UPPER, 2);
	} catch (CharSectionsDB::NotFound) {
		wxLogMessage(wxT("DBC facial feature Error: %s : line #%i : %s"), __FILE__, __LINE__, __FUNCTION__);
	}
	//HairStyle GeoSet那部分还没有考虑
	// hair
	bool bald = true; //光头 先不考虑这个纹理
	int hairTex = 0; //纹理的id
	int hairStyle = 2;
	int hairColor = 1;
	try {
		rec = chardb.getByParams(race, gender, CharSectionsDB::HairType, hairStyle, hairColor, 0);
		wxString hairTexfn = rec.getString(CharSectionsDB::Tex1);
		if (!hairTexfn.IsEmpty()) 
		{
			hairTex = texturemanager.add(hairTexfn);
			//UpdateTextureList(hairTexfn, TEXTURE_HAIR);
		}
		else {
			// oops, looks like we're missing a hair texture. Let's try with hair style #0.
			// (only a problem for orcs with no hair but some beard
			try {
				rec = chardb.getByParams(race, gender, CharSectionsDB::HairType, hairStyle, hairColor, 0);
				hairTexfn = rec.getString(CharSectionsDB::Tex1);
				if (!hairTexfn.IsEmpty()) 
				{
					hairTex = texturemanager.add(hairTexfn);
					//UpdateTextureList(hairTexfn, TEXTURE_HAIR);
				}
				else 
					hairTex = 0;
			} catch (CharSectionsDB::NotFound) {
				// oh well, give up.
				hairTex = 0; // or chartex?
			}
		}
		if (!bald) {
			c_tex.addLayer(rec.getString(CharSectionsDB::Tex2), CR_FACE_LOWER, 3);
			c_tex.addLayer(rec.getString(CharSectionsDB::Tex3), CR_FACE_UPPER, 3);
		}

	} catch (CharSectionsDB::NotFound) {
		wxLogMessage(wxT("DBC hair Error: %s : line #%i : %s"), __FILE__, __LINE__, __FUNCTION__);
		hairTex = 0;
	}
	//這裡我讓人物 穿上了一個外袍 外袍的id是48750
	unsigned int itemID = 48750;
	ItemDisplayDB::Record r = itemdisplaydb.getById(itemID);
	//layer 不清楚 
	int layer = 17;

	c_tex.addLayer(makeItemTexture(CR_ARM_UPPER, r.getString(ItemDisplayDB::TexArmUpper)), CR_ARM_UPPER, layer);
	c_tex.addLayer(makeItemTexture(CR_ARM_LOWER, r.getString(ItemDisplayDB::TexArmLower)), CR_ARM_LOWER, layer);

	c_tex.addLayer(makeItemTexture(CR_TORSO_UPPER, r.getString(ItemDisplayDB::TexChestUpper)), CR_TORSO_UPPER, layer);
	c_tex.addLayer(makeItemTexture(CR_TORSO_LOWER, r.getString(ItemDisplayDB::TexChestLower)), CR_TORSO_LOWER, layer);

	//if (type == IT_ROBE || r.getUInt(ItemDisplayDB::RobeGeosetFlags)==1) {
	c_tex.addLayer(makeItemTexture(CR_LEG_UPPER, r.getString(ItemDisplayDB::TexLegUpper)), CR_LEG_UPPER, layer);
	c_tex.addLayer(makeItemTexture(CR_LEG_LOWER, r.getString(ItemDisplayDB::TexLegLower)), CR_LEG_LOWER, layer);
	//}
	//這裡我讓人物帶上手套 Glove layer = 20    id =   16488
	r = itemdisplaydb.getById(16488);
	c_tex.addLayer(makeItemTexture(CR_HAND, r.getString(ItemDisplayDB::TexHands)), CR_HAND, 20);
	c_tex.addLayer(makeItemTexture(CR_ARM_LOWER, r.getString(ItemDisplayDB::TexArmLower)), CR_ARM_LOWER, 20);
	//讓人物穿上鞋子 Boot layer = 15    id = 16703
	r = itemdisplaydb.getById(16703);
	c_tex.addLayer(makeItemTexture(CR_LEG_LOWER, r.getString(ItemDisplayDB::TexLegLower)), CR_LEG_LOWER, 15);
	//下邊的一行 是不顯示腳的時候調用的 也就是 鞋子 分為腿的部分 和小的部分
	c_tex.addLayer(makeItemTexture(CR_FOOT, r.getString(ItemDisplayDB::TexFeet)), CR_FOOT, 15);

	//纹理blend
	unsigned int new_id = c_tex.compose(0,"Body.blp");
	replaceTextures[TEXTURE_BODY] = new_id;
}
int hair_style = 7;
int hairColor = 5;

void CharacterModel::InitHairTexture(MPQFile& f,MPQFile& g)
{
	// 先弄头发
	CharSectionsDB::Record rec = chardb.getRecord(0);
	unsigned int new_id = 0;
	// hair
	try {
		rec = chardb.getByParams(race, gender, CharSectionsDB::HairType, hair_style, hairColor, 0);
		wxString hairTexfn = rec.getString(CharSectionsDB::Tex1);
		if (!hairTexfn.IsEmpty()) 
		{
			new_id = texturemanager.add(hairTexfn);
		}
		else {
			// oops, looks like we're missing a hair texture. Let's try with hair style #0.
			// (only a problem for orcs with no hair but some beard
			try {
				rec = chardb.getByParams(race, gender, CharSectionsDB::HairType, 0, hairColor, 0);
				hairTexfn = rec.getString(CharSectionsDB::Tex1);
				if (!hairTexfn.IsEmpty()) 
				{
					new_id = texturemanager.add(hairTexfn);
				}
				else 
					new_id = 0;
			} catch (CharSectionsDB::NotFound) {
				// oh well, give up.
				new_id = 0; // or chartex?
			}
		}
		//if (!bald) {
			//这里有头发的纹理 我们先不考虑
			//tex.addLayer(rec.getString(CharSectionsDB::Tex2), CR_FACE_LOWER, 3);
			//tex.addLayer(rec.getString(CharSectionsDB::Tex3), CR_FACE_UPPER, 3);
		//}

	} catch (CharSectionsDB::NotFound) {
		wxLogMessage(wxT("DBC hair Error: %s : line #%i : %s"), __FILE__, __LINE__, __FUNCTION__);
		new_id = 0;
	}
	//增加纹理 然后返回纹理的id
	printf("Hair Texture id is %d\n",new_id);	
	replaceTextures[TEXTURE_HAIR] = new_id;
	
	

}

void CharacterModel::InitCapeTexture(MPQFile& f,MPQFile& g)
{	
	//我这里随便家一个 capde 的纹理
	unsigned int new_id = texturemanager.add("Item\\ObjectComponents\\Cape\\Cape_Leather_D_01Red.BLP");
	//replaceTextures[TEXTURE_CAPE] = new_id;
	
}



void CharacterModel::InitBounding(MPQFile& f,MPQFile& g)
{
	if (model_header_.nBoundingVertices > 0) {
		bounds = new Vec3D[model_header_.nBoundingVertices];
		Vec3D *b = (Vec3D*)(f.getBuffer() + model_header_.ofsBoundingVertices);
		for (size_t i=0; i<model_header_.nBoundingVertices; i++) {
			bounds[i] = fixCoordSystem(b[i]);
		}
	}
	if (model_header_.nBoundingTriangles > 0) {
		boundTris = new uint16[model_header_.nBoundingTriangles];
		memcpy(boundTris, f.getBuffer() + model_header_.ofsBoundingTriangles, model_header_.nBoundingTriangles*sizeof(uint16));
	}
	//Fill In Vertex Buffer And index Buffer
	if(!boundTris || !bounds){
		printf("This Model no Bounding Vetex\n");
		return ;
	}
	if( FAILED( d3d_device_->CreateVertexBuffer( model_header_.nBoundingVertices * sizeof( BOUNDINGVERTEX ),
		0, D3DFVF_BOUNDINGVERTEX,
		D3DPOOL_DEFAULT, &bounding_buffer_, NULL ) ) )
	{
		printf("Error In CreateVetex\n");
		return ;
	}
	// Fill the vertex buffer.
	VOID* pVertices;
	if( FAILED( bounding_buffer_->Lock( 0, 0, ( void** )&pVertices, 0 ) ) ){
		printf("Error in Fill Vertex Buffer\n");		
		return ;
	}
	memcpy( pVertices, bounds, model_header_.nBoundingVertices * sizeof(BOUNDINGVERTEX) );
	bounding_buffer_->Unlock();

	//這裡進行索引的初始化工作
	if( FAILED( d3d_device_->CreateIndexBuffer( model_header_.nBoundingTriangles  * sizeof( uint16 ),
		0, D3DFMT_INDEX16,
		D3DPOOL_DEFAULT, &bounding_index_, NULL ) ) )
	{
		return ;
	}

	// Fill the Index buffer.
	if( FAILED( bounding_index_->Lock( 0, model_header_.nBoundingTriangles , ( void** )&pVertices, 0 ) ) )
		return ;
	memcpy( pVertices, boundTris, model_header_.nBoundingTriangles*sizeof(uint16) );
	bounding_index_->Unlock();
}

void CharacterModel::FillVetex(MPQFile& f)
{
	//创建设备顶点缓冲
	// Create the vertex buffer.
	if( FAILED( d3d_device_->CreateVertexBuffer( count_vetices_ * sizeof( CHARACTERVERTEX ),
		0, D3DFVF_CUSTOMVERTEX,
		D3DPOOL_DEFAULT, &vetex_buffer_, NULL ) ) )
	{
		printf("Error In CreateVetex\n");
		return ;
	}
	ModelVertex * origVertices = (ModelVertex *)(f.getBuffer() + model_header_.ofsVertices );
	CHARACTERVERTEX* buffer = new CHARACTERVERTEX[count_vetices_];
	for(int i = 0;i < count_vetices_;i++){  //这里没考虑color 因为不理解color究竟如何用
		/*Vec3D color;
		int color_index = 0;
		//这里应该是通过查找这个顶点属于哪个pass 来确定这个顶点的颜色
		for(int j = 0;j < passes_.size();j++){
			if( i >= passes_[j].vertexStart && i < passes_[j].vertexStart + passes_[j].vertexEnd){
				color_index = passes_[j].color;
				break;
			}
		}
		if(color_index == -1)
			color_index = 0;
		color = colors_[color_index].color.getValue(0,0);*/
		
		CHARACTERVERTEX vertex;
		vertex.color = RGB(255,255,255);
		vertex.x = vertices_[i].x;
		vertex.y = vertices_[i].y;
		vertex.z = vertices_[i].z;
		vertex.u = origVertices[i].texcoords.x;
		vertex.v = origVertices[i].texcoords.y;
		vertex.nx = normals_[i].x;
		vertex.ny = normals_[i].y;
		vertex.nz = normals_[i].z;
		buffer[i] = vertex;
	}
	// Fill the vertex buffer.
	VOID* pVertices;
	if( FAILED( vetex_buffer_->Lock( 0, 0, ( void** )&pVertices, 0 ) ) ){
		printf("Error in Fill Vertex Buffer\n");		
		return ;
	}
	memcpy( pVertices, buffer, count_vetices_ * sizeof(CHARACTERVERTEX) );
	vetex_buffer_->Unlock();
	delete[]  buffer;


}

void CharacterModel::FillIndex(MPQFile& f,MPQFile& g)
{
	//這裡進行索引的初始化工作
	if( FAILED( d3d_device_->CreateIndexBuffer( count_indices_ * sizeof( uint16 ),
		0, D3DFMT_INDEX16,
		D3DPOOL_DEFAULT, &index_buffer_, NULL ) ) )
	{
		return ;
	}

	// Fill the vertex buffer.
	void* pVertices; 
	if( FAILED( index_buffer_->Lock( 0, count_indices_, ( void** )&pVertices, 0 ) ) )
		return ;
	memcpy( pVertices, indices_, count_indices_*sizeof(uint16) );
	index_buffer_->Unlock();

}

void CharacterModel::FillTexture(MPQFile& f,MPQFile& g)
{

}


