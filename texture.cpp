#include "texture.h"
#include "mpq.h"
#include "ximage.h"
#include <d3dx9.h>
#include <algorithm>
extern LPDIRECT3DDEVICE9  g_pd3dDevice; // Our rendering device

extern TextureManager texturemanager;


LPDIRECT3DTEXTURE9 TextureManager::GetTexture(int id)
{
	return maps_[id];
}


unsigned int TextureManager::add(wxString name)
{
	// if the item already exists, return the existing ID
	if (names.find(name) != names.end()) {
		id = names[name];
		items[id]->addref();
		return id;
	}

	// Else, create the texture

	Texture *tex = new Texture(name);
	if (tex) {
		// clear old texture memory from vid card
		//glDeleteTextures(1, &id);
		// create new texture and put it in memory
		//glGenTextures(1, &id);
		
		tex->id = id;
		tex->texture_name = name;
		LoadBLP(id, tex,name);

		do_add(name, id, tex);
		id++;
		return tex->id;
	}

	return 0;
}

unsigned int TextureManager::add(wxString name,LPDIRECT3DTEXTURE9 texture)
{
	Texture *tex = new Texture(name);
	if(tex){
		tex->id = id;
		tex->texture_name = name;
		tex->tex = texture;
		do_add(name,id,tex);
		id++;
	}
	return tex->id;

}

LPDIRECT3DTEXTURE9 TextureManager::GetTexture(const wxString& name)
{
	if(!Exists(name)){ //加载纹理
		this->add(name);
	}
	return maps_string_[name];

}

void TextureManager::doDelete(unsigned int id)
{
	//if (glIsTexture(id)) {
		//glDeleteTextures(1, &id);
	//}
}

