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
#include "N3sMath.hpp"

using namespace DirectX;
using namespace std;

enum ShaderType { color = 0, overlay = 1, outline = 2 };
const int shaderCount = 3;	// UPDATE THIS WHEN ADDING SHADERS

enum PlaneAxis { x_axis, y_axis, z_axis };

struct ColorVertex {
	XMFLOAT4 Pos;
	XMFLOAT2 Uv;
	XMFLOAT4 Normal;
	UINT32 Color;
};

struct OutlineVertex {
	XMFLOAT4 Pos;
};

struct OverlayVertex {
	XMFLOAT4 Pos;
};

struct VoxelMesh {
	ID3D11Buffer *buffer;
	void releaseBuffers();
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

struct OutlineCache
{
	int stencilValue;
	VoxelMesh *outlineMesh;
	int palette;
	int color;
	float posX;
	float posY;
	bool mirrorH;
	bool mirrorV;
};

enum StencilMode
{
	stencil_nowrite,
	stencil_write,
	stencil_mask,
	stencil_mask_unequal,
	stencil_highlight_write
};

class N3s3d {
public:
	static void initPipeline(N3sD3dContext context);
	static ID3D11Buffer* createBufferFromColorVertices(std::vector<ColorVertex> * vertices, int arraySize);
	static ID3D11Buffer* createBufferFromOverlayVertices(std::vector<OverlayVertex> * vertices, int arraySize);
	static void updateMatricesWithCamera(shared_ptr<Camera> camera);
	static void updateViewMatrices(XMFLOAT4X4 view, XMFLOAT4X4 perspective);
	static void updateWorldMatrix(float xPos, float yPos, float zPos);
	static void updateWorldMatrix(float xPos, float yPos, float zPos, float xRot, float yRot, float zRot, float scale);
	static void updateWorldMatrix(float xPos, float yPos, float zPos, float xRot, float yRot, float zRot, float xScale, float yScale, float zScale);
	static void updateMirroring(bool horizontal, bool vertical);
	static void updatePalette(float palette[72], Hue bg);
	static void selectPalette(int palette);
	static void selectOutlinePalette(int palette, int color);
	static void setCameraPos(float x, float y, float z);
	static void setOverlayColor(float r, float g, float b, float a);
	static void setOverlayColor(int r, int g, int b, int a);
	static XMMATRIX getProjectionMatrix(const float near_plane, const float far_plane, const float fov_horiz, const float fov_vert);
	static void setShader(ShaderType type);
	static void renderMesh(VoxelMesh *voxelMesh);
	static void setIndexBuffer();
	static void setDepthBufferState(bool active);
	static void setStencilingState(StencilMode mode, int referenceNumber);
	static void setDepthStencilState(bool depthTest, bool stencilWrite, bool stencilTest);
	static void prepareStencilForOutline(bool increment);
	static void stopStencilingForOutline();
	static void setStencilingState(ID3D11DepthStencilState * state, int value);
	static void cacheOutlineMesh(VoxelMesh * outlineMesh, int palette, int color, float x, float y, bool mirrorH, bool mirrorV);
	static void renderOutlines();
	static void setRasterFillState(bool fill);
	static void incrementStencilReference();
	static void setGuiProjection();
	static void clear();
	static D3D11_VIEWPORT viewport;
	static void updateViewport(D3D11_VIEWPORT viewport);
	static void updateViews(D3D11_VIEWPORT v, ID3D11RenderTargetView* r, ID3D11DepthStencilView* s);
	static void attachRenderTarget(bool attach);
	static XMVECTOR getMouseVector(Camera * camera, int mouseX, int mouseY);
	static XMFLOAT3 getPlaneIntersection(PlaneAxis axis, int pixel, Camera * camera, int mouseX, int mouseY);
	static Vector3D getPixelCoordsFromFloat3(XMFLOAT3 pos);
private:
	static void initShaders();
	static void initShaderExtras();
	static void initSampleState();
	static void initRasterDesc();
	static void createIndexBuffers();
	static bool initDepthStencils();
};

bool toggleBool(bool b);
string replaceExt(string input, string newExt);
