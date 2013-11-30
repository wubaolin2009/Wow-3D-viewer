#include <Windows.h>
#include <zmouse.h>
#include <stdio.h>
#include "camera.h"


#define new VNEW

//这里设定视角的初始位置
CCamera::CCamera() :
m_fNear(0.2f),  //这里不能为0 如果为0 Z缓冲那里会有问题
m_fFar(6000.f),
m_fFOV(D3DX_PI / 4),
m_fAspect(1024.0/768.0f),
m_bIsRot(false),
m_bIsTrans(false),
m_fCameraYawAngle(0.f),
m_fCameraPitchAngle(0.f),
m_vDelta(0.f, 0.f, 0.f),
m_fVelocity(0.001f),
m_fMaxPitch(D3DX_PI*0.49f),
m_fMinPitch(-D3DX_PI*0.49f)
{
	/*m_EyePos = D3DXVECTOR3(20.0f, 0.0f, -10.f);
	m_LookAt = D3DXVECTOR3(0.0f, 0.0f, -0.0f);
	m_Up	 = D3DXVECTOR3(0.0f, -10.0f,0.0f);*/

	m_EyePos = D3DXVECTOR3(0.0f, 7.0f, -7.0f);
	//m_EyePos = D3DXVECTOR3(17056.0f, 400.0f, 23452.0f);

	m_LookAt = D3DXVECTOR3(0.0f, 0.0f, -0.0f);
	//m_LookAt = D3DXVECTOR3(17056.0f, 0.0f, 23452.0f);

	m_Up	 = D3DXVECTOR3(0.0f, 1.0f,1.0f);
	
	D3DXMatrixIdentity( &m_ViewTrans );
	D3DXMatrixLookAtLH( &m_ViewTrans, &m_EyePos, &m_LookAt, &m_Up );

	D3DXMatrixIdentity( &m_ProjTrans );
	D3DXMatrixPerspectiveFovLH( &m_ProjTrans, m_fFOV,m_fAspect, m_fNear, m_fFar );
	//MyCamera():mEye(30.0f,50.0f,-20.0f),mLookAt(10.0f,15.0f,20.0f), mUpVec(0.0f,1.0f,0.0f)

}

D3DXVECTOR3 CCamera::GetEyePosition()
{
	return m_EyePos;
}

CCamera::~CCamera()
{

}


void CCamera::SetViewParams( D3DXVECTOR3 &pos, D3DXVECTOR3 &lookat, D3DXVECTOR3 &up )
{
	m_EyePos = pos;
	m_LookAt = lookat;
	m_Up	 = up;
	m_Direction = m_LookAt - m_EyePos;
	D3DXVec3Cross( &m_Right, &m_Up, &m_Direction );


	D3DXMatrixLookAtLH(&m_ViewTrans, &m_EyePos, &m_LookAt, &m_Up);

	D3DXMATRIX mInvView;
	D3DXMatrixInverse( &mInvView, NULL, &m_ViewTrans );
	D3DXVECTOR3* pZBasis = (D3DXVECTOR3*) &mInvView._31;
	m_fCameraYawAngle   = atan2f( pZBasis->x, pZBasis->z );
	float fLen = sqrtf(pZBasis->z*pZBasis->z + pZBasis->x*pZBasis->x);
	m_fCameraPitchAngle = -atan2f( pZBasis->y, fLen );
}

void CCamera::SetProjParams( float fFOV, float fAspect, float fNear, float fFar  )
{
	m_fFOV		= fFOV;
	m_fAspect	= fAspect;
	m_fNear     = fNear;
	m_fFar      = fFar;

	D3DXMatrixPerspectiveFovLH( &m_ProjTrans, m_fFOV, m_fAspect, m_fNear, m_fFar );
}
void CCamera::SetOrthoProjParams( float w, float h, float fNear, float fFar )
{
	D3DXMatrixOrthoLH( &m_ProjTrans, w, h, fNear, fFar );
}
const D3DXMATRIX *CCamera::GetViewTrans() const
{
	return &m_ViewTrans;
}
const D3DXMATRIX *CCamera::GetProjTrans() const
{
	return &m_ProjTrans;
}

void CCamera::SetMoveVelocity( float fVelocity )
{
	m_fVelocity = fVelocity;
}


