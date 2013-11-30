#include <wx/wx.h>
#include "creature.h"
#include "mpq.h"
#include "texture.h"
#include "DB.h"


extern ItemDisplayDB itemdisplaydb;
extern TextureManager texturemanager;

void CreatureModel::InitBodyTexture(MPQFile& f,MPQFile& g)
{

}

void CreatureModel::InitHairTexture(MPQFile& f,MPQFile& g)
{

}

void CreatureModel::InitCapeTexture(MPQFile& f,MPQFile& g)
{

}

void CreatureModel::Draw()
{
	//这里暂时和CharacterModel一样
	CharacterModel::Draw();

}