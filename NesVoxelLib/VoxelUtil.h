// Obviously thanks to http://www.directxtutorial.com/ for a bunch of this
// As well as http://www.braynzarsoft.net/viewtutorial/q16390-4-begin-drawing

#pragma once
#include <vector>
#include "VoxelCamera.h"

static float pixelSizeW = (2.0f / 256.0f);
static float pixelSizeH = (2.0f / 240.0f);

using namespace DirectX;

enum ShaderType { color, texture };

struct ColorVertex {
	XMFLOAT4 Pos;
	XMFLOAT4 Col;
};

struct TextureVertex {
	XMFLOAT4 Pos;
	XMFLOAT2 UV;
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

struct ShaderSet
{
	ID3D11VertexShader *vertexShader;
	ID3D11PixelShader *pixelShader;
};

class VoxelUtil {
public:
	static void initPipeline(
		Microsoft::WRL::ComPtr<ID3D11Device> d3dDevice, Microsoft::WRL::ComPtr<ID3D11DeviceContext> d3dContext,
		Microsoft::WRL::ComPtr<ID3D11Device1> d3dDevice1, Microsoft::WRL::ComPtr<ID3D11DeviceContext1> d3dContext1);
	static ID3D11Buffer* createBufferFromColorVertices(ColorVertex vertices[], int arraySize);
	static ID3D11Buffer* createBufferFromColorVerticesV(std::vector<ColorVertex> &vertices, int arraySize);
	static ID3D11Buffer* createBufferFromTextureVertices(TextureVertex vertices[], int arraySize);
	static VoxelMesh* CreateRectangle(ShaderType type);
	static VoxelMesh* CreateSpriteMarker();
	static void updateMatricesWithCamera(VoxelCamera * camera);
	static void updateWorldMatrix(float, float, float);
	static XMMATRIX getProjectionMatrix(const float near_plane, const float far_plane, const float fov_horiz, const float fov_vert);
	static void setShader(ShaderType type);
	static void renderMesh(VoxelMesh *voxelMesh);
	static void updateGameTexture(const void *data);
private:
	static Microsoft::WRL::ComPtr<ID3D11Device>            device;
	static Microsoft::WRL::ComPtr<ID3D11Device1>           device1;
	static Microsoft::WRL::ComPtr<ID3D11DeviceContext>     context;
	static Microsoft::WRL::ComPtr<ID3D11DeviceContext1>    context1;
	static void initShaders();
	static void initSampleState();
	static ShaderSet shaderSets[2];
	static ID3D11InputLayout *inputLayouts[2];
	static ID3D11Buffer *worldMatrixBuffer;
	static ID3D11Buffer *viewMatrixBuffer;
	static ID3D11Buffer *projectionMatrixBuffer;
	static MatrixBuffer *worldMatrixPtr;
	static MatrixBuffer *viewMatrixPtr;
	static MatrixBuffer *projectionMatrixPtr;
	static ID3D11SamplerState* sampleState;
	static ID3D11Texture2D* texture2d;
	static ID3D11ShaderResourceView* textureView;
	static ShaderType activeShader;
	static D3D11_SUBRESOURCE_DATA subData;
};