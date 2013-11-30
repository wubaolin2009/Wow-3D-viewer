#include "camera.h"
#include "db.h"
#include "enums.h"
#include "texture.h"
#include "Character.h"
#include "Item.h"
#include "creature.h"
#include "ray.h"
#include "CharControl.h"
#include "my_CEGUI.h"
#include "world.h"

#include <Windows.h>
#include <mmsystem.h>
#include <d3dx9.h>
#pragma warning( disable : 4996 ) // disable deprecated warning 
#include <strsafe.h>
#pragma warning( default : 4996 )


//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------
LPDIRECT3D9             g_pD3D = NULL; // Used to create the D3DDevice
LPDIRECT3DDEVICE9       g_pd3dDevice = NULL; // Our rendering device

ItemDisplayDB		itemdisplaydb;
CharSectionsDB      chardb;

World* g_wr[16];

CCamera g_camera;

//Texture 管理器
TextureManager texturemanager;
//这里暂时用个全局变量 保存屏幕上人物的控制
CharControl* g_char_control;
CharacterModel* g_spell;

CharacterModel* g_model;

//這個暫時為敵人
CharControl* g_char_enemy_control;

HWND g_hwnd;

//GUI系统
//CeGUI g_gui;


HRESULT InitD3D( HWND hWnd )
{
	// Create the D3D object.
	if( NULL == ( g_pD3D = Direct3DCreate9( D3D_SDK_VERSION ) ) )
		return E_FAIL;

	// Set up the structure used to create the D3DDevice
	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory( &d3dpp, sizeof( d3dpp ) );
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat  = D3DFMT_D16;

	// Create the D3DDevice 11月18日增加Z缓冲支持
	if( FAILED( g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
		D3DCREATE_HARDWARE_VERTEXPROCESSING,
		&d3dpp, &g_pd3dDevice ) ) )
	{
		return E_FAIL;
	}
	HRESULT hr = g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
	//Init Light
	/*D3DLIGHT9 light;
	ZeroMemory(&light,sizeof(light));
	light.Type = D3DLIGHT_POINT ;
	light.Position = D3DXVECTOR3(0,0,-20);
	light.Direction = D3DXVECTOR3(0,0,10);
	light.Ambient.g = 0.0f;
	light.Ambient.r = 0.0f;
	light.Ambient.b = 0.0f;
	light.Specular.r = 1.0f;
	light.Specular.b = 0.0f;
	light.Diffuse.r = 0.0f;
	light.Range = 1000.0f;
	light.Attenuation0 = 0.1f;
	g_pd3dDevice->SetLight(0,&light);
	g_pd3dDevice->LightEnable(0,true);*/
	//Lights
	D3DXVECTOR3 vecDir;
	D3DLIGHT9 light;
	ZeroMemory( &light, sizeof( D3DLIGHT9 ) );
	light.Type = D3DLIGHT_DIRECTIONAL;
	light.Diffuse.r = 1.0f;
	light.Diffuse.g = 1.0f;
	light.Diffuse.b = 1.0f;
	vecDir = D3DXVECTOR3( 0.0f,-1.0f,0.5f ) ;
	D3DXVec3Normalize( ( D3DXVECTOR3* )&light.Direction, &vecDir );
	light.Range = 1000.0f;
	g_pd3dDevice->SetLight( 0, &light );
	g_pd3dDevice->LightEnable( 0, TRUE );
	g_pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE );

	// Finally, turn on some ambient light.
	g_pd3dDevice->SetRenderState( D3DRS_AMBIENT, 0x00000000 );

	//Init GUI
	//g_gui.InitGUI(g_pd3dDevice);

	return S_OK;
}

//坐标轴
LPDIRECT3DVERTEXBUFFER9 g_vetex3_buffer_;

//地面
LPDIRECT3DVERTEXBUFFER9 g_vetex_ground_buffer_;
//画3个坐标轴 1月26日 x是红 y是绿 z是蓝 一个地面

