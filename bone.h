#ifndef _BONE_H_
#define _BONE_H_
#include "mpq.h"
#include "vector3d.h"
#include "enums.h"
#include "animated.h"
#include "raw_model.h"
#include "matrix.h"

#define	MODELBONE_BILLBOARD	8
#define	MODELBONE_TRANSFORM	512
// block E - bones
struct ModelBoneDef {
	int32 keyboneid; // Back-reference to the key bone lookup table. -1 if this is no key bone.
	int32 flags; // Only known flags: 8 - billboarded and 512 - transformed
	int16 parent; // parent bone index
	int16 geoid; // A geoset for this bone.
	int32 unknown; // new int added to the bone definitions.  Added in WoW 2.0
	AnimationBlock translation; // (Vec3D)
	AnimationBlock rotation; // (QuatS)
	AnimationBlock scaling; // (Vec3D)
	Vec3D pivot;
};


//骨骼信息 
//骨骼 Bones
class CharacterModel;
class Bone {
public:
	Animated<Vec3D> trans;
	//Animated<Quaternion> rot;
	Animated<Quaternion, PACK_QUATERNION, Quat16ToQuat32> rot;
	Animated<Vec3D> scale;

	Vec3D pivot, transPivot;
	int16 parent;

	bool billboard;
	//這個應該是骨骼的當前位移和旋轉變換矩陣
	Matrix mat;  //這個應該是頂點的位移 和旋轉矩陣乘好了的？
	Matrix mrot; //這個應該是法向量的變換矩陣

	ModelBoneDef boneDef;

	bool calc;
	CharacterModel *model;
	void calcMatrix(Bone* allbones,ssize_t anim, size_t time, bool rotate=true);
	void initV3(MPQFile &f, ModelBoneDef &b, uint32 *global, MPQFile *animfiles);
	///void initV2(MPQFile &f, ModelBoneDef &b, uint32 *global);  //應該是廢棄的
};

enum KeyBoneTable { // wxString Bone_Names[]
	//Block F - Key Bone lookup table.
	//---------------------------------
	BONE_LARM = 0,		// 0, ArmL: Left upper arm
	BONE_RARM,			// 1, ArmR: Right upper arm
	BONE_LSHOULDER,		// 2, ShoulderL: Left Shoulder / deltoid area
	BONE_RSHOULDER,		// 3, ShoulderR: Right Shoulder / deltoid area
	BONE_STOMACH,		// 4, SpineLow: (upper?) abdomen
	BONE_WAIST,			// 5, Waist: (lower abdomen?) waist
	BONE_HEAD,			// 6, Head
	BONE_JAW,			// 7, Jaw: jaw/mouth
	BONE_RFINGER1,		// 8, IndexFingerR: (Trolls have 3 "fingers", this points to the 2nd one.
	BONE_RFINGER2,		// 9, MiddleFingerR: center finger - only used by dwarfs.. don't know why
	BONE_RFINGER3,		// 10, PinkyFingerR: (Trolls have 3 "fingers", this points to the 3rd one.
	BONE_RFINGERS,		// 11, RingFingerR: Right fingers -- this is -1 for trolls, they have no fingers, only the 3 thumb like thingys
	BONE_RTHUMB,		// 12, ThumbR: Right Thumb
	BONE_LFINGER1,		// 13, IndexFingerL: (Trolls have 3 "fingers", this points to the 2nd one.
	BONE_LFINGER2,		// 14, MiddleFingerL: Center finger - only used by dwarfs.
	BONE_LFINGER3,		// 15, PinkyFingerL: (Trolls have 3 "fingers", this points to the 3rd one.
	BONE_LFINGERS,		// 16, RingFingerL: Left fingers
	BONE_LTHUMB,		// 17, ThubbL: Left Thumb
	BONE_BTH,			// 18, $BTH: In front of head
	BONE_CSR,			// 19, $CSR: Left hand
	BONE_CSL,			// 20, $CSL: Left hand
	BONE_BREATH,		// 21, _Breath
	BONE_NAME,			// 22, _Name
	BONE_NAMEMOUNT,		// 23, _NameMount
	BONE_CHD,			// 24, $CHD: Head
	BONE_CCH,			// 25, $CCH: Bust
	BONE_ROOT,			// 26, Root: The "Root" bone,  this controls rotations, transformations, etc of the whole model and all subsequent bones.
	BONE_WHEEL1,		// 27, Wheel1
	BONE_WHEEL2,		// 28, Wheel2
	BONE_WHEEL3,		// 29, Wheel3
	BONE_WHEEL4,		// 30, Wheel4
	BONE_WHEEL5,		// 31, Wheel5
	BONE_WHEEL6,		// 32, Wheel6
	BONE_WHEEL7,		// 33, Wheel7
	BONE_WHEEL8,		// 34, Wheel8
	BONE_MAX
};


#endif