#include "mpq.h"
#include "Item.h"
#include "texture.h"
#include "DB.h"

extern ItemDisplayDB itemdisplaydb;
extern TextureManager texturemanager;

void ItemModel::InitBodyTexture(MPQFile& f,MPQFile& g)
{

}

void ItemModel::InitHairTexture(MPQFile& f,MPQFile& g)
{

}

wxString sFilterDir;
bool filterDir(wxString fn)
{
	wxString tmp = fn.Lower();
	return (tmp.StartsWith(sFilterDir) && tmp.EndsWith(wxT("blp")));
}

void ItemModel::InitCapeTexture(MPQFile& f,MPQFile& g)
{
	//CharacterModel::InitCapeTexture(f,g);
	//这里应该根据Item的Id 去加载相应的纹理 
	//代码参考于 AnimControl.cpp 中的 UpdateItemModel
	wxString fn = model_name_;

	// change M2 to mdx
	fn = fn.BeforeLast(wxT('.')) + wxT(".mdx");

	// Check to see if its a helmet model, if so cut off the race
	// and gender specific part of the filename off
	if (fn.Find(wxT("\\head\\")) > wxNOT_FOUND || fn.Find(wxT("\\Head\\")) > wxNOT_FOUND) {
		fn = fn.BeforeLast('_') + wxT(".mdx");
	}

	// just get the file name, exclude the path.
	fn = fn.AfterLast(wxT('\\'));

	TextureSet skins;

	for (ItemDisplayDB::Iterator it=itemdisplaydb.begin(); it!=itemdisplaydb.end(); ++it) {
		if (fn.IsSameAs(it->getString(ItemDisplayDB::Model), false)) {
			TextureGroup grp;
			grp.base = TEXTURE_ITEM;
			grp.count = 1;
			wxString skin = it->getString(ItemDisplayDB::Skin);
			grp.tex[0] = skin;
			if (grp.tex[0].length() > 0) 
				skins.insert(grp);
		}

		//if (!strcmp(it->getString(ItemDisplayDB::Model2), fn.c_str())) {
		if (fn.IsSameAs(it->getString(ItemDisplayDB::Model2), false)) {
			TextureGroup grp;
			grp.base = TEXTURE_ITEM;
			grp.count = 1;
			wxString skin = it->getString(ItemDisplayDB::Skin2);
			grp.tex[0] = skin;
			if (grp.tex[0].length() > 0) 
				skins.insert(grp);
		}
	}


	// Search the same directory for BLPs
	std::set<FileTreeItem> filelist;
	sFilterDir = model_name_.BeforeLast(wxT('.')).Lower();
	getFileLists(filelist, filterDir);
	if (filelist.begin() != filelist.end()) {
		TextureGroup grp;
		grp.base = TEXTURE_ITEM;
		grp.count = 1;
		for (std::set<FileTreeItem>::iterator it = filelist.begin(); it != filelist.end(); ++it) {
			grp.tex[0] = (*it).displayName.BeforeLast(wxT('.')).AfterLast(wxT('\\'));
			skins.insert(grp);
		}
	}

	bool ret = false;

	if (!skins.empty()) {
		//这里我们主要打印一些信息 然后 选择最后一个Skin作为物品的纹理
		printf("We have found %d skins for the Item\n",skins.size() );
		for(TextureSet::iterator it = skins.begin(); it != skins.end(); ++it){
			printf("Skin name is %s\n",it->tex[0]);
		}
		//ret = FillSkinSelector(skins);
		//int mySkin = 0;
		for(TextureSet::iterator it = skins.begin(); it != skins.end(); ++it){
			wxString name_ = model_name_.BeforeLast(wxT('\\')) + wxT('\\') + it->tex[0] + wxT(".blp"); 
			for (ssize_t j=0; j<TEXTURE_MAX; j++) {
				if (it->base == specialTextures[j]) {
					TextureList[j] = name_;
					replaceTextures[it->base] = texturemanager.add(name_);
					printf("We use texture %s as special id %d\n",name_,it->base);
					return;
				}
			}
		}
	}


}

void ItemModel::calcBones(ssize_t anim, size_t time)
{
	for (size_t i=0; i<model_header_.nBones; i++) {
		bones[i].calc = false;
	}
	size_t a, t;
	a = anim;
	t = time;
	for (size_t i=0; i<model_header_.nBones; i++) {
		bones[i].calcMatrix(bones, anim, time);
	}
}

void ItemModel::Draw()
{
	//这里暂时和CharacterModel一样
	CharacterModel::Draw();

}