//检测 点击的是地面的那个点
// 地面的 (-10,0,10)是 原点
// 二维的x方向是三维的x  y方向是 三维的z
void DectecGroud()
{
	V3VERTEX bufferg[] = {
		{-10,0,-10,RGB(255,255,255)},
		{-10,0,10,RGB(255,255,255)},
		{10,0,10,RGB(255,255,255)},

		{10,0,10,RGB(255,255,255)},
		{10,0,-10,RGB(255,255,255)},
		{-10,0,-10,RGB(255,255,255)},
	};
	POINT pt;
	::GetCursorPos(&pt);
	::ScreenToClient(g_hwnd,&pt);
	RayDetect ray(pt.x,pt.y);
	FLOAT dis = 0;
	D3DXVECTOR3 vec_point;
	if( ray.Intersect(bufferg,6,&dis,&vec_point)){
		//printf("Ground %f %f %f\n",vec_point.x,vec_point.y,vec_point.z);
		if(g_char_control){
			g_char_control->MoveTo(vec_point.x,vec_point.y,vec_point.z);
		}

	}	
}
void Init3()
{
	V3VERTEX buffer[] = {
		{0,0,0,RGB(0,0,255)},
		{5,0,0,RGB(0,0,255)},

		{0,0,0,RGB(0,255,0)},
		{0,5,0,RGB(0,255,0)},

		{0,0,0,RGB(255,0,0)},
		{0,0,5,RGB(255,0,0)},
	};

	if( FAILED( g_pd3dDevice->CreateVertexBuffer( 100 * sizeof( V3VERTEX ),
		0, D3DFVF_3VERTEX,
		D3DPOOL_DEFAULT, &g_vetex3_buffer_, NULL ) ) )
	{
		printf("Error In CreateVetex\n");
		return ;
	}

	VOID* pVertices;
	if( FAILED( g_vetex3_buffer_->Lock( 0, 0, ( void** )&pVertices, 0 ) ) ){
		printf("Error in Fill Vertex Buffer\n");		
		return ;
	}
	memcpy( pVertices, buffer, sizeof(buffer) );
	g_vetex3_buffer_->Unlock();

	//地面
	V3VERTEX bufferg[] = {
		{-10,0,-10,RGB(255,255,255)},
		{-10,0,10,RGB(255,255,255)},
		{10,0,10,RGB(255,255,255)},

		{10,0,10,RGB(255,255,255)},
		{10,0,-10,RGB(255,255,255)},
		{-10,0,-10,RGB(255,255,255)},
	};

	if( FAILED( g_pd3dDevice->CreateVertexBuffer( 100 * sizeof( V3VERTEX ),
		0, D3DFVF_3VERTEX,
		D3DPOOL_DEFAULT, &g_vetex_ground_buffer_, NULL ) ) )
	{
		printf("Error In CreateVetex\n");
		return ;
	}

	if( FAILED( g_vetex_ground_buffer_->Lock( 0, 0, ( void** )&pVertices, 0 ) ) ){
		printf("Error in Fill Vertex Buffer\n");		
		return ;
	}
	memcpy( pVertices, bufferg, sizeof(buffer) );
	g_vetex3_buffer_->Unlock();
}

//全局的一个渲染时间
int globalTime = 0;

VOID Render()
{
	// Clear the backbuffer to a black color 1月18 加入Z缓冲
	g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB( 0, 0, 0), 1.0f, 0 );

	g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, D3DZB_FALSE );       //默认激活
	//g_pd3dDevice->SetRenderState( D3DRS_ZFUNC, D3DCMP_GREATER );  //设置比较函数，默认为D3DCMP_LESSEQUAL
	//g_pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);    //默认值为TRUE

	//g_pDevcie->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
	//g_pDevice->SetRenderState(D3DRS_FUNC, TRUE);


	//g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW);
	g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE);
	//HRESULT hr = g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
	
	
	g_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME);
	g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE); 

	if(g_char_control){
		g_char_control->Update();
	}
	// Begin the scene
	if( SUCCEEDED( g_pd3dDevice->BeginScene() ) )
	{
		//这里做个世界坐标变换
		D3DXMATRIX world;
		D3DXMatrixIdentity(&world);
		g_pd3dDevice->SetTransform( D3DTS_WORLD, &world );
		//画坐标轴
		g_pd3dDevice->SetStreamSource( 0, g_vetex3_buffer_, 0, sizeof( V3VERTEX ) );
		g_pd3dDevice->SetTexture(0,NULL);
		g_pd3dDevice->SetFVF( D3DFVF_3VERTEX );
		g_pd3dDevice->DrawPrimitive( D3DPT_LINELIST, 0, 3 );
		//画地面
		/*g_pd3dDevice->SetStreamSource( 0, g_vetex_ground_buffer_, 0, sizeof( V3VERTEX ) );
		g_pd3dDevice->SetTexture(0,NULL);
		g_pd3dDevice->SetFVF( D3DFVF_3VERTEX );
		g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, 2 );*/

		//用攝像機類调整矩阵
		g_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID);

		if(g_char_control){
			g_char_control->Draw(g_pd3dDevice);
		}
		//Enemy
		if(g_char_enemy_control){
			g_char_enemy_control->Draw(g_pd3dDevice);
		}
		for(int i = 0;i < 16;i++){
			if(g_wr[i] )
				g_wr[i]->Draw();
		}
		static DWORD start_time = timeGetTime();
		static DWORD last_time = start_time;
		DWORD now_time = timeGetTime();
		FLOAT speed = 1.0f;

		if(now_time - start_time >= 4667/speed){
			//printf("===========\n");
			//last_time = now_time;
			//now_time = start_time;
			start_time = now_time;
		}		
		//g_spell->Animate(0,(int)((now_time - start_time)*speed));
		//g_spell->UpdateParticle( (now_time-last_time)/1000.0*speed );
		globalTime += (now_time - last_time);
		last_time = now_time;
		//g_spell->Draw();
		//g_gui.RenderGUI();

		//g_model->DrawBoundingVolume();
		g_camera.ApplyDevice(g_pd3dDevice);	

		
		// End the scene
		g_pd3dDevice->EndScene();
	}

	// Present the backbuffer contents to the display
	g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
}

