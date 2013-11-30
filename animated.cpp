#include "animated.h"

void ModelColor::init(MPQFile &f, ModelColorDef &mcd, uint32 *global)
{
	color.init(mcd.color, f, global);
	opacity.init(mcd.opacity, f, global);
}

void ModelTransparency::init(MPQFile &f, ModelTransDef &mcd, uint32 *global)
{
	trans.init(mcd.trans, f, global);
}

//1月26日加入 TexAnim
void TextureAnim::calc(ssize_t anim, size_t time)
{
	if (trans.uses(anim)) {
		tval = trans.getValue(anim, time);
	}
	if (rot.uses(anim)) {
		rval = rot.getValue(anim, time);
	}
	if (scale.uses(anim)) {
		sval = scale.getValue(anim, time);
	}
}
//1月29日 对纹理进行变换
void TextureAnim::setup(ssize_t anim,LPDIRECT3DDEVICE9 p_d3ddevice)
{
	/*glLoadIdentity();
	if (trans.uses(anim)) {
		glTranslatef(tval.x, tval.y, tval.z);
	}
	if (rot.uses(anim)) {
		glRotatef(rval.x, 0, 0, 1); // this is wrong, I have no idea what I'm doing here ;)
	}
	if (scale.uses(anim)) {
		glScalef(sval.x, sval.y, sval.z);
	}*/
	D3DXMATRIX MatTexture, MatScale, MatTrans, MatRotate; 
	D3DXMatrixIdentity(&MatScale); 
	D3DXMatrixIdentity(&MatTrans); 
	D3DXMatrixIdentity(&MatRotate); 
	if(trans.uses(anim)){
		D3DXMatrixTranslation(&MatTrans,tval.x,tval.y,tval.z);
	}
	if(rot.uses(anim)){
		//这个暂时不写
	}
	if(scale.uses(anim)){
		D3DXMatrixScaling(&MatScale,sval.x,sval.y,sval.z);
	}
	//Bingo!
	MatTrans.m[2][0] = MatTrans.m[3][0]; //u offset
	MatTrans.m[2][1] = MatTrans.m[3][1];   //v offset

	MatTexture =  MatScale * MatTrans; 

	p_d3ddevice->SetTransform(D3DTS_TEXTURE0, &MatTexture); // 无效 
	//不小的这句干什么的
	p_d3ddevice->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2); 


}

void TextureAnim::init(MPQFile &f, ModelTexAnimDef &mta, uint32 *global)
{
	trans.init(mta.trans, f, global);
	rot.init(mta.rot, f, global);
	scale.init(mta.scale, f, global);
}

