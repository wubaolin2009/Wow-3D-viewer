#include "world.h"
#include "mpq.h"
#include "vector3d.h"
#include "texture.h"
#include "camera.h"
#include <vector>
#include <string>
#include <d3d9.h>
#include <map>
#define TILESIZE (533.33333f)

extern CCamera  g_camera;
extern TextureManager texturemanager;

//这个是地面的Vetext
#define D3DFVF_TERRAINVERTEX (D3DFVF_XYZ | D3DFVF_DIFFUSE |D3DFVF_NORMAL|D3DFVF_TEX5 )
struct TERRAIBVERTEX
{
	FLOAT x, y, z;      // The untransformed, 3D position for the vertex
	FLOAT nx,ny,nz;
	DWORD color;
	FLOAT u,v;
	FLOAT u2,v2;
	FLOAT u3,v3;
	FLOAT u4,v4;
	FLOAT u5,v5;
};

extern LPDIRECT3DDEVICE9 g_pd3dDevice;

//一下是 2月3日加入的 地形渲染 shader相關
//下边是地形渲染的 shader相关
IDirect3DPixelShader9*  World::Terrain_shader;  //P shader
IDirect3DVertexShader9*  World::Terrain_shader_vetex;  //V Shader

D3DXHANDLE World::Samp0Handle                = 0;
D3DXHANDLE World::Samp1Handle                = 0;
D3DXHANDLE World::Samp2Handle                = 0;
D3DXHANDLE World::Samp3Handle                = 0;
D3DXHANDLE World::SampAlphaHandle            = 0;
D3DXHANDLE World::SampShadowHandle             = 0;

D3DXCONSTANT_DESC World::Samp0Desc;
D3DXCONSTANT_DESC World::Samp1Desc;
D3DXCONSTANT_DESC World::Samp2Desc;
D3DXCONSTANT_DESC World::Samp3Desc;
D3DXCONSTANT_DESC World::SampAlphaDesc;
D3DXCONSTANT_DESC World::SampShadowDesc;

//V Shader 的参数
D3DXHANDLE World::Vetex_matWorldViewProj              = 0;
D3DXHANDLE World::Vetex_matInverseWorld                = 0;
D3DXHANDLE World::Vetex_vLightDirection                = 0;

LPD3DXCONSTANTTABLE World::TransformConstantTableVetex;


void World::InitPShader()
{
	DWORD dwShaderFlags = D3DXFX_NOT_CLONEABLE;
	char  str[MAX_PATH] = "c:\\s.txt";	 

	LPD3DXBUFFER shader;
	LPD3DXBUFFER errorBuffer;
	LPD3DXCONSTANTTABLE TransformConstantTable;

	HRESULT hr = D3DXCompileShaderFromFile(
		str,
		0,
		0,
		"PS_Main",
		D3DXGetPixelShaderProfile(g_pd3dDevice),
		D3DXSHADER_DEBUG,
		&shader,
		&errorBuffer,
		&TransformConstantTable);

	//printf("%s",errorBuffer->GetBufferPointer() );


	hr = g_pd3dDevice->CreatePixelShader(
		(DWORD*)shader->GetBufferPointer(),
		&Terrain_shader);

	

	/*hr = g_pd3dDevice->CreateVertexShader(
		(DWORD*)shader->GetBufferPointer(),
		&g_terrain_shader_vetex);	*/

	//得到各常量句柄
	Samp0Handle = TransformConstantTable->GetConstantByName(0, "Samp0");
	Samp1Handle = TransformConstantTable->GetConstantByName(0, "Samp1");
	Samp2Handle = TransformConstantTable->GetConstantByName(0, "Samp2");
	Samp3Handle = TransformConstantTable->GetConstantByName(0, "Samp3");
	SampAlphaHandle = TransformConstantTable->GetConstantByName(0, "SampAlpha");
	SampShadowHandle = TransformConstantTable->GetConstantByName(0, "SampShadow");

	//得到 AlphaMap的句柄
	//AlphamapHandle = TransformConstantTable->GetConstantByName(0, "alpha_maps");
	
	//常量描述结构
	
	UINT count;
	TransformConstantTable->GetConstantDesc(Samp0Handle, & Samp0Desc, &count);
	TransformConstantTable->GetConstantDesc(Samp1Handle, & Samp1Desc, &count);
	TransformConstantTable->GetConstantDesc(Samp2Handle, & Samp2Desc, &count);
	TransformConstantTable->GetConstantDesc(Samp3Handle, & Samp3Desc, &count);
	TransformConstantTable->GetConstantDesc(SampAlphaHandle, & SampAlphaDesc, &count);
	TransformConstantTable->GetConstantDesc(SampShadowHandle, &SampShadowDesc, &count);

	TransformConstantTable->SetDefaults(g_pd3dDevice);

}

void World::InitVShader()
{
	DWORD dwShaderFlags = D3DXFX_NOT_CLONEABLE;
	char  str[MAX_PATH] = "c:\\vs.txt";	 

	LPD3DXBUFFER shader;
	LPD3DXBUFFER errorBuffer;
	

	//HRESULT hr = D3DXAssembleShader(str, (UINT)strlen(str),
		//NULL, NULL, D3DXSHADER_DEBUG, &shader, NULL);
	HRESULT hr = D3DXCompileShaderFromFile(
		str,
		0,
		0,
		"VS_Main",
		D3DXGetVertexShaderProfile(g_pd3dDevice),
		D3DXSHADER_DEBUG,
		&shader,
		&errorBuffer,
		&TransformConstantTableVetex);

	//HRESULT hr = D3DXCompileShaderFromFileA("vertexshader.txt", 0, 0, "Main", "vs_1_1",  
		//D3DXSHADER_ENABLE_BACKWARDS_COMPATIBILITY, &codeBuffer, &errorBuffer, &g_pConstantTable) ; 

	//char* er = (char*)errorBuffer->GetBufferPointer();




	hr = g_pd3dDevice->CreateVertexShader(
	(DWORD*)shader->GetBufferPointer(),
	&Terrain_shader_vetex);	

	//得到各常量句柄
	Vetex_matWorldViewProj = TransformConstantTableVetex->GetConstantByName(0, "matWorldViewProj");
	Vetex_matInverseWorld = TransformConstantTableVetex->GetConstantByName(0, "matInverseWorld");
	Vetex_vLightDirection = TransformConstantTableVetex->GetConstantByName(0, "vLightDirection");
	
	//设置光照
	// 0.0f,-1.0f,0.5f  ;
	D3DXVECTOR4 vec_light(0.0,1.0,-0.5f,1.0f);
	TransformConstantTableVetex->SetVector(g_pd3dDevice,Vetex_vLightDirection,&vec_light);


	//TransformConstantTable->SetVector(g_pd3dDevice, ScalarHandle, &scalar);

}