VOID Cleanup();

LRESULT WINAPI MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	g_camera.HandleMessage(hWnd,msg,wParam,lParam);
	//g_gui.HandleMessage(hWnd,msg,wParam,lParam);
	POINT point;
	//檢測到的人物和視點之間的距離
	FLOAT dis = 0;
	//人物的hit 坐標
	Vec3D world;
	switch( msg )
	{
	case WM_LBUTTONDOWN:
		//if(g_model){
			//g_model->InterSect();
		//}
		//首先檢測是否點擊到了 敵人
		::GetCursorPos(&point);
		::ScreenToClient(hWnd,&point);
		if(g_char_enemy_control->IsIntersect(point.x,point.y,&dis,&world) ){
			printf("Detect Eneymy %f %f %f",world.x,world.y,world.z);
		}
		//如果沒有點擊到 才進行DetectGroud
		else{	
			DectecGroud();
		}
		break;
	case WM_KEYDOWN:
		if(wParam == VK_SPACE){	
			if(g_char_control){
				g_char_control->Attack();
			}
		}
		if(wParam == VK_UP){	
			if(g_char_control){
				g_char_control->Cast(g_spell);
			}
		}
		break;		
	case WM_DESTROY:
		Cleanup();
		PostQuitMessage( 0 );
		return 0;
	}

	return DefWindowProc( hWnd, msg, wParam, lParam );
}

//-----------------------------------------------------------------------------
// Name: InitGeometry()
// Desc: Creates the scene geometry
//-----------------------------------------------------------------------------

void searchMPQs(bool firstTime);
//INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
void InitVShader();
int main()
{
	
	//这里要先初始化D3D然后才能初始化各种模型
	
	// Register the window class
	WNDCLASSEX wc =
	{
		sizeof( WNDCLASSEX ), CS_CLASSDC, MsgProc, 0L, 0L,
		GetModuleHandle( NULL ), NULL, NULL, NULL, NULL,
		"D3D Tutorial", NULL
	};
	RegisterClassEx( &wc );

	// Create the application's window
	HWND hWnd = CreateWindow( "D3D Tutorial", "D3D Tutorial 03: Matrices",
		WS_OVERLAPPEDWINDOW, 100, 100, WINDOW_WIDTH, WINDOW_HEIGHT,
		NULL, NULL, wc.hInstance, NULL );
	g_hwnd = hWnd;
	

	// Initialize Direct3Dn
	if( SUCCEEDED( InitD3D( hWnd ) ) )
	{
		//初始化World的Shader
		World::InitPShader();
		World::InitVShader();
		//讀取 MPQ文件
		//这里测试MPQ文件的读入 
		//InitVShader();
		searchMPQs(true);
		//这里打开数据库
		itemdisplaydb.open();
		chardb.open();
		Init3();
		//Model model_blood_elf("World\\Kalimdor\\tanaris\\passivedoodads\\goblin\\go_large_rocket_2.m2");
		CharacterModel model_blood_elf("Character\\Bloodelf\\female\\bloodelffemale.m2",g_pd3dDevice);
		//Model model_blood_elf("Item\\Objectcomponents\\shield\\buckler_damaged_a_01.m2");
		ItemModel model("Item\\Objectcomponents\\Weapon\\sword_2h_frostmourne_d_01.m2",g_pd3dDevice);
		ItemModel model_new("Item\\Objectcomponents\\Weapon\\sword_2h_ashbringer.m2",g_pd3dDevice);
		ItemModel model_arthas("Creature\\Arthaslichking\\arthaslichking.m2",g_pd3dDevice);
		ItemModel model_enemy("Creature\\Arthaslichking\\arthaslichking.m2",g_pd3dDevice);

		//////static World wr0("world\\maps\\Kalimdor\\Kalimdor_35_50",0,0);  //static 是因为 World类太大 stack 溢出
		//ItemModel model_sky("Environments\\Stars\\nexusraid_runeeffects_nebula.m2",g_pd3dDevice);
		ItemModel model_sky("Spells\\Frost_nova_area.m2",g_pd3dDevice);
		/*static World wr0("World\\maps\\Azeroth\\Azeroth_34_31",0,0);  //static 是因为 World类太大 stack 溢出
		static World wr1("World\\maps\\Azeroth\\Azeroth_34_32",0,1);  //static 是因为 World类太大 stack 溢出
		static World wr2("World\\maps\\Azeroth\\Azeroth_35_31",1,0);  //static 是因为 World类太大 stack 溢出

		static World wr3("World\\maps\\Azeroth\\Azeroth_35_32",1,1);  //static 是因为 World类太大 stack 溢出
		static World wr4("world\\maps\\Kalimdor\\Kalimdor_19_12",-1,0);  //static 是因为 World类太大 stack 溢出
		static World wr5("world\\maps\\Kalimdor\\Kalimdor_19_13",-1,1);*/  //static 是因为 World类太大 stack 溢出

		static World wr0("world\\maps\\Kalimdor\\Kalimdor_35_47",0,0);  //static 是因为 World类太大 stack 溢出
		static World wr1("world\\maps\\Kalimdor\\Kalimdor_35_48",0,1);  //static 是因为 World类太大 stack 溢出
		
		static World wr2("World\\maps\\Stormwind\\Stormwind_28_49",0,2);  //static 是因为 World类太大 stack 溢出

		static World wr3("World\\maps\\Stormwind\\Stormwind_29_47",1,0);  //static 是因为 World类太大 stack 溢出
		static World wr4("World\\maps\\Stormwind\\Stormwind_29_48",1,1);  //static 是因为 World类太大 stack 溢出

		static World wr5("World\\maps\\Stormwind\\Stormwind_29_49",1,2);  //static 是因为 World类太大 stack 溢出

		//static World wr6("world\\maps\\Kalimdor\\Kalimdor_35_47",2,0);  //static 是因为 World类太大 stack 溢出
		//static World wr7("world\\maps\\Kalimdor\\Kalimdor_35_48",2,1);  //static 是因为 World类太大 stack 溢出
		//static World wr8("world\\maps\\Kalimdor\\Kalimdor_35_49",2,2);  //static 是因为 World类太大 stack 溢出
		
		//World wr1();

		wr0.InitADT();
		wr1.InitADT();
		//wr2.InitADT();
		//wr3.InitADT();
		//wr4.InitADT();
		//wr5.InitADT();
		//wr6.InitADT();
		//wr7.InitADT();
		//wr8.InitADT();

		g_wr[0] = &wr0;
		g_wr[1] = &wr1;
		//g_wr[2] = &wr2;
		//g_wr[3] = &wr3;

		//g_wr[4] = &wr4;
		//g_wr[5] = &wr5;
		//g_wr[6] = &wr6;
		//g_wr[7] = &wr7;
		//g_wr[8] = &wr8;
		
		//g_wr1 = &wr1;
		//ItemModel model_sky("Spells\\Frost_nova_state.m2",g_pd3dDevice);
		/*model_arthas.Init();
		model_arthas.InitAnimation();
		model_arthas.InitParticle();
		
		model_enemy.Init();
		model_enemy.InitAnimation();
		model_enemy.InitParticle();
		
		CharControl con(model_arthas,Vec3D(0,0,0),Vec3D(0,0,0) );
		g_char_control = &con;
		CharControl con_enemy(model_enemy,Vec3D(5,0,0),Vec3D(0,PI/2.0,0));
		g_char_enemy_control = &con_enemy;
		//g_char_enemy_control->MoveTo(5.0,0.0,0.0);

		//一个法术
		model_sky.Init();
		model_sky.InitAnimation();
		model_sky.InitParticle();

		g_spell = & model_sky;*/

		//g_model = &model_arthas;

		// Show the window
		ShowWindow( hWnd, SW_SHOWDEFAULT );
		UpdateWindow( hWnd );

		// Enter the message loop
		MSG msg;
		ZeroMemory( &msg, sizeof( msg ) );
		while( msg.message != WM_QUIT )
		{
			if( PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE ) )
			{
				TranslateMessage( &msg );
				DispatchMessage( &msg );
			}
			else
				Render();
		}
	}


	UnregisterClass( "D3D Tutorial", wc.hInstance );
	return 0;
}

VOID Cleanup()
{
	if( g_pd3dDevice != NULL )
		g_pd3dDevice->Release();

	if( g_pD3D != NULL )
		g_pD3D->Release();
}