void TextureManager::LoadBLP(unsigned int id, Texture *tex,const wxString& name)
{
	
	// Vars
	int offsets[16], sizes[16], w=0, h=0, type=0;
	int format = 0;
	char attr[4];

	//我们这里先不考虑useLocalFiles
	/*
	if (useLocalFiles) {
		wxString texName(tex->name.c_str(), wxConvUTF8);
		BYTE *buffer = NULL;
		CxImage *image = NULL;

		if ((TryLoadLocalTexture(texName, CXIMAGE_FORMAT_PNG, &image) || 
			TryLoadLocalTexture(texName, CXIMAGE_FORMAT_TGA, &image)) 
			&& image) {
				long size = image->GetWidth() * image->GetHeight() * 4;
				if (image->Encode2RGBA(buffer, size, true)) {
					tex->w = image->GetWidth();
					tex->h = image->GetHeight();
					tex->compressed = true;

					GLuint texFormat = GL_TEXTURE_2D;
					glBindTexture(texFormat, id);

					glTexImage2D(texFormat, 0, GL_RGBA8, image->GetWidth(), image->GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);

					glTexParameteri(texFormat, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	// Linear Filtering
					glTexParameteri(texFormat, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	// Linear Filtering

					wxDELETE(image);
					wxDELETE(buffer);
					return;
				}
		}

	}*/

	// bind the texture
	//glBindTexture(GL_TEXTURE_2D, id);

	MPQFile f(tex->name);
	//if (g_modelViewer) {
		//g_modelViewer->modelOpened->Add(wxString(tex->name.c_str(), wxConvUTF8));
	//}
	if (f.isEof()) {
		tex->id = 0;
		wxLogMessage(wxT("Error: Could not load the texture '%s'"), wxString(tex->name.c_str(), wxConvUTF8).c_str());
		f.close();
		return;
	} else {
		//tex->id = id; // I don't see the id being set anywhere,  should I set it now?
		wxLogMessage(wxT("Loading texture: %s"), wxString(tex->name.c_str(), wxConvUTF8).c_str());
	}

	f.seek(4);
	f.read(&type,4);
	f.read(attr,4);
	f.read(&w,4);
	f.read(&h,4);
	f.read(offsets,4*16);
	f.read(sizes,4*16);

	tex->w = w;
	tex->h = h;

	bool hasmipmaps = (attr[3]>0);
	int mipmax = hasmipmaps ? 16 : 1;

	//
	//reference: http://en.wikipedia.org/wiki/.BLP
	//
	//wxLogMessage(wxT("[BLP]: type: %d, encoding: %d, alphadepth: %d, alphaencoding: %d, mipmap: %d, %d*%d"), type, attr[0], attr[1], attr[2], attr[3], w, h);
	if (type == 0) { // JPEG compression
		/*
		* DWORD JpegHeaderSize;
		* BYTE[JpegHeaderSize] JpegHeader;
		* struct MipMap[16]
		* {
		*     BYTE[???] JpegData;
		* }
		*/
		wxLogMessage(wxT("Error: %s:%s#%d type=%d"), __FILE__, __FUNCTION__, __LINE__, type);

		BYTE *buffer = NULL;
		CxImage *image = NULL;
		unsigned char *buf = new unsigned char[sizes[0]];

		f.seek(offsets[0]);
		f.read(buf,sizes[0]);
		image = new CxImage(buf, sizes[0], CXIMAGE_FORMAT_JPG);

		if (image == NULL)
			return;

		long size = image->GetWidth() * image->GetHeight() * 4;
		image->Encode2RGBA(buffer, size);

		//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, image->GetWidth(), image->GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);

		wxDELETE(image);
		wxDELETE(buffer);
		wxDELETE(buf);
	} else if (type == 1) {
		_D3DFORMAT format = D3DFMT_DXT1;
		if (attr[0] == 2) {
			/*
			Type 1 Encoding 2 AlphaDepth 0 (DXT1 no alpha)
			The image data is formatted using DXT1 compression with no alpha channel.

			Type 1 Encoding 2 AlphaDepth 1 (DXT1 one bit alpha)
			The image data is formatted using DXT1 compression with a one-bit alpha channel.

			Type 1 Encoding 2 AlphaDepth 8 (DXT3)
			The image data is formatted using DXT3 compression.

			Type 1 Encoding 2 AlphaDepth 8 AlphaEncoding 7 (DXT5)
			The image data are formatted using DXT5 compression.
			*/
			// encoding 2, directx compressed
			unsigned char *ucbuf = NULL;
			//if (!video.supportCompression) //这里我认为所有的显卡都不支持support 这个其实该不该无所谓
			ucbuf = new unsigned char[w*h*4];

			//format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
			int blocksize = 8;

			// guesswork here :(
			// new alpha bit depth == 4 for DXT3, alfred 2008/10/11
			if (attr[1]==8 || attr[1]==4) {
				//format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
				format = D3DFMT_DXT3;
				blocksize = 16;
			}

			// Fix to the BLP2 format required in WoW 2.0 thanks to Linghuye (creator of MyWarCraftStudio)
			if (attr[1]==8 && attr[2]==7) {
				//format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
				format = D3DFMT_DXT5;
				blocksize = 16;
			}

			tex->compressed = true;

			unsigned char *buf = new unsigned char[sizes[0]];

			// do every mipmap level
			////for (size_t i=0; i<mipmax; i++) {  //这里我们只考虑第一个miplevel 就是最高的那个好了
			for (size_t i=0; i<1; i++){
				if (w==0) w = 1;
				if (h==0) h = 1;
				if (offsets[i] && sizes[i]) {
					f.seek(offsets[i]);
					f.read(buf,sizes[i]);

					int size = ((w+3)/4) * ((h+3)/4) * blocksize;

					LPDIRECT3DTEXTURE9 pTex = NULL;
					HRESULT hr = g_pd3dDevice->CreateTexture(w,h,1,D3DUSAGE_DYNAMIC,format,D3DPOOL_DEFAULT,&pTex,NULL);
					tex->tex = pTex;
					maps_[id] = pTex;
					maps_string_[name] = pTex;
					D3DLOCKED_RECT LockedRect;
					pTex->LockRect(0,&LockedRect,NULL,0);
					memcpy(LockedRect.pBits,buf,size);
					pTex->UnlockRect(0);
					
					//D3DXCreateTexture(g_pd3dDevice,w,h,0,D3DUSAGE_RENDERTARGET,D3DFMT_DXT5,D3DPOOL_DEFAULT,&pTex);
					//pTex = NULL;

					//if (video.supportCompression) {
						//glCompressedTexImage2DARB(GL_TEXTURE_2D, (GLint)i, format, w, h, 0, size, buf);
					//} else {
						//decompressDXTC(format, w, h, size, buf, ucbuf);					
						//glTexImage2D(GL_TEXTURE_2D, (GLint)i, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, ucbuf);
					//}

				} else break;
				w >>= 1;
				h >>= 1;
			}

			//wxDELETEA(buf);
			//if (!video.supportCompression) 
				///wxDELETEA(ucbuf);

		} else if (attr[0]==1) {
			/*
			Type 1 Encoding 0 AlphaDepth 0 (uncompressed paletted image with no alpha)
			Each by of the image data is an index into Palette which contains the actual RGB value for the pixel. Although the palette entries are 32-bits, the alpha value of each Palette entry may contain garbage and should be discarded.

			Type 1 Encoding 1 AlphaDepth 1 (uncompressed paletted image with 1-bit alpha)
			This is the same as Type 1 Encoding 1 AlphaDepth 0 except that immediately following the index array is a second image array containing 1-bit alpha values for each pixel. The first byte of the array is for pixels 0 through 7, the second byte for pixels 8 through 15 and so on. Bit 0 of each byte corresponds to the first pixel (leftmost) in the group, bit 7 to the rightmost. A set bit indicates the pixel is opaque while a zero bit indicates a transparent pixel.

			Type 1 Encoding 1 AlphaDepth 8(uncompressed paletted image with 8-bit alpha)
			This is the same as Type 1 Encoding 1 AlphaDepth 0 except that immediately following the index array is a second image array containing the actual 8-bit alpha values for each pixel. This second array starts at BLP2Header.Offset[0] + BLP2Header.Width * BLP2Header.Height.
			*/
			// encoding 1, uncompressed
			unsigned int pal[256];
			f.read(pal, 1024);

			unsigned char *buf = new unsigned char[sizes[0]];
			unsigned int *buf2 = new unsigned int[w*h];
			unsigned int *p = NULL;
			unsigned char *c = NULL, *a = NULL;

			int alphabits = attr[1];
			bool hasalpha = (alphabits!=0);

			tex->compressed = false;

			///for (size_t i=0; i<mipmax; i++) {
			for(size_t i = 0;i < 1;i++){
				if (w==0) w = 1;
				if (h==0) h = 1;
				if (offsets[i] && sizes[i]) {
					f.seek(offsets[i]);
					f.read(buf,sizes[i]);

					int cnt = 0;
					int alpha = 0;

					p = buf2;
					c = buf;
					a = buf + w*h;
					for (size_t y=0; y<h; y++) {
						for (size_t x=0; x<w; x++) {
							unsigned int k = pal[*c++];

							k = ((k&0x00FF0000)>>16) | ((k&0x0000FF00)) | ((k& 0x000000FF)<<16);

							if (hasalpha) {
								if (alphabits == 8) {
									alpha = (*a++);
								} else if (alphabits == 4) {
									alpha = (*a & (0xf << cnt++)) * 0x11;
									if (cnt == 2) {
										cnt = 0;
										a++;
									}
								} else if (alphabits == 1) {
									//alpha = (*a & (128 >> cnt++)) ? 0xff : 0;
									alpha = (*a & (1 << cnt++)) ? 0xff : 0;
									if (cnt == 8) {
										cnt = 0;
										a++;
									}
								}
							} else alpha = 0xff;

							k |= alpha << 24;				 
							*p++ = k;
						}
					}

					////glTexImage2D(GL_TEXTURE_2D, (GLint)i, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, buf2);
					//1月16日 这里要创建纹理
					LPDIRECT3DTEXTURE9 pTex = NULL;
					g_pd3dDevice->CreateTexture(w,h,0,D3DUSAGE_DYNAMIC,D3DFMT_A8R8G8B8,D3DPOOL_DEFAULT,&pTex,NULL);
					tex->tex = pTex;
					//hr =         g_pd3dDevice->CreateTexture(w,h,0,D3DUSAGE_DYNAMIC,D3DFMT_A8R8G8B8,D3DPOOL_DEFAULT,&pNewText,NULL);

					maps_[id] = pTex;
					maps_string_[name] = pTex;
					D3DLOCKED_RECT LockedRect;
					pTex->LockRect(0,&LockedRect,NULL,0);
					memcpy(LockedRect.pBits,buf2,w*h*4);
					pTex->UnlockRect(0);

				} else break;

				w >>= 1;
				h >>= 1;
			}

			wxDELETEA(buf2);
			wxDELETEA(buf);
		} else {
			wxLogMessage(wxT("Error: %s:%s#%d type=%d, attr[0]=%d"), __FILE__, __FUNCTION__, __LINE__, type, attr[0]);
		}
	}

	f.close();

	/*
	// TODO: Add proper support for mipmaps
	if (hasmipmaps) {
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	} else {
	*/
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//}
}

