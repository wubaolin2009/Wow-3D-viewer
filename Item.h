//1月17日 这个用来渲染 item 具体是某个部件什么的
//因为可能和Character的代码相同 所以我先继承自 Character
#ifndef _ITEM_H_
#define _ITEM_H_
#include "enums.h"
#include "Character.h"

class ItemModel:public CharacterModel
{
public:
	ItemModel(const wxString& model_name,LPDIRECT3DDEVICE9 device):CharacterModel(model_name,device){}
	virtual void Draw();
protected:
	//重写这几个纹理函数 使其什么也不做
	virtual void InitBodyTexture(MPQFile& f,MPQFile& g);
	//初始化 Hair的纹理 原理类似于Body
	virtual void InitHairTexture(MPQFile& f,MPQFile& g);
	//初始化 Capde的纹理 类似于Body
	//对于Item来说 Cape就是TextureItem
	virtual void InitCapeTexture(MPQFile& f,MPQFile& g);
	//1月29日重写
	virtual void calcBones(ssize_t anim, size_t time);

};



#endif