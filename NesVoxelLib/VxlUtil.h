// Obviously thanks to http://www.directxtutorial.com/ for a bunch of this
// As well as http://www.braynzarsoft.net/viewtutorial/q16390-4-begin-drawing

#pragma once
#include <vector>
#include "VxlCamera.h"
#include "VxlD3DContext.h"
#include "VxlPalette.h"

static float pixelSizeW = (2.0f / 256.0f);
static float pixelSizeH = (2.0f / 240.0f);

using namespace DirectX;

enum ShaderType { color, texture };

struct ColorVertex {
	XMFLOAT4 Pos;
	XMFLOAT4 Col;
	XMFLOAT4 Nor;
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

struct Sides
{
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

class VxlUtil {
public:
	static void initPipeline(VxlD3DContext context);
	static ID3D11Buffer* createBufferFromColorVertices(ColorVertex vertices[], int arraySize);
	static ID3D11Buffer* createBufferFromColorVerticesV(std::vector<ColorVertex> &vertices, int arraySize);
	static ID3D11Buffer* createBufferFromTextureVertices(TextureVertex vertices[], int arraySize);
	static VoxelMesh* CreateRectangle(ShaderType type);
	static VoxelMesh* CreateSpriteMarker();
	static void updateMatricesWithCamera(VxlCamera * camera);
	static void updateWorldMatrix(float, float, float);
	static void updateMirroring(bool horizontal, bool vertical);
	static void updatePalette(float palette[72]);
	static void selectPalette(int palette);
	static XMMATRIX getProjectionMatrix(const float near_plane, const float far_plane, const float fov_horiz, const float fov_vert);
	static void setShader(ShaderType type);
	static void renderMesh(VoxelMesh *voxelMesh);
	static void updateGameTexture(const void *data);
	static PPUHueStandardCollection ppuHueStandardCollection;
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
	static ID3D11Buffer *mirrorBuffer;
	static ID3D11Buffer *paletteBuffer;
	static ID3D11Buffer *paletteSelectionBuffer;
	static MatrixBuffer *worldMatrixPtr;
	static MatrixBuffer *viewMatrixPtr;
	static MatrixBuffer *projectionMatrixPtr;
	static ID3D11SamplerState* sampleState;
	static ID3D11Texture2D* texture2d;
	static ID3D11ShaderResourceView* textureView;
	static ShaderType activeShader;
	static D3D11_SUBRESOURCE_DATA subData;
	static MirrorState mirrorState;
	static int selectedPalette;
};