//1月16日 人物纹理 CharTexture
//1月18日 基本能用 但是有很多不理想的地方 先不管了 以後回頭看
unsigned int  CharTexture::compose(TextureID texID,wxString name)
{
	// if we only have one texture then don't bother with compositing
	if (components.size()==1) {
		Texture temp(components[0].name);
		texturemanager.LoadBLP(texID, &temp,"TEST");
		printf("This SHould not happen now !\n");
		return 0;
	}

	std::sort(components.begin(), components.end());

	//加载所有的纹理
	/*for(int i = 0;i<components.size();i++){
		if(!texturemanager.Exists(components[i].name)){
			texturemanager.GetTexture(components[i].name);  //加载这些纹理
		}
	}*/

	unsigned char *destbuf, *tempbuf;
	destbuf = (unsigned char*)malloc(REGION_PX*REGION_PX*4);
	memset(destbuf, 0, REGION_PX*REGION_PX*4);

	int file_no = 0;

	for (std::vector<CharTextureComponent>::iterator it = components.begin(); it != components.end(); ++it) {
		CharTextureComponent &comp = *it;
		const CharRegionCoords &coords = regions[comp.region];
		TextureID temptex = texturemanager.add(comp.name);
		Texture &tex = *((Texture*)texturemanager.items[temptex]);

		// Alfred 2009.07.03, tex width or height can't be zero
		if (tex.w == 0 || tex.h == 0) {
			texturemanager.del(temptex);
			continue;
		}
		tempbuf = (unsigned char*)malloc(REGION_PX*REGION_PX*4);
		if (!tempbuf)
			continue;
		memset(tempbuf, 0, REGION_PX*REGION_PX*4);
		CxImage *newImage = NULL;
		if (tex.w!=coords.xsize || tex.h!=coords.ysize)
		{
			tex.getPixels(tempbuf, GL_BGRA_EXT); //这句我注释掉 我猜他的含义是通过纹理得到raw字节 这个字节序是RGBA的 然后弄到newImage中 对newImage进行操作形成新的纹理
			newImage = new CxImage(0);
			if (newImage) {
				newImage->AlphaCreate();	// Create the alpha layer
				newImage->IncreaseBpp(32);	// set image to 32bit 
				newImage->CreateFromArray(tempbuf, tex.w, tex.h, 32, (tex.w*4), false);
				//newImage->Save("c:\\median1.jpg",CXIMAGE_FORMAT_JPG);
				newImage->Resample(coords.xsize, coords.ysize, 0); // 0: hight quality, 1: normal quality
				//wxDELETE(tempbuf);
				//tempbuf = NULL;
				long size = coords.xsize * coords.ysize * 4;
				newImage->Encode2RGBA(tempbuf, size, false);
				//newImage->Save("c:\\median2.jpg",CXIMAGE_FORMAT_JPG);
				//wxDELETE(newImage);
			} else {
				free(tempbuf);
				continue;
			}
		} else
			tex.getPixels(tempbuf,0);
	
		// blit the texture region over the original
		//這裡涉及到一個字節序的問題 我修正過dest[0] 為alpha通道
		for (ssize_t y=0, dy=coords.ypos; y<coords.ysize; y++,dy++) {
			for (ssize_t x=0, dx=coords.xpos; x<coords.xsize; x++,dx++) {
				//unsigned char *src = tempbuf + y*coords.xsize*4 + x*4;
				//unsigned char *dest = destbuf + dy*REGION_PX*4 + dx*4;

				unsigned char *src = tempbuf + y*coords.xsize*4 + x*4;
				unsigned char *dest = destbuf + dy*512*4 + dx*4;

				// this is slow and ugly but I don't care
				float r = src[3] / 255.0f;
				float ir = 1.0f - r;
				// zomg RGBA?
				//      dest[3]*ir 
				//這裡我也不管 什麼Blend的 直接BitBlt額 因為r這個東西好像有問題 也可能是 下邊乘法的精度不夠 不管了 以後再說
				if(!newImage){
					//這裡要改動下 ARGB  dest[0] 是Red
					//dest[0] = (unsigned char)( dest[0]*ir + src[2]*r );
					//dest[1] = (unsigned char)( dest[1]*ir + src[1]*r);
					//dest[2] = (unsigned char)( dest[2]*ir + src[0]*r);
					dest[0] = (unsigned char)(src[2]);
					dest[1] = (unsigned char)(src[1]);
					dest[2] = (unsigned char)(src[0]);
					//dest[2] = 255;
					//dest[0] = dest[1] = 0 ;
				}
				else  //這個也是不得已而為之 以後改進 通過src來確定dest好了 
				{
					dest[0] = newImage->GetPixelColor(x,y).rgbRed;
					dest[1] = newImage->GetPixelColor(x,y).rgbGreen;
					dest[2] = newImage->GetPixelColor(x,y).rgbBlue;
					//dest[0] = 255;
					//dest[1] = dest[2] = 0 ;
				}
				//dest[3] = dest[2] = dest[1] = 255;
				dest[3] = 255;
			}
		}
		
		char buffer_name[20];
		sprintf(buffer_name,"c:\\%d.jpg",file_no ++);
		//保存到文件 看看贴出来的纹理是什么样子
		//CxImage image(tex.w,tex.h,24);
		/*CxImage image(512,512,24);
		for(int i = 0 ;i< 512;i++){
			for(int j = 0; j<512;j++){
				RGBQUAD color;
				color.rgbRed = destbuf[j* 512*4 + i*4 + 3];
				color.rgbGreen = destbuf[j* 512*4 + i*4 + 2];
				color.rgbBlue = destbuf[j* 512*4 + i*4 + 1];
				image.SetPixelColor(i,j,color);
			}
		}
		image.Save(buffer_name,CXIMAGE_FORMAT_JPG);*/
		free(tempbuf);
		
		texturemanager.del(temptex);
	}
	//用这个destbuff 构建新的纹理 
	LPDIRECT3DTEXTURE9 new_texture;
	HRESULT hr = g_pd3dDevice->CreateTexture(REGION_PX,REGION_PX,0,D3DUSAGE_DYNAMIC,D3DFMT_X8R8G8B8,D3DPOOL_DEFAULT,&new_texture,NULL);
	
	D3DLOCKED_RECT LockedRect;	
	new_texture->LockRect(0,&LockedRect,NULL,0);
	memcpy(LockedRect.pBits,destbuf,REGION_PX*REGION_PX*4);
	new_texture->UnlockRect(0);

	//保存到文件 看看贴出来的纹理是什么样子
	/*CxImage image(512,512,24);
	for(int i = 0 ;i< 512;i++){
		for(int j = 0; j<512;j++){
			RGBQUAD color;
			color.rgbRed = destbuf[j* 512*4 + i*4 ];
			color.rgbGreen = destbuf[j* 512*4 + i*4 + 1] ;;
			color.rgbBlue = destbuf[j* 512*4 + i*4 + 2];
			image.SetPixelColor(i,j,color);
		}
	}
	image.Save("c:\\result.jpg",CXIMAGE_FORMAT_JPG);*/
	//创建新的纹理加入到texturemanager中
	TextureID new_id = texturemanager.add(name,new_texture);
	printf("Compose New Texture name is %s,id is %d\n",name ,new_id);
	

	// good, upload this to video
	/*glBindTexture(GL_TEXTURE_2D, texID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, REGION_PX, REGION_PX, 0, GL_RGBA, GL_UNSIGNED_BYTE, destbuf);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);*/
	free(destbuf);
	return new_id;
}

