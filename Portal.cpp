﻿#include "Portal.h"
#include <iostream>

#define _USE_MATH_DEFINES
#include <math.h>

#include <glm/gtc/matrix_transform.hpp>

using namespace portal;

namespace
{
	const float PORTAL_FRAME_WIDTH = 19.f;
	const float PORTAL_FRAME_HEIGHT = 16.f;
	const float PORTAL_GUT_WIDTH = 4.5f;
	const float PORTAL_GUT_HEIGHT = 6.8f;

	std::vector<Vertex>
	generate_portal_frame()
	{
		return {
			{ { -PORTAL_FRAME_WIDTH / 2.f, -PORTAL_FRAME_HEIGHT / 2.f, 0.f }, { 1.f, 1.f, 1.f, 1.f }, { 0.f, 0.f }, { 0.f, 0.f, 1.f } },
			{ { -PORTAL_FRAME_WIDTH / 2.f,  PORTAL_FRAME_HEIGHT / 2.f, 0.f }, { 1.f, 1.f, 1.f, 1.f }, { 0.f, 1.f }, { 0.f, 0.f, 1.f } },
			{ { PORTAL_FRAME_WIDTH / 2.f,  -PORTAL_FRAME_HEIGHT / 2.f, 0.f }, { 1.f, 1.f, 1.f, 1.f }, { 1.f, 0.f }, { 0.f, 0.f, 1.f } },
			{ { PORTAL_FRAME_WIDTH / 2.f,  -PORTAL_FRAME_HEIGHT / 2.f, 0.f }, { 1.f, 1.f, 1.f, 1.f }, { 1.f, 0.f }, { 0.f, 0.f, 1.f } },
			{ { -PORTAL_FRAME_WIDTH / 2.f,  PORTAL_FRAME_HEIGHT / 2.f, 0.f }, { 1.f, 1.f, 1.f, 1.f }, { 0.f, 1.f }, { 0.f, 0.f, 1.f } },
			{ { PORTAL_FRAME_WIDTH / 2.f,   PORTAL_FRAME_HEIGHT / 2.f, 0.f }, { 1.f, 1.f, 1.f, 1.f }, { 1.f, 1.f }, { 0.f, 0.f, 1.f } },
		};
	}

	const int ELLIPSE_NUM_SIDES = 20;
	std::vector<Vertex>
	generate_portal_ellipse_hole( float radius_x, float radius_y )
	{
		const glm::vec4 black{ 0.f, 0.f, 0.f, 1.f };
		const glm::vec2 fake_uv( 0.f, 0.f );
		const glm::vec3 normal{ 0.f, 0.f, 1.f };

		constexpr int num_vertices = ELLIPSE_NUM_SIDES + 2;
		const float two_pi = 2.f * static_cast<float>( M_PI );

		std::vector<Vertex> vertices;
		vertices.reserve( num_vertices );
		vertices.emplace_back( Vertex{ { 0.f, 0.f, 0.f }, black, fake_uv, normal } );
		
		for( int i = 0; i < num_vertices; i++ )
		{
			float rad = (num_vertices - i) * two_pi / ELLIPSE_NUM_SIDES;
			vertices.emplace_back( Vertex{
				{ cos( rad ) * radius_x, sin( rad ) * radius_y, 0.f },
				black, fake_uv, normal
			});
		}
		return vertices;
	}
}

Portal::Portal( unsigned int texture, float view_width, float view_height )
	: mFaceDir( 0.f, 0.f, 1.f )
	, mPosition( 0.f, 0.f, 0.f )
	, mOriginFaceDir( 0.f, 0.f, 1.f )
	, mFrameRenderable( generate_portal_frame(), Renderer::PORTAL_FRAME_SHADER, texture )
	, mHoleRenderable( generate_portal_ellipse_hole( PORTAL_GUT_WIDTH, PORTAL_GUT_HEIGHT ), Renderer::PORTAL_HOLE_SHADER, 0, Renderer::Renderable::DrawType::TRIANGLE_FANS )
	, mCamera( view_width, view_height, Camera::Type::FREE, mPosition, mOriginFaceDir, mOriginFaceDir )
	, mHasBeenPlaced( false )
	, mPairedPortal( nullptr )
	, mPlayerCamera( nullptr )
{}

Portal::~Portal()
{}

void 
Portal::SetPair( Portal* paired_portal, Camera* player_camera )
{
	mPairedPortal = paired_portal;
	mPlayerCamera = player_camera;
}

bool 
Portal::UpdatePosition( glm::vec3 pos, glm::vec3 dir )
{
	if( pos == mPosition && dir == mFaceDir )
	{
		return false;
	}

	// 求它们之间的夹角
	float theta = std::acos( glm::dot( mOriginFaceDir, dir ) );
	// 求垂直于它们的向量用作转轴
	glm::vec3 cross_product = glm::cross( mOriginFaceDir, dir );
	cross_product = glm::normalize( cross_product );

	mPosition = pos;
	mFaceDir = dir;

	mFrameRenderable.Translate( pos + mFaceDir * 0.2f );
	mFrameRenderable.Rotate( theta, cross_product );
	mHoleRenderable.Translate( pos + mFaceDir * 0.1f );
	mHoleRenderable.Rotate( theta, cross_product );

	mHasBeenPlaced = true;

	return true;
}

Renderer::Renderable*
Portal::GetFrameRenderable()
{
	return &mFrameRenderable;
}

Renderer::Renderable* 
Portal::GetHoleRenderable()
{
	return &mHoleRenderable;
}

Camera* 
Portal::GetCamera()
{
	return &mCamera;
}

bool 
Portal::HasBeenPlaced()
{
	return mHasBeenPlaced;
}

bool
Portal::IsLinkActive()
{
	if( mPairedPortal && mPlayerCamera )
	{
		return mHasBeenPlaced && mPairedPortal->HasBeenPlaced();
	}
	return false;
}
