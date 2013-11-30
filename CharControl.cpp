//1月27日 
#include "CharControl.h"
#include "Character.h"
#include "Ray.h"

const FLOAT walk_speed = 8.0;  //每秒移動的距離
const FLOAT rot_speed = PI;   //转圈的速度 每秒做多转多少弧度


MyAnimate::MyAnimate(const Vec3D& start,const Vec3D& end,const Vec3D& speed,FLOAT d)
{
	ResetParam(start,end,speed,d);
}
//dt ms单位 如果 已经到了end 则 返回end的状态
void MyAnimate::Update(int dt)  //dt is 毫秒
{
	Vec3D dir = end_ - start_;
	dir = dir.normalize();
	if(!is_anim_end_){
		//一种检测是否动画结束的方式 上边的有可能因为一个big step 导致永远停不下来
		bool x_ok = (dir.x > 0)?current_.x >= end_.x:current_.x <= end_.x;
		bool y_ok = (dir.y > 0)?current_.y >= end_.y:current_.y <= end_.y;
		bool z_ok = (dir.z > 0)?current_.z >= end_.x:current_.z <= end_.z;
		
		current_.x += dir.x * speed_.x *dt /1000.0f;
		current_.y += dir.y * speed_.y *dt /1000.0f;
		current_.z += dir.z * speed_.z *dt /1000.0f;
		if( fabs(current_.x - end_.x) < d_ && fabs(current_.y - end_.y) < d_ && fabs(current_.z - end_.z) < d_){
			is_anim_end_ = true;
			current_ = end_;
		}		
		if(x_ok && y_ok && z_ok){
			is_anim_end_ = true;
			current_ = end_;
		}
	}
}

void MyAnimate::ResetParam(const Vec3D& start,const Vec3D& end,const Vec3D& speed,FLOAT d)
{
	start_ = start;
	end_ = end;
	current_ = start_;
	is_anim_end_ = false;
	speed_ = speed;
	d_ = d;

}
//得到当前的状态
Vec3D MyAnimate::GetCurrent()
{
	return current_;
}

bool MyAnimate::IsAnimEnd()
{
	return is_anim_end_;
}

bool CharControl::IsIntersect(int x,int y,FLOAT* distance,Vec3D* cord_world)
{
	RayDetect ray(x,y);
	FLOAT dis = 0;
	bool is_intersect = false;
	//D3DXVECTOR3 vec_point;
	if( model_.InterSect(x,y,&dis,cord_world,&position_,&rot_)){ //相交
		printf("Char Selected (%f %f %f)\n",cord_world->x,cord_world->y,cord_world->z,&position_,&rot_);
		is_intersect = true;
		if(distance)
			*distance = dis;
	}
	return is_intersect;
}


