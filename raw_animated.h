#ifndef _RAW_ANIMATED_H_
#define _RAW_ANIMATED_H_
//这个文件是 MPQ中的一些动画信息的头
struct FakeAnimationBlock {
	uint32 nTimes;
	uint32 ofsTimes;
	uint32 nKeys;
	uint32 ofsKeys;
};

struct AnimationBlockHeader
{
	uint32 nEntrys;
	uint32 ofsEntrys;
};

#define	ANIMATION_HANDSCLOSED	15
#define	ANIMATION_MOUNT			91
#define	ANIMATION_LOOPED		0x20 // flags
// block B - animations, size 68 bytes, WotLK 64 bytes
struct ModelAnimation {
	uint32 animID; // AnimationDataDB.ID
	uint32 timeStart;
	uint32 timeEnd;

	float moveSpeed;

	uint32 flags;
	uint16 probability;
	uint16 unused;
	uint32 d1;
	uint32 d2;
	uint32 playSpeed;  // note: this can't be play speed because it's 0 for some models

	Sphere boundSphere;

	int16 NextAnimation;
	int16 Index;
};

struct ModelAnimationWotLK {
	int16 animID; // AnimationDataDB.ID
	int16 subAnimID;
	uint32 length;

	float moveSpeed;

	uint32 flags;
	uint16 probability; // This is used to determine how often the animation is played. For all animations of the same type, this adds up to 0x7FFF (32767).
	uint16 unused;
	uint32 d1;
	uint32 d2;
	uint32 playSpeed;  // note: this can't be play speed because it's 0 for some models

	Sphere boundSphere;

	int16 NextAnimation;
	int16 Index;
};

// In WoW 2.0+ Blizzard are now storing rotation data in 16bit values instead of 32bit.
// I don't really understand why as its only a very minor saving in model sizes and adds extra overhead in
// processing the models.  Need this structure to read the data into.
struct PACK_QUATERNION {  
	int16 x,y,z,w;  
}; 
class Quat16ToQuat32 {
public:
	static const Quaternion conv(const PACK_QUATERNION t)
	{
		return Quaternion(
			float(t.x < 0? t.x + 32768 : t.x - 32767)/ 32767.0f, 
			float(t.y < 0? t.y + 32768 : t.y - 32767)/ 32767.0f,
			float(t.z < 0? t.z + 32768 : t.z - 32767)/ 32767.0f,
			float(t.w < 0? t.w + 32768 : t.w - 32767)/ 32767.0f);
	}
};

//Texanim 1月26日加入
struct ModelTexAnimDef {
	AnimationBlock trans; // (Vec3D)
	AnimationBlock rot; // (QuatS)
	AnimationBlock scale; // (Vec3D)
};

#endif