void World::InitShader()
{
	int ofsbuf[64][64];
	InitPShader();
	InitVShader();

	/*MPQFile f("world\\maps\\Blackfathom\\Blackfathom.wdl");
	f.seek(0x14);
	f.read(ofsbuf,64*64*4);
	for (int j=0; j<64; j++) {
		for (int i=0; i<64; i++) {
			if (ofsbuf[j][i]) {
				f.seek(ofsbuf[j][i]+8);
				// read height values ^_^
				f.read(tilebuf,17*17*2);
				//我們先不關心 紋理
			}
		}
	}*/
	/*char fourcc[5]; bool maps[64][64]; int nMaps;  int gnWMO;
	size_t size;
	nMaps = 0;
	//这里的画的是 low resolution的地形 先画出来看看什么样子
	short tilebuf[17*17];
	short tilebuf2[16*16];
	Vec3D lowres[17][17];
	Vec3D lowsub[16][16];

	int offt = -1;

	MPQFile f("world\\maps\\BlackwingLair\\BlackwingLair.wdl");
	//这里我们只画 44 32 也就是 32_44.adt
	while (!f.isEof()) {
		f.read(fourcc,4);
		f.read(&size, 4);

		flipcc(fourcc);
		fourcc[4] = 0;
		//printf("%s\n",fourcc);
		fourcc[4] = 0;

		size_t nextpos = f.getPos() + size;

		

		if (!strcmp(fourcc,"MAOF")) {
			for (int j=0; j<64; j++) {
				for (int i=0; i<64; i++) {
					if(offt == -1)
						offt = f.getPos();
					int d;
					f.read(&d, 4);
					if (d) {
						maps[j][i] = true;
						nMaps++;
						printf("Line %d Col %d True %d\n",j,i,d);
					} 
					else maps[j][i] = false;
					//f.read(&d, 4);
				}
			}
		}
		else if (!strcmp(fourcc,"MODF")) {
			// global wmo instance data
			gnWMO = (int)size / 64;
			// WMOS and WMO-instances are handled below in initWMOs()
		}
		f.seek((int)nextpos);
	}


memset(tilebuf,0,sizeof(tilebuf));
	memset(tilebuf2,0,sizeof(tilebuf2));
		


	for (int j=0; j<64; j++) {
		for (int i=0; i<64; i++) {
			if (ofsbuf[j][i]) {
				f.seek(ofsbuf[j][i]+8);
				f.read(tilebuf,17*17*2);
				f.read(tilebuf2,16*16*2);

				for (int y=0; y<17; y++) {
					for (int x=0; x<17; x++) {
						lowres[y][x] = Vec3D(TILESIZE*(i+x/16.0f), tilebuf[y*17+x], TILESIZE*(j+y/16.0f));
						//lowres[y][x] = Vec3D(TILESIZE*(i+x/16.0f), 0, TILESIZE*(j+y/16.0f));
					}
				}
				for (int y=0; y<16; y++) {
					for (int x=0; x<16; x++) {
						lowsub[y][x] = Vec3D(TILESIZE*(i+(x+0.5f)/16.0f), tilebuf2[y*16+x], TILESIZE*(j+(y+0.5f)/16.0f));
						//lowsub[y][x] = Vec3D(TILESIZE*(i+(x+0.5f)/16.0f), 0, TILESIZE*(j+(y+0.5f)/16.0f));
					}
				}
				for (int y=0; y<16; y++) {
					for (int x=0; x<16; x++) {
						g_terrain.push_back(lowres[y][x]); g_terrain.push_back(lowsub[y][x]); g_terrain.push_back(lowres[y][x]);
						g_terrain.push_back(lowres[y][x+1]); g_terrain.push_back(lowsub[y][x]); g_terrain.push_back(lowres[y][x]);
						g_terrain.push_back(lowres[y+1][x+1]); g_terrain.push_back(lowsub[y][x]); g_terrain.push_back(lowres[y][x]);
						g_terrain.push_back(lowres[y+1][x]); g_terrain.push_back(lowsub[y][x]); g_terrain.push_back(lowres[y][x]);
					}
				}
			}
		}
	}

	//填充Buffer
	TERRAIBVERTEX * p_buffer = new TERRAIBVERTEX[g_terrain.size()];
	//填充
	for(int i = 0;i<g_terrain.size();i++){
		p_buffer[i].x = g_terrain[i].x;
		p_buffer[i].y = g_terrain[i].y;
		p_buffer[i].z = g_terrain[i].z;
	}

	if( FAILED( g_pd3dDevice->CreateVertexBuffer( g_terrain.size() * sizeof( TERRAIBVERTEX ),
		0, D3DFVF_TERRAINVERTEX,
		D3DPOOL_DEFAULT, &terrain_buffer, NULL ) ) )
	{
		printf("Error In CreateVetex\n");
		return ;
	}
	// Fill the vertex buffer.
	VOID* pVertices;
	if( FAILED( terrain_buffer->Lock( 0, 0, ( void** )&pVertices, 0 ) ) ){
		printf("Error in Fill Vertex Buffer\n");		
		return ;
	}
	memcpy( pVertices, p_buffer, g_terrain.size() * sizeof(TERRAIBVERTEX) );
	terrain_buffer->Unlock();
	delete[] p_buffer;*/
}
struct Header_
{
	unsigned int filenameOffset;
	unsigned int txmdOffset; // TXMD-chunk offset relative to end of TXFN-chunk.
	char sizex;
	char sizey;
	char type;
	char flags;
};

//這個是 tex0.adt 中 MCLY的頭
struct TexHeader_
{
	unsigned int textureId;
	unsigned int flags; // these may have changed or some additional values
	unsigned int offsetInMCAL;
	unsigned short effectId;
	unsigned short padding;
};
//拷貝字符串 不包括 end
std::string CopyToString(char* start,char* end)
{
	assert(end >= start && end - start < 1024);
	char* p = start;
	char buffer[1024];
	int i = 0;
	while(p < end){
		buffer[i++] = *p;
		p++;
	}
	buffer[i] = '\0';
	return std::string(buffer);
}

//將 "adgbc\0bcdd"等 轉換成 abgbc bcdd數組
std::vector<std::string> SlitString(char* string ,int size)
{
	std::vector<std::string> rets;
	char* p = string;
	char buffer[1024];
	int i = 0;
	char* b = strchr(string,'\0');
	while(b && b < string + size){  //這個代碼有問題 以後改
		std::string str = CopyToString(p,b);
		p = b+1;
		b = strchr(p,'\0');
		rets.push_back(str);
	}
	return rets;
}


