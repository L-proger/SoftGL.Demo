#define WIN32_LEAN_AND_MEAN
#include <SoftGL/LString.h>
#include <SoftGL/Buffer.h>
#include <SoftGL/IRenderWindow.h>
#include <SoftGL/Texture.h>
#include <SoftGL/BlockRasterizer.h>
#include <SoftGL/Platform/Windows/RenderWindow.h>
#include <SoftGL/Vertex.h>
#include <SoftGL/texture_utils.h>
#include <SoftGL/VSDefault.h>
#include <SoftGL/PSDefault.h>
#include <SoftGL/StaticBuffer.h>

#include <LMath/lmath.h>
#include <SoftGL/Engine/FpsCounter.h>
#include <SoftGL/Engine/transform.h>
#include <SoftGL/Engine/gameobject.h>
#include <SoftGL/Engine/Camera.h>

#include <SoftGL/Engine/camera_controller.h>
#include <SoftGL/Engine/mesh.h>
#include <SoftGL/Engine/cube_generator.h>
#include <SoftGL/Engine/plane_generator.h>
#include <SoftGL/Engine/PsNormalMap.h>
#include <SoftGL/StaticTexture.h>
#include <SoftGL/Engine/shaders/textured_no_lit.h>

#include <iostream>
#include <cstdint>
#include <array>
#include <windows.h>
#include <WinBase.h>


void DrawMesh(BlockRasterizer* rasterizer, IMesh* mesh){
	rasterizer->SetVertexBuffer(mesh->GetVertexBuffer(), 0, mesh->GetVertexBuffer()->ItemSize());
	for (size_t i = 0; i < mesh->GetSubmeshCount(); ++i){
		auto ib = mesh->GetSubmeshBuffer(i);
		rasterizer->SetIndexBuffer(ib, 0);
		rasterizer->DrawIndexed(ib->Size() / ib->ItemSize(), 0);
	}
}

static constexpr size_t sx = 320;
static constexpr size_t sy = 240;

alignas(32) MipChain<sizeof(uint32_t), sx, sy, 0> bb_data;
alignas(32) MipChain<sizeof(float), sx, sy, 0> db_data;

/*bool isAvxSupportedByWindows() {
	const DWORD64 avxFeatureMask = XSTATE_MASK_LEGACY_SSE | XSTATE_MASK_GSSE;
	return GetEnabledExtendedFeatures(avxFeatureMask) == avxFeatureMask;
}*/

int main()
{

	std::string resourcesDir = RESOURCES_DIR;
	auto input = Input::Instance();

	auto tex_normal = texture_utils::LoadTexture(resourcesDir + "/normal.bmp");
	auto tex_diffuse = texture_utils::LoadTexture(resourcesDir + "/diffuse.bmp");
	auto tex_ao = texture_utils::LoadTexture(resourcesDir + "ao.bmp");

	Game_object go;
	Camera camera(&go);
	camera.SetZFar(30);
	camera.SetAspect((float)sx / (float)sy);
	CameraController camController(&camera);

	go.transform.SetLocalPosition(float3(0.0f, 0.0f, -10.0f));

	RenderWindow* wnd = new RenderWindow();
	wnd->SetSize(sx, sy);
	wnd->SetCaption("SoftGL");
	wnd->CenterWindow();

	TextureDescription desc;
	desc.Width = sx;
	desc.Height = sy;
	desc.BytesPerPixel = sizeof(uint32_t);
	desc.MipCount = bb_data.MipsCount;

	StaticTexture<decltype(&bb_data)> backBuffer(&bb_data, desc);
	StaticTexture<decltype(&db_data)> depthBuffer(&db_data, desc);

	Mesh<1> plane;
	static_buffer<Vertex, CubeGenerator<Vertex, indices_t>::VertexCount> quad_vb;
	static_buffer<indices_t, CubeGenerator<Vertex, indices_t>::IndexCount> quad_ib;
	plane.vertexBuffer = &quad_vb;
	plane.submeshes[0] = &quad_ib;


	CubeGenerator<Vertex, indices_t>::Generate(&plane);

	BlockRasterizer rasterizer;



	StaticInputLayout<3> layout;
	layout.elements = {
		InputElement("POSITION", 0, RegType::float4, 0),
		InputElement("NORMAL", 16, RegType::float3, 0),
		InputElement("TEXCOORD", 28, RegType::float2, 0)
	};

	rasterizer.SetInputLayout(&layout);
	rasterizer.SetPrimitiveType(PT_TRIANGLE_LIST);

	rasterizer.setColorBuffer(&backBuffer);
	rasterizer.setDepthBuffer(&depthBuffer);

	auto vs = VSDefault();
	auto ps = PSTexturedNoLit();
	ps.diffuseMap = tex_diffuse;


	rasterizer.SetVertexShader(&vs);
	rasterizer.SetPixelShader(&ps);
	
	FpsCounter<40> fps;
	//Stopwatch sw;
	float threadTime = 0;
	int frames = 0;
	float angle = 0.0f;
	while (true){
		frames++;
		fps.ComputeFPS();
		threadTime += fps.GetFrameTimeSeconds();
		if (threadTime >= 1.0f) {
			std::cout << "FPS: " << (int)(frames / threadTime) << std::endl;
			threadTime = 0;
			frames = 0;
		}
		wnd->Update();
		input->strobe();
		camController.Tick(fps.GetFrameTimeSeconds());

		//clear render targets
		texture_utils::fill<uint32_t>(&backBuffer, 0x00ff0000 );//0x00232327
		texture_utils::fill<float>(&depthBuffer, 1.0f);

		//setup material
		vs.mWorld = lm::mul(matrix4x4Scale<float>(1, 1, 1), matrix4x4RotationQuaternion(Quaternion_f::angleAxis(-3.1415f/2*0, float3(1.0f,0.0f,0.0f))));
		vs.mView = camera.world_to_camera_matrix();
		vs.mProj = camera.GetProjection();
		
		DrawMesh(&rasterizer, &plane);

		vs.mWorld = lm::mul(lm::matrix4x4Translation(0.0f, 3.0f, 0.0f),  lm::mul(matrix4x4Scale<float>(1, 1, 1), matrix4x4RotationQuaternion(Quaternion_f::angleAxis(-3.1415f / 2 * 0, float3(1.0f, 0.0f, 0.0f)))));
		DrawMesh(&rasterizer, &plane);

		//present
		wnd->Present(&backBuffer);

		
		angle += fps.GetFrameTimeSeconds();
	}

	getchar();
	return 0;
}