void CharControl::Update()  //更新 Char的各種信息
{
	//last_position_ = position_;
	//更新position
	//找到方向向量
	Vec3D dir = target_position_ - last_position_;
	//這個方向應該移動的位移
	dir = dir.normalize();
	//得到2幀之間的時間 ms單位
	int time_diff = 0.0f;
	if(time_last_ == -1){
		time_last_ = timeGetTime();
	}
	else{
		time_diff = timeGetTime() - time_last_;
		time_last_ += time_diff;
	}
	if(current_status_ == CHAR_STATUS_RUN){
		//更新 position
		pos_anim_.Update(time_diff);
		rot_anim_.Update(time_diff);
		position_ = pos_anim_.GetCurrent();	
		rot_ = rot_anim_.GetCurrent();
	}
	//如果人物已經移動到了target 則更新人物的狀態
	if(pos_anim_.IsAnimEnd() && current_status_ == CHAR_STATUS_RUN){
		current_status_ = CHAR_STATUS_STAND;
		//current_anim_frame_ = 0;
		//current_anim_id_ = 19; //Stand
		last_position_ = position_;
	}	
	//更新人物的动画信息 这里暂时不会改变动画的种类
	current_anim_frame_ += time_diff;
	//防止渲染到不存在的帧
	int max_frame = model_.FrameCount(current_anim_id_);
	if(current_status_ == CHAR_STATUS_STAND && current_anim_frame_ >= max_frame){
		current_anim_id_ = 19;
		current_anim_frame_ = 0;
	}
	else if(current_status_ == CHAR_STATUS_ATTACK && current_anim_frame_ >= max_frame){
		current_anim_id_ = 19;
		current_anim_frame_ = 0;
	}
	else if(current_status_ == CHAR_STATUS_CAST && current_anim_frame_ >= max_frame){
		current_anim_id_ = 19;
		current_anim_frame_ = 0;
	}
	else{
		while( current_anim_frame_ >= max_frame){
			current_anim_frame_ -= max_frame;
		}
	}
	model_.Animate(current_anim_id_,current_anim_frame_);
	//粒子效果的更新
	model_.UpdateParticle((FLOAT)time_diff/1000.0);
	//Spell
	if(current_status_ == CHAR_STATUS_CAST){
		current_spell_frame_ += time_diff;
		if(current_spell_frame_ >= current_cast_->FrameCount(0) ){
			current_cast_ = NULL;
			current_status_ = CHAR_STATUS_STAND;
			current_spell_frame_ = 0;
		}
		else{
			current_cast_->Animate(0,current_spell_frame_);
			current_cast_->UpdateParticle((FLOAT)time_diff/1000.0);
		}
	}
}
void CharControl::Draw(LPDIRECT3DDEVICE9 d3d_device)  //畫出模型和各種例子效果
{
	//应用 变换的世界矩阵
	D3DXMATRIX world;
	D3DXMATRIX rotx1,roty1,rotz1; 
	D3DXMATRIX rot;

	D3DXMatrixRotationX(&rotx1,rot_.x);
	D3DXMatrixRotationY(&roty1,rot_.y);
	D3DXMatrixRotationZ(&rotz1,rot_.z); 
	D3DXMatrixMultiply(&rot,&rotx1,&roty1);
	D3DXMatrixMultiply(&rot,&rot,&rotz1);
	D3DXMatrixTranslation(&world,position_.x,position_.y,position_.z);
	D3DXMatrixMultiply(&world,&rot,&world);
	d3d_device->SetTransform( D3DTS_WORLD, &world );	
	if(current_status_ == CHAR_STATUS_CAST){
		D3DXMATRIX scale,world_spell;
		D3DXMatrixScaling(&scale,0.6,0.6,0.6);
		D3DXMatrixMultiply(&world_spell,&scale,&world);
		d3d_device->SetTransform( D3DTS_WORLD, &world_spell );		
		current_cast_ ->Draw();
	}
	d3d_device->SetTransform( D3DTS_WORLD, &world );
	model_.Draw();	
	
}

//移動到xyz 這裡先簡單的進行動畫
void CharControl::MoveTo(FLOAT x,FLOAT y,FLOAT z)
{
	Vec3D speed(sqrt((double)walk_speed),0,sqrt((double)walk_speed));
	last_position_ = position_;
	target_position_.x = x;
	target_position_.y = y;
	target_position_.z = z;
	pos_anim_.ResetParam(position_,target_position_,speed);	

	//更新方向向量 最简单的情况应该只包含 绕着Y轴的旋转
	Vec3D dir = target_position_ - last_position_;
	Vec3D old_rot = rot_;
	Vec3D new_rot;
	if(dir.x >= 0.0){
		new_rot.y = -atan(dir.z / dir.x) ;	
	}
	else{
		new_rot.y = PI -atan(dir.z /dir.x) ;	
	}
	//这里重置下 转的角度 使得其角度尽可能小 我现在只管绕着y轴的旋转
	while(new_rot.y - old_rot.y > PI ){
		new_rot.y = (new_rot.y ) - 2*PI;
	}
	while(new_rot.y - old_rot.y < -PI){
		new_rot.y = (new_rot.y ) + 2*PI;
	}
	printf("Rot is %f\n",(new_rot.y - old_rot.y)*180.0/PI);
	//这里我让转身时间固定 不管都少度都是0.5秒完成
	rot_anim_.ResetParam(old_rot,new_rot,Vec3D(0,(new_rot.y - old_rot.y)/0.5,0),0.05);

	current_status_ = CHAR_STATUS_RUN;
	//变成run的动画
	current_anim_frame_ = 0;
	current_anim_id_ = 15; //Run
	if(time_last_ == -1){
		time_last_ = timeGetTime();
	}

}

void CharControl::Attack()
{
	current_anim_frame_ = 0;
	current_anim_id_ = 36; //Attck 2H
	current_status_ = CHAR_STATUS_ATTACK;
}

void CharControl::Cast(CharacterModel* current_cast)
{
	current_cast_ = current_cast;
	current_anim_frame_ = 0;
	current_anim_id_ = 29; //CastOmni
	current_status_ = CHAR_STATUS_CAST;

}