//讀取 壓縮的 Alphamap buffer為第一個字節（Alphamap的真正數據） size 為MCAL的大小 start 第幾個chunk0-255 
//返回有幾個  Alphamap
//2月11日重写 每次只读一个Alphamap 由调用者确定 要解析的大小 并制定 
//赋给那个 alpha_maps_ which_map 为 0 1 2 
int World::ReadAlphaMapCompressed(char* buffer,size_t start_offset,int start,int which_map)
{
	//int count_alpha_map = 0;

	unsigned offI = 0; //offset IN buffer
	unsigned offO = 0; //offset OUT buffer
	unsigned char* buffIn = (unsigned char*)buffer; // pointer to data in adt file
	unsigned char buffOut[4096]; // the resulting alpha map

	//while(offI < alpha_map_size_byte){
	alpha_offset_to_index_[start][start_offset] = which_map;
	while( offO < 4096 )
	{
		// fill or copy mode
		bool fill = buffIn[offI] & 0x80;
		unsigned n = buffIn[offI] & 0x7F;
		offI++;
		for( unsigned k = 0; k < n; k++ )
		{
			if (offO == 4096) break;
			buffOut[offO] = buffIn[offI];
			offO++;
			if( !fill )
				offI++;
		}
		if( fill ) offI++;
	}
	//此時的offI指向下一個 alphamap
	for(int i = 0;i < 64;i++){
		for(int j = 0;j < 64;j++){
			alpha_maps_[start][which_map][i][j] = (buffOut[i*64+j])/255.0;
		}
	}
		
		//count_alpha_map ++;
		//offO = 0;
		//这里只读入一个Alphamap
		//break;
	//}
	return 1;
}

int World::ReadAlphaMapUnCompressed(char* buffer,int start_offset,size_t alpha_map_size_byte,int start,int which_map)
{
	//我們這裡認為 每個alphamap 是 2048個字節 或4096
	assert( alpha_map_size_byte % 2048 == 0);

	if(alpha_map_size_byte == 2048){
		unsigned char* tmp_buffer = (unsigned char*)buffer;
		for(int i = 0;i < 1;i++){
			alpha_offset_to_index_[start][start_offset] = which_map;
			for(int j = 0;j<64;j++){
				for(int m = 0;m < 64;m++){
					unsigned char bit = tmp_buffer[32*j +m/2];
					unsigned int bit_high = (bit & 0xf0)>>4;
					unsigned int bit_low = bit & 0xf;
					if(m%2 == 0){	
						alpha_maps_[start][i][j][m] = (int)bit_high /15.0;
					}
					else{
						alpha_maps_[start][i][j][m] = (int)bit_low / 15.0;
					}
				}
			}
		}
	}
	else{
		//这里认为每个字节都是一个4096
		int c_macl = 1;
		unsigned char* tmp_buffer = (unsigned char*)buffer;
		for(int i = 0;i < 1;i++){
			alpha_offset_to_index_[start][start_offset] = which_map;
			for(int j = 0;j<64;j++){
				for(int m = 0;m < 64;m++){
					unsigned char bit = tmp_buffer[64*j +m];
					alpha_maps_[start][which_map][j][m] = (int)bit/255.0 ;


				}
			}
		}
	}	
	return 1;
}


LPDIRECT3DTEXTURE9      g_pTexture0 = NULL; // Our texture
LPDIRECT3DTEXTURE9      g_pTexture1 = NULL; // Our texture
LPDIRECT3DTEXTURE9       g_pTexture2 = NULL;

