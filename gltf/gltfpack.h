/**
 * gltfpack - version 0.13
 *
 * Copyright (C) 2016-2020, by Arseny Kapoulkine (arseny.kapoulkine@gmail.com)
 * Report bugs and download new versions at https://github.com/zeux/meshoptimizer
 *
 * This application is distributed under the MIT License. See notice at the end of this file.
 */

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "../extern/cgltf.h"

#include <assert.h>

#include <string>
#include <vector>

struct Attr
{
	float f[4];
};

struct Stream
{
	cgltf_attribute_type type;
	int index;
	int target; // 0 = base mesh, 1+ = morph target

	std::vector<Attr> data;
};

struct Transform
{
	float data[16];
};

struct Mesh
{
	std::vector<cgltf_node*> nodes;
	std::vector<Transform> instances;

	cgltf_material* material;
	cgltf_skin* skin;

	cgltf_primitive_type type;

	std::vector<Stream> streams;
	std::vector<unsigned int> indices;

	size_t targets;
	std::vector<float> target_weights;
	std::vector<const char*> target_names;
};

struct Track
{
	cgltf_node* node;
	cgltf_animation_path_type path;

	bool dummy;

	size_t components; // 1 unless path is cgltf_animation_path_type_weights

	cgltf_interpolation_type interpolation;

	std::vector<float> time; // empty for resampled or constant animations
	std::vector<Attr> data;
};

struct Animation
{
	const char* name;

	float start;
	int frames;

	std::vector<Track> tracks;
};

struct Settings
{
	int pos_bits;
	int tex_bits;
	int nrm_bits;

	int trn_bits;
	int rot_bits;
	int scl_bits;

	int anim_freq;
	bool anim_const;

	bool keep_nodes;
	bool keep_materials;
	bool keep_extras;

	bool mesh_merge;
	bool mesh_instancing;

	float simplify_threshold;
	bool simplify_aggressive;
	float simplify_debug;

	int meshlet_debug;

	// TODO: Remove texture_basis and texture_toktx
	bool texture_embed;
	bool texture_basis;
	bool texture_ktx2;
	bool texture_uastc;
	mutable bool texture_toktx;

	int texture_quality;
	float texture_scale;
	bool texture_pow2;

	bool quantize;

	bool compress;
	bool compressmore;
	bool fallback;

	int verbose;
};

struct QuantizationPosition
{
	float offset[3];
	float scale;
	int bits;
};

struct QuantizationTexture
{
	float offset[2];
	float scale[2];
	int bits;
};

struct StreamFormat
{
	enum Filter
	{
		Filter_None = 0,
		Filter_Oct = 1,
		Filter_Quat = 2,
		Filter_Exp = 3,
	};

	cgltf_type type;
	cgltf_component_type component_type;
	bool normalized;
	size_t stride;
	Filter filter;
};

struct NodeInfo
{
	bool keep;
	bool animated;

	unsigned int animated_paths;

	int remap;
	std::vector<size_t> meshes;
};

struct MaterialInfo
{
	bool keep;

	int remap;
};

struct ImageInfo
{
	bool normal_map;
	bool srgb;
};

struct ExtensionInfo
{
	const char* name;

	bool used;
	bool required;
};

struct BufferView
{
	enum Kind
	{
		Kind_Vertex,
		Kind_Index,
		Kind_Skin,
		Kind_Time,
		Kind_Keyframe,
		Kind_Instance,
		Kind_Image,
		Kind_Count
	};

	enum Compression
	{
		Compression_None = -1,
		Compression_Attribute,
		Compression_Index,
		Compression_IndexSequence,
	};

	Kind kind;
	StreamFormat::Filter filter;
	Compression compression;
	size_t stride;
	int variant;

	std::string data;

	size_t bytes;
};

struct TempFile
{
	std::string path;
	int fd;

	TempFile(const char* suffix);
	~TempFile();
};

std::string getFullPath(const char* path, const char* base_path);
std::string getFileName(const char* path);
bool readFile(const char* path, std::string& data);
bool writeFile(const char* path, const std::string& data);

cgltf_data* parseObj(const char* path, std::vector<Mesh>& meshes, const char** error);
cgltf_data* parseGltf(const char* path, std::vector<Mesh>& meshes, std::vector<Animation>& animations, std::string& extras, const char** error);

void processAnimation(Animation& animation, const Settings& settings);
void processMesh(Mesh& mesh, const Settings& settings);

void debugSimplify(const Mesh& mesh, Mesh& kinds, Mesh& loops, float ratio);
void debugMeshlets(const Mesh& mesh, Mesh& meshlets, Mesh& bounds, int max_vertices);

bool compareMeshTargets(const Mesh& lhs, const Mesh& rhs);
bool compareMeshNodes(const Mesh& lhs, const Mesh& rhs);
void mergeMeshInstances(Mesh& mesh);
void mergeMeshes(std::vector<Mesh>& meshes, const Settings& settings);
void filterEmptyMeshes(std::vector<Mesh>& meshes);

bool usesTextureSet(const cgltf_material& material, int set);
void mergeMeshMaterials(cgltf_data* data, std::vector<Mesh>& meshes, const Settings& settings);
void markNeededMaterials(cgltf_data* data, std::vector<MaterialInfo>& materials, const std::vector<Mesh>& meshes, const Settings& settings);

