#include "Ray.h"
#include "enums.h"

extern LPDIRECT3DDEVICE9 g_pd3dDevice;

void RayDetect::Dir(D3DXMATRIX* new_world_matrix)
{
	Vec3D ret_vec;  //要返回的Vector
	//首先映射到投影空间中
	Vec3D proj_pt;

	proj_pt.x = (x_ - (float)WINDOW_WIDTH/2.0)/(float)WINDOW_WIDTH*2.0;
	proj_pt.y = -(y_ - (float)WINDOW_HEIGHT/2.0)/(float)WINDOW_HEIGHT*2.0;
	proj_pt.z = 0;

	//映射到观察空间中 
	Vec3D view_pt;
	//得到当前的投影矩阵
	D3DXMATRIX current_pro_matrix;
	g_pd3dDevice->GetTransform(D3DTS_PROJECTION,&current_pro_matrix);
	/*FLOAT Zn = -(current_pro_matrix.m[4][3])/current_pro_matrix.m[3][3];
	view_pt.x = proj_pt.x * Zn / current_pro_matrix.m[1][1];
	view_pt.y = proj_pt.y * Zn / current_pro_matrix.m[2][2];
	view_pt.z = Zn;*/
	//这里 我们得到了 未经过世界变换矩阵的射线响亮
	ret_vec.x = (2*x_/(float)WINDOW_WIDTH-1)/current_pro_matrix.m[0][0];
	ret_vec.y = -(2*y_/(float)WINDOW_HEIGHT-1)/current_pro_matrix.m[1][1];
	ret_vec.z = 1;
	//接下来转换Dir到世界坐标空间 并得到观察点在世界坐标系中的坐标
	D3DXMATRIX current_view_matrix;
	D3DXMATRIX current_inverse_view_matrix;
	g_pd3dDevice->GetTransform(D3DTS_VIEW,&current_view_matrix);
	//求view matrix的逆矩阵
	D3DXMatrixInverse(&current_inverse_view_matrix,0,&current_view_matrix);
	//通过ret_vec 算出 在世界坐标中的方向向量
	D3DXMATRIX matrix_dir;
	memset(&matrix_dir,0,sizeof(matrix_dir) );
	matrix_dir.m[0][0] = ret_vec.x; matrix_dir.m[0][1] = ret_vec.y; matrix_dir.m[0][2] = ret_vec.z;
	D3DXMatrixMultiply(&matrix_dir,&matrix_dir,&current_inverse_view_matrix);
	

	//算出在世界坐标中 射线原点的位置
	Vec3D origin_world(
		current_inverse_view_matrix.m[3][0],
		current_inverse_view_matrix.m[3][1],
		current_inverse_view_matrix.m[3][2]
		);
	
	D3DXMATRIX world_matrix;
	if(new_world_matrix){
		world_matrix = *new_world_matrix;
	}
	else{
		g_pd3dDevice->GetTransform(D3DTS_WORLD,&world_matrix);
	}
	
	//D3DXMatrixIdentity(&world_matrix);
	//D3DXMatrixIdentity(&world_matrix);
	D3DXMatrixInverse(&world_matrix,0,&world_matrix);
	//1月29日修正
	matrix_dir.m[0][3] = 1;

	D3DXMatrixMultiply(&matrix_dir,&matrix_dir,&world_matrix);
	ret_vec.x = matrix_dir.m[0][0];
	ret_vec.y = matrix_dir.m[0][1];
	ret_vec.z = matrix_dir.m[0][2];

	//用来运算的originworld
	D3DXMATRIX o_world;
	memset(&o_world,0,sizeof(o_world) );
	o_world.m[0][0] = origin_world.x;
	o_world.m[0][1] = origin_world.y;
	o_world.m[0][2] = origin_world.z;
	D3DXMatrixMultiply(&o_world,&o_world,&world_matrix);
	

	ray_position_.x  = o_world.m[0][0];
	ray_position_.y  = o_world.m[0][1];
	ray_position_.z  = o_world.m[0][2];

	ray_dir_.x = ret_vec.x;
	ray_dir_.y = ret_vec.y;
	ray_dir_.z = ret_vec.z;

}