void World::InitADT()
{
	//初始化 
	//MPQFile tex("world\\maps\\BlackwingLair\\BlackwingLair_34_44.adt");
	//MPQFile tex("world\\maps\\Stormwind\\Stormwind_33_46.adt");

	//MPQFile tex("world\\maps\\Kalimdor\\Kalimdor_35_50.adt");

	//紋理
	//MPQFile f_tex("world\\maps\\BlackwingLair\\BlackwingLair_34_44_tex0.adt");
	//MPQFile f_tex("world\\maps\\Stormwind\\Stormwind_34_46_tex0.adt");
	
	//MPQFile f_tex("world\\maps\\Kalimdor\\Kalimdor_35_50_tex0.adt");
	MPQFile tex(map_name_);
	MPQFile f_tex(tex_0_name_);

	if( FAILED( D3DXCreateTextureFromFile( g_pd3dDevice, "c:\\1t.bmp", &g_pTexture0 ) ) )
	{
		MessageBox( NULL, "Could not find banana.bmp", "Textures.exe", MB_OK );
	}
	if( FAILED( D3DXCreateTextureFromFile( g_pd3dDevice, "c:\\2t.bmp", &g_pTexture1 ) ) )
	{
		MessageBox( NULL, "Could not find banana.bmp", "Textures.exe", MB_OK );
	}
	if( FAILED( D3DXCreateTextureFromFile( g_pd3dDevice, "c:\\3t.bmp", &g_pTexture2 ) ) )
	{
		MessageBox( NULL, "Could not find banana.bmp", "Textures.exe", MB_OK );
	}


	char buffer_tex[2048];

	char fourcc[5]; 
	size_t size;
	int count = 0;

	struct Header_ header1;

	int xxx = 0;  //计数器 当前渲染的是哪个chunk
	int yyy = 0;
	
	while (!tex.isEof()) {
		tex.read(fourcc,4);
		tex.read(&size, 4); 

		flipcc(fourcc);
		fourcc[4] = 0;
		//printf("%s %d\n",fourcc,size);
		fourcc[4] = 0;

		size_t nextpos = tex.getPos() + size;


		if (!strcmp(fourcc,"MHDR")) {
			/*char b[64];
			tex.read(&b, sizeof(b) );
			b[64] = 0;*/
			//tex.read(&d, 4);
		}
		if (!strcmp(fourcc,"MTEX")) {
			/*char buffer[1024];
			int cur_pos = tex.getPos();

			tex.read(buffer, 1024 );
			for(int i = 0;i < 2;i++){
				printf("%s\n",buffer);
				cur_pos += strlen(buffer) + 1;
				tex.seek(cur_pos);
				tex.read(buffer,1024);
			}*/
			
			//tex.read(&d, 4);
		}

		if (!strcmp(fourcc,"MCNK")) {   //纹理Data
			//得到 当前位置
			int c_p = tex.getPos();
			//得到 z x y
			tex.seekRelative(104);  float z_; float x_; float y_;
			tex.read(&z_,sizeof(float) ); tex.read(&x_,sizeof(float) ); tex.read(&y_,sizeof(float) );
			///printf("Z %f X %f Y %f\n",z_,x_,y_);
			tex.seek(c_p);
			tex.seekRelative(0x14); //得到offset of MCVT
			int off_set = 0;
			tex.read(&off_set,sizeof(int));
			tex.seek(c_p + off_set);
			//接下来的数据应该是 (9*9 + 8*8 )*4个字节 应该是 float
			float coords[9*9 + 8*8];
			tex.read(coords,sizeof(coords) );
			//应该到了 MCNK了
			char bbcc[8];
			tex.read(bbcc,8);
			Vec3D normals[9*9 + 8*8];
			for(int i = 0;i< 9*9 + 8*8;i++){
				char n_x,n_y,n_z;
				tex.read(&n_x,1);
				tex.read(&n_z,1);
				tex.read(&n_y,1);
				normals[i] = Vec3D( -1.0 + (n_x +  127.0)*2.0/254.0, -1.0 + (n_y +  127.0)*2.0/254.0,-1.0 + (n_z +  127.0)*2.0/254.0 );
			}
			//tex.seekRelative(8);
			for(int i = 0;i< 9*9 + 8*8;i++){						
				coords[i] = coords[i] + (y_+230);
				//printf("%f\n",coords[i]);

			} 
			for(int i = 0;i < 8;i ++){
				for(int j = 0;j < 8;j ++){
					float l1 = coords[i*17 + j];
					float l2 = coords[i*17 + 17 + j];
					float l3 = coords[i*17 + j + 1];
					float l4 = coords[i*17 + 17 + j + 1];
					float lm = coords[i*17 + 9 + j];

					Vec3D v1 = normals[i*17 + j];
					Vec3D v2 = normals[i*17 + 17 + j];
					Vec3D v3 = normals[i*17 + j + 1];
					Vec3D v4 = normals[i*17 + 17 + j + 1];
					Vec3D vm = normals[i*17 + 9 + j];
					
					
					//const FLOAT UNIT_SIZE = 533.333333/128*5;
					const FLOAT UNIT_SIZE = 50.0;
					int left = 8 * xxx * UNIT_SIZE;
					int top =  8 * yyy * UNIT_SIZE;
					left += 16*8*UNIT_SIZE*start_x_;
					top += 16*8*UNIT_SIZE*start_y_;
					terrain_.push_back(Vec3D(left + (j)*UNIT_SIZE,l1,top+(i)*UNIT_SIZE) );   terrain_.push_back(Vec3D(left + (j)*UNIT_SIZE,l2,top+(i+1)*UNIT_SIZE) ); terrain_.push_back(Vec3D(left + (j+0.5)*UNIT_SIZE,lm,top+(i+0.5)*UNIT_SIZE) );
					terrain_.push_back(Vec3D(left + (j+1)*UNIT_SIZE,l3,top+(i)*UNIT_SIZE) ); terrain_.push_back(Vec3D(left + (j)*UNIT_SIZE,l1,top+(i)*UNIT_SIZE) ); terrain_.push_back(Vec3D(left + (j+0.5)*UNIT_SIZE,lm,top+(i+0.5)*UNIT_SIZE) );
					terrain_.push_back(Vec3D(left + (j+1)*UNIT_SIZE,l4,top+(i+1)*UNIT_SIZE) ); terrain_.push_back(Vec3D(left + (j+1)*UNIT_SIZE,l3,top+(i)*UNIT_SIZE) );  terrain_.push_back(Vec3D(left + (j+0.5)*UNIT_SIZE,lm,top+(i+0.5)*UNIT_SIZE) );
					terrain_.push_back(Vec3D(left + (j)*UNIT_SIZE,l2,top+(i+1)*UNIT_SIZE) ); terrain_.push_back(Vec3D(left + (j+1)*UNIT_SIZE,l4,top+(i+1)*UNIT_SIZE) ); terrain_.push_back(Vec3D(left + (j+0.5)*UNIT_SIZE,lm,top+(i+0.5)*UNIT_SIZE) );

					terrain_normal_.push_back(v1); terrain_normal_.push_back(v2); terrain_normal_.push_back(vm);
					terrain_normal_.push_back(v3); terrain_normal_.push_back(v1); terrain_normal_.push_back(vm);
					terrain_normal_.push_back(v4); terrain_normal_.push_back(v3); terrain_normal_.push_back(vm);
					terrain_normal_.push_back(v2); terrain_normal_.push_back(v4); terrain_normal_.push_back(vm);

					/*g_terrain_uv.push_back(Vec2D(j*0.125,i*0.125+0.125) ); g_terrain_uv.push_back(Vec2D(j*0.125,i*0.125) ); g_terrain_uv.push_back(Vec2D(j*0.125+0.125/2,i*0.125+0.125/2) );
					g_terrain_uv.push_back(Vec2D(j*0.125+0.125,i*0.125+0.125) ); g_terrain_uv.push_back(Vec2D(j*0.125,i*0.125+0.125) ); g_terrain_uv.push_back(Vec2D(j*0.125+0.125/2,i*0.125+0.125/2) );
					g_terrain_uv.push_back(Vec2D(j*0.125+0.125,i*0.125) ); g_terrain_uv.push_back(Vec2D(j*0.125+0.125,i*0.125+0.125) ); g_terrain_uv.push_back(Vec2D(j*0.125+0.125/2,i*0.125+0.125/2) );
					g_terrain_uv.push_back(Vec2D(j*0.125,i*0.125) ); g_terrain_uv.push_back(Vec2D(j*0.125+0.125,i*0.125) ); g_terrain_uv.push_back(Vec2D(j*0.125+0.125/2,i*0.125+0.125/2) );*/

					terrain_uv_.push_back(Vec2D(j*0.125,i*0.125) );  terrain_uv_.push_back(Vec2D(j*0.125,i*0.125+0.125) );  terrain_uv_.push_back(Vec2D(j*0.125 + 0.125/2,i*0.125+ 0.125/2) ); 
					terrain_uv_.push_back(Vec2D(j*0.125+0.125,i*0.125) );  terrain_uv_.push_back(Vec2D(j*0.125,i*0.125) );  terrain_uv_.push_back(Vec2D(j*0.125 + 0.125/2,i*0.125+ 0.125/2) ); 
					terrain_uv_.push_back(Vec2D(j*0.125+0.125,i*0.125+0.125) );  terrain_uv_.push_back(Vec2D(j*0.125+0.125,i*0.125) );    terrain_uv_.push_back(Vec2D(j*0.125 + 0.125/2,i*0.125+ 0.125/2) ); 
					terrain_uv_.push_back(Vec2D(j*0.125,i*0.125+0.125) );  terrain_uv_.push_back(Vec2D(j*0.125+0.125,i*0.125+0.125) );  terrain_uv_.push_back(Vec2D(j*0.125 + 0.125/2,i*0.125+ 0.125/2) ); 
					/*terrain_uv_.push_back(Vec2D(0.0,1.0) );  terrain_uv_.push_back(Vec2D(0.0,0.0) );  terrain_uv_.push_back(Vec2D(0.5,0.5) ); 
					terrain_uv_.push_back(Vec2D(1.0,1.0) );  terrain_uv_.push_back(Vec2D(0.0,1.0) );  terrain_uv_.push_back(Vec2D(0.5,0.5) ); 
					terrain_uv_.push_back(Vec2D(1.0,0.0) );  terrain_uv_.push_back(Vec2D(1.0,1.0) );  terrain_uv_.push_back(Vec2D(0.5,0.5) ); 
					terrain_uv_.push_back(Vec2D(0.0,0.0) );  terrain_uv_.push_back(Vec2D(1.0,0.0) );  terrain_uv_.push_back(Vec2D(0.5,0.5) ); */ 
				}
			}
			xxx ++; 
			if(xxx == 16){
				//break;
				xxx = 0;
				yyy ++;
			}
		}

		tex.seek((int)nextpos);
	}
	//printf("%d\n",count);
	count = 0;
	//2月2日 紋理
	printf("Init Texture\n");
	int current_mcnk_to_get_tex = 0;
	
	while (!f_tex.isEof()) {
		f_tex.read(fourcc,4);
		f_tex.read(&size, 4); 

		flipcc(fourcc);
		fourcc[4] = 0;
		//printf("%s %d\n",fourcc,size);
		fourcc[4] = 0;

		size_t nextpos = f_tex.getPos() + size;
		if (!strcmp(fourcc,"MTEX")) {
			printf("Texture list:\n");
			char tem_buffer[1024];
			f_tex.read(tem_buffer,size);
			std::vector<std::string> texture_names = SlitString(tem_buffer,size);
			for(int i = 0;i < texture_names.size();i ++){
				printf("%s\n",texture_names[i].c_str());
				tex_id_map[i] = texturemanager.add(texture_names[i] );
			}

		}
		
		if (!strcmp(fourcc,"MCNK")) {
			if(current_mcnk_to_get_tex == 138){
				int	fdsfds = 0;
			}
			f_tex.seekRelative(4); //得到有几个 Texture
			int count_header;
			f_tex.read(&count_header,sizeof(count_header) );	
			count_header /= 16;
			int saved_count_header = count_header;
			passes_[current_mcnk_to_get_tex].tex_layers = count_header;

			TexHeader_ tex_header[4];
			bool compressed[4] = {false,false,false,false};
			f_tex.read(&tex_header,sizeof(TexHeader_)*count_header );
			
			for(int i_header = 0;i_header < count_header;i_header++){
				//if(tex_header[i_header].flags & 0x200)
					//compressed[i_header] = true;
				passes_[current_mcnk_to_get_tex].textures[i_header] =  ((Texture*)texturemanager.items[tex_id_map[tex_header[i_header].textureId]])->tex ;
				passes_[current_mcnk_to_get_tex].map_to_alpha[i_header] = tex_header[i_header].offsetInMCAL;

			}
			//g_maps.push_back(tex_header.textureId);
			//printf("Texture ID %d\n",tex_header.textureId);
			/*if(tex_header.textureId == 0)
				count_0 ++;
			else 
				count_1 ++;*/
			/*bool compressed = false; //是否壓縮的alpha map
			if(tex_header.flags & 0x200)
				compressed = true;
			passes_[current_mcnk_to_get_tex].textures[0] =  ((Texture*)texturemanager.items[tex_id_map[tex_header.textureId]])->tex ;
			passes_[current_mcnk_to_get_tex].map_to_alpha[0] = tex_header.offsetInMCAL;
			count_header --;
			int count_for_tex_layes = 1;
			while(count_header > 0){
				//f_tex.seekRelative(16*count_header - 16);
				f_tex.read(&tex_header,sizeof(tex_header) );
				if(tex_header.flags & 0x200)
					compressed = true;
				if(compressed == false ){
					//printf("index %d flags %x\n",current_mcnk_to_get_tex,tex_header.flags);
				}
				passes_[current_mcnk_to_get_tex].textures[count_for_tex_layes] = ((Texture*)texturemanager.items[tex_id_map[tex_header.textureId]])->tex;
				passes_[current_mcnk_to_get_tex].map_to_alpha[count_for_tex_layes] = tex_header.offsetInMCAL;
				count_for_tex_layes ++;
				count_header --;
			}*/
			passes_[current_mcnk_to_get_tex].tex_layers = saved_count_header;
			
			f_tex.read(fourcc,4);
			f_tex.read(&size, 4); 

			flipcc(fourcc);
			bool _use_alph_map = false;
			int c_macl = 0; //alpha_map 的层数

			//处理Shadow Map
			passes_[current_mcnk_to_get_tex].use_shadow = false;
			passes_[current_mcnk_to_get_tex].shadow_texture = NULL;
			if(strcmp(fourcc,"MCSH") ==0 ){
				assert(size == 64*8);
				passes_[current_mcnk_to_get_tex].use_shadow = true;
				char shadow_tmp_buffer[64*8];
				f_tex.read(shadow_tmp_buffer,64*8);
				unsigned char* p_shadow_tmp_buffer = passes_[current_mcnk_to_get_tex].shadow_map;
				for(int i_line_sd = 0;i_line_sd < 64*8;i_line_sd++){
					for(int b_to_and = 0x01;b_to_and!= 0x100;b_to_and=b_to_and<<1){
						*p_shadow_tmp_buffer++ = (shadow_tmp_buffer[i_line_sd]&b_to_and )?85:0;
					}
				}	
				if(current_mcnk_to_get_tex == 138){
					for(int ixx = 0;ixx < 64;ixx++){
						for(int iyy = 0;iyy < 64;iyy++){
							if(passes_[current_mcnk_to_get_tex].shadow_map[ixx*64+iyy])
								printf("XX %d %d\n",ixx,iyy);
						}
					}
					printf("Found Okay\n");
					fourcc[0] = 0;
				}

				//读取下个fource
				f_tex.read(fourcc,4);
				f_tex.read(&size, 4);
				flipcc(fourcc);
			}		
			while(strcmp(fourcc,"MCAL") != 0 && saved_count_header != 1 ){  //这里是为了跳过 ShadowMap
				if(current_mcnk_to_get_tex >=128)
					printf("MCSH %d\n",current_mcnk_to_get_tex);
				f_tex.seekRelative(size);
				f_tex.read(fourcc,4);
				f_tex.read(&size, 4);
				flipcc(fourcc);
			}
			if(!strcmp(fourcc,"MCAL")){
				_use_alph_map = true;
			///	printf("MCAL size %d\n",size);
				/*c_macl = size / 2048;
				char tmp_buffer[2048];
				for(int i = 0;i < c_macl;i++){
					f_tex.read(tmp_buffer,2048);
					for(int j = 0;j<64;j++){
						for(int m = 0;m < 64;m++){
							int bit = tmp_buffer[32*j +m/2];
							int bit_high = (bit & 0xf0)>>4;
							int bit_low = bit & 0xf;
							if(m%2 == 0){	
								//g_alpha_maps[current_mcnk_to_get_tex][i][j][m] = bit_high /15.0;
							}
							else{
								//g_alpha_maps[current_mcnk_to_get_tex][i][j][m] = bit_low / 15.0;
							}
						}
					}					
				}
			}*/
				//我們這裡暫時只考慮 compressed alphamap的情況
				char buffer_alpha_map[100000];
				if(current_mcnk_to_get_tex == 16+13){
					buffer_alpha_map[1] = 0;
				}
				f_tex.read(buffer_alpha_map,size);
				//对上述进行处理
				///int next_offset = 0; //从哪个offset开始读
				for(int i_header = 1;i_header < saved_count_header;i_header++){
					if(tex_header[i_header].flags & 0x200){  //Compressed
						ReadAlphaMapCompressed(buffer_alpha_map + tex_header[i_header].offsetInMCAL,tex_header[i_header].offsetInMCAL,current_mcnk_to_get_tex,i_header-1);
					}
					else{  //uncompressed
						//如果不是最后一个 通过 下一个的offset 确定 这里假定 next的offset 》 现在的offset 如果不是 那么以后再处理
						if(i_header < saved_count_header - 1){
							int next_offset = tex_header[i_header+1].offsetInMCAL;
							int read_size = next_offset - tex_header[i_header].offsetInMCAL;
							assert(read_size != 0 && read_size % 2048 == 0);
							ReadAlphaMapUnCompressed(buffer_alpha_map + tex_header[i_header].offsetInMCAL,tex_header[i_header].offsetInMCAL,read_size,current_mcnk_to_get_tex,i_header-1);
						}
						else {
							int next_offset = tex_header[i_header+1].offsetInMCAL;
							int read_size = size - tex_header[i_header].offsetInMCAL;
							assert(read_size != 0 && read_size % 2048 == 0);
							ReadAlphaMapUnCompressed(buffer_alpha_map + tex_header[i_header].offsetInMCAL,tex_header[i_header].offsetInMCAL,read_size,current_mcnk_to_get_tex,i_header-1);
						}
					}
				}
				/*if(compressed)
				    ReadAlphaMapCompressed(buffer_alpha_map,size,current_mcnk_to_get_tex);
				else
					ReadAlphaMapUnCompressed(buffer_alpha_map,size,current_mcnk_to_get_tex);*/


				//memset(&passes_[current_mcnk_to_get_tex].alpha_maps,0,sizeof(passes_[current_mcnk_to_get_tex].alpha_maps) ); //这个会在 PrepareAlphaMap 中初始化
				passes_[current_mcnk_to_get_tex].alpha_texture = NULL;
				passes_[current_mcnk_to_get_tex].use_alpha_map = _use_alph_map;	
				//得到這個 Chunk 對應的AlphaMap
				for(int ii = 0;ii < saved_count_header;ii++){
					int formal_offset = passes_[current_mcnk_to_get_tex].map_to_alpha[ii];
					passes_[current_mcnk_to_get_tex].map_to_alpha[ii] =  alpha_offset_to_index_[current_mcnk_to_get_tex][formal_offset];
					//passes_[current_mcnk_to_get_tex].map_to_alpha[count_for_tex_layes] = 0;
				}
			}
			current_mcnk_to_get_tex ++;
			
			//printf("Chunck id %d\n" ,tex_header.textureId);

		}
		f_tex.seek((int)nextpos);
	}
	//printf("%d\n",count);

	//填充Buffer
	TERRAIBVERTEX * p_buffer = new TERRAIBVERTEX[terrain_.size()];
	//填充
	for(int i = 0;i<terrain_.size();i++){
		p_buffer[i].x = terrain_[i].x;
		p_buffer[i].y = terrain_[i].y;
		p_buffer[i].z = terrain_[i].z;

		p_buffer[i].nx = terrain_normal_[i].x;
		p_buffer[i].ny = terrain_normal_[i].y;
		p_buffer[i].nz = terrain_normal_[i].z;

		p_buffer[i].color = RGB(255,255,255);
		
		p_buffer[i].u = terrain_uv_[i].x;
		p_buffer[i].v = terrain_uv_[i].y;
		//p_buffer[i].u = 0;
		//p_buffer[i].v = 0;
		p_buffer[i].u2 = terrain_uv_[i].x;
		p_buffer[i].v2 = terrain_uv_[i].y;
		p_buffer[i].u3 = terrain_uv_[i].x;
		p_buffer[i].v3 = terrain_uv_[i].y;
		p_buffer[i].u4 = terrain_uv_[i].x;
		p_buffer[i].v4 = terrain_uv_[i].y;
		p_buffer[i].u5 = terrain_uv_[i].x;
		p_buffer[i].v5 = terrain_uv_[i].y;
	}
	if( FAILED( g_pd3dDevice->CreateVertexBuffer( terrain_.size() * sizeof( TERRAIBVERTEX ),
		0, D3DFVF_TERRAINVERTEX,
		D3DPOOL_DEFAULT, &terrain_buffer_pass0, NULL ) ) )
	{
		printf("Error In CreateVetex\n");
		return ;
	}
	VOID* pVertices;
	if( FAILED( terrain_buffer_pass0->Lock( 0, 0, ( void** )&pVertices, 0 ) ) ){
		printf("Error in Fill Vertex Buffer\n");		
		return ;
	}
	memcpy(pVertices, p_buffer,terrain_.size() * sizeof( TERRAIBVERTEX ));
	terrain_buffer_pass0->Unlock();

	/*if( FAILED( g_pd3dDevice->CreateVertexBuffer( count_0 * 64 * 12 * sizeof( TERRAIBVERTEX ),
		0, D3DFVF_TERRAINVERTEX,
		D3DPOOL_DEFAULT, &terrain_buffer_pass0, NULL ) ) )
	{
		printf("Error In CreateVetex\n");
		return ;
	}
	if( FAILED( g_pd3dDevice->CreateVertexBuffer( count_1 * 64 * 12 * sizeof( TERRAIBVERTEX ),
		0, D3DFVF_TERRAINVERTEX,
		D3DPOOL_DEFAULT, &terrain_buffer_pass1, NULL ) ) )
	{
		printf("Error In CreateVetex\n");
		return ;
	}
	// Fill the vertex buffer.
	VOID* pVertices;
	if( FAILED( terrain_buffer_pass0->Lock( 0, 0, ( void** )&pVertices, 0 ) ) ){
		printf("Error in Fill Vertex Buffer\n");		
		return ;
	}
	TERRAIBVERTEX* pp = (TERRAIBVERTEX*)pVertices;
	for(int i = 0;i < g_terrain.size();i++){
		if(g_maps[i/(16*16*3)] == 0){
			pp->u = g_terrain_uv[i].x;
			pp->v = g_terrain_uv[i].y;
			pp->x = g_terrain[i].x;
			pp->y = g_terrain[i].y;
			pp->z = g_terrain[i].z;
			pp->nx = g_terrain_normal[i].x;
			pp->ny = g_terrain_normal[i].y;
			pp->nz = g_terrain_normal[i].z;
			pp++;
		}
	}

	
	terrain_buffer_pass0->Unlock();
	if( FAILED( terrain_buffer_pass1->Lock( 0, 0, ( void** )&pVertices, 0 ) ) ){
		printf("Error in Fill Vertex Buffer\n");		
		return ;
	}
	pp = (TERRAIBVERTEX*)pVertices;
	for(int i = 0;i < g_terrain.size();i++){
		if(g_maps[i/(16*16*3)] == 1){
			pp->u = g_terrain_uv[i].x;
			pp->v = g_terrain_uv[i].y;
			pp->x = g_terrain[i].x;
			pp->y = g_terrain[i].y;
			pp->z = g_terrain[i].z;
			pp->nx = g_terrain_normal[i].x;
			pp->ny = g_terrain_normal[i].y;
			pp->nz = g_terrain_normal[i].z;
			pp++;
		}
	}

	terrain_buffer_pass1->Unlock();
	delete[] p_buffer;*/

	//輸出 G_ALpha Maps[0][0] 
	/*FILE* f_to_write_array = fopen("c:\\map.txt","w");
	char buffer[1024];
	sprintf(buffer,"FLOAT g_maps0[64][64] = {\n");
	fwrite(buffer,1,strlen(buffer),f_to_write_array);

	for(int i = 0;i < 64;i++){
		for(int j = 0;j < 64;j++){
			sprintf(buffer,"%f,",g_alpha_maps[2][0][i][j]);
			fwrite(buffer,1,strlen(buffer),f_to_write_array);
		}
		sprintf(buffer,"\n");
		fwrite(buffer,1,strlen(buffer),f_to_write_array);
	}

	sprintf(buffer,"};\n");
	fwrite(buffer,1,strlen(buffer),f_to_write_array);

	sprintf(buffer,"FLOAT g_maps1[64][64] = {\n");
	fwrite(buffer,1,strlen(buffer),f_to_write_array);

	for(int i = 0;i < 64;i++){
		for(int j = 0;j < 64;j++){
			sprintf(buffer,"%f,",g_alpha_maps[2][1][i][j]);
			fwrite(buffer,1,strlen(buffer),f_to_write_array);
		}
		sprintf(buffer,"\n");
		fwrite(buffer,1,strlen(buffer),f_to_write_array);
	}

	sprintf(buffer,"};");
	fwrite(buffer,1,strlen(buffer),f_to_write_array);

	fclose(f_to_write_array);*/
	PrepareAlphaMap();

	

}


void World::PrepareAlphaMap()
{
	//這裡開始 填充Alpha 圖片 不用alpha map的话 其都为0
	for(int i = 0; i < 16;i ++){
		for(int j = 0;j < 16;j++){
			if(!passes_[i*16+j].use_alpha_map)
				continue;
			HRESULT hr = g_pd3dDevice->CreateTexture(64,64,0,D3DUSAGE_DYNAMIC,D3DFMT_A8R8G8B8,D3DPOOL_DEFAULT,&passes_[i*16+j].alpha_texture, NULL);

			D3DLOCKED_RECT LockedRect;
			passes_[i*16+j].alpha_texture->LockRect(0,&LockedRect,NULL,0);
		
			unsigned char* p = (unsigned char*)LockedRect.pBits;
			for(int ii = 0;ii < 64;ii++){
				for(int jj = 0;jj < 64;jj++){
					//B G R A  //是0
					for(int w = 0;w < 1;w++){
						if(i == 1 && j == 13){
							int a0 =  255*alpha_maps_[i*16+j][0][ii][jj];
							int a1 =   255* alpha_maps_[i*16+j][1][ii][jj];
							int a2 =   255* alpha_maps_[i*16+j][2][ii][jj];
							//printf("%d ",a0);
							//if(jj == 63)
								//printf("\n");

						}
						*p++ = 0;
						//*p++ = 0; *p++=0;*p++=0;
						*p++ = (unsigned char)(alpha_maps_[i*16+j][2][ii][jj] * 255.0 );
						*p++ = (unsigned char)(alpha_maps_[i*16+j][1][ii][jj] * 255.0 );  //這個對應 Shader的 r 或者 x
						*p++ = (unsigned char)(alpha_maps_[i*16+j][0][ii][jj] * 255.0 );
					}
				}
			}
			passes_[i*16+j].alpha_texture->UnlockRect(0);
		}
	}
	/*for(int i = 0;i < 16;i++){
		for(int j = 0;j < 16;j++){
			if(!passes_[i*16+j].use_alpha_map)
				continue;
			for(int ii = 0;ii < 64;ii++){
				for(int jj = 0;jj < 64;jj++){
					passes_[i*16+j].alpha_maps[ii*64+jj].x = (unsigned char)(alpha_maps_[i*16+j][2][ii][jj] * 128.0 ); 
					passes_[i*16+j].alpha_maps[ii*64+jj].y = (unsigned char)(alpha_maps_[i*16+j][1][ii][jj] * 128.0 ); 
					passes_[i*16+j].alpha_maps[ii*64+jj].z = (unsigned char)(alpha_maps_[i*16+j][0][ii][jj] * 128.0 ); 
				}
			}

		}
	}*/

	//加入 shadow_map 纹理的处理
	for(int i = 0; i < 16;i ++){
		for(int j = 0;j < 16;j++){
			if(!passes_[i*16+j].use_shadow)
				continue;
			HRESULT hr = g_pd3dDevice->CreateTexture(64,64,0,D3DUSAGE_DYNAMIC,D3DFMT_A8R8G8B8,D3DPOOL_DEFAULT,&passes_[i*16+j].shadow_texture, NULL);

			D3DLOCKED_RECT LockedRect;
			passes_[i*16+j].shadow_texture->LockRect(0,&LockedRect,NULL,0);

			unsigned char* p = (unsigned char*)LockedRect.pBits;
			for(int ii = 0;ii < 64;ii++){
				for(int jj = 0;jj < 64;jj++){
					*p++ = 0;
					*p++ = 0;
					*p++ = passes_[i*16+j].shadow_map[ii*64+jj]/1.0 ;  //這個對應 Shader的 r 或者 x
					*p++ = 0;
				}
			}
			passes_[i*16+j].shadow_texture->UnlockRect(0);
		}
	}
}


void World::Draw()
{
	g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE);
	//g_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME);

	D3DXMATRIXA16 matWorld;
	D3DXMatrixIdentity( &matWorld );
	//D3DXMatrixRotationY( &matWorld, timeGetTime() / 500.0f );
	g_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );
	
	
	
	g_pd3dDevice->SetFVF( D3DFVF_TERRAINVERTEX );

	// g_pd3dDevice->SetRenderState(D3DRS_AMBIENT, D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f));
	 D3DMATERIAL9 mtrl;
	 ::ZeroMemory(&mtrl, sizeof(mtrl));
	 mtrl.Ambient  = D3DXCOLOR(0.8f, 0.8f, 0.8f, 0.8f);
	 mtrl.Diffuse  = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	 mtrl.Specular = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	// g_pd3dDevice->SetMaterial(&mtrl);

	//g_pd3dDevice->DrawPrimitive( D3DPT_POINTLIST, 0, g_terrain.size() );
	 g_pd3dDevice->SetStreamSource( 0, terrain_buffer_pass0, 0, sizeof( TERRAIBVERTEX ) );
	 int tex_id0 = tex_id_map[0];
	  int tex_id1 = tex_id_map[4];
	 Texture& c_tex0 = *((Texture*)texturemanager.items[tex_id0] );
	 Texture& c_tex1 = *((Texture*)texturemanager.items[tex_id1] );

	 //设置 Pixel Shader的参数
	 D3DXMATRIX mat_world_view_proj;
	 D3DXMATRIX mat_inverse_world;
	 D3DXMATRIX world_matrix;
	 D3DXMatrixIdentity(&mat_inverse_world);
	  D3DXMatrixIdentity(&mat_world_view_proj);

	  g_pd3dDevice->GetTransform(D3DTS_WORLD,&world_matrix);

	  D3DXMATRIX view ; 
	  D3DXMATRIX proj ; 
	  g_pd3dDevice->GetTransform(D3DTS_VIEW,&view);;
	  g_pd3dDevice->GetTransform(D3DTS_PROJECTION,&proj);;

	  D3DXMatrixInverse(&mat_inverse_world,0,&world_matrix);


	  mat_world_view_proj = view * proj ; 


	 TransformConstantTableVetex->SetMatrix(g_pd3dDevice,Vetex_matWorldViewProj,&mat_world_view_proj);
	 TransformConstantTableVetex->SetMatrix(g_pd3dDevice,Vetex_matInverseWorld,&mat_inverse_world);

	 
	// g_pd3dDevice->SetTexture(1,c_tex1.tex);
	 g_pd3dDevice->SetPixelShader(Terrain_shader);
	 g_pd3dDevice->SetVertexShader(Terrain_shader_vetex);
	// g_pd3dDevice->SetPixelShader(NULL);
	// g_pd3dDevice->SetVertexShader(NULL);

	// g_pd3dDevice->SetTexture(0,c_tex0.tex);

	 //问题出在 8 9  9 11 这里

	 for(int i = 0;i< 16;i++){
		 for(int j = 0;j < 16;j++){

			

			 g_pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
			 g_pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
			 g_pd3dDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

			 g_pd3dDevice->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
			 g_pd3dDevice->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
			 g_pd3dDevice->SetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

			 g_pd3dDevice->SetSamplerState(2, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
			 g_pd3dDevice->SetSamplerState(2, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
			 g_pd3dDevice->SetSamplerState(2, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR); 

			 g_pd3dDevice->SetSamplerState(3, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
			 g_pd3dDevice->SetSamplerState(3, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
			 g_pd3dDevice->SetSamplerState(3, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

			 g_pd3dDevice->SetSamplerState(4, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
			 g_pd3dDevice->SetSamplerState(4, D3DSAMP_MINFILTER, D3DTEXF_NONE);
			 g_pd3dDevice->SetSamplerState(4, D3DSAMP_MIPFILTER, D3DTEXF_NONE);

			 g_pd3dDevice->SetSamplerState(5, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
			 g_pd3dDevice->SetSamplerState(5, D3DSAMP_MINFILTER, D3DTEXF_NONE);
			 g_pd3dDevice->SetSamplerState(5, D3DSAMP_MIPFILTER, D3DTEXF_NONE);

			/* g_pd3dDevice->SetSamplerState(4, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
			 g_pd3dDevice->SetSamplerState(4, D3DSAMP_MINFILTER, D3DTEXF_POINT);
			 g_pd3dDevice->SetSamplerState(4, D3DSAMP_MIPFILTER, D3DTEXF_POINT);*/

			/* g_pd3dDevice->SetSamplerState(0, D3DSAMP_MAXANISOTROPY, 4);
			 g_pd3dDevice->SetSamplerState(1, D3DSAMP_MAXANISOTROPY, 4);
			 g_pd3dDevice->SetSamplerState(2, D3DSAMP_MAXANISOTROPY, 4);
			 g_pd3dDevice->SetSamplerState(3, D3DSAMP_MAXANISOTROPY, 4);*/

			 g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);

			 		 	 

			 g_pd3dDevice->SetTexture(Samp0Desc.RegisterIndex,  passes_[i*16+j].textures[0]);
			 
			 g_pd3dDevice->SetTexture(Samp1Desc.RegisterIndex, passes_[i*16+j].textures[1]);
			 g_pd3dDevice->SetTexture(Samp2Desc.RegisterIndex, passes_[i*16+j].textures[2]);
			 g_pd3dDevice->SetTexture(Samp3Desc.RegisterIndex, passes_[i*16+j].textures[3]);

			/* g_pd3dDevice->SetTexture(0,g_pTexture0);
			 g_pd3dDevice->SetTexture(1,g_pTexture1);
			 g_pd3dDevice->SetTexture(2,g_pTexture2);
			  g_pd3dDevice->SetTexture(3,NULL);*/

			 g_pd3dDevice->SetTexture(SampAlphaDesc.RegisterIndex, passes_[i*16+j].alpha_texture); 
			 //Shadow Map 的处理
			 g_pd3dDevice->SetTexture(SampShadowDesc.RegisterIndex,passes_[i*16+j].shadow_texture);
			//// g_pd3dDevice->SetTexture(SampShadowDesc.RegisterIndex,NULL);
			// TransformConstantTableVetex->SetVectorArray(g_pd3dDevice,AlphamapHandle,passes_[i*16+j].alpha_maps,64*64);

			// g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST, terrain_.size()/3/16*(i*16+j)*3,terrain_.size()/3/16);

			 g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST, terrain_.size()/3/16/16*(i*16+j)*3,terrain_.size()/3/16/16);
			 
		 }
	 }
	/*g_pd3dDevice->SetStreamSource( 0, terrain_buffer_pass0, 0, sizeof( TERRAIBVERTEX ) );
	int tex_id0 = tex_id_map[0];
	Texture& c_tex0 = *((Texture*)texturemanager.items[tex_id0] );
	g_pd3dDevice->SetTexture(0,c_tex0.tex);
	g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0,count_0 * 64 * 12/3);

	g_pd3dDevice->SetStreamSource( 0, terrain_buffer_pass1, 0, sizeof( TERRAIBVERTEX ) );
	int tex_id1 = tex_id_map[1];
	Texture& c_tex1 = *((Texture*)texturemanager.items[tex_id1] );
	g_pd3dDevice->SetTexture(0,c_tex1.tex);
	g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0,count_1 * 64 * 12/3);*/

}