#include <math.h>
#include "Character.h"
#include "particle.h"
#include "texture.h"
#include "camera.h"
extern CCamera g_camera;
extern TextureManager texturemanager;

bool bZeroParticle = true; //我让其始终为true
extern LPDIRECT3DDEVICE9       g_pd3dDevice;

//这个是D3D的顶点格式
#define D3DFVF_PARTICLEVERTEX (D3DFVF_XYZ| D3DFVF_DIFFUSE | D3DFVF_TEX1)
struct PARTICLEVERTEX
{
	FLOAT x, y, z;      // The untransformed, 3D position for the vertex
	DWORD color;
	FLOAT u,v;
};

float frand()
{
	return rand()/(float)RAND_MAX;
}

float randfloat(float lower, float upper)
{
	return lower + (upper-lower)*(rand()/(float)RAND_MAX);
}
int randint(int lower, int upper)
{
	return lower + (int)((upper+1-lower)*frand());
}

template<class T>
T lifeRamp(float life, float mid, const T &a, const T &b, const T &c)
{
	if (life<=mid) 
		return interpolate<T>(life / mid,a,b);
	else 
		return interpolate<T>((life-mid) / (1.0f-mid),b,c);
}

void ParticleSystem::init(MPQFile &f, ModelParticleEmitterDef &mta, uint32 *globals)
{
	speed.init (mta.EmissionSpeed, f, globals);
	variation.init (mta.SpeedVariation, f, globals);
	spread.init (mta.VerticalRange, f, globals);
	lat.init (mta.HorizontalRange, f, globals);
	gravity.init (mta.Gravity, f, globals);
	lifespan.init (mta.Lifespan, f, globals);
	rate.init (mta.EmissionRate, f, globals);
	areal.init (mta.EmissionAreaLength, f, globals);
	areaw.init (mta.EmissionAreaWidth, f, globals);
	deacceleration.init (mta.Gravity2, f, globals);
	enabled.init (mta.en, f, globals);

	Vec3D colors2[3];
	memcpy(colors2, f.getBuffer()+mta.p.colors.ofsKeys, sizeof(Vec3D)*3);
	for (size_t i=0; i<3; i++) {
		float opacity = *(short*)(f.getBuffer()+mta.p.opacity.ofsKeys+i*2);
		colors[i] = Vec4D(colors2[i].x/255.0f, colors2[i].y/255.0f, colors2[i].z/255.0f, opacity/32767.0f);
		sizes[i] = (*(float*)(f.getBuffer()+mta.p.sizes.ofsKeys+i*sizeof(Vec2D)))*mta.p.scales[i];		
	}
	mid = 0.5; // mid can't be 0 or 1, TODO, Alfred


	slowdown = mta.p.slowdown;
	rotation = mta.p.rotation;
	pos = fixCoordSystem(mta.pos);
	texture = model->textures_[mta.texture];
	blend = mta.blend;
	rows = mta.rows;
	if (rows == 0)
		rows = 1;
	cols = mta.cols;
	if (cols == 0)
		cols = 1;
	ParticleType = mta.ParticleType;
	//order = mta.s2;
	order = mta.ParticleType>0 ? -1 : 0;
	parent = model->bones + mta.bone;

	//transform = mta.flags & 1024;

	// Type 2
	// 3145 = water ele
	// 1305 = water ele
	// 1049 = water elemental
	// 1033 = water elemental
	// 281 = water ele
	// 256 = Water elemental
	// 57 = Faith halo, ring?
	// 9 = water elemental

	billboard = !(mta.flags & MODELPARTICLE_FLAGS_DONOTBILLBOARD);

	// diagnosis test info
	EmitterType = mta.EmitterType;
	flags = mta.flags; // 0x10	Do not Trail

	manim = mtime = 0;
	rem = 0;

	emitter = 0;
	switch (EmitterType) {
	case MODELPARTICLE_EMITTER_PLANE:
		emitter = new PlaneParticleEmitter(this);
		break;
	case MODELPARTICLE_EMITTER_SPHERE:
		emitter = new SphereParticleEmitter(this);
		break;
	case MODELPARTICLE_EMITTER_SPLINE: // Spline? (can't be bothered to find one)
	default:
		wxLogMessage(wxT("[Error] Unknown Emitter: %d\n"), EmitterType);
		break;
	}

	tofs = frand();

	// init tiles, slice the texture
	for (size_t i=0; i<rows*cols; i++) {
		TexCoordSet tc;
		initTile(tc.tc, (int)i);
		tiles.push_back(tc);
	}
	//初始化顶点缓冲
	// Create the vertex buffer.
	if( FAILED( g_pd3dDevice->CreateVertexBuffer( 4 * MAX_PARTICLES * sizeof( CHARACTERVERTEX ),
		0, D3DFVF_PARTICLEVERTEX,
		D3DPOOL_DEFAULT, &vertex_buffer_, NULL ) ) )
	{
		printf("Error In CreateVetex\n");
		return ;
	}
	//索引的初始化
	if( FAILED( g_pd3dDevice->CreateIndexBuffer( MAX_PARTICLES * 6 * sizeof( uint16 ),
		0, D3DFMT_INDEX16,
		D3DPOOL_DEFAULT, &index_buffer_, NULL ) ) )
	{
		return ;
	}
	//这里初始化后就不会再改变其值了
	uint16* buffer_index = new uint16[MAX_PARTICLES * 6];
	//下边注释的代码卡了我5个小时 没想到是这里的 问题。。。 应该是i×4
	/*for(int i = 0;i < MAX_PARTICLES;i++){
		buffer_index[i * 6] = 0 + i*3;
		buffer_index[i * 6 + 1] = 1 + i*3;
		buffer_index[i * 6 + 2] = 2 + i*3;
		buffer_index[i * 6 + 3] = 0 + i*3;
		buffer_index[i * 6 + 4] = 2 + i*3;
		buffer_index[i * 6 + 5] = 3 + i*3;
	}*/
	for(int i = 0;i < MAX_PARTICLES;i++){
		buffer_index[i * 6] = 0 + i*4;
		buffer_index[i * 6 + 1] = 1 + i*4;
		buffer_index[i * 6 + 2] = 2 + i*4;
		buffer_index[i * 6 + 3] = 0 + i*4;
		buffer_index[i * 6 + 4] = 2 + i*4;
		buffer_index[i * 6 + 5] = 3 + i*4;
	}
	// Fill the vertex buffer.
	void* pVertices; 
	if( FAILED( index_buffer_->Lock( 0, 0, ( void** )&pVertices, 0 ) ) )
		return ;
	memcpy( pVertices, buffer_index, MAX_PARTICLES * 6 * sizeof(uint16) );
	index_buffer_->Unlock();
	delete[] buffer_index;
}

