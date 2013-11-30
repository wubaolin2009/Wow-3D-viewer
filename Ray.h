#ifndef _RAY_H_
#define _RAY_H_
#include <d3d9.h>
#include <d3dx9math.h>
#include "vector3d.h"
#include "enums.h"
//先放在这里 以后铲除掉
#define D3DFVF_3VERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE )
struct V3VERTEX
{
	FLOAT x, y, z;      // The untransformed, 3D position for the vertex
	DWORD color;        // The vertex color
};

//1月25日 简单的射线检测
class RayDetect
{
public:
	//x y为窗口中 “鼠标”的x y坐标 是相对于窗口的坐标
	RayDetect(int x,int y):x_(x),y_(y) {}
	//检测是否和某个模型相交 返回true 并置distance的值 如果distance部位空
	//如果不相交 返回false distance不会进行修改
	//vetexes为顶点 indexes为其索引 每3个是一个三角形
	//索引是 123 2 3 4 则有2个三角形 这个是为了和Model的BoundingVolumn对应
	//1月29日 coords_world 的加入 可以為空 為交點的世界坐標
	// Vec3D char_potion Vec3D char_rotation  為 當前應用於人物的坐標變換和旋轉 
	// 以此來得到世界變換矩陣 而不是通過d3ddevice 這個對於檢測不在原點的人物 暫時是唯一的辦法
	// If char_pos and char_rot. either is NULL ,We'll use The D3dDevice as the way to get
	// the world matrix
	bool Intersect(Vec3D* vetexes,int num_vetexes,uint16* indexes,int num_indexes,FLOAT* distance,Vec3D* coords_world,Vec3D* char_pos ,Vec3D* char_rot);
	//一个重载版本 用于test  p_out 为交点
	bool Intersect(V3VERTEX* vetexes,int num_vetexes,FLOAT* distance,D3DXVECTOR3* p_out);


private:
	//根据x y的值 反算出一条射线 这个射线的坐标被转换到世界坐标中 下边的分别是应用于
	//场景的世界变换 视角变换 投影变换矩阵
	//1月29日改動 如果world_matrix Not NULL 則是用詞作為world matrix 否則用d3d device的
	void Dir(D3DXMATRIX* new_world_matrix);
	int x_;
	int y_;
	D3DXVECTOR3 ray_position_; //射线起点
	D3DXVECTOR3 ray_dir_;  //射线方向
};
#endif