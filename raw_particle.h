#ifndef _RAW_PARTICLE_H_
#define _RAW_PARTICLE_H_
//粒子文件的文件头 信息
#include "animated.h"
#include "enums.h"

struct ModelParticleParams {
	FakeAnimationBlock colors; 	// (Vec3D)	This one points to 3 floats defining red, green and blue.
	FakeAnimationBlock opacity;      // (UInt16)		Looks like opacity (short), Most likely they all have 3 timestamps for {start, middle, end}.
	FakeAnimationBlock sizes; 		// (Vec2D)	It carries two floats per key. (x and y scale)
	int32 d[2];
	FakeAnimationBlock Intensity; 	// (UInt16) Some kind of intensity values seen: 0,16,17,32(if set to different it will have high intensity) 
	FakeAnimationBlock unk2; 		// (UInt16)
	float unk[3];
	Vec3D scales;
	float slowdown;
	float unknown1[2];
	float rotation;				//Sprite Rotation
	float unknown2[2];
	Vec3D Rot1;					//Model Rotation 1
	Vec3D Rot2;					//Model Rotation 2
	Vec3D Trans;				//Model Translation
	float f2[4];
	int32 nUnknownReference;
	int32 ofsUnknownReferenc;
};

#define	MODELPARTICLE_FLAGS_DONOTTRAIL		0x10
#define	MODELPARTICLE_FLAGS_DONOTBILLBOARD	0x1000
#define	MODELPARTICLE_EMITTER_PLANE			1
#define	MODELPARTICLE_EMITTER_SPHERE		2
#define	MODELPARTICLE_EMITTER_SPLINE		3
struct ModelParticleEmitterDef {
	int32 id;
	int32 flags; // MODELPARTICLE_FLAGS_*
	Vec3D pos; // The position. Relative to the following bone.
	int16 bone; // The bone its attached to.
	int16 texture; // And the texture that is used.
	int32 nModelFileName;
	int32 ofsModelFileName;
	int32 nParticleFileName;
	int32 ofsParticleFileName; // TODO
	int8 blend;
	int8 EmitterType; // EmitterType	 1 - Plane (rectangle), 2 - Sphere, 3 - Spline? (can't be bothered to find one)
	int16 ParticleColor; // This one is used so you can assign a color to specific particles. They loop over all 
	// particles and compare +0x2A to 11, 12 and 13. If that matches, the colors from the dbc get applied.
	int8 ParticleType; // 0 "normal" particle, 
	// 1 large quad from the particle's origin to its position (used in Moonwell water effects)
	// 2 seems to be the same as 0 (found some in the Deeprun Tram blinky-lights-sign thing)
	int8 HeaderTail; // 0 - Head, 1 - Tail, 2 - Both
	int16 TextureTileRotation; // TODO, Rotation for the texture tile. (Values: -1,0,1)
	int16 cols; // How many different frames are on that texture? People should learn what rows and cols are.
	int16 rows; // (2, 2) means slice texture to 2*2 pieces
	AnimationBlock EmissionSpeed; // (Float) All of the following blocks should be floats.
	AnimationBlock SpeedVariation; // (Float) Variation in the flying-speed. (range: 0 to 1)
	AnimationBlock VerticalRange; // (Float) Drifting away vertically. (range: 0 to pi)
	AnimationBlock HorizontalRange; // (Float) They can do it horizontally too! (range: 0 to 2*pi)
	AnimationBlock Gravity; // (Float) Fall down, apple!
	AnimationBlock Lifespan; // (Float) Everyone has to die.
	int32 unknown;
	AnimationBlock EmissionRate; // (Float) Stread your particles, emitter.
	int32 unknown2;
	AnimationBlock EmissionAreaLength; // (Float) Well, you can do that in this area.
	AnimationBlock EmissionAreaWidth; // (Float) 
	AnimationBlock Gravity2; // (Float) A second gravity? Its strong.
	ModelParticleParams p;
	AnimationBlock en; // (UInt16)
};

//这应该是1.0 版本时候的Particle发射器的头信息 我这里不知道要不要引用 这个在Character::InitParticle()中被调用
struct ModelParticleEmitterDefV10 {
	int32 id;
	int32 flags;
	Vec3D pos; // The position. Relative to the following bone.
	int16 bone; // The bone its attached to.
	int16 texture; // And the texture that is used.
	int32 nModelFileName;
	int32 ofsModelFileName;
	int32 nParticleFileName;
	int32 ofsParticleFileName; // TODO
	int8 blend;
	int8 EmitterType; // EmitterType	 1 - Plane (rectangle), 2 - Sphere, 3 - Spline? (can't be bothered to find one)
	int16 ParticleColor; // This one is used so you can assign a color to specific particles. They loop over all 
	// particles and compare +0x2A to 11, 12 and 13. If that matches, the colors from the dbc get applied.
	int8 ParticleType; // 0 "normal" particle, 
	// 1 large quad from the particle's origin to its position (used in Moonwell water effects)
	// 2 seems to be the same as 0 (found some in the Deeprun Tram blinky-lights-sign thing)
	int8 HeaderTail; // 0 - Head, 1 - Tail, 2 - Both
	int16 TextureTileRotation; // TODO, Rotation for the texture tile. (Values: -1,0,1)
	int16 cols; // How many different frames are on that texture? People should learn what rows and cols are.
	int16 rows; // (2, 2) means slice texture to 2*2 pieces
	AnimationBlock EmissionSpeed; // (Float) All of the following blocks should be floats.
	AnimationBlock SpeedVariation; // (Float) Variation in the flying-speed. (range: 0 to 1)
	AnimationBlock VerticalRange; // (Float) Drifting away vertically. (range: 0 to pi)
	AnimationBlock HorizontalRange; // (Float) They can do it horizontally too! (range: 0 to 2*pi)
	AnimationBlock Gravity; // (Float) Fall down, apple!
	AnimationBlock Lifespan; // (Float) Everyone has to die.
	int32 unknown;
	AnimationBlock EmissionRate; // (Float) Stread your particles, emitter.
	int32 unknown2;
	AnimationBlock EmissionAreaLength; // (Float) Well, you can do that in this area.
	AnimationBlock EmissionAreaWidth; // (Float) 
	AnimationBlock Gravity2; // (Float) A second gravity? Its strong.
	ModelParticleParams p;
	AnimationBlock en; // (UInt16), seems unused in cataclysm
	int32 unknown3; // 12319, cataclysm
	int32 unknown4; // 12319, cataclysm
	int32 unknown5; // 12319, cataclysm
	int32 unknown6; // 12319, cataclysm
};


struct ModelRibbonEmitterDef {
	int32 id;
	int32 bone;
	Vec3D pos;
	int32 nTextures;
	int32 ofsTextures;
	int32 nUnknown;
	int32 ofsUnknown;
	AnimationBlock color; // (Vec3D)
	AnimationBlock opacity; // (UInt16) And an alpha value in a short, where: 0 - transparent, 0x7FFF - opaque.
	AnimationBlock above; // (Float) The height above.
	AnimationBlock below; // (Float) The height below. Do not set these to the same!
	float res; // This defines how smooth the ribbon is. A low value may produce a lot of edges.
	float length; // The length aka Lifespan.
	float Emissionangle; // use arcsin(val) to get the angle in degree
	int16 s1, s2;
	AnimationBlock unk1; // (short)
	AnimationBlock unk2; // (boolean)
	int32 unknown; // This looks much like just some Padding to the fill up the 0x10 Bytes, always 0
};

#endif