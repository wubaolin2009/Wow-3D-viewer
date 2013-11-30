#ifndef _CREATURE_H_
#define _CREATURE_H_
//1月18日 話生物的
#include "enums.h"
#include "Character.h"

class CreatureModel:public CharacterModel
{
public:
	CreatureModel(const wxString& model_name,LPDIRECT3DDEVICE9 device):CharacterModel(model_name,device){}
	virtual void Draw();
protected:
	//重写这几个纹理函数 使其什么也不做
	virtual void InitBodyTexture(MPQFile& f,MPQFile& g);
	//初始化 Hair的纹理 原理类似于Body
	virtual void InitHairTexture(MPQFile& f,MPQFile& g);
	//初始化 Capde的纹理 类似于Body
	//对于Item来说 Cape就是TextureItem
	virtual void InitCapeTexture(MPQFile& f,MPQFile& g);

};

#endif