void ParticleSystem::initTile(Vec2D *tc, int num)
{
	Vec2D otc[4];
	Vec2D a,b;
	int x = num % cols;
	int y = num / cols;
	a.x = x * (1.0f / cols);
	b.x = (x+1) * (1.0f / cols);
	a.y = y * (1.0f / rows);
	b.y = (y+1) * (1.0f / rows);

	otc[0] = a;
	otc[2] = b;
	otc[1].x = b.x;
	otc[1].y = a.y;
	otc[3].x = a.x;
	otc[3].y = b.y;

	for (size_t i=0; i<4; i++) {
		tc[(i+4-order) & 3] = otc[i];
	}
}

void ParticleSystem::update(float dt)
{
	size_t l_manim = manim;
	if (bZeroParticle)
		l_manim = 0;
	float grav = gravity.getValue(l_manim, mtime);
	float deaccel = deacceleration.getValue(l_manim, mtime);

	// spawn new particles
	if (emitter) {
		float frate = rate.getValue(l_manim, mtime);
		float flife = lifespan.getValue(l_manim, mtime);
		//frate = ;

		float ftospawn;
		if (flife)
			ftospawn = (dt * frate / flife) + rem;
		else
			ftospawn = rem;
		////ftospawn = 100; //HACK Here to check the err

		if (ftospawn < 1.0f) {
			rem = ftospawn;
			if (rem < 0) 
				rem = 0;
		} else {
			unsigned int tospawn = (int)ftospawn;

			if ((tospawn + particles.size()) > MAX_PARTICLES) // Error check to prevent the program from trying to load insane amounts of particles.
				tospawn = (unsigned int)(MAX_PARTICLES - particles.size());

			rem = ftospawn - (float)tospawn;

			float w = areal.getValue(l_manim, mtime) * 0.5f;
			float l = areaw.getValue(l_manim, mtime) * 0.5f;
			float spd = speed.getValue(l_manim, mtime);
			float var = variation.getValue(l_manim, mtime);
			float spr = spread.getValue(l_manim, mtime);
			float spr2 = lat.getValue(l_manim, mtime);
			bool en = true;
			if (enabled.uses(manim))
				en = enabled.getValue(manim, mtime)!=0;

			//rem = 0;
			if (en) {
				for (size_t i=0; i<tospawn; i++) {
					Particle p = emitter->newParticle(manim, mtime, w, l, spd, var, spr, spr2);
					// sanity check:
					if (particles.size() < MAX_PARTICLES) // No need to check this every loop iteration. Already checked above.
						particles.push_back(p);
				}
			}
		}
	}

	float mspeed = 1.0f;

	for (ParticleList::iterator it = particles.begin(); it != particles.end(); ) {
		Particle &p = *it;
		p.speed += p.down * grav * dt - p.dir * deaccel * dt;
		//p.speed = 0;

		if (slowdown>0) {
			mspeed = expf(-1.0f * slowdown * p.life);
		}
		p.pos += p.speed * mspeed * dt;

		p.life += dt;
		float rlife = p.life / p.maxlife;
		// calculate size and color based on lifetime
		p.size = lifeRamp<float>(rlife, mid, sizes[0], sizes[1], sizes[2]);
		p.color = lifeRamp<Vec4D>(rlife, mid, colors[0], colors[1], colors[2]);

		// kill off old particles
		if (rlife >= 1.0f) {
			particles.erase(it++);
		}
		else 
			++it;
	}
	/*printf("=======\n");
	for (ParticleList::iterator it = particles.begin(); it != particles.end();++it ) {
		Particle &p = *it;
		printf("%f %f %f\n",p.pos.x,p.pos.y,p.pos.z);
	}
	printf("=======\n\n");*/
}

void ParticleSystem::setup(size_t anim, size_t time)
{
	manim = anim;
	mtime = time;
}



void ParticleSystem::draw()  //这个改动较多 请参考Wmv的相应代码 
{
	// setup blend mode 我这里先忽略所有 blen信息
	switch (blend) {
		case BM_OPAQUE:
			g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE); 
			g_pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE); 
			break;
		case BM_TRANSPARENT:
			g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE); 
			////glBlendFunc(GL_SRC_COLOR, GL_ONE);
			g_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCCOLOR);  
			g_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
			g_pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
			break;
		case BM_ALPHA_BLEND:
			g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE); 
			// originally
			//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
			// Changed blending mode to this in 0.5.08, seems to fix a few of the render problems
			////glBlendFunc(GL_SRC_ALPHA, GL_ONE); 
			g_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);  
			g_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
			g_pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
			break;
		case BM_ADDITIVE:
			g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE); 
			g_pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
			break;
		case BM_ADDITIVE_ALPHA:   //
			g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE); 
			////glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			g_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);  //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			g_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
			g_pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
			break;
		case BM_MODULATE:
		case BM_MODULATEX2:
		default:
			wxLogMessage(wxT("blend unknown: %d"), blend);
	}

	if(particles.size() == 0){
		return;
	}
	//得到纹理
   	Texture& c_tex = *((Texture*)texturemanager.items[texture] );
	///glBindTexture(GL_TEXTURE_2D, texture);

	Vec3D vRight(1,0,0);
	Vec3D vUp(0,1,0);

	// position stuff
	const float f = 1;//0.707106781f; // sqrt(2)/2
	Vec3D bv0 = Vec3D(-f,+f,0);
	Vec3D bv1 = Vec3D(+f,+f,0);
	Vec3D bv2 = Vec3D(+f,-f,0);
	Vec3D bv3 = Vec3D(-f,-f,0);

	if (billboard) {  //忽略 暂时
		float modelview[16];
		///glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
		const D3DXMATRIX* view = g_camera.GetViewTrans();
		vRight = Vec3D(view->m[0][0],view->m[1][0],view->m[2][0]);
		vUp = Vec3D(view->m[0][1],view->m[1][1],view->m[2][1]);


		//vRight = Vec3D(modelview[0], modelview[4], modelview[8]);
		//vUp = Vec3D(modelview[1], modelview[5], modelview[9]); // Spherical billboarding
		//vUp = Vec3D(0,1,0); // Cylindrical billboarding
	}
	/*
	* type:
	* 0	 "normal" particle
	* 1	large quad from the particle's origin to its position (used in Moonwell water effects)
	* 2	seems to be the same as 0 (found some in the Deeprun Tram blinky-lights-sign thing)
	*/
	//要渲染的点的集合
	static PARTICLEVERTEX vertexes[MAX_PARTICLES*4];
	int counts_to_fill = particles.size() * 4;  //要写入多少个CHARACTERVERTEX
	int current_to_fill = 0;  //当前写入的是第几个点 用来模拟OpenGL的glVertex3fv这种操作
	if (ParticleType==0 || ParticleType==2) {
		// TODO: figure out type 2 (deeprun tram subway sign)
		// - doesn't seem to be any different from 0 -_-
		// regular particles
		//memset(vertexes,0,sizeof(vertexes));
		

		if (billboard) {
			//glBegin(GL_QUADS);
			// TODO: per-particle rotation in a non-expensive way?? :|
			for (ParticleList::iterator it = particles.begin(); it != particles.end(); ++it) {
				if (tiles.size() - 1 < it->tile) // Alfred, 2009.08.07, error prevent
					break;
				const float size = it->size;// / 2;
				DWORD color = RGB(0,0,0);
				int r = (int)(it->color.x * 255.0);
				int g = (int)(it->color.y * 255.0);
				int b = (int)(it->color.z * 255.0);
				int a = (int)(it->color.w * 255.0);
				//这里貌似有字节顺序的问题 先这样 RGB(b g r)好了 //1月29日加入透明度的处理 应该颜色都正确了
				//简单的就是 将r g b分别与a相乘
				r*=it->color.w; g*=it->color.w; b*=it->color.w;
				color = RGB(b,g,r);
				color = a << 24 | color;
				//color = RGB(0,0,0);

				vertexes[current_to_fill].color = color;
				//vertexes[current_to_fill].nx = vertexes[current_to_fill].ny = vertexes[current_to_fill].nz = 0;
				vertexes[current_to_fill].x = (it->pos - (vRight + vUp) * size).x;
				vertexes[current_to_fill].y = (it->pos - (vRight + vUp) * size).y;
				vertexes[current_to_fill].z = (it->pos - (vRight + vUp) * size).z;
				vertexes[current_to_fill].u = (tiles[it->tile].tc[0]).x;
				vertexes[current_to_fill++].v = (tiles[it->tile].tc[0]).y;

				vertexes[current_to_fill].color = color;
				//vertexes[current_to_fill].nx = vertexes[current_to_fill].ny = vertexes[current_to_fill].nz = 0;
				vertexes[current_to_fill].x = (it->pos + (vRight - vUp) * size).x;
				vertexes[current_to_fill].y = (it->pos + (vRight - vUp) * size).y;
				vertexes[current_to_fill].z = (it->pos + (vRight - vUp) * size).z;
				vertexes[current_to_fill].u = (tiles[it->tile].tc[1]).x;
				vertexes[current_to_fill++].v = (tiles[it->tile].tc[1]).y;

				vertexes[current_to_fill].color = color;
				//vertexes[current_to_fill].nx = vertexes[current_to_fill].ny = vertexes[current_to_fill].nz = 0;
				vertexes[current_to_fill].x = (it->pos + (vRight + vUp)* size).x;
				vertexes[current_to_fill].y = (it->pos + (vRight + vUp)* size).y;
				vertexes[current_to_fill].z = (it->pos + (vRight + vUp)* size).z;
				vertexes[current_to_fill].u = (tiles[it->tile].tc[2]).x;
				vertexes[current_to_fill++].v = (tiles[it->tile].tc[2]).y;

				vertexes[current_to_fill].color = color;
				//vertexes[current_to_fill].nx = vertexes[current_to_fill].ny = vertexes[current_to_fill].nz = 0;
				vertexes[current_to_fill].x = (it->pos - (vRight - vUp) * size).x;
				vertexes[current_to_fill].y = (it->pos - (vRight - vUp) * size).y;
				vertexes[current_to_fill].z = (it->pos - (vRight - vUp) * size).z;
				vertexes[current_to_fill].u = (tiles[it->tile].tc[3]).x;
				vertexes[current_to_fill++].v = (tiles[it->tile].tc[3]).y;
				
				/*glColor4fv(it->color);

				glTexCoord2fv(tiles[it->tile].tc[0]);
				glVertex3fv(it->pos - (vRight + vUp) * size);

				glTexCoord2fv(tiles[it->tile].tc[1]);
				glVertex3fv(it->pos + (vRight - vUp) * size);

				glTexCoord2fv(tiles[it->tile].tc[2]);
				glVertex3fv(it->pos + (vRight + vUp) * size);

				glTexCoord2fv(tiles[it->tile].tc[3]);
				glVertex3fv(it->pos - (vRight - vUp) * size);*/
			}
			//glEnd();

		} else {
			//glBegin(GL_QUADS);
			for (ParticleList::iterator it = particles.begin(); it != particles.end(); ++it) {
				if (tiles.size() - 1 < it->tile) // Alfred, 2009.08.07, error prevent
					break;
				const float size = it->size;// / 2;
				DWORD color = RGB(0,0,0);
				int r = (int)(it->color.x * 255.0);
				int g = (int)(it->color.y * 255.0);
				int b = (int)(it->color.z * 255.0);
				int a = (int)(it->color.w * 255.0);
				//这里貌似有字节顺序的问题 先这样 RGB(b g r)好了
				color = RGB(b,g,r);
				color = a << 24 | color;

				vertexes[current_to_fill].color = color;
				//vertexes[current_to_fill].nx = vertexes[current_to_fill].ny = vertexes[current_to_fill].nz = 0;
				vertexes[current_to_fill].x = (it->pos + it->corners[0] * it->size).x;
				vertexes[current_to_fill].y = (it->pos + it->corners[0] * it->size).y;
				vertexes[current_to_fill].z = (it->pos + it->corners[0] * it->size).z;
				vertexes[current_to_fill].u = (tiles[it->tile].tc[0]).x;
				vertexes[current_to_fill++].v = (tiles[it->tile].tc[0]).y;

				vertexes[current_to_fill].color = color;
				//vertexes[current_to_fill].nx = vertexes[current_to_fill].ny = vertexes[current_to_fill].nz = 0;
				vertexes[current_to_fill].x = (it->pos + it->corners[1] * it->size).x;
				vertexes[current_to_fill].y = (it->pos + it->corners[1] * it->size).y;
				vertexes[current_to_fill].z = (it->pos + it->corners[1] * it->size).z;
				vertexes[current_to_fill].u = (tiles[it->tile].tc[1]).x;
				vertexes[current_to_fill++].v = (tiles[it->tile].tc[1]).y;

				vertexes[current_to_fill].color = color;
				//vertexes[current_to_fill].nx = vertexes[current_to_fill].ny = vertexes[current_to_fill].nz = 0;
				vertexes[current_to_fill].x = (it->pos + it->corners[2] * it->size).x;
				vertexes[current_to_fill].y = (it->pos + it->corners[2] * it->size).y;
				vertexes[current_to_fill].z = (it->pos + it->corners[2] * it->size).z;
				vertexes[current_to_fill].u = (tiles[it->tile].tc[2]).x;
				vertexes[current_to_fill++].v = (tiles[it->tile].tc[2]).y;

				vertexes[current_to_fill].color = color;
				//vertexes[current_to_fill].nx = vertexes[current_to_fill].ny = vertexes[current_to_fill].nz = 0;
				vertexes[current_to_fill].x = (it->pos + it->corners[3] * it->size).x;
				vertexes[current_to_fill].y = (it->pos + it->corners[3] * it->size).y;
				vertexes[current_to_fill].z = (it->pos + it->corners[3] * it->size).z;
				vertexes[current_to_fill].u = (tiles[it->tile].tc[3]).x;
				vertexes[current_to_fill++].v = (tiles[it->tile].tc[3]).y;

				/*glColor4fv(it->color);

				glTexCoord2fv(tiles[it->tile].tc[0]);
				glVertex3fv(it->pos + it->corners[0] * it->size);

				glTexCoord2fv(tiles[it->tile].tc[1]);
				glVertex3fv(it->pos + it->corners[1] * it->size);

				glTexCoord2fv(tiles[it->tile].tc[2]);
				glVertex3fv(it->pos + it->corners[2] * it->size);

				glTexCoord2fv(tiles[it->tile].tc[3]);
				glVertex3fv(it->pos + it->corners[3] * it->size);*/
			}
			//glEnd();
		}
	} else if (ParticleType==1) { // Sphere particles
		// particles from origin to position
		/*
		bv0 = mbb * Vec3D(0,-1.0f,0);
		bv1 = mbb * Vec3D(0,+1.0f,0);


		bv0 = mbb * Vec3D(-1.0f,0,0);
		bv1 = mbb * Vec3D(1.0f,0,0);
		*/

		//glBegin(GL_QUADS);
		for (ParticleList::iterator it = particles.begin(); it != particles.end(); ++it) {
			if (tiles.size() - 1 < it->tile) // Alfred, 2009.08.07, error prevent
				break;
			/*vertexes[current_to_fill].color = RGB(it->color.x,it->color.y,it->color.z);
			vertexes[current_to_fill].nx = vertexes[current_to_fill].ny = vertexes[current_to_fill].nz = 0;
			vertexes[current_to_fill].x = (it->pos + bv0 * it->size).x;
			vertexes[current_to_fill].y = (it->pos + bv0 * it->size).y;
			vertexes[current_to_fill].z = (it->pos + bv0 * it->size).z;
			vertexes[current_to_fill].u = (tiles[it->tile].tc[0]).x;
			vertexes[current_to_fill++].v = (tiles[it->tile].tc[0]).y;

			vertexes[current_to_fill].color = RGB(it->color.x,it->color.y,it->color.z);
			vertexes[current_to_fill].nx = vertexes[current_to_fill].ny = vertexes[current_to_fill].nz = 0;
			vertexes[current_to_fill].x = (it->pos + bv1 * it->size).x;
			vertexes[current_to_fill].y = (it->pos + bv1 * it->size).y;
			vertexes[current_to_fill].z = (it->pos + bv1 * it->size).z;
			vertexes[current_to_fill].u = (tiles[it->tile].tc[1]).x;
			vertexes[current_to_fill++].v = (tiles[it->tile].tc[1]).y;

			vertexes[current_to_fill].color = RGB(it->color.x,it->color.y,it->color.z);
			vertexes[current_to_fill].nx = vertexes[current_to_fill].ny = vertexes[current_to_fill].nz = 0;
			vertexes[current_to_fill].x = (it->origin + bv1 * it->size).x;
			vertexes[current_to_fill].y = (it->origin + bv1 * it->size).y;
			vertexes[current_to_fill].z = (it->origin + bv1 * it->size).z;
			vertexes[current_to_fill].u = (tiles[it->tile].tc[2]).x;
			vertexes[current_to_fill++].v = (tiles[it->tile].tc[2]).y;

			vertexes[current_to_fill].color = RGB(it->color.x,it->color.y,it->color.z);
			vertexes[current_to_fill].nx = vertexes[current_to_fill].ny = vertexes[current_to_fill].nz = 0;
			vertexes[current_to_fill].x = (it->origin + bv0 * it->size).x;
			vertexes[current_to_fill].y = (it->origin + bv0 * it->size).y;
			vertexes[current_to_fill].z = (it->origin + bv0 * it->size).z;
			vertexes[current_to_fill].u = (tiles[it->tile].tc[3]).x;
			vertexes[current_to_fill++].v = (tiles[it->tile].tc[3]).y;*/

			/*
			glColor4fv(it->color);

			glTexCoord2fv(tiles[it->tile].tc[0]);
			glVertex3fv(it->pos + bv0 * it->size);

			glTexCoord2fv(tiles[it->tile].tc[1]);
			glVertex3fv(it->pos + bv1 * it->size);

			glTexCoord2fv(tiles[it->tile].tc[2]);
			glVertex3fv(it->origin + bv1 * it->size);

			glTexCoord2fv(tiles[it->tile].tc[3]);
			glVertex3fv(it->origin + bv0 * it->size);*/
		}
		//glEnd();

	}
	//Debug Info
	/*printf("=========%d=======",current_to_fill);
	for(int i = 0;i < current_to_fill;i++){
		printf("%f %f %f\n",vertexes[i].x,vertexes[i].y,vertexes[i].z);
	}*/

	//这里将其写入设备 
	VOID* pVertices;
	if( FAILED( vertex_buffer_->Lock( 0, current_to_fill , ( void** )&pVertices, 0 ) ) ){
		printf("Error in Fill Vertex Buffer\n");		
		return ;
	}
	////memset(pVertices,0,MAX_PARTICLES*sizeof(PARTICLEVERTEX) );
	memcpy( pVertices, vertexes, current_to_fill * sizeof(PARTICLEVERTEX) );
	vertex_buffer_->Unlock();
	//Render it !
	//g_camera.ApplyDevice(g_pd3dDevice);
	g_pd3dDevice->SetStreamSource( 0, vertex_buffer_, 0, sizeof( PARTICLEVERTEX ) );
	g_pd3dDevice->SetTexture(0,c_tex.tex);
	g_pd3dDevice->SetFVF( D3DFVF_PARTICLEVERTEX );
	g_pd3dDevice->SetIndices(index_buffer_);
	g_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,0,0,current_to_fill,0,current_to_fill/2);		

	//g_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,0,0,current_to_fill,4,current_to_fill/2);	

	//g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN,0,current_to_fill- 1);
	//printf("%d \n",counts_to_fill);
	/*printf("=======\n");
	for(int i = 0;i < counts_to_fill;i++){ 
		printf("%f %f %f\n",vertexes[i].x,vertexes[i].y,vertexes[i].z);
	}
	printf("=======\n\n");*/
}


void RibbonEmitter::init(MPQFile &f, ModelRibbonEmitterDef &mta, uint32 *globals)
{
	color.init(mta.color, f, globals);
	opacity.init(mta.opacity, f, globals);
	above.init(mta.above, f, globals);
	below.init(mta.below, f, globals);

	parent = model->bones + mta.bone;
	int *texlist = (int*)(f.getBuffer() + mta.ofsTextures);
	// just use the first texture for now; most models I've checked only had one
	texture = model->textures_[texlist[0]];

	tpos = pos = fixCoordSystem(mta.pos);

	// TODO: figure out actual correct way to calculate length
	// in BFD, res is 60 and len is 0.6, the trails are very short (too long here)
	// in CoT, res and len are like 10 but the trails are supposed to be much longer (too short here)
	numsegs = (int)mta.res;
	seglen = mta.length;
	length = mta.res * seglen;

	// create first segment
	RibbonSegment rs;
	rs.pos = tpos;
	rs.len = 0;
	segs.push_back(rs);
}

void RibbonEmitter::setup(size_t anim, size_t time)
{
	Vec3D ntpos = parent->mat * pos;
	Vec3D ntup = parent->mat * (pos + Vec3D(0,0,1));
	ntup -= ntpos;
	ntup.normalize();
	float dlen = (ntpos-tpos).length();

	manim = anim;
	mtime = time;

	// move first segment
	RibbonSegment &first = *segs.begin();
	if (first.len > seglen) {
		// add new segment
		first.back = (tpos-ntpos).normalize();
		first.len0 = first.len;
		RibbonSegment newseg;
		newseg.pos = ntpos;
		newseg.up = ntup;
		newseg.len = dlen;
		segs.push_front(newseg);
	} else {
		first.up = ntup;
		first.pos = ntpos;
		first.len += dlen;
	}

	// kill stuff from the end
	float l = 0;
	bool erasemode = false;
	for (std::list<RibbonSegment>::iterator it = segs.begin(); it != segs.end(); ) {
		if (!erasemode) {
			l += it->len;
			if (l > length) {
				it->len = l - length;
				erasemode = true;
			}
			++it;
		} else {
			segs.erase(it++);
		}
	}

	tpos = ntpos;
	tcolor = Vec4D(color.getValue(anim, time), opacity.getValue(anim, time));

	tabove = above.getValue(anim, time);
	tbelow = below.getValue(anim, time);
}

//Generates the rotation matrix based on spread
static Matrix	SpreadMat;
void CalcSpreadMatrix(float Spread1,float Spread2, float w, float l)
{
	int i,j;
	float a[2],c[2],s[2];
	Matrix	Temp;

	SpreadMat.unit();

	a[0]=randfloat(-Spread1,Spread1)/2.0f;
	a[1]=randfloat(-Spread2,Spread2)/2.0f;

	/*SpreadMat.m[0][0]*=l;
	SpreadMat.m[1][1]*=l;
	SpreadMat.m[2][2]*=w;*/

	for(i=0;i<2;i++)
	{		
		c[i]=cos(a[i]);
		s[i]=sin(a[i]);
	}
	Temp.unit();
	Temp.m[1][1]=c[0];
	Temp.m[2][1]=s[0];
	Temp.m[2][2]=c[0];
	Temp.m[1][2]=-s[0];

	SpreadMat=SpreadMat*Temp;

	Temp.unit();
	Temp.m[0][0]=c[1];
	Temp.m[1][0]=s[1];
	Temp.m[1][1]=c[1];
	Temp.m[0][1]=-s[1];

	SpreadMat=SpreadMat*Temp;

	float Size=abs(c[0])*l+abs(s[0])*w;
	for(i=0;i<3;i++)
		for(j=0;j<3;j++)
			SpreadMat.m[i][j]*=Size;
}


Particle PlaneParticleEmitter::newParticle(size_t anim, size_t time, float w, float l, float spd, float var, float spr, float spr2)
{
	// Model Flags - *shrug* gotta write this down somewhere.
	// 0x1 =
	// 0x2 =
	// 0x4 =
	// 0x8 = 
	// 0x10 = 
	// 19 = 0x13 = blue ball in thunderfury = should be billboarded?

	// Particle Flags
	// 0x0	/ 0		= Basilisk has no flags?
	// 0x1	/ 1		= Pretty much everything I know of except Basilisks have this flag..  Billboard?
	// 0x2	/ 2		=
	// 0x4	/ 4		=
	// 0x8  / 8		= 
	// 0x10	/ 16	= Position Relative to bone pivot?
	// 0x20	/ 32	=
	// 0x40	/ 64	=
	// 0x80 / 128	=
	// 0x100 / 256	=
	// 0x200 / 512	=
	// 0x400 / 1024 =
	// 0x800 / 2048 =
	// 0x1000/ 4096 =
	// 0x0000/ 1593 = [1,8,16,32,512,1024]"Warp Storm" - aura type particle effect
	// 0x419 / 1049 = [1,8,16,1024] Forest Wind shoulders
	// 0x411 / 1041 = [1,16,1024] Halo
	// 0x000 / 541	= [1,4,8,16,512] Staff glow
	// 0x000 / 537 = "Warp Storm"
	// 0x31 / 49 = [1,16,32] particle moving up?
	// 0x00 / 41 = [1,8,32] Blood elf broom, dust spread out on the ground (X, Z axis)
	// 0x1D / 29 = [1,4,8,16] particle being static
	// 0x19 / 25 = [1,8,16] flame on weapon - move up/along the weapon
	// 17 = 0x11 = [1,16] glow on weapon - static, random direction.  - Aurastone Hammer
	// 1 = 0x1 = perdition blade
	// 4121 = water ele
	// 4097 = water elemental
	// 1041 = Transcendance Halo
	// 1039 = water ele

	Particle p;

	//Spread Calculation
	Matrix mrot;

	CalcSpreadMatrix(spr,spr,1.0f,1.0f);
	mrot=sys->parent->mrot*SpreadMat;

	if (sys->flags == 1041) { // Trans Halo
		p.pos = sys->parent->mat * (sys->pos + Vec3D(randfloat(-l,l), 0, randfloat(-w,w)));

		const float t = randfloat(0.0f, float(2*PI));

		p.pos = Vec3D(0.0f, sys->pos.y + 0.15f, sys->pos.z) + Vec3D(cos(t)/8, 0.0f, sin(t)/8); // Need to manually correct for the halo - why?

		// var isn't being used, which is set to 1.0f,  whats the importance of this?
		// why does this set of values differ from other particles

		Vec3D dir(0.0f, 1.0f, 0.0f);
		p.dir = dir;

		p.speed = dir.normalize() * spd * randfloat(0, var);
	} else if (sys->flags == 25 && sys->parent->parent<1) { // Weapon Flame
		p.pos = sys->parent->pivot * (sys->pos + Vec3D(randfloat(-l,l), randfloat(-l,l), randfloat(-w,w)));
		Vec3D dir = mrot * Vec3D(0.0f, 1.0f, 0.0f);
		p.dir = dir.normalize();
		//Vec3D dir = sys->model->bones[sys->parent->parent].mrot * sys->parent->mrot * Vec3D(0.0f, 1.0f, 0.0f);
		//p.speed = dir.normalize() * spd;

	} else if (sys->flags == 25 && sys->parent->parent > 0) { // Weapon with built-in Flame (Avenger lightsaber!)
		p.pos = sys->parent->mat * (sys->pos + Vec3D(randfloat(-l,l), randfloat(-l,l), randfloat(-w,w)));
		Vec3D dir = Vec3D(sys->parent->mat.m[1][0],sys->parent->mat.m[1][1], sys->parent->mat.m[1][2]) * Vec3D(0.0f, 1.0f, 0.0f);
		p.speed = dir.normalize() * spd * randfloat(0, var*2);

	} else if (sys->flags == 17 && sys->parent->parent<1) { // Weapon Glow
		p.pos = sys->parent->pivot * (sys->pos + Vec3D(randfloat(-l,l), randfloat(-l,l), randfloat(-w,w)));
		Vec3D dir = mrot * Vec3D(0,1,0);
		p.dir = dir.normalize();

	} else {
		p.pos = sys->pos + Vec3D(randfloat(-l,l),0, randfloat(-w,w));
		p.pos = sys->parent->mat * p.pos;

		//Vec3D dir = mrot * Vec3D(0,1,0);
		Vec3D dir = sys->parent->mrot * Vec3D(0,1,0);
		//dir = Vec3D(0,1,0);

		p.dir = dir;//.normalize();
		p.down = Vec3D(0,-1.0f,0); // dir * -1.0f;
		p.speed = dir.normalize() * spd * (1.0f+randfloat(-var,var));
		//p.pos = fixCoordSystem(p.pos);
	}

	if(!sys->billboard)	{
		p.corners[0] = mrot * Vec3D(-1,0,+1);
		p.corners[1] = mrot * Vec3D(+1,0,+1);
		p.corners[2] = mrot * Vec3D(+1,0,-1);
		p.corners[3] = mrot * Vec3D(-1,0,-1);
	}

	p.life = 0;
	size_t l_anim = anim;
	if (bZeroParticle)  //我这里让bZeroParticle始终为true
		l_anim = 0;
	p.maxlife = sys->lifespan.getValue(l_anim, time);
	if (p.maxlife == 0)
		p.maxlife = 1;

	p.origin = p.pos;

	p.tile = randint(0, sys->rows*sys->cols-1);
	return p;
}

Particle SphereParticleEmitter::newParticle(size_t anim, size_t time, float w, float l, float spd, float var, float spr, float spr2)
{
	Particle p;
	Vec3D dir;
	float radius;

	radius = randfloat(0,1);

	// Old method
	//float t = randfloat(0,2*PI);

	// New
	// Spread should never be zero for sphere particles ?
	float t = 0;
	if (spr == 0)
		t = randfloat((float)-PI,(float)PI);
	else
		t = randfloat(-spr,spr);

	//Spread Calculation
	Matrix mrot;

	CalcSpreadMatrix(spr*2,spr2*2,w,l);
	mrot=sys->parent->mrot*SpreadMat;

	// New
	// Length should never technically be zero ?
	//if (l==0)
	//	l = w;

	// New method
	// Vec3D bdir(w*cosf(t), 0.0f, l*sinf(t));
	// --

	// TODO: fix shpere emitters to work properly
	/* // Old Method
	//Vec3D bdir(l*cosf(t), 0, w*sinf(t));
	//Vec3D bdir(0, w*cosf(t), l*sinf(t));


	float theta_range = sys->spread.getValue(anim, time);
	float theta = -0.5f* theta_range + randfloat(0, theta_range);
	Vec3D bdir(0, l*cosf(theta), w*sinf(theta));

	float phi_range = sys->lat.getValue(anim, time);
	float phi = randfloat(0, phi_range);
	rotate(0,0, &bdir.z, &bdir.x, phi);
	*/

	if (sys->flags == 57 || sys->flags == 313) { // Faith Halo
		Vec3D bdir(w*cosf(t)*1.6, 0.0f, l*sinf(t)*1.6);

		p.pos = sys->pos + bdir;
		p.pos = sys->parent->mat * p.pos;

		if (bdir.lengthSquared()==0) 
			p.speed = Vec3D(0,0,0);
		else {
			dir = sys->parent->mrot * (bdir.normalize());//mrot * Vec3D(0, 1.0f,0);
			p.speed = dir.normalize() * spd * (1.0f+randfloat(-var,var));   // ?
		}

	} else {
		Vec3D bdir;
		float temp;

		bdir = mrot * Vec3D(0,1,0) * radius;
		temp = bdir.z;
		bdir.z = bdir.y;
		bdir.y = temp;

		p.pos = sys->parent->mat * sys->pos + bdir;


		//p.pos = sys->pos + bdir;
		//p.pos = sys->parent->mat * p.pos;


		if ((bdir.lengthSquared()==0) && ((sys->flags&0x100)!=0x100))
		{
			p.speed = Vec3D(0,0,0);
			dir = sys->parent->mrot * Vec3D(0,1,0);
		}
		else {
			if(sys->flags&0x100)
				dir = sys->parent->mrot * Vec3D(0,1,0);
			else
				dir = bdir.normalize();

			p.speed = dir.normalize() * spd * (1.0f+randfloat(-var,var));   // ?
		}
	}

	p.dir =  dir.normalize();//mrot * Vec3D(0, 1.0f,0);
	p.down = Vec3D(0,-1.0f,0);

	p.life = 0;
	size_t l_anim = anim;
	if (bZeroParticle)   //我这里让bZeroParticle始终为true
		l_anim = 0;
	p.maxlife = sys->lifespan.getValue(l_anim, time);
	if (p.maxlife == 0)
		p.maxlife = 1;

	p.origin = p.pos;

	p.tile = randint(0, sys->rows*sys->cols-1);
	return p;
}

