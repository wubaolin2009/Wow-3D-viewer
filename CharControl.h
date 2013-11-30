//1月27日 加入 主要控制人物或模型的 旋转 移动 动画等
#ifndef _CHAR_CONTROL_H_
#define _CHAR_CONTROL_H_
#include <d3d9.h>
#include "enums.h"
#include "vector3d.h"
class CharacterModel;
//这个是简单的Animate类  暂时用于实现位移和旋转
class MyAnimate
{
public:
	//默认构造函数
	MyAnimate(){
		is_anim_end_ = false; d_ = 0.1f;
	}
	//Speed 为 每秒多少单位  d是误差 +- 0.1f 默认
	MyAnimate(const Vec3D& start,const Vec3D& end,const Vec3D& speed,FLOAT d = 0.1f);
	//dt ms单位 如果 已经到了end 则 返回end的状态 dt ms为单位
	void Update(int dt);
	//重新设置参数
	void ResetParam(const Vec3D& art,const Vec3D& end,const Vec3D& speed,FLOAT d = 0.1f);
	//得到当前的状态
	Vec3D GetCurrent();
	//动画是否结束
	bool IsAnimEnd();
private:
	Vec3D start_;
	Vec3D end_;
	Vec3D current_;
	bool is_anim_end_;
	Vec3D speed_;
	FLOAT d_;
};
class CharControl
{
public:
	//當前人物的狀態 這個會慢慢豐富起來
	enum CHAR_STATUS{
		CHAR_STATUS_STAND,  //原地不動
		CHAR_STATUS_RUN,  //正在移動
		CHAR_STATUS_ATTACK, //正在移动
		CHAR_STATUS_CAST, //正在施法
	};
public:
	//1月28日加入了 初始位置和初始偏移
	CharControl(CharacterModel& model,const Vec3D& position,const Vec3D& rotation):model_(model) {
		position_ = position;
		rot_ = rotation;
		current_anim_id_ = 19; //Run
		current_anim_frame_ = 0;
		time_last_ = -1;
		current_status_ = CHAR_STATUS_STAND;
		current_cast_ = NULL;
		current_spell_frame_ = 0;
	}
	//返回這個屏幕上的點 是否會選中此目標
	//distance 為視角到這個模型的距離 
	//cord world 為 最近的那點的世界坐標 distance和cord_world都可以為空
	bool IsIntersect(int x,int y,FLOAT* distance,Vec3D* cord_world);
	void Update();  //更新 Char的各種信息
	void Draw(LPDIRECT3DDEVICE9 d3d_device);  //畫出模型和各種例子效果

	//移動到xyz 這裡先簡單的進行動畫
	void MoveTo(FLOAT x,FLOAT y,FLOAT z);
	//这个是Test用的 Attack 一下
	void Attack();
	//同上
	void Cast(CharacterModel* current_cast);
private:
	CharacterModel& model_;
	int current_anim_id_; //當前的動畫號 如果沒有動畫 則設為-1
	int current_anim_frame_; //當前動畫的幀數

	//动画信息
	MyAnimate pos_anim_;
	MyAnimate rot_anim_;

	//位置和角度信息
	Vec3D position_;
	Vec3D rot_;
	//上次移動動畫的起點
	Vec3D last_position_;
	//本次移動的目的地
	Vec3D target_position_;

	//上次動畫播放的時間
	int time_last_;

	//當前人物所處的狀態
	CHAR_STATUS current_status_;
	//当前人物释放的法术
	CharacterModel* current_cast_;
	//当前法术的帧数
	int current_spell_frame_;
};

#endif