void Texture::getPixels(unsigned char *buff, unsigned int format)
{
	//得到纹理的Surface
	LPDIRECT3DTEXTURE9 pText = this->tex;
	LPDIRECT3DSURFACE9 p_surface_orig;
	HRESULT hr = pText->GetSurfaceLevel(0,&p_surface_orig);
	//得到纹理的信息
	D3DSURFACE_DESC pDesc;
	p_surface_orig->GetDesc(&pDesc);
	int w = pDesc.Width;
	int h = pDesc.Height;
	//创建新的纹理
	LPDIRECT3DTEXTURE9 pNewText;
	//D3DFORMAT formatt = D3DFMT_A8R8G8B8;
	hr = g_pd3dDevice->CreateTexture(w,h,0,D3DUSAGE_DYNAMIC,D3DFMT_A8R8G8B8,D3DPOOL_DEFAULT,&pNewText,NULL);
	//纹理复制
	//得到目标的surface
	LPDIRECT3DSURFACE9 p_surface_target;
	hr = pNewText->GetSurfaceLevel(0,&p_surface_target);
	hr = D3DXLoadSurfaceFromSurface(p_surface_target,NULL,NULL,p_surface_orig,NULL,NULL,D3DX_DEFAULT,0);

	//static char new_buffer[512*512*4]; //为了保存坐标信息*

	D3DLOCKED_RECT LockedRect;	
	int size = w*h*4;
	pNewText->LockRect(0,&LockedRect,NULL,0);
	memcpy(buff,LockedRect.pBits,size);
	pNewText->UnlockRect(0);
	
	//删除新创建的纹理
	pNewText->Release();	
}