LRESULT CCamera::HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	//m_vDelta = D3DXVECTOR3(0.f, 0.f, 0.f);
	// setcapture getcursorPos
	m_vDelta = D3DXVECTOR3(0.f, 0.f, 0.f);
	float fVelocity = m_fVelocity*15500 ;
	switch( msg )
	{	
	case WM_RBUTTONDOWN:
		{
			m_bIsRot = true;
			//SetCapture(hWnd);
			GetCursorPos(&m_LastPoint);
		}
		break;
	case WM_RBUTTONUP:
		{
			m_bIsRot = false;
			//ReleaseCapture();
		}
		break;
	/*case WM_MOUSEMOVE:
		if(!m_bIsRot)
			break;
		GetCursorPos(&ptCurrentPos);*/
		//ptCurrentPos.x = LOWORD(lParam);
		//ptCurrentPos.y = HIWORD(lParam);
		break;
	case WM_KEYDOWN:
		if(wParam ==87) //w
		{
			m_bIsTrans = true;
			m_vDelta.z += fVelocity;

		}
		if(wParam ==83) //s
		{
			m_bIsTrans = true;
			m_vDelta.z -= fVelocity;
		}
		if(wParam ==65) //a
		{
			m_bIsTrans = true;
			m_vDelta.x -= fVelocity;
		}
		if(wParam ==68) //d
		{
			m_bIsTrans = true;
			m_vDelta.x += fVelocity;
		}
		if(wParam == VK_SPACE)
		{
		}
		break;
	case WM_DESTROY:
		// Cleanup();
		PostQuitMessage( 0 );
		return 0;
	}
	Update();

	return 0;
}

void CCamera::Update()
{
	POINT ptCurrentPos = {0, 0};
	POINT ptDeltaPos = {0, 0};

	if(m_bIsRot)
	{
		//计算鼠标偏移
		GetCursorPos(&ptCurrentPos);
		//printf("current pos %d %d\n",ptCurrentPos.x,ptCurrentPos.y);
		ptDeltaPos.x = ptCurrentPos.x - m_LastPoint.x;
		ptDeltaPos.y = ptCurrentPos.y - m_LastPoint.y;
		
		m_LastPoint = ptCurrentPos;

		float fYaw = ptDeltaPos.x*0.01f;
		float fPitch = ptDeltaPos.y*0.01f;

		//根据鼠标便宜计算欧拉角
		m_fCameraYawAngle   += fYaw;
		m_fCameraPitchAngle += fPitch;

	}

	// 根据欧拉角Yaw，Pitch计算摄像机的旋转矩阵
	D3DXMATRIX matCameraRot;
	ZeroMemory(&matCameraRot, sizeof(D3DXMATRIX));
	D3DXMatrixRotationYawPitchRoll(&matCameraRot, m_fCameraYawAngle, m_fCameraPitchAngle,0.f);

	// 根据旋转矩阵将摄像机的局部方向向量和上方向向量转为全局向量
	D3DXVECTOR3 vWorldUp, vWorldAhead;
	D3DXVECTOR3 vLocalUp    = D3DXVECTOR3(0,1,0);
	D3DXVECTOR3 vLocalAhead = D3DXVECTOR3(0,0,1);
	D3DXVec3TransformCoord( &vWorldUp, &vLocalUp, &matCameraRot );
	D3DXVec3TransformCoord( &vWorldAhead, &vLocalAhead, &matCameraRot );

	if(m_bIsTrans)
	{
		// 将局部偏移量转到全局坐标
		D3DXVECTOR3 vWorldDelta;
		D3DXVec3TransformCoord( &vWorldDelta, &m_vDelta, &matCameraRot );
		// 根据偏移量计算视点位置
		m_EyePos += vWorldDelta;
	}
	// 计算观察点位置 
	m_LookAt = m_EyePos + vWorldAhead;

	// 更新视矩阵
	D3DXMatrixLookAtLH( &m_ViewTrans, &m_EyePos, &m_LookAt, &vWorldUp );

	m_Direction = m_LookAt - m_EyePos;
	m_Up = vWorldUp;

	D3DXVec3Cross( &m_Right, &vWorldUp, &m_Direction );
}


void CCamera::ApplyDevice( LPDIRECT3DDEVICE9  pDevice )
{
	if( pDevice )
	{
		D3DXMATRIXA16 matWorld;
		D3DXMatrixScaling(&matWorld,1,1,1);
		pDevice->SetTransform( D3DTS_WORLD, &matWorld );
		pDevice->SetTransform(D3DTS_VIEW, &m_ViewTrans);
		pDevice->SetTransform(D3DTS_PROJECTION, &m_ProjTrans);
	}
}

