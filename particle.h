#ifndef _PARTICLE_H_
#define _PARTICLE_H_

#include <list>
#include "enums.h"
#include "raw_particle.h"
//粒子系统 1月23日
class ParticleSystem;
class RibbonEmitter;

//最多支持MAX_PARTICLES个粒子
#define MAX_PARTICLES 10000

struct Particle {
	Vec3D pos, speed, down, origin, dir;
	Vec3D	corners[4];
	//Vec3D tpos;
	float size, life, maxlife;
	size_t tile;
	Vec4D color;
};

typedef std::list<Particle> ParticleList;

class ParticleEmitter {
protected:
	ParticleSystem *sys;
public:
	ParticleEmitter(ParticleSystem *sys): sys(sys) {}
	virtual Particle newParticle(size_t anim, size_t time, float w, float l, float spd, float var, float spr, float spr2) = 0;
};

class PlaneParticleEmitter: public ParticleEmitter {
public:
	PlaneParticleEmitter(ParticleSystem *sys): ParticleEmitter(sys) {}
	Particle newParticle(size_t anim, size_t time, float w, float l, float spd, float var, float spr, float spr2);
};

class SphereParticleEmitter: public ParticleEmitter {
public:
	SphereParticleEmitter(ParticleSystem *sys): ParticleEmitter(sys) {}
	Particle newParticle(size_t anim, size_t time, float w, float l, float spd, float var, float spr, float spr2);
};

struct TexCoordSet {
	Vec2D tc[4];
};
class Bone;
class CharacterModel;
class ParticleSystem {
	float mid, slowdown, rotation;
	Vec3D pos;
	GLuint texture;
	ParticleEmitter *emitter;
	ParticleList particles;
	int blend, order, ParticleType;
	size_t manim, mtime;
	int rows, cols;
	std::vector<TexCoordSet> tiles;
	void initTile(Vec2D *tc, int num);
	bool billboard;

	float rem;
	//bool transform;

	// unknown parameters omitted for now ...
	int32 flags;
	int16 EmitterType;

	Bone *parent;
	//这里加入和D3D渲染相关的 顶点缓冲  在init中初始化
	LPDIRECT3DVERTEXBUFFER9 vertex_buffer_;
	//这个是我后来加入的Indexbuffer 模拟GL_QUAD
	LPDIRECT3DINDEXBUFFER9  index_buffer_;

public:
	CharacterModel *model;
	float tofs;

	Animated<uint16> enabled;
	Animated<float> speed, variation, spread, lat, gravity, lifespan, rate, areal, areaw, deacceleration;
	Vec4D colors[3];
	float sizes[3];

	ParticleSystem(): mid(0), emitter(0), rem(0)
	{
		blend = 0;
		order = 0;
		ParticleType = 0;
		manim = 0;
		mtime = 0;
		rows = 0;
		cols = 0;

		model = 0;
		parent = 0;
		texture = 0;

		slowdown = 0;
		rotation = 0;
		tofs = 0;
	}
	~ParticleSystem() { delete emitter; }

	void init(MPQFile &f, ModelParticleEmitterDef &mta, uint32 *globals);
	void update(float dt);

	void setup(size_t anim, size_t time);
	void draw();

	friend class PlaneParticleEmitter;
	friend class SphereParticleEmitter;
};

struct RibbonSegment {
	Vec3D pos, up, back;
	float len,len0;
};
class CharacterModel;

class RibbonEmitter {
	Animated<Vec3D> color;
	AnimatedShort opacity;
	Animated<float> above, below;

	Bone *parent;
	float f1, f2;

	Vec3D pos;

	size_t manim, mtime;
	float length, seglen;
	int numsegs;

	Vec3D tpos;
	Vec4D tcolor;
	float tabove, tbelow;

	GLuint texture;

	std::list<RibbonSegment> segs;

public:
	CharacterModel *model;

	void init(MPQFile &f, ModelRibbonEmitterDef &mta, uint32 *globals);
	void setup(size_t anim, size_t time);
	void draw();
};




#endif