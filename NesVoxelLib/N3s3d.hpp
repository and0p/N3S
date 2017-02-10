// Obviously thanks to http://www.directxtutorial.com/ for a bunch of this
// As well as http://www.braynzarsoft.net/viewtutorial/q16390-4-begin-drawing

#pragma once
#include <vector>
#include "Camera.hpp"
#include "N3sPalette.hpp"
#include <d3d11_1.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <wrl/client.h>
#include "N3sD3DContext.h"

static float pixelSizeW = (2.0f / 256.0f);
static float pixelSizeH = (2.0f / 240.0f);

using namespace DirectX;
using namespace std;

enum ShaderType { color = 0, overlay = 1 };
const int shaderCount = 2;	// UPDATE THIS WHEN ADDING SHADERS

struct ColorVertex {
	XMFLOAT4 Pos;
	XMFLOAT4 Col;
};

struct OverlayVertex {
	XMFLOAT4 Pos;
};

struct VoxelMesh {
	ID3D11Buffer *buffer;
	ShaderType type;
	int size;
};

struct MatrixBuffer
{
	XMMATRIX matrix;
};

struct ShaderMatrixCollection
{
	ID3D11Buffer *worldMatrixBuffer;
	ID3D11Buffer *viewMatrixBuffer;
	ID3D11Buffer *projectionMatrixBuffer;
};

struct Shader
{
	ID3D11VertexShader *vertexShader;
	ID3D11PixelShader *pixelShader;
	ID3D11InputLayout *inputLayout;
	ShaderMatrixCollection matrices;
};

struct Crop
{
	bool zeroed();
	int top;
	int left;
	int bottom;
	int right;
};

struct MirrorState
{
	int x;
	int y;
};

class N3s3d {
public:
	static void initPipeline(N3sD3dContext context);
	static ID3D11Buffer* createBufferFromColorVertices(std::vector<ColorVertex> * vertices, int arraySize);
	static ID3D11Buffer* createBufferFromOverlayVertices(std::vector<OverlayVertex> * vertices, int arraySize);
	static void updateMatricesWithCamera(Camera * camera);
	static void updateViewMatrices(XMFLOAT4X4 view, XMFLOAT4X4 perspective);
	static void updateWorldMatrix(float xPos, float yPos, float zPos);
	static void updateWorldMatrix(float xPos, float yPos, float zPos, float xRot, float yRot, float zRot, float scale);
	static void updateWorldMatrix(float xPos, float yPos, float zPos, float xRot, float yRot, float zRot, float xScale, float yScale, float zScale);
	static void updateMirroring(bool horizontal, bool vertical);
	static void updatePalette(float palette[72]);
	static void selectPalette(int palette);
	static void setOverlayColor(float r, float g, float b, float a);
	static void setOverlayColor(int r, int g, int b, int a);
	static XMMATRIX getProjectionMatrix(const float near_plane, const float far_plane, const float fov_horiz, const float fov_vert);
	static void setShader(ShaderType type);
	static void renderMesh(VoxelMesh *voxelMesh);
	static void setIndexBuffer();
	static void setDepthBufferState(bool active);
	static void setRasterFillState(bool fill);
	static void setGuiProjection();
	static D3D11_VIEWPORT viewport;
	static void updateViewport(D3D11_VIEWPORT viewport);
	static XMVECTOR getMouseVector(Camera * camera, int mouseX, int mouseY);
	static XMFLOAT3 getZIntersection(Camera * camera, int mouseX, int mouseY);
private:
	static void initShaders();
	static void initShaderExtras();
	static void initSampleState();
	static void initRasterDesc();
	static void createIndexBuffer();
	static bool initDepthStencils();
};

bool toggleBool(bool b);
string replaceExt(string input, string newExt);