bool RayDetect::Intersect(Vec3D* vetexes,int num_vetexes,uint16* indexes,int num_indexes,FLOAT* distance,Vec3D* coords_world,Vec3D* char_pos ,Vec3D* char_rot)
{
	//检查参数
	if(vetexes == NULL || indexes == NULL || num_vetexes <= 0 || num_indexes <= 0 || num_indexes %3 != 0){
		printf("Error Intersect, Params Error!\n");
		return false;
	}
	//更新ray信息
	if(char_pos == NULL || char_rot == NULL){
		Dir(NULL);  //從d3d device中得到世界變換矩陣
	}
	else {
		//算出一個世界變換矩陣
		D3DXMATRIX world_to_use;
		D3DXMATRIX world_rot_x,world_rot_y,world_rot_z,world_trans;
		D3DXMatrixRotationX(&world_rot_x,char_rot->x);
		D3DXMatrixRotationX(&world_rot_y,char_rot->y);
		D3DXMatrixRotationX(&world_rot_z,char_rot->z);
		D3DXMatrixTranslation(&world_trans,char_pos->x,char_pos->y,char_pos->z);
		D3DXMatrixMultiply(&world_to_use,&world_rot_x,&world_rot_y);
		D3DXMatrixMultiply(&world_to_use,&world_to_use,&world_rot_z);
		D3DXMatrixMultiply(&world_to_use,&world_to_use,&world_trans);
		Dir(&world_to_use);
	}

	FLOAT min_distance = 100000.0f; 
	bool is_insected = false;  //不相交
	for(int i = 0;i < num_indexes;i += 3){
		//得到三个顶点
		D3DXVECTOR3  p0,p1,p2;
		p0.x = vetexes[indexes[i] ].x;
		p0.y = vetexes[indexes[i] ].y;
		p0.z = vetexes[indexes[i] ].z;

		p1.x = vetexes[indexes[i+1] ].x;
		p1.y = vetexes[indexes[i+1] ].y;
		p1.z = vetexes[indexes[i+1] ].z;

		p2.x = vetexes[indexes[i+2] ].x;
		p2.y = vetexes[indexes[i+2] ].y;
		p2.z = vetexes[indexes[i+2] ].z;
		FLOAT dis;
		//调用API
		FLOAT pu,pv;
		BOOL ret_value =  D3DXIntersectTri(
			&p0,
			&p1,
			&p2,
			&ray_position_,
			&ray_dir_,
			&pu,
			&pv,
			&dis
			);
		if(ret_value){
			is_insected = true;
			D3DXVECTOR3 out = p0 + pu * (p1 - p0) + pv * (p2 - p0);
			if(coords_world){
				coords_world->x = out.x;
				coords_world->y = out.y;
				coords_world->z = out.z;
			}
			if(dis < min_distance){
				min_distance = dis;
			}
		}
	}
	if(distance){
		*distance = min_distance;
	}
	return is_insected;
}

bool RayDetect::Intersect(V3VERTEX* vetexes,int num_vetexes,FLOAT* distance,D3DXVECTOR3* p_out)
{
	//更新ray信息
	Dir(NULL);

	FLOAT min_distance = 100000.0f; 
	bool is_insected = false;  //不相交
	for(int i = 0;i < num_vetexes;i += 3){
		//得到三个顶点
		D3DXVECTOR3  p0,p1,p2;
		p0.x = vetexes[i ].x;
		p0.y = vetexes[i].y;
		p0.z = vetexes[i].z;

		p1.x = vetexes[i+1].x;
		p1.y = vetexes[i+1 ].y;
		p1.z = vetexes[i+1 ].z;

		p2.x = vetexes[i+2].x;
		p2.y = vetexes[i+2].y;
		p2.z = vetexes[i+2].z;
		FLOAT dis;
		//调用API
		FLOAT pu,pv;
		BOOL ret_value =  D3DXIntersectTri(
			&p0,
			&p1,
			&p2,
			&ray_position_,
			&ray_dir_,
			&pu,
			&pv,
			&dis
			);
		if(ret_value){
			is_insected = true;
			if(p_out){
				*p_out = p0 + pu * (p1 - p0) + pv * (p2 - p0);
			}
			if(dis < min_distance){
				min_distance = dis;
			}
		}
	}
	if(distance){
		*distance = min_distance;
	}
	return is_insected;
}