void analyzeImages(cgltf_data* data, std::vector<ImageInfo>& images);
const char* inferMimeType(const char* path);
bool checkBasis(bool verbose);
bool encodeBasis(const std::string& data, const char* mime_type, std::string& result, bool normal_map, bool srgb, int quality, float scale, bool pow2, bool uastc, bool verbose);
std::string basisToKtx(const std::string& data, bool srgb, bool uastc);
bool checkKtx(bool verbose);
bool encodeKtx(const std::string& data, const char* mime_type, std::string& result, bool normal_map, bool srgb, int quality, float scale, bool pow2, bool uastc, bool verbose);

void markAnimated(cgltf_data* data, std::vector<NodeInfo>& nodes, const std::vector<Animation>& animations);
void markNeededNodes(cgltf_data* data, std::vector<NodeInfo>& nodes, const std::vector<Mesh>& meshes, const std::vector<Animation>& animations, const Settings& settings);
void remapNodes(cgltf_data* data, std::vector<NodeInfo>& nodes, size_t& node_offset);
void decomposeTransform(float translation[3], float rotation[4], float scale[3], const float* transform);

QuantizationPosition prepareQuantizationPosition(const std::vector<Mesh>& meshes, const Settings& settings);
void prepareQuantizationTexture(cgltf_data* data, std::vector<QuantizationTexture>& result, const std::vector<Mesh>& meshes, const Settings& settings);
void getPositionBounds(float min[3], float max[3], const Stream& stream, const QuantizationPosition* qp);

StreamFormat writeVertexStream(std::string& bin, const Stream& stream, const QuantizationPosition& qp, const QuantizationTexture& qt, const Settings& settings);
StreamFormat writeIndexStream(std::string& bin, const std::vector<unsigned int>& stream);
StreamFormat writeTimeStream(std::string& bin, const std::vector<float>& data);
StreamFormat writeKeyframeStream(std::string& bin, cgltf_animation_path_type type, const std::vector<Attr>& data, const Settings& settings);

void compressVertexStream(std::string& bin, const std::string& data, size_t count, size_t stride);
void compressIndexStream(std::string& bin, const std::string& data, size_t count, size_t stride);
void compressIndexSequence(std::string& bin, const std::string& data, size_t count, size_t stride);

size_t getBufferView(std::vector<BufferView>& views, BufferView::Kind kind, StreamFormat::Filter filter, BufferView::Compression compression, size_t stride, int variant = 0);

void comma(std::string& s);
void append(std::string& s, size_t v);
void append(std::string& s, float v);
void append(std::string& s, const char* v);
void append(std::string& s, const std::string& v);
void appendJson(std::string& s, const char* begin, const char* end);

const char* attributeType(cgltf_attribute_type type);
const char* animationPath(cgltf_animation_path_type type);

void writeMaterial(std::string& json, const cgltf_data* data, const cgltf_material& material, const QuantizationTexture* qt);
void writeBufferView(std::string& json, BufferView::Kind kind, StreamFormat::Filter filter, size_t count, size_t stride, size_t bin_offset, size_t bin_size, BufferView::Compression compression, size_t compressed_offset, size_t compressed_size);
void writeImage(std::string& json, std::vector<BufferView>& views, const cgltf_image& image, const ImageInfo& info, size_t index, const char* input_path, const char* output_path, const Settings& settings);
void writeTexture(std::string& json, const cgltf_texture& texture, cgltf_data* data, const Settings& settings);
void writeMeshAttributes(std::string& json, std::vector<BufferView>& views, std::string& json_accessors, size_t& accr_offset, const Mesh& mesh, int target, const QuantizationPosition& qp, const QuantizationTexture& qt, const Settings& settings);
size_t writeMeshIndices(std::vector<BufferView>& views, std::string& json_accessors, size_t& accr_offset, const Mesh& mesh, const Settings& settings);
size_t writeJointBindMatrices(std::vector<BufferView>& views, std::string& json_accessors, size_t& accr_offset, const cgltf_skin& skin, const QuantizationPosition& qp, const Settings& settings);
size_t writeInstances(std::vector<BufferView>& views, std::string& json_accessors, size_t& accr_offset, const std::vector<Transform>& transforms, const QuantizationPosition& qp, const Settings& settings);
void writeMeshNode(std::string& json, size_t mesh_offset, cgltf_node* node, cgltf_skin* skin, cgltf_data* data, const QuantizationPosition* qp);
void writeMeshNodeInstanced(std::string& json, size_t mesh_offset, size_t accr_offset);
void writeSkin(std::string& json, const cgltf_skin& skin, size_t matrix_accr, const std::vector<NodeInfo>& nodes, cgltf_data* data);
void writeNode(std::string& json, const cgltf_node& node, const std::vector<NodeInfo>& nodes, cgltf_data* data);
void writeAnimation(std::string& json, std::vector<BufferView>& views, std::string& json_accessors, size_t& accr_offset, const Animation& animation, size_t i, cgltf_data* data, const std::vector<NodeInfo>& nodes, const Settings& settings);
void writeCamera(std::string& json, const cgltf_camera& camera);
void writeLight(std::string& json, const cgltf_light& light);
void writeArray(std::string& json, const char* name, const std::string& contents);
void writeExtensions(std::string& json, const ExtensionInfo* extensions, size_t count);
void writeExtras(std::string& json, const std::string& data, const cgltf_extras& extras);

/**
 * Copyright (c) 2016-2020 Arseny Kapoulkine
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */
