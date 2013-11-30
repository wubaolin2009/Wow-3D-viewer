#pragma once


#include <Windows.h>
#include <mmsystem.h>
#include <d3dx9.h>
class MyD3DEngine;
class CCamera
{
public:
	CCamera();
	virtual ~CCamera();

	D3DXVECTOR3 GetEyePosition();

	/** 设置生成视矩阵所需的参数
	*/
	void SetViewParams( D3DXVECTOR3 &pos, D3DXVECTOR3 &lookat, D3DXVECTOR3 &up );
	/** 设置生成透视投影矩阵所需的参数
	*/
	void SetProjParams( float fFOV, float fAspect, float fNear, float fFar  );
	/** 设置生成视平行投影矩阵所需的参数
	*/
	void SetOrthoProjParams( float w, float h, float fNear, float fFar );
	/** 返回当前的视矩阵
	*/
	const D3DXMATRIX *GetViewTrans() const;
	/** 返回当前的投影矩阵
	*/
	const D3DXMATRIX *GetProjTrans() const;

	/** 设置摄像机移动速度
	*/
	void SetMoveVelocity( float fVelocity );

	/** 在消息循环里处理消息
	*/
	virtual LRESULT HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	/**  在逻辑帧里面处理消息
	*/
	//virtual void ProcessKey(float fElapsedTime);

	/** 更新摄像机属性
		@remarks
		通常在逻辑帧里面调用此函数，函数内部会自动判断需要更新哪些属性。
	*/
	virtual void	Update();
	/** 将视矩阵和投影矩阵设置给D3D设备
	*/
	void	ApplyDevice( LPDIRECT3DDEVICE9  pDevice );

	D3DXVECTOR3& getEyePos()
	{
		return m_EyePos;
	}

	D3DXVECTOR3& getLookAtPos()
	{
		return m_LookAt;
	}

	D3DXVECTOR3& getUpPos()
	{
		return m_Up;
	}


public:
	/// 视矩阵
	D3DXMATRIX	m_ViewTrans;
	/// 投影矩阵
	D3DXMATRIX	m_ProjTrans;

	/// 摄像机位置
	D3DXVECTOR3 m_EyePos;
	/// 摄像机的观察点
	D3DXVECTOR3 m_LookAt;
	/// 摄像机的UP向量
	D3DXVECTOR3 m_Up;
	/// 摄像机的RIGHT向量
	D3DXVECTOR3 m_Right;
	/// 摄像机的方向
	D3DXVECTOR3 m_Direction;

	/// 近视表面的距离
	float		m_fNear;
	/// 远视表面的距离
	float		m_fFar;
	/// 视角，弧度
	float		m_fFOV;
	/// 长宽比
	float		m_fAspect;

	/// 上一个鼠标位置
	POINT		m_LastPoint;
	/// 用户是否旋转了摄像机
	bool		m_bIsRot;
	/// 用户是否平移了摄像机
	bool		m_bIsTrans;

	/// 摄像机初始状态的欧拉角Yaw，在SetViewParams时初始化
	float		m_fCameraYawAngle;		
	/// 摄像机初始状态的欧拉角Pitch，在SetViewParams时初始化
	float		m_fCameraPitchAngle;

	/// 摄像机的平移量
	D3DXVECTOR3	m_vDelta;
	/// 摄像机的移动速度，单位/秒
	float		m_fVelocity;

	/// 摄像机俯仰角最大值
	float m_fMaxPitch;
	/// 摄像机俯仰角最小值
	float m_fMinPitch;

	//保存鼠标的当前坐标
	POINT ptCurrentPos;

};