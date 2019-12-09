﻿#include "EffekseerNative.h"  // UE4
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCKAPI_
#define NOMINMAX
#include "EffekseerNative.h"
#include <math.h>
#include <cmath>
#include <float.h>
#include <assert.h>
#include <string>
#include <vector>
#include <map>
#include <list>
#include <set>
#include <queue>
#include <fstream>
#include <memory>
#include <thread>
#include <mutex>
#include <iostream>
#include <array>
#ifdef _WIN32
#include "AllowWindowsPlatformTypes.h"  // UE4
#include <winsock2.h>
#include "HideWindowsPlatformTypes.h"  // UE4
#pragma comment( lib, "ws2_32.lib" )
#else
#include <sys/types.h>
#include <sys/socket.h>
#endif
#undef far

namespace Effekseer
{
void NodeRendererTextureUVTypeParameter::Load(uint8_t*& pos, int32_t version)
{
	memcpy(&Type, pos, sizeof(int));
	pos += sizeof(int);

	if (Type == TextureUVType::Strech)
	{
	}
	else if (Type == TextureUVType::Tile)
	{
		memcpy(&TileEdgeHead, pos, sizeof(TileEdgeHead));
		pos += sizeof(TileEdgeHead);

		memcpy(&TileEdgeTail, pos, sizeof(TileEdgeTail));
		pos += sizeof(TileEdgeTail);

		memcpy(&TileLoopAreaBegin, pos, sizeof(TileLoopAreaBegin));
		pos += sizeof(TileLoopAreaBegin);

		memcpy(&TileLoopAreaEnd, pos, sizeof(TileLoopAreaEnd));
		pos += sizeof(TileLoopAreaEnd);
	}
}
} // namespace Effekseer

namespace Effekseer
{

void* EFK_STDCALL InternalMalloc(unsigned int size) { return (void*)new char*[size]; }

void EFK_STDCALL InternalFree(void* p, unsigned int size)
{
	char* pData = (char*)p;
	delete[] pData;
}

MallocFunc mallocFunc_ = InternalMalloc;

FreeFunc freeFunc_ = InternalFree;

MallocFunc GetMallocFunc() { return mallocFunc_; }

void SetMallocFunc(MallocFunc func) { mallocFunc_ = func; }

FreeFunc GetFreeFunc() { return freeFunc_; }

void SetFreeFunc(FreeFunc func) { freeFunc_ = func; }

} // namespace Effekseer

namespace Effekseer
{

bool Material::Load(const uint8_t* data, int32_t size)
{
	int offset = 0;

	// header
	char prefix[5];

	memcpy(prefix, data + offset, 4);
	offset += sizeof(int);

	prefix[4] = 0;

	if (std::string("EFKM") != std::string(prefix))
		return false;

	int version = 0;
	memcpy(&version, data + offset, 4);
	offset += sizeof(int);

	memcpy(&guid_, data + offset, 8);
	offset += sizeof(uint64_t);

	while (0 <= offset && offset < size)
	{
		char chunk[5];
		memcpy(chunk, data + offset, 4);
		offset += sizeof(int);
		chunk[4] = 0;

		int chunk_size = 0;
		memcpy(&chunk_size, data + offset, 4);
		offset += sizeof(int);

		if (std::string("PRM_") == std::string(chunk))
		{
			memcpy(&shadingModel_, data + offset, 4);
			offset += sizeof(int);

			int hasNormal = 0;
			memcpy(&hasNormal, data + offset, 4);
			offset += sizeof(int);

			int hasRefraction = 0;
			memcpy(&hasRefraction, data + offset, 4);
			offset += sizeof(int);

			hasRefraction_ = hasRefraction > 0;

			memcpy(&customData1Count_, data + offset, 4);
			offset += sizeof(int);

			memcpy(&customData2Count_, data + offset, 4);
			offset += sizeof(int);

			int textureCount = 0;
			memcpy(&textureCount, data + offset, 4);
			offset += sizeof(int);

			for (auto i = 0; i < textureCount; i++)
			{
				int strNameLength = 0;
				memcpy(&strNameLength, data + offset, 4);
				offset += sizeof(int);

				auto name = std::string((const char*)(data + offset));
				offset += strNameLength;

				// name is for human, uniformName is a variable name after 3
				if (version >= 3)
				{
					int strUniformNameLength = 0;
					memcpy(&strUniformNameLength, data + offset, 4);
					offset += sizeof(int);

					name = std::string((const char*)(data + offset));
					offset += strUniformNameLength;
				}

				int strDefaultPathLength = 0;
				memcpy(&strDefaultPathLength, data + offset, 4);
				offset += sizeof(int);

				// defaultpath
				offset += strDefaultPathLength;

				int index = 0;
				memcpy(&index, data + offset, 4);
				offset += sizeof(int);

				// priority
				offset += sizeof(int);

				// param
				offset += sizeof(int);

				// valuetexture
				offset += sizeof(int);

				// sampler
				int sampler = 0;
				memcpy(&sampler, data + offset, 4);
				offset += sizeof(int);

				Texture texture;
				texture.Name = name;
				texture.Index = index;
				texture.Wrap = static_cast<TextureWrapType>(sampler);
				textures_.push_back(texture);
			}

			int uniformCount = 0;
			memcpy(&uniformCount, data + offset, 4);
			offset += sizeof(int);

			for (auto i = 0; i < uniformCount; i++)
			{
				int strLength = 0;
				memcpy(&strLength, data + offset, 4);
				offset += sizeof(int);

				auto name = std::string((const char*)(data + offset));
				offset += strLength;

				// name is for human, uniformName is a variable name after 3
				if (version >= 3)
				{
					int strUniformNameLength = 0;
					memcpy(&strUniformNameLength, data + offset, 4);
					offset += sizeof(int);

					name = std::string((const char*)(data + offset));
					offset += strUniformNameLength;
				}

				// offset
				offset += sizeof(int);

				// priority
				offset += sizeof(int);

				int type = 0;
				memcpy(&type, data + offset, 4);
				offset += sizeof(int);

				// default values
				offset += sizeof(int) * 4;

				Uniform uniform;
				uniform.Name = name;
				uniform.Index = type;
				uniforms_.push_back(uniform);
			}
		}
		else if (std::string("GENE") == std::string(chunk))
		{
			int codeLength = 0;
			memcpy(&codeLength, data + offset, 4);
			offset += sizeof(int);

			auto str = std::string((const char*)(data + offset));
			genericCode_ = str;
			offset += codeLength;
		}
		else
		{
			offset += chunk_size;
		}
	}

	return true;
}

ShadingModelType Material::GetShadingModel() const { return shadingModel_; }

void Material::SetShadingModel(ShadingModelType shadingModel) { shadingModel_ = shadingModel; }

bool Material::GetIsSimpleVertex() const { return isSimpleVertex_; }

void Material::SetIsSimpleVertex(bool isSimpleVertex) { isSimpleVertex_ = isSimpleVertex; }

bool Material::GetHasRefraction() const { return hasRefraction_; }

void Material::SetHasRefraction(bool hasRefraction) { hasRefraction_ = hasRefraction; }

const char* Material::GetGenericCode() const { return genericCode_.c_str(); }

void Material::SetGenericCode(const char* code) { genericCode_ = code; }

uint64_t Material::GetGUID() const { return guid_; }

void Material::SetGUID(uint64_t guid) { guid_ = guid; }

TextureWrapType Material::GetTextureWrap(int32_t index) const { return textures_.at(index).Wrap; }

void Material::SetTextureWrap(int32_t index, TextureWrapType value) { textures_.at(index).Wrap = value; }

int32_t Material::GetTextureIndex(int32_t index) const { return textures_.at(index).Index; }

void Material::SetTextureIndex(int32_t index, int32_t value) { textures_.at(index).Index = value; }

const char* Material::GetTextureName(int32_t index) const { return textures_.at(index).Name.c_str(); }

void Material::SetTextureName(int32_t index, const char* name) { textures_.at(index).Name = name; }

int32_t Material::GetTextureCount() const { return static_cast<int32_t>(textures_.size()); }

void Material::SetTextureCount(int32_t count) { textures_.resize(count); }

int32_t Material::GetUniformIndex(int32_t index) const { return uniforms_.at(index).Index; }

void Material::SetUniformIndex(int32_t index, int32_t value) { uniforms_.at(index).Index = value; }

const char* Material::GetUniformName(int32_t index) const { return uniforms_.at(index).Name.c_str(); }

void Material::SetUniformName(int32_t index, const char* name) { uniforms_.at(index).Name = name; }

int32_t Material::GetUniformCount() const { return static_cast<int32_t>(uniforms_.size()); }

void Material::SetUniformCount(int32_t count) { uniforms_.resize(count); }

int32_t Material::GetCustomData1Count() const
{
	if (customData1Count_ == 0)
		return 0;

	// because opengl doesn't support swizzle with float
	return std::max(customDataMinCount_, customData1Count_);
}

void Material::SetCustomData1Count(int32_t count) { customData1Count_ = count; }

int32_t Material::GetCustomData2Count() const
{
	if (customData2Count_ == 0)
		return 0;

	// because opengl doesn't support swizzle with float
	return std::max(customDataMinCount_, customData2Count_);
}

void Material::SetCustomData2Count(int32_t count) { customData2Count_ = count; }

} // namespace Effekseer

namespace Effekseer
{

}

namespace Effekseer
{

class CompiledMaterialBinaryInternal : public CompiledMaterialBinary, ReferenceObject
{
private:
	std::array<std::vector<uint8_t>, static_cast<int32_t>(MaterialShaderType::Max)> vertexShaders_;

	std::array<std::vector<uint8_t>, static_cast<int32_t>(MaterialShaderType::Max)> pixelShaders_;

public:
	CompiledMaterialBinaryInternal() {}

	virtual ~CompiledMaterialBinaryInternal() {}

	void SetVertexShaderData(MaterialShaderType type, const std::vector<uint8_t>& data)
	{
		vertexShaders_.at(static_cast<int>(type)) = data;
	}

	void SetPixelShaderData(MaterialShaderType type, const std::vector<uint8_t>& data) { pixelShaders_.at(static_cast<int>(type)) = data; }

	const uint8_t* GetVertexShaderData(MaterialShaderType type) const override { return vertexShaders_.at(static_cast<int>(type)).data(); }

	int32_t GetVertexShaderSize(MaterialShaderType type) const override { return vertexShaders_.at(static_cast<int>(type)).size(); }

	const uint8_t* GetPixelShaderData(MaterialShaderType type) const override { return pixelShaders_.at(static_cast<int>(type)).data(); }

	int32_t GetPixelShaderSize(MaterialShaderType type) const override { return pixelShaders_.at(static_cast<int>(type)).size(); }

	int AddRef() override { return ReferenceObject::AddRef(); }

	int Release() override { return ReferenceObject::Release(); }

	int GetRef() override { return ReferenceObject::GetRef(); }
};

const std::vector<uint8_t>& CompiledMaterial::GetOriginalData() const { return originalData; }

bool CompiledMaterial::Load(const uint8_t* data, int32_t size)
{

	int offset = 0;

	// header
	char prefix[5];

	memcpy(prefix, data + offset, 4);
	offset += sizeof(int);

	prefix[4] = 0;

	if (std::string("eMCB") != std::string(prefix))
		return false;

	int version = 0;
	memcpy(&version, data + offset, 4);
	offset += sizeof(int);

	uint64_t guid = 0;
	memcpy(&guid, data + offset, 8);
	offset += sizeof(uint64_t);

	// info
	int32_t platformCount = 0;
	memcpy(&platformCount, data + offset, 4);
	offset += sizeof(uint32_t);

	offset += sizeof(uint32_t) * platformCount;

	// data
	uint32_t originalDataSize = 0;
	memcpy(&originalDataSize, data + offset, 4);
	offset += sizeof(uint32_t);

	originalData.resize(originalDataSize);
	memcpy(originalData.data(), data + offset, originalDataSize);

	offset += originalDataSize;

	while (0 <= offset && offset < size)
	{
		int chunk;
		memcpy(&chunk, data + offset, 4);
		offset += sizeof(int);

		int chunk_size = 0;
		memcpy(&chunk_size, data + offset, 4);
		offset += sizeof(int);

		auto binary = new CompiledMaterialBinaryInternal();

		auto loadFunc = [](const uint8_t* data, std::vector<uint8_t>& buffer, int32_t& offset) {
			int size = 0;
			memcpy(&size, data + offset, 4);
			offset += sizeof(int);

			buffer.resize(size);
			memcpy(buffer.data(), data + offset, size);
			offset += size;
		};

		std::vector<uint8_t> standardVS;
		std::vector<uint8_t> standardPS;
		std::vector<uint8_t> modelVS;
		std::vector<uint8_t> modelPS;
		std::vector<uint8_t> standardRefractionVS;
		std::vector<uint8_t> standardRefractionPS;
		std::vector<uint8_t> modelRefractionVS;
		std::vector<uint8_t> modelRefractionPS;

		loadFunc(data, standardVS, offset);
		loadFunc(data, standardPS, offset);
		loadFunc(data, modelVS, offset);
		loadFunc(data, modelPS, offset);
		loadFunc(data, standardRefractionVS, offset);
		loadFunc(data, standardRefractionPS, offset);
		loadFunc(data, modelRefractionVS, offset);
		loadFunc(data, modelRefractionPS, offset);

		binary->SetVertexShaderData(MaterialShaderType::Standard, standardVS);
		binary->SetPixelShaderData(MaterialShaderType::Standard, standardPS);
		binary->SetVertexShaderData(MaterialShaderType::Model, modelVS);
		binary->SetPixelShaderData(MaterialShaderType::Model, modelPS);
		binary->SetVertexShaderData(MaterialShaderType::Refraction, standardRefractionVS);
		binary->SetPixelShaderData(MaterialShaderType::Refraction, standardRefractionPS);
		binary->SetVertexShaderData(MaterialShaderType::RefractionModel, modelRefractionVS);
		binary->SetPixelShaderData(MaterialShaderType::RefractionModel, modelRefractionPS);

		platforms[static_cast<CompiledMaterialPlatformType>(chunk)] = CreateUniqueReference(static_cast<CompiledMaterialBinary*>(binary));
	}

	return true;
}

void CompiledMaterial::Save(std::vector<uint8_t>& dst, uint64_t guid, std::vector<uint8_t>& originalData)
{
	dst.reserve(1024 * 64);
	int32_t offset = 0;

	struct Header
	{
		char header[4];
		int version = 0;
		uint64_t guid = 0;
	};

	Header h;
	h.header[0] = 'e';
	h.header[1] = 'M';
	h.header[2] = 'C';
	h.header[3] = 'B';
	h.guid = guid;

	dst.resize(sizeof(Header));
	memcpy(dst.data() + offset, &h, sizeof(Header));
	offset = dst.size();

	// info
	int32_t platformCount = platforms.size();
	dst.resize(dst.size() + sizeof(uint32_t));
	memcpy(dst.data() + offset, &platformCount, sizeof(uint32_t));
	offset = dst.size();

	for (auto& kv : platforms)
	{
		auto platform = kv.first;
		dst.resize(dst.size() + sizeof(uint32_t));
		memcpy(dst.data() + offset, &platform, sizeof(uint32_t));
		offset = dst.size();
	}

	// data
	uint32_t originalDataSize = originalData.size();
	dst.resize(dst.size() + sizeof(uint32_t));
	memcpy(dst.data() + offset, &originalDataSize, sizeof(uint32_t));
	offset = dst.size();

	dst.resize(dst.size() + originalData.size());
	memcpy(dst.data() + offset, originalData.data(), originalData.size());
	offset = dst.size();

	// shaders
	for (auto& kv : platforms)
	{
		int32_t bodySize = 0;

		bodySize += sizeof(int) + kv.second->GetVertexShaderSize(MaterialShaderType::Standard);
		bodySize += sizeof(int) + kv.second->GetPixelShaderSize(MaterialShaderType::Standard);
		bodySize += sizeof(int) + kv.second->GetVertexShaderSize(MaterialShaderType::Model);
		bodySize += sizeof(int) + kv.second->GetPixelShaderSize(MaterialShaderType::Model);
		bodySize += sizeof(int) + kv.second->GetVertexShaderSize(MaterialShaderType::Refraction);
		bodySize += sizeof(int) + kv.second->GetPixelShaderSize(MaterialShaderType::Refraction);
		bodySize += sizeof(int) + kv.second->GetVertexShaderSize(MaterialShaderType::RefractionModel);
		bodySize += sizeof(int) + kv.second->GetPixelShaderSize(MaterialShaderType::RefractionModel);

		dst.resize(dst.size() + sizeof(int));
		memcpy(dst.data() + offset, &(kv.first), sizeof(int));
		offset = dst.size();

		dst.resize(dst.size() + sizeof(int));
		memcpy(dst.data() + offset, &(bodySize), sizeof(int));
		offset = dst.size();

		std::array<const uint8_t*, 8> bodies = {
			kv.second->GetVertexShaderData(MaterialShaderType::Standard),
			kv.second->GetPixelShaderData(MaterialShaderType::Standard),
			kv.second->GetVertexShaderData(MaterialShaderType::Model),
			kv.second->GetPixelShaderData(MaterialShaderType::Model),
			kv.second->GetVertexShaderData(MaterialShaderType::Refraction),
			kv.second->GetPixelShaderData(MaterialShaderType::Refraction),
			kv.second->GetVertexShaderData(MaterialShaderType::RefractionModel),
			kv.second->GetPixelShaderData(MaterialShaderType::RefractionModel),
		};

		std::array<int32_t, 8> bodySizes = {
			kv.second->GetVertexShaderSize(MaterialShaderType::Standard),
			kv.second->GetPixelShaderSize(MaterialShaderType::Standard),
			kv.second->GetVertexShaderSize(MaterialShaderType::Model),
			kv.second->GetPixelShaderSize(MaterialShaderType::Model),
			kv.second->GetVertexShaderSize(MaterialShaderType::Refraction),
			kv.second->GetPixelShaderSize(MaterialShaderType::Refraction),
			kv.second->GetVertexShaderSize(MaterialShaderType::RefractionModel),
			kv.second->GetPixelShaderSize(MaterialShaderType::RefractionModel),
		};

		for (size_t i = 0; i < 8; i++)
		{
			int32_t bodySize = bodySizes[i];

			dst.resize(dst.size() + sizeof(int));
			memcpy(dst.data() + offset, &(bodySize), sizeof(int));
			offset = dst.size();

			dst.resize(dst.size() + bodySize);
			memcpy(dst.data() + offset, bodies[i], bodySize);
			offset = dst.size();
		}
	}
}

bool CompiledMaterial::GetHasValue(CompiledMaterialPlatformType type) const
{
	auto it = platforms.find(type);
	if (it == platforms.end())
		return false;

	// TODO improve it
	return it->second->GetVertexShaderSize(MaterialShaderType::Standard) > 0;
}

CompiledMaterialBinary* CompiledMaterial::GetBinary(CompiledMaterialPlatformType type) const
{

	auto it = platforms.find(type);
	if (it == platforms.end())
		return nullptr;

	return it->second.get();
}

void CompiledMaterial::UpdateData(const std::vector<uint8_t>& standardVS,
								  const std::vector<uint8_t>& standardPS,
								  const std::vector<uint8_t>& modelVS,
								  const std::vector<uint8_t>& modelPS,
								  const std::vector<uint8_t>& standardRefractionVS,
								  const std::vector<uint8_t>& standardRefractionPS,
								  const std::vector<uint8_t>& modelRefractionVS,
								  const std::vector<uint8_t>& modelRefractionPS,
								  CompiledMaterialPlatformType type)
{
	auto binary = new CompiledMaterialBinaryInternal();

	binary->SetVertexShaderData(MaterialShaderType::Standard, standardVS);
	binary->SetPixelShaderData(MaterialShaderType::Standard, standardPS);
	binary->SetVertexShaderData(MaterialShaderType::Model, modelVS);
	binary->SetPixelShaderData(MaterialShaderType::Model, modelPS);
	binary->SetVertexShaderData(MaterialShaderType::Refraction, standardRefractionVS);
	binary->SetPixelShaderData(MaterialShaderType::Refraction, standardRefractionPS);
	binary->SetVertexShaderData(MaterialShaderType::RefractionModel, modelRefractionVS);
	binary->SetPixelShaderData(MaterialShaderType::RefractionModel, modelRefractionPS);

	platforms[type] = CreateUniqueReference(static_cast<CompiledMaterialBinary*>(binary));
}

} // namespace Effekseer

namespace Effekseer
{

bool EfkEfcFactory::OnLoading(Effect* effect, const void* data, int32_t size, float magnification, const EFK_CHAR* materialPath)
{
	BinaryReader<true> binaryReader(const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(data)), size);

	// EFKP
	int head = 0;
	binaryReader.Read(head);
	if (memcmp(&head, "EFKE", 4) != 0)
		return false;

	int32_t version = 0;

	binaryReader.Read(version);

	// load chunk
	while (binaryReader.GetOffset() < size)
	{
		int chunk = 0;
		binaryReader.Read(chunk);
		int chunkSize = 0;
		binaryReader.Read(chunkSize);

		if (memcmp(&chunk, "INFO", 4) == 0)
		{
		}

		if (memcmp(&chunk, "EDIT", 4) == 0)
		{
		}

		if (memcmp(&chunk, "BIN_", 4) == 0)
		{
			return LoadBody(
				effect, reinterpret_cast<const uint8_t*>(data) + binaryReader.GetOffset(), chunkSize, magnification, materialPath);
		}

		binaryReader.AddOffset(chunkSize);
	}

	return false;
}

bool EfkEfcFactory::OnCheckIsBinarySupported(const void* data, int32_t size)
{
	BinaryReader<true> binaryReader(const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(data)), size);

	// EFKP
	int head = 0;
	binaryReader.Read(head);
	if (memcmp(&head, "EFKE", 4) != 0)
		return false;

	return true;
}

bool EfkEfcProperty::Load(const void* data, int32_t size)
{
	BinaryReader<true> binaryReader(const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(data)), size);

	// EFKP
	int head = 0;
	binaryReader.Read(head);
	if (memcmp(&head, "EFKE", 4) != 0)
		return false;

	int32_t version = 0;

	binaryReader.Read(version);

	// load chunk
	while (binaryReader.GetOffset() < size)
	{
		int chunk = 0;
		binaryReader.Read(chunk);
		int chunkSize = 0;
		binaryReader.Read(chunkSize);

		if (memcmp(&chunk, "INFO", 4) == 0)
		{
			int32_t version = 0;

			auto loadStr = [this, &binaryReader, &version](std::vector<std::u16string>& dst) {
				int32_t dataCount = 0;
				binaryReader.Read(dataCount);

				// compatibility
				if (dataCount >= 1500)
				{
					version = dataCount;
					binaryReader.Read(dataCount);
				}

				dst.resize(dataCount);

				std::vector<char16_t> strBuf;

				for (int i = 0; i < dataCount; i++)
				{
					int length = 0;
					binaryReader.Read(length);
					strBuf.resize(length);
					binaryReader.Read(strBuf.data(), length);
					dst.at(i) = strBuf.data();
				}
			};

			loadStr(colorImages_);
			loadStr(normalImages_);
			loadStr(distortionImages_);
			loadStr(models_);
			loadStr(sounds_);

			if (version >= 1500)
			{
				loadStr(materials_);
			}
		}

		binaryReader.AddOffset(chunkSize);
	}

	return false;
}

const std::vector<std::u16string>& EfkEfcProperty::GetColorImages() const { return colorImages_; }
const std::vector<std::u16string>& EfkEfcProperty::GetNormalImages() const { return normalImages_; }
const std::vector<std::u16string>& EfkEfcProperty::GetDistortionImages() const { return distortionImages_; }
const std::vector<std::u16string>& EfkEfcProperty::GetSounds() const { return sounds_; }
const std::vector<std::u16string>& EfkEfcProperty::GetModels() const { return models_; }
const std::vector<std::u16string>& EfkEfcProperty::GetMaterials() const { return materials_; }

} // namespace Effekseer

#ifndef	__CULLING3D_CULLING3D_H__
#define	__CULLING3D_CULLING3D_H__



namespace Culling3D
{
	/**
	@brief	最大値取得
	*/
	template <typename T, typename U>
	T Max(T t, U u)
	{
		if (t > (T) u)
		{
			return t;
		}
		return u;
	}

	/**
	@brief	最小値取得
	*/
	template <typename T, typename U>
	T Min(T t, U u)
	{
		if (t < (T) u)
		{
			return t;
		}
		return u;
	}

	/**
	@brief	範囲内値取得
	*/
	template <typename T, typename U, typename V>
	T Clamp(T t, U max_, V min_)
	{
		if (t > (T) max_)
		{
			t = (T) max_;
		}

		if (t < (T) min_)
		{
			t = (T) min_;
		}

		return t;
	}

	template <class T>
	void SafeAddRef(T& t)
	{
		if (t != NULL)
		{
			t->AddRef();
		}
	}

	template <class T>
	void SafeRelease(T& t)
	{
		if (t != NULL)
		{
			t->Release();
			t = NULL;
		}
	}

	template <class T>
	void SafeSubstitute(T& target, T& value)
	{
		SafeAddRef(value);
		SafeRelease(target);
		target = value;
	}

	template <typename T>
	inline void SafeDelete(T*& p)
	{
		if (p != NULL)
		{
			delete (p);
			(p) = NULL;
		}
	}

	template <typename T>
	inline void SafeDeleteArray(T*& p)
	{
		if (p != NULL)
		{
			delete [](p);
			(p) = NULL;
		}
	}

	class World;
	class Object;

	struct Vector3DF
	{
		float	X;
		float	Y;
		float	Z;

		Vector3DF();
		Vector3DF(float x, float y, float z);

		bool operator == (const Vector3DF& o);
		bool operator != (const Vector3DF& o);

		Vector3DF operator-();

		Vector3DF operator + (const Vector3DF& o) const;

		Vector3DF operator - (const Vector3DF& o) const;

		Vector3DF operator * (const Vector3DF& o) const;

		Vector3DF operator / (const Vector3DF& o) const;

		Vector3DF operator * (const float& o) const;

		Vector3DF operator / (const float& o) const;

		Vector3DF& operator += (const Vector3DF& o);

		Vector3DF& operator -= (const Vector3DF& o);

		Vector3DF& operator *= (const Vector3DF& o);

		Vector3DF& operator /= (const Vector3DF& o);

		Vector3DF& operator *= (const float& o);

		Vector3DF& operator /= (const float& o);

		/**
		@brief	このベクトルの長さを取得する。
		*/
		float GetLength() const
		{
			return sqrtf(GetSquaredLength());
		}

		/**
		@brief	このベクトルの長さの二乗を取得する。
		*/
		float GetSquaredLength() const
		{
			return X * X + Y * Y + Z * Z;
		}

		/**
		@brief	このベクトルの長さを設定する。
		*/
		void SetLength(float value)
		{
			float length = GetLength();
			(*this) *= (value / length);
		}

		/**
		@brief	このベクトルの単位ベクトルを取得する。
		*/
		Vector3DF GetNormal()
		{
			float length = GetLength();
			return Vector3DF(X / length, Y / length, Z / length);
		}

		/**
		@brief	このベクトルの単位ベクトル化する。
		*/
		void Normalize()
		{
			float length = GetLength();
			(*this) /= length;
		}

		/**
		@brief	内積を取得する。
		*/
		static float Dot(const Vector3DF& v1, const Vector3DF& v2);

		/**
		@brief	外積を取得する。
		@note
		右手系の場合、右手の親指がv1、人差し指がv2としたとき、中指の方向を返す。<BR>
		左手系の場合、左手の親指がv1、人差し指がv2としたとき、中指の方向を返す。<BR>
		*/
		static Vector3DF Cross(const Vector3DF& v1, const Vector3DF& v2);

		/**
		@brief	2点間の距離を取得する。
		*/
		static float Distance(const Vector3DF& v1, const Vector3DF& v2);
	};
	
	struct Matrix44
	{
		float	Values[4][4];

		Matrix44();
		Matrix44& SetInverted();
		Vector3DF Transform3D(const Vector3DF& in) const;

		/**
		@brief	カメラ行列(右手系)を設定する。
		@param	eye	カメラの位置
		@param	at	カメラの注視点
		@param	up	カメラの上方向
		@return	このインスタンスへの参照
		*/
		Matrix44& SetLookAtRH(const Vector3DF& eye, const Vector3DF& at, const Vector3DF& up);

		/**
		@brief	カメラ行列(左手系)を設定する。
		@param	eye	カメラの位置
		@param	at	カメラの注視点
		@param	up	カメラの上方向
		@return	このインスタンスへの参照
		*/
		Matrix44& SetLookAtLH(const Vector3DF& eye, const Vector3DF& at, const Vector3DF& up);

		/**
		@brief	射影行列(右手系)を設定する。
		@param	ovY	Y方向への視野角(ラジアン)
		@param	aspect	画面のアスペクト比
		@param	zn	最近距離
		@param	zf	最遠距離
		@return	このインスタンスへの参照
		*/
		Matrix44& SetPerspectiveFovRH(float ovY, float aspect, float zn, float zf);

		/**
		@brief	OpenGL用射影行列(右手系)を設定する。
		@param	ovY	Y方向への視野角(ラジアン)
		@param	aspect	画面のアスペクト比
		@param	zn	最近距離
		@param	zf	最遠距離
		@return	このインスタンスへの参照
		*/
		Matrix44& SetPerspectiveFovRH_OpenGL(float ovY, float aspect, float zn, float zf);

		/**
		@brief	射影行列(左手系)を設定する。
		@param	ovY	Y方向への視野角(ラジアン)
		@param	aspect	画面のアスペクト比
		@param	zn	最近距離
		@param	zf	最遠距離
		@return	このインスタンスへの参照
		*/
		Matrix44& SetPerspectiveFovLH(float ovY, float aspect, float zn, float zf);

		/**
		@brief	正射影行列(右手系)を設定する。
		@param	width	横幅
		@param	height	縦幅
		@param	zn	最近距離
		@param	zf	最遠距離
		@return	このインスタンスへの参照
		*/
		Matrix44& SetOrthographicRH(float width, float height, float zn, float zf);

		/**
		@brief	正射影行列(左手系)を設定する。
		@param	width	横幅
		@param	height	縦幅
		@param	zn	最近距離
		@param	zf	最遠距離
		@return	このインスタンスへの参照
		*/
		Matrix44& SetOrthographicLH(float width, float height, float zn, float zf);

		Matrix44 operator * (const Matrix44& right) const;

		Vector3DF operator*(const Vector3DF& right) const;

		/**
		@brief	乗算を行う。
		@param	o	出力先
		@param	in1	行列1
		@param	in2	行列2
		@return	出力先の参照
		*/
		static Matrix44& Mul(Matrix44& o, const Matrix44& in1, const Matrix44& in2);
	};

	enum eObjectShapeType
	{
		OBJECT_SHAPE_TYPE_NONE,
		OBJECT_SHAPE_TYPE_SPHERE,
		OBJECT_SHAPE_TYPE_CUBOID,
		OBJECT_SHAPE_TYPE_ALL,
	};


	class IReference
	{
	public:
		/**
		@brief	参照カウンタを加算する。
		@return	加算後の参照カウンタ
		*/
		virtual int AddRef() = 0;

		/**
		@brief	参照カウンタを取得する。
		@return	参照カウンタ
		*/
		virtual int GetRef() = 0;

		/**
		@brief	参照カウンタを減算する。0になった時、インスタンスを削除する。
		@return	減算後の参照カウンタ
		*/
		virtual int Release() = 0;
	};

	class World
		: public IReference
	{
	public:
		virtual void AddObject(Object* o) = 0;
		virtual void RemoveObject(Object* o) = 0;

		virtual void CastRay(Vector3DF from, Vector3DF to) = 0;

		virtual void Culling(const Matrix44& cameraProjMat, bool isOpenGL) = 0;
		virtual int32_t GetObjectCount() = 0;
		virtual Object* GetObject(int32_t index) = 0;

		virtual bool Reassign() = 0;

		virtual void Dump(const char* path, const Matrix44& cameraProjMat, bool isOpenGL) = 0;

		static World* Create(float xSize, float ySize, float zSize, int32_t layerCount);
	};

	class Object
		: public IReference
	{
	public:
		virtual Vector3DF GetPosition() = 0;
		virtual void SetPosition(Vector3DF pos) = 0;
		virtual void ChangeIntoAll() = 0;
		virtual void ChangeIntoSphere(float radius) = 0;
		virtual void ChangeIntoCuboid(Vector3DF size) = 0;

		virtual void* GetUserData() = 0;
		virtual void SetUserData(void* data) = 0;

		static Object* Create();
	};
}

#endif




namespace Culling3D
{
	class ReferenceObject
		: public IReference
	{
	private:
		int32_t	m_reference;

	public:
		ReferenceObject();

		virtual ~ReferenceObject();

		virtual int32_t AddRef();

		virtual int32_t GetRef();

		virtual int32_t Release();
	};
}






namespace Culling3D
{
	class Grid
	{
	private:
		std::vector<Object*>	objects;

	public:
		Grid();

		void AddObject(Object* o);

		void RemoveObject(Object* o);

		std::vector<Object*>& GetObjects() { return objects; }

		bool IsScanned;
	};
}




namespace Culling3D
{
	class Layer
	{
	private:
		int32_t		gridXCount;
		int32_t		gridYCount;
		int32_t		gridZCount;

		float		offsetX;
		float		offsetY;
		float		offsetZ;


		float		gridSize;
		std::vector<Grid>	grids;

	public:
		Layer(int32_t gridXCount, int32_t gridYCount, int32_t gridZCount, float offsetX, float offsetY, float offsetZ, float gridSize);
		virtual ~Layer();

		bool AddObject(Object* o);

		bool RemoveObject(Object* o);

		void AddGrids(Vector3DF max_, Vector3DF min_, std::vector<Grid*>& grids_);

		int32_t GetGridXCount() { return gridXCount; }
		int32_t GetGridYCount() { return gridYCount; }
		int32_t GetGridZCount() { return gridZCount; }

		float GetOffsetX() { return offsetX; }
		float GetOffsetY() { return offsetY; }
		float GetOffsetZ() { return offsetZ; }

		float GetGridSize() { return gridSize; }
		std::vector<Grid>& GetGrids() { return grids; }
	};
}



namespace Culling3D
{
	class ObjectInternal
		: public Object
		, public ReferenceObject
	{
	public:
		struct Status
		{
			Vector3DF	Position;

			union
			{
				struct
				{
					float Radius;
				} Sphere;

				struct
				{
					float X;
					float Y;
					float Z;
				} Cuboid;
			} Data;

			float		radius;
			eObjectShapeType	Type;

			void CalcRadius()
			{
				radius = 0.0f;
				if (Type == OBJECT_SHAPE_TYPE_NONE) radius = 0.0f;
				if (Type == OBJECT_SHAPE_TYPE_SPHERE) radius = Data.Sphere.Radius;
				if (Type == OBJECT_SHAPE_TYPE_CUBOID) radius = sqrtf(Data.Cuboid.X * Data.Cuboid.X + Data.Cuboid.Y * Data.Cuboid.Y + Data.Cuboid.Z * Data.Cuboid.Z) / 2.0f;
			}

			float GetRadius()
			{
				return radius;
			}
		};

	private:
		void*		userData;
		World*		world;

		Status	currentStatus;
		Status	nextStatus;

	public:
		ObjectInternal();
		virtual ~ObjectInternal();

		Vector3DF GetPosition() override;
		void SetPosition(Vector3DF pos) override;

		void ChangeIntoAll() override;

		void ChangeIntoSphere(float radius) override;

		void ChangeIntoCuboid(Vector3DF size) override;

		void* GetUserData() override;
		void SetUserData(void* userData_) override;

		void SetWorld(World* world_);

		Status	GetCurrentStatus() { return currentStatus; }
		Status	GetNextStatus() { return nextStatus; }

		int32_t ObjectIndex;

		virtual int32_t GetRef() override { return ReferenceObject::GetRef(); }
		virtual int32_t AddRef() override { return ReferenceObject::AddRef(); }
		virtual int32_t Release() override { return ReferenceObject::Release(); }
	};
}






namespace Culling3D
{
	class WorldInternal
		: public World
		, public ReferenceObject
	{
	private:
		float xSize;
		float ySize;
		float zSize;

		float	gridSize;
		float	minGridSize;
		int32_t	layerCount;

		std::vector<Layer*>	layers;

		Grid	outofLayers;
		Grid	allLayers;

		std::vector<Object*> objs;

		std::vector<Grid*> grids;

		std::set<Object*>	containedObjects;

	public:
		WorldInternal(float xSize, float ySize, float zSize, int32_t layerCount);
		virtual ~WorldInternal();

		void AddObject(Object* o) override;
		void RemoveObject(Object* o) override;

		void AddObjectInternal(Object* o);
		void RemoveObjectInternal(Object* o);

		void CastRay(Vector3DF from, Vector3DF to) override;

		void Culling(const Matrix44& cameraProjMat, bool isOpenGL) override;

		bool Reassign() override;

		void Dump(const char* path, const Matrix44& cameraProjMat, bool isOpenGL) override;

		int32_t GetObjectCount() override { return (int32_t)objs.size(); }
		Object* GetObject(int32_t index) override { return objs[index]; }

		virtual int32_t GetRef() override { return ReferenceObject::GetRef(); }
		virtual int32_t AddRef() override { return ReferenceObject::AddRef(); }
		virtual int32_t Release() override { return ReferenceObject::Release(); }
	};
}



namespace Culling3D
{
	Grid::Grid()
	{
		IsScanned = false;
	}

	void Grid::AddObject(Object* o)
	{
		assert(o != NULL);

		ObjectInternal* o_ = (ObjectInternal*) o;
		assert(o_->ObjectIndex == -1);

		objects.push_back(o_);
		o_->ObjectIndex = (int32_t)objects.size() - 1;
	}

	void Grid::RemoveObject(Object* o)
	{
		assert(o != NULL);

		ObjectInternal* o_ = (ObjectInternal*) o;
		assert(o_->ObjectIndex != -1);

		if (objects.size() == 1)
		{
			objects.clear();
		}
		else if (objects.size() - 1 == o_->ObjectIndex)
		{
			objects.resize(objects.size() - 1);
		}
		else
		{
			ObjectInternal* moved = (ObjectInternal*) objects[objects.size() - 1];
			moved->ObjectIndex = o_->ObjectIndex;
			objects[o_->ObjectIndex] = moved;
			objects.resize(objects.size() - 1);
		}

		o_->ObjectIndex = -1;
	}
}



namespace Culling3D
{
	Layer::Layer(int32_t gridXCount, int32_t gridYCount, int32_t gridZCount, float offsetX, float offsetY, float offsetZ, float gridSize)
	{
		this->gridXCount = gridXCount;
		this->gridYCount = gridYCount;
		this->gridZCount = gridZCount;
		this->offsetX = offsetX;
		this->offsetY = offsetY;
		this->offsetZ = offsetZ;
		this->gridSize = gridSize;

		grids.resize(this->gridXCount * this->gridYCount * this->gridZCount);
	}

	Layer::~Layer()
	{

	}

	bool Layer::AddObject(Object* o)
	{
		assert(o != NULL);

		ObjectInternal* o_ = (ObjectInternal*) o;

		float x = o_->GetNextStatus().Position.X + offsetX;
		float y = o_->GetNextStatus().Position.Y + offsetY;
		float z = o_->GetNextStatus().Position.Z + offsetZ;

		int32_t xind = (int32_t)(x / gridSize);
		int32_t yind = (int32_t)(y / gridSize);
		int32_t zind = (int32_t)(z / gridSize);

		int32_t ind = xind + yind * this->gridXCount + zind * this->gridXCount * this->gridYCount;

		if (xind < 0 ||
			xind >= this->gridXCount ||
			yind < 0 ||
			yind >= this->gridYCount ||
			zind < 0 ||
			zind >= this->gridZCount) return false;

		if (ind < 0 || ind >= (int32_t)grids.size()) return false;

		grids[ind].AddObject(o);

		return true;
	}

	bool Layer::RemoveObject(Object* o)
	{
		assert(o != NULL);

		ObjectInternal* o_ = (ObjectInternal*) o;

		float x = o_->GetCurrentStatus().Position.X + offsetX;
		float y = o_->GetCurrentStatus().Position.Y + offsetY;
		float z = o_->GetCurrentStatus().Position.Z + offsetZ;

		int32_t xind = (int32_t) (x / gridSize);
		int32_t yind = (int32_t) (y / gridSize);
		int32_t zind = (int32_t) (z / gridSize);

		int32_t ind = xind + yind * this->gridXCount + zind * this->gridXCount * this->gridYCount;

		if (xind < 0 ||
			xind >= this->gridXCount ||
			yind < 0 ||
			yind >= this->gridYCount ||
			zind < 0 ||
			zind >= this->gridZCount) return false;

		if (ind < 0 || ind >= (int32_t) grids.size()) return false;

		grids[ind].RemoveObject(o);

		return true;
	}

	void Layer::AddGrids(Vector3DF max_, Vector3DF min_, std::vector<Grid*>& grids_)
	{
		int32_t maxX = (int32_t) ((max_.X + offsetX) / gridSize) + 1;
		int32_t maxY = (int32_t) ((max_.Y + offsetY) / gridSize) + 1;
		int32_t maxZ = (int32_t) ((max_.Z + offsetZ) / gridSize) + 1;

		int32_t minX = (int32_t) ((min_.X + offsetX) / gridSize) - 1;
		int32_t minY = (int32_t) ((min_.Y + offsetY) / gridSize) - 1;
		int32_t minZ = (int32_t) ((min_.Z + offsetZ) / gridSize) - 1;

		maxX = Clamp(maxX, gridXCount - 1, 0);
		maxY = Clamp(maxY, gridYCount - 1, 0);
		maxZ = Clamp(maxZ, gridZCount - 1, 0);

		minX = Clamp(minX, gridXCount - 1, 0);
		minY = Clamp(minY, gridYCount - 1, 0);
		minZ = Clamp(minZ, gridZCount - 1, 0);

		for (int32_t z = minZ; z <= maxZ; z++)
		{
			for (int32_t y = minY; y <= maxY; y++)
			{
				for (int32_t x = minX; x <= maxX; x++)
				{
					int32_t ind = x + y * this->gridXCount + z * this->gridXCount * this->gridYCount;

					if (!grids[ind].IsScanned)
					{
						grids_.push_back(&grids[ind]);
						grids[ind].IsScanned = true;
					}
				}
			}
		}
	}
}


namespace Culling3D
{
	Matrix44::Matrix44()
	{
		for (int32_t c = 0; c < 4; c++)
		{
			for (int32_t r = 0; r < 4; r++)
			{
				Values[c][r] = 0.0f;
			}
		}

		for (int32_t i = 0; i < 4; i++)
		{
			Values[i][i] = 1.0f;
		}
	}

	Matrix44& Matrix44::SetInverted()
	{
		float a11 = this->Values[0][0];
		float a12 = this->Values[0][1];
		float a13 = this->Values[0][2];
		float a14 = this->Values[0][3];
		float a21 = this->Values[1][0];
		float a22 = this->Values[1][1];
		float a23 = this->Values[1][2];
		float a24 = this->Values[1][3];
		float a31 = this->Values[2][0];
		float a32 = this->Values[2][1];
		float a33 = this->Values[2][2];
		float a34 = this->Values[2][3];
		float a41 = this->Values[3][0];
		float a42 = this->Values[3][1];
		float a43 = this->Values[3][2];
		float a44 = this->Values[3][3];

		/* 行列式の計算 */
		float b11 = +a22 * (a33 * a44 - a43 * a34) - a23 * (a32 * a44 - a42 * a34) + a24 * (a32 * a43 - a42 * a33);
		float b12 = -a12 * (a33 * a44 - a43 * a34) + a13 * (a32 * a44 - a42 * a34) - a14 * (a32 * a43 - a42 * a33);
		float b13 = +a12 * (a23 * a44 - a43 * a24) - a13 * (a22 * a44 - a42 * a24) + a14 * (a22 * a43 - a42 * a23);
		float b14 = -a12 * (a23 * a34 - a33 * a24) + a13 * (a22 * a34 - a32 * a24) - a14 * (a22 * a33 - a32 * a23);

		float b21 = -a21 * (a33 * a44 - a43 * a34) + a23 * (a31 * a44 - a41 * a34) - a24 * (a31 * a43 - a41 * a33);
		float b22 = +a11 * (a33 * a44 - a43 * a34) - a13 * (a31 * a44 - a41 * a34) + a14 * (a31 * a43 - a41 * a33);
		float b23 = -a11 * (a23 * a44 - a43 * a24) + a13 * (a21 * a44 - a41 * a24) - a14 * (a21 * a43 - a41 * a23);
		float b24 = +a11 * (a23 * a34 - a33 * a24) - a13 * (a21 * a34 - a31 * a24) + a14 * (a21 * a33 - a31 * a23);

		float b31 = +a21 * (a32 * a44 - a42 * a34) - a22 * (a31 * a44 - a41 * a34) + a24 * (a31 * a42 - a41 * a32);
		float b32 = -a11 * (a32 * a44 - a42 * a34) + a12 * (a31 * a44 - a41 * a34) - a14 * (a31 * a42 - a41 * a32);
		float b33 = +a11 * (a22 * a44 - a42 * a24) - a12 * (a21 * a44 - a41 * a24) + a14 * (a21 * a42 - a41 * a22);
		float b34 = -a11 * (a22 * a34 - a32 * a24) + a12 * (a21 * a34 - a31 * a24) - a14 * (a21 * a32 - a31 * a22);

		float b41 = -a21 * (a32 * a43 - a42 * a33) + a22 * (a31 * a43 - a41 * a33) - a23 * (a31 * a42 - a41 * a32);
		float b42 = +a11 * (a32 * a43 - a42 * a33) - a12 * (a31 * a43 - a41 * a33) + a13 * (a31 * a42 - a41 * a32);
		float b43 = -a11 * (a22 * a43 - a42 * a23) + a12 * (a21 * a43 - a41 * a23) - a13 * (a21 * a42 - a41 * a22);
		float b44 = +a11 * (a22 * a33 - a32 * a23) - a12 * (a21 * a33 - a31 * a23) + a13 * (a21 * a32 - a31 * a22);

		// 行列式の逆数をかける
		float Det = (a11 * b11) + (a12 * b21) + (a13 * b31) + (a14 * b41);
		if ((-FLT_MIN <= Det) && (Det <= +FLT_MIN))
		{
			return *this;
		}

		float InvDet = 1.0f / Det;

		Values[0][0] = b11 * InvDet;
		Values[0][1] = b12 * InvDet;
		Values[0][2] = b13 * InvDet;
		Values[0][3] = b14 * InvDet;
		Values[1][0] = b21 * InvDet;
		Values[1][1] = b22 * InvDet;
		Values[1][2] = b23 * InvDet;
		Values[1][3] = b24 * InvDet;
		Values[2][0] = b31 * InvDet;
		Values[2][1] = b32 * InvDet;
		Values[2][2] = b33 * InvDet;
		Values[2][3] = b34 * InvDet;
		Values[3][0] = b41 * InvDet;
		Values[3][1] = b42 * InvDet;
		Values[3][2] = b43 * InvDet;
		Values[3][3] = b44 * InvDet;

		return *this;
	}

	Matrix44& Matrix44::SetLookAtRH(const Vector3DF& eye, const Vector3DF& at, const Vector3DF& up)
	{
		// F=正面、R=右方向、U=上方向
		Vector3DF F = (eye - at).GetNormal();
		Vector3DF R = Vector3DF::Cross(up, F).GetNormal();
		Vector3DF U = Vector3DF::Cross(F, R).GetNormal();

		Values[0][0] = R.X;
		Values[0][1] = R.Y;
		Values[0][2] = R.Z;
		Values[0][3] = 0.0f;

		Values[1][0] = U.X;
		Values[1][1] = U.Y;
		Values[1][2] = U.Z;
		Values[1][3] = 0.0f;

		Values[2][0] = F.X;
		Values[2][1] = F.Y;
		Values[2][2] = F.Z;
		Values[2][3] = 0.0f;

		Values[0][3] = -Vector3DF::Dot(R, eye);
		Values[1][3] = -Vector3DF::Dot(U, eye);
		Values[2][3] = -Vector3DF::Dot(F, eye);
		Values[3][3] = 1.0f;
		return *this;
	}

	Matrix44& Matrix44::SetLookAtLH(const Vector3DF& eye, const Vector3DF& at, const Vector3DF& up)
	{
		// F=正面、R=右方向、U=上方向
		Vector3DF F = (at - eye).GetNormal();
		Vector3DF R = Vector3DF::Cross(up, F).GetNormal();
		Vector3DF U = Vector3DF::Cross(F, R).GetNormal();

		Values[0][0] = R.X;
		Values[0][1] = R.Y;
		Values[0][2] = R.Z;
		Values[0][3] = 0.0f;

		Values[1][0] = U.X;
		Values[1][1] = U.Y;
		Values[1][2] = U.Z;
		Values[1][3] = 0.0f;

		Values[2][0] = F.X;
		Values[2][1] = F.Y;
		Values[2][2] = F.Z;
		Values[2][3] = 0.0f;

		Values[0][3] = -Vector3DF::Dot(R, eye);
		Values[1][3] = -Vector3DF::Dot(U, eye);
		Values[2][3] = -Vector3DF::Dot(F, eye);
		Values[3][3] = 1.0f;
		return *this;
	}

	Matrix44& Matrix44::SetPerspectiveFovRH(float ovY, float aspect, float zn, float zf)
	{
		float yScale = 1 / tanf(ovY / 2);
		float xScale = yScale / aspect;

		Values[0][0] = xScale;
		Values[1][0] = 0;
		Values[2][0] = 0;
		Values[3][0] = 0;

		Values[0][1] = 0;
		Values[1][1] = yScale;
		Values[2][1] = 0;
		Values[3][1] = 0;

		Values[0][2] = 0;
		Values[1][2] = 0;
		Values[2][2] = zf / (zn - zf);
		Values[3][2] = -1;

		Values[0][3] = 0;
		Values[1][3] = 0;
		Values[2][3] = zn * zf / (zn - zf);
		Values[3][3] = 0;
		return *this;
	}

	Matrix44& Matrix44::SetPerspectiveFovRH_OpenGL(float ovY, float aspect, float zn, float zf)
	{
		float yScale = 1 / tanf(ovY / 2);
		float xScale = yScale / aspect;
		float dz = zf - zn;

		Values[0][0] = xScale;
		Values[1][0] = 0;
		Values[2][0] = 0;
		Values[3][0] = 0;

		Values[0][1] = 0;
		Values[1][1] = yScale;
		Values[2][1] = 0;
		Values[3][1] = 0;

		Values[0][2] = 0;
		Values[1][2] = 0;
		Values[2][2] = -(zf + zn) / dz;
		Values[3][2] = -1.0f;

		Values[0][3] = 0;
		Values[1][3] = 0;
		Values[2][3] = -2.0f * zn * zf / dz;
		Values[3][3] = 0.0f;

		return *this;
	}

	Matrix44& Matrix44::SetPerspectiveFovLH(float ovY, float aspect, float zn, float zf)
	{
		float yScale = 1 / tanf(ovY / 2);
		float xScale = yScale / aspect;

		Values[0][0] = xScale;
		Values[1][0] = 0;
		Values[2][0] = 0;
		Values[3][0] = 0;

		Values[0][1] = 0;
		Values[1][1] = yScale;
		Values[2][1] = 0;
		Values[3][1] = 0;

		Values[0][2] = 0;
		Values[1][2] = 0;
		Values[2][2] = zf / (zf - zn);
		Values[3][2] = 1;

		Values[0][3] = 0;
		Values[1][3] = 0;
		Values[2][3] = -zn * zf / (zf - zn);
		Values[3][3] = 0;
		return *this;
	}

	Matrix44& Matrix44::SetOrthographicRH(float width, float height, float zn, float zf)
	{
		Values[0][0] = 2 / width;
		Values[1][0] = 0;
		Values[2][0] = 0;
		Values[3][0] = 0;

		Values[0][1] = 0;
		Values[1][1] = 2 / height;
		Values[2][1] = 0;
		Values[3][1] = 0;

		Values[0][2] = 0;
		Values[1][2] = 0;
		Values[2][2] = 1 / (zn - zf);
		Values[3][2] = 0;

		Values[0][3] = 0;
		Values[1][3] = 0;
		Values[2][3] = zn / (zn - zf);
		Values[3][3] = 1;
		return *this;
	}

	Matrix44& Matrix44::SetOrthographicLH(float width, float height, float zn, float zf)
	{
		Values[0][0] = 2 / width;
		Values[1][0] = 0;
		Values[2][0] = 0;
		Values[3][0] = 0;

		Values[0][1] = 0;
		Values[1][1] = 2 / height;
		Values[2][1] = 0;
		Values[3][1] = 0;

		Values[0][2] = 0;
		Values[1][2] = 0;
		Values[2][2] = 1 / (zf - zn);
		Values[3][2] = 0;

		Values[0][3] = 0;
		Values[1][3] = 0;
		Values[2][3] = zn / (zn - zf);
		Values[3][3] = 1;
		return *this;
	}

	Vector3DF Matrix44::Transform3D(const Vector3DF& in) const
	{
		float values[4];

		for (int i = 0; i < 4; i++)
		{
			values[i] = 0;
			values[i] += in.X * Values[i][0];
			values[i] += in.Y * Values[i][1];
			values[i] += in.Z * Values[i][2];
			values[i] += Values[i][3];
		}

		Vector3DF o;
		o.X = values[0] / values[3];
		o.Y = values[1] / values[3];
		o.Z = values[2] / values[3];
		return o;
	}

	Matrix44 Matrix44::operator * (const Matrix44& right) const
	{
		Matrix44 o_;
		Mul(o_, *this, right);
		return o_;
	}

	Vector3DF Matrix44::operator * (const Vector3DF& right) const
	{
		return Transform3D(right);
	}

	Matrix44& Matrix44::Mul(Matrix44& o, const Matrix44& in1, const Matrix44& in2)
	{
		Matrix44 _in1 = in1;
		Matrix44 _in2 = in2;

		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				float v = 0.0f;
				for (int k = 0; k < 4; k++)
				{
					v += _in1.Values[i][k] * _in2.Values[k][j];
				}
				o.Values[i][j] = v;
			}
		}
		return o;
	}
}


namespace Culling3D
{
	Object* Object::Create()
	{
		return new ObjectInternal();
	}

	ObjectInternal::ObjectInternal()
		: userData(NULL)
		, world(NULL)
		, ObjectIndex(-1)
	{
		currentStatus.Position = Vector3DF();
		currentStatus.radius = 0.0f;
		currentStatus.Type = OBJECT_SHAPE_TYPE_NONE;

		nextStatus.Position = Vector3DF();
		nextStatus.radius = 0.0f;
		nextStatus.Type = OBJECT_SHAPE_TYPE_NONE;
	}

	ObjectInternal::~ObjectInternal()
	{
	}

	Vector3DF ObjectInternal::GetPosition()
	{
		return nextStatus.Position;
	}

	void ObjectInternal::SetPosition(Vector3DF pos)
	{
		nextStatus.Position = pos;

		if (world != NULL)
		{
			WorldInternal* w = (WorldInternal*) world;
			w->RemoveObjectInternal(this);
			w->AddObjectInternal(this);
		}

		currentStatus = nextStatus;
	}

	void ObjectInternal::ChangeIntoAll()
	{
		nextStatus.Type = OBJECT_SHAPE_TYPE_ALL;
		nextStatus.CalcRadius();

		if (world != NULL)
		{
			WorldInternal* w = (WorldInternal*) world;
			w->RemoveObjectInternal(this);
			w->AddObjectInternal(this);
		}

		currentStatus = nextStatus;
	}

	void ObjectInternal::ChangeIntoSphere(float radius)
	{
		nextStatus.Data.Sphere.Radius = radius;
		nextStatus.Type = OBJECT_SHAPE_TYPE_SPHERE;
		nextStatus.CalcRadius();

		if (world != NULL)
		{
			WorldInternal* w = (WorldInternal*) world;
			w->RemoveObjectInternal(this);
			w->AddObjectInternal(this);
		}

		currentStatus = nextStatus;
	}

	void ObjectInternal::ChangeIntoCuboid(Vector3DF size)
	{
		nextStatus.Data.Cuboid.X = size.X;
		nextStatus.Data.Cuboid.Y = size.Y;
		nextStatus.Data.Cuboid.Z = size.Z;
		nextStatus.Type = OBJECT_SHAPE_TYPE_CUBOID;
		nextStatus.CalcRadius();

		if (world != NULL)
		{
			WorldInternal* w = (WorldInternal*) world;
			w->RemoveObjectInternal(this);
			w->AddObjectInternal(this);
		}

		currentStatus = nextStatus;
	}

	void* ObjectInternal::GetUserData()
	{
		return userData;
	}

	void ObjectInternal::SetUserData(void* userData_)
	{
		this->userData = userData_;
	}


	void ObjectInternal::SetWorld(World* world_)
	{
		this->world = world_;
	}
}


namespace Culling3D
{
	ReferenceObject::ReferenceObject()
		: m_reference(1)
	{
	}

	ReferenceObject::~ReferenceObject()
	{
	}

	int32_t ReferenceObject::AddRef()
	{
		m_reference++;
		return m_reference;
	}

	int32_t ReferenceObject::GetRef()
	{
		return m_reference;
	}

	int32_t ReferenceObject::Release()
	{
		assert(m_reference > 0);

		m_reference--;
		bool destroy = m_reference == 0;
		if (destroy)
		{
			delete this;
			return 0;
		}

		return m_reference;
	}
}


namespace Culling3D
{
	Vector3DF::Vector3DF()
		: X(0)
		, Y(0)
		, Z(0)
	{
	}

	Vector3DF::Vector3DF(float x, float y, float z)
		: X(x)
		, Y(y)
		, Z(z)
	{
	}

	bool Vector3DF::operator == (const Vector3DF& o)
	{
		return X == o.X && Y == o.Y && Z == o.Z;
	}

	bool Vector3DF::operator != (const Vector3DF& o)
	{
		return !(X == o.X && Y == o.Y && Z == o.Z);
	}

	Vector3DF Vector3DF::operator-()
	{
		return Vector3DF(-X, -Y, -Z);
	}

	Vector3DF Vector3DF::operator + (const Vector3DF& o) const
	{
		return Vector3DF(X + o.X, Y + o.Y, Z + o.Z);
	}

	Vector3DF Vector3DF::operator - (const Vector3DF& o) const
	{
		return Vector3DF(X - o.X, Y - o.Y, Z - o.Z);
	}

	Vector3DF Vector3DF::operator * (const Vector3DF& o) const
	{
		return Vector3DF(X * o.X, Y * o.Y, Z * o.Z);
	}

	Vector3DF Vector3DF::operator / (const Vector3DF& o) const
	{
		return Vector3DF(X / o.X, Y / o.Y, Z / o.Z);
	}

	Vector3DF Vector3DF::operator * (const float& o) const
	{
		return Vector3DF(X * o, Y * o, Z * o);
	}

	Vector3DF Vector3DF::operator / (const float& o) const
	{
		return Vector3DF(X / o, Y / o, Z / o);
	}

	Vector3DF& Vector3DF::operator += (const Vector3DF& o)
	{
		X += o.X;
		Y += o.Y;
		Z += o.Z;
		return *this;
	}

	Vector3DF& Vector3DF::operator -= (const Vector3DF& o)
	{
		X -= o.X;
		Y -= o.Y;
		Z -= o.Z;
		return *this;
	}

	Vector3DF& Vector3DF::operator *= (const float& o)
	{
		X *= o;
		Y *= o;
		Z *= o;
		return *this;
	}

	Vector3DF& Vector3DF::operator /= (const float& o)
	{
		X /= o;
		Y /= o;
		Z /= o;
		return *this;
	}

	float Vector3DF::Dot(const Vector3DF& v1, const Vector3DF& v2)
	{
		return v1.X * v2.X + v1.Y * v2.Y + v1.Z * v2.Z;
	}

	Vector3DF Vector3DF::Cross(const Vector3DF& v1, const Vector3DF& v2)
	{
		Vector3DF o;

		float x = v1.Y * v2.Z - v1.Z * v2.Y;
		float y = v1.Z * v2.X - v1.X * v2.Z;
		float z = v1.X * v2.Y - v1.Y * v2.X;
		o.X = x;
		o.Y = y;
		o.Z = z;
		return o;
	}

	float Vector3DF::Distance(const Vector3DF& v1, const Vector3DF& v2)
	{
		float dx = v1.X - v2.X;
		float dy = v1.Y - v2.Y;
		float dz = v1.Z - v2.Z;
		return sqrtf(dx * dx + dy * dy + dz * dz);
	}
}



namespace Culling3D
{
	const int32_t viewCullingXDiv = 2;
	const int32_t viewCullingYDiv = 2;
	const int32_t viewCullingZDiv = 3;

	bool IsInView(Vector3DF position, float radius, Vector3DF facePositions[6], Vector3DF faceDir[6])
	{
		for (int32_t i = 0; i < 6; i++)
		{
			Vector3DF diff = position - facePositions[i];
			float distance = Vector3DF::Dot(diff, faceDir[i]);
		
			if (distance > radius) return false;
		}

		return true;
	}

	World* World::Create(float xSize, float ySize, float zSize, int32_t layerCount)
	{
		return new WorldInternal(xSize, ySize, zSize, layerCount);
	}

	WorldInternal::WorldInternal(float xSize, float ySize, float zSize, int32_t layerCount)
	{
		this->xSize = xSize;
		this->ySize = ySize;
		this->zSize = zSize;

		this->gridSize = Max(Max(this->xSize, this->ySize), this->zSize);
		
		this->layerCount = layerCount;

		layers.resize(this->layerCount);

		for (size_t i = 0; i < layers.size(); i++)
		{
			float gridSize_ = this->gridSize / powf(2.0f, (float)i);

			int32_t xCount = (int32_t) (this->xSize / gridSize_);
			int32_t yCount = (int32_t) (this->ySize / gridSize_);
			int32_t zCount = (int32_t) (this->zSize / gridSize_);

			if (xCount * gridSize_ < this->xSize) xCount++;
			if (yCount * gridSize_ < this->ySize) yCount++;
			if (zCount * gridSize_ < this->zSize) zCount++;

			layers[i] = new Layer(xCount, yCount, zCount, xSize / 2.0f, ySize / 2.0f, zSize / 2.0f, gridSize_);
			
			this->minGridSize = gridSize_;
		}
	}

	WorldInternal::~WorldInternal()
	{
		for (size_t i = 0; i < layers.size(); i++)
		{
			delete layers[i];
		}

		layers.clear();

		for (std::set<Object*>::iterator it = containedObjects.begin(); it != containedObjects.end(); it++)
		{
			(*it)->Release();
		}
	}

	void WorldInternal::AddObject(Object* o)
	{
		SafeAddRef(o);
		containedObjects.insert(o);
		AddObjectInternal(o);
	}

	void WorldInternal::RemoveObject(Object* o)
	{
		RemoveObjectInternal(o);
		containedObjects.erase(o);
		SafeRelease(o);
	}

	void WorldInternal::AddObjectInternal(Object* o)
	{
		assert(o != NULL);

		ObjectInternal* o_ = (ObjectInternal*) o;

		if (o_->GetNextStatus().Type == OBJECT_SHAPE_TYPE_ALL)
		{
			allLayers.AddObject(o);
			o_->SetWorld(this);
			return;
		}

		float radius = o_->GetNextStatus().GetRadius();
		if (o_->GetNextStatus().Type == OBJECT_SHAPE_TYPE_NONE || radius <= minGridSize)
		{
			if (layers[layers.size() - 1]->AddObject(o))
			{
			}
			else
			{
				outofLayers.AddObject(o);
			}
			o_->SetWorld(this);
			return;
		}

		int32_t gridInd = (int32_t) (gridSize / (radius * 2.0f));

		if (gridInd * (radius * 2) < gridSize) gridInd++;

		int32_t ind = 1;
		bool found = false;
		for (size_t i = 0; i < layers.size(); i++)
		{
			if (ind <= gridInd && gridInd < ind * 2)
			{
				if (layers[i]->AddObject(o))
				{
					((ObjectInternal*) o)->SetWorld(this);
					found = true;
				}
				else
				{
					break;
				}
			}

			ind *= 2;
		}

		if (!found)
		{
			((ObjectInternal*) o)->SetWorld(this);
			outofLayers.AddObject(o);
		}
	}

	void WorldInternal::RemoveObjectInternal(Object* o)
	{
		assert(o != NULL);

		ObjectInternal* o_ = (ObjectInternal*) o;

		if (o_->GetCurrentStatus().Type == OBJECT_SHAPE_TYPE_ALL)
		{
			allLayers.RemoveObject(o);
			o_->SetWorld(NULL);
			return;
		}

		float radius = o_->GetCurrentStatus().GetRadius();
		if (o_->GetCurrentStatus().Type == OBJECT_SHAPE_TYPE_NONE || radius <= minGridSize)
		{
			if (layers[layers.size() - 1]->RemoveObject(o))
			{
			}
			else
			{
				outofLayers.RemoveObject(o);
			}
			o_->SetWorld(NULL);
			return;
		}

		int32_t gridInd = (int32_t) (gridSize / (radius * 2.0f));

		if (gridInd * (radius * 2.0f) < gridSize) gridInd++;

		int32_t ind = 1;
		bool found = false;
		for (size_t i = 0; i < layers.size(); i++)
		{
			if (ind <= gridInd && gridInd < ind * 2)
			{
				if (layers[i]->RemoveObject(o))
				{
					((ObjectInternal*) o)->SetWorld(NULL);
					found = true;
				}
				else
				{
					break;
				}
			}

			ind *= 2;
		}

		if (!found)
		{
			((ObjectInternal*) o)->SetWorld(NULL);
			outofLayers.RemoveObject(o);
		}
	}

	void WorldInternal::CastRay(Vector3DF from, Vector3DF to)
	{
		objs.clear();

		Vector3DF aabb_max;
		Vector3DF aabb_min;

		aabb_max.X = Max(from.X, to.X);
		aabb_max.Y = Max(from.Y, to.Y);
		aabb_max.Z = Max(from.Z, to.Z);

		aabb_min.X = Min(from.X, to.X);
		aabb_min.Y = Min(from.Y, to.Y);
		aabb_min.Z = Min(from.Z, to.Z);

		/* 範囲内に含まれるグリッドを取得 */
		for (size_t i = 0; i < layers.size(); i++)
		{
			layers[i]->AddGrids(aabb_max, aabb_min, grids);
		}

		/* 外領域追加 */
		grids.push_back(&outofLayers);
		grids.push_back(&allLayers);

		/* グリッドからオブジェクト取得 */
		
		/* 初期計算 */
		auto ray_dir = (to - from);
		auto ray_len = ray_dir.GetLength();
		ray_dir.Normalize();

		for (size_t i = 0; i < grids.size(); i++)
		{
			for (size_t j = 0; j < grids[i]->GetObjects().size(); j++)
			{
				Object* o = grids[i]->GetObjects()[j];
				ObjectInternal* o_ = (ObjectInternal*) o;

				if (o_->GetNextStatus().Type == OBJECT_SHAPE_TYPE_ALL)
				{
					objs.push_back(o);
					continue;
				}

				// 球線分判定
				{
					auto radius = o_->GetNextStatus().GetRadius();
					auto pos = o_->GetNextStatus().Position;

					auto from2pos = pos - from;
					auto from2nearLen = Vector3DF::Dot(from2pos, ray_dir);
					auto pos2ray = from2pos - ray_dir * from2nearLen;

					if (pos2ray.GetLength() > radius) continue;
					if (from2nearLen < 0 || from2nearLen > ray_len) continue;
				}

				if (o_->GetNextStatus().Type == OBJECT_SHAPE_TYPE_SPHERE)
				{
					objs.push_back(o);
					continue;
				}

				// AABB判定
				// 参考：http://marupeke296.com/COL_3D_No18_LineAndAABB.html

				if (o_->GetNextStatus().Type == OBJECT_SHAPE_TYPE_CUBOID)
				{
					// 交差判定
					float p[3], d[3], min[3], max[3];
					auto pos = o_->GetCurrentStatus().Position;
					memcpy(p, &from, sizeof(Vector3DF));
					memcpy(d, &ray_dir, sizeof(Vector3DF));
					memcpy(min, &pos, sizeof(Vector3DF));
					memcpy(max, &pos, sizeof(Vector3DF));

					min[0] -= o_->GetNextStatus().Data.Cuboid.X / 2.0f;
					min[1] -= o_->GetNextStatus().Data.Cuboid.Y / 2.0f;
					min[2] -= o_->GetNextStatus().Data.Cuboid.Z / 2.0f;

					max[0] += o_->GetNextStatus().Data.Cuboid.X / 2.0f;
					max[1] += o_->GetNextStatus().Data.Cuboid.Y / 2.0f;
					max[2] += o_->GetNextStatus().Data.Cuboid.Z / 2.0f;

					float t = -FLT_MAX;
					float t_max = FLT_MAX;

					for (int k = 0; k < 3; ++k)
					{
						if (std::abs(d[k]) < FLT_EPSILON)
						{
							if (p[k] < min[k] || p[k] > max[k])
							{
								// 交差していない
								continue;
							}
						}
						else
						{
							// スラブとの距離を算出
							// t1が近スラブ、t2が遠スラブとの距離
							float odd = 1.0f / d[k];
							float t1 = (min[k] - p[k]) * odd;
							float t2 = (max[k] - p[k]) * odd;
							if (t1 > t2)
							{
								float tmp = t1; t1 = t2; t2 = tmp;
							}

							if (t1 > t) t = t1;
							if (t2 < t_max) t_max = t2;

							// スラブ交差チェック
							if (t >= t_max)
							{
								// 交差していない
								continue;
							}
						}
					}

					// 交差している
					if (0 <= t  && t <= ray_len)
					{
						objs.push_back(o);
						continue;
					}
				}
			}
		}

		/* 取得したグリッドを破棄 */
		for (size_t i = 0; i < grids.size(); i++)
		{
			grids[i]->IsScanned = false;
		}

		grids.clear();
	}

	void WorldInternal::Culling(const Matrix44& cameraProjMat, bool isOpenGL)
	{
		objs.clear();
	

		if (!std::isinf(cameraProjMat.Values[2][2]) &&
			cameraProjMat.Values[0][0] != 0.0f &&
			cameraProjMat.Values[1][1] != 0.0f)
		{

			Matrix44 cameraProjMatInv = cameraProjMat;
			cameraProjMatInv.SetInverted();

			float maxx = 1.0f;
			float minx = -1.0f;

			float maxy = 1.0f;
			float miny = -1.0f;

			float maxz = 1.0f;
			float minz = 0.0f;
			if (isOpenGL) minz = -1.0f;

			Vector3DF eyebox[8];

			eyebox[0 + 0] = Vector3DF(minx, miny, maxz);
			eyebox[1 + 0] = Vector3DF(maxx, miny, maxz);
			eyebox[2 + 0] = Vector3DF(minx, maxy, maxz);
			eyebox[3 + 0] = Vector3DF(maxx, maxy, maxz);

			eyebox[0 + 4] = Vector3DF(minx, miny, minz);
			eyebox[1 + 4] = Vector3DF(maxx, miny, minz);
			eyebox[2 + 4] = Vector3DF(minx, maxy, minz);
			eyebox[3 + 4] = Vector3DF(maxx, maxy, minz);

			for (int32_t i = 0; i < 8; i++)
			{
				eyebox[i] = cameraProjMatInv.Transform3D(eyebox[i]);
			}

			// 0-right 1-left 2-top 3-bottom 4-front 5-back
			Vector3DF facePositions[6];
			facePositions[0] = eyebox[5];
			facePositions[1] = eyebox[4];
			facePositions[2] = eyebox[6];
			facePositions[3] = eyebox[4];
			facePositions[4] = eyebox[4];
			facePositions[5] = eyebox[0];

			Vector3DF faceDir[6];
			faceDir[0] = Vector3DF::Cross(eyebox[1] - eyebox[5], eyebox[7] - eyebox[5]);
			faceDir[1] = Vector3DF::Cross(eyebox[6] - eyebox[4], eyebox[0] - eyebox[4]);

			faceDir[2] = Vector3DF::Cross(eyebox[7] - eyebox[6], eyebox[2] - eyebox[6]);
			faceDir[3] = Vector3DF::Cross(eyebox[0] - eyebox[4], eyebox[5] - eyebox[4]);

			faceDir[4] = Vector3DF::Cross(eyebox[5] - eyebox[4], eyebox[6] - eyebox[4]);
			faceDir[5] = Vector3DF::Cross(eyebox[2] - eyebox[0], eyebox[1] - eyebox[5]);

			for (int32_t i = 0; i < 6; i++)
			{
				faceDir[i].Normalize();
			}

			for (int32_t z = 0; z < viewCullingZDiv; z++)
			{
				for (int32_t y = 0; y < viewCullingYDiv; y++)
				{
					for (int32_t x = 0; x < viewCullingXDiv; x++)
					{
						Vector3DF eyebox_[8];

						float xsize = 1.0f / (float) viewCullingXDiv;
						float ysize = 1.0f / (float) viewCullingYDiv;
						float zsize = 1.0f / (float) viewCullingZDiv;

						for (int32_t e = 0; e < 8; e++)
						{
							float x_ = 0.0f, y_ = 0.0f, z_ = 0.0f;
							if (e == 0){ x_ = xsize * x; y_ = ysize * y; z_ = zsize * z; }
							if (e == 1){ x_ = xsize * (x + 1); y_ = ysize * y; z_ = zsize * z; }
							if (e == 2){ x_ = xsize * x; y_ = ysize * (y + 1); z_ = zsize * z; }
							if (e == 3){ x_ = xsize * (x + 1); y_ = ysize * (y + 1); z_ = zsize * z; }
							if (e == 4){ x_ = xsize * x; y_ = ysize * y; z_ = zsize * (z + 1); }
							if (e == 5){ x_ = xsize * (x + 1); y_ = ysize * y; z_ = zsize * (z + 1); }
							if (e == 6){ x_ = xsize * x; y_ = ysize * (y + 1); z_ = zsize * (z + 1); }
							if (e == 7){ x_ = xsize * (x + 1); y_ = ysize * (y + 1); z_ = zsize * (z + 1); }

							Vector3DF yzMid[4];
							yzMid[0] = eyebox[0] * x_ + eyebox[1] * (1.0f - x_);
							yzMid[1] = eyebox[2] * x_ + eyebox[3] * (1.0f - x_);
							yzMid[2] = eyebox[4] * x_ + eyebox[5] * (1.0f - x_);
							yzMid[3] = eyebox[6] * x_ + eyebox[7] * (1.0f - x_);

							Vector3DF zMid[2];
							zMid[0] = yzMid[0] * y_ + yzMid[1] * (1.0f - y_);
							zMid[1] = yzMid[2] * y_ + yzMid[3] * (1.0f - y_);

							eyebox_[e] = zMid[0] * z_ + zMid[1] * (1.0f - z_);
						}



						Vector3DF max_(-FLT_MAX, -FLT_MAX, -FLT_MAX);
						Vector3DF min_(FLT_MAX, FLT_MAX, FLT_MAX);

						for (int32_t i = 0; i < 8; i++)
						{
							if (eyebox_[i].X > max_.X) max_.X = eyebox_[i].X;
							if (eyebox_[i].Y > max_.Y) max_.Y = eyebox_[i].Y;
							if (eyebox_[i].Z > max_.Z) max_.Z = eyebox_[i].Z;

							if (eyebox_[i].X < min_.X) min_.X = eyebox_[i].X;
							if (eyebox_[i].Y < min_.Y) min_.Y = eyebox_[i].Y;
							if (eyebox_[i].Z < min_.Z) min_.Z = eyebox_[i].Z;
						}

						/* 範囲内に含まれるグリッドを取得 */
						for (size_t i = 0; i < layers.size(); i++)
						{
							layers[i]->AddGrids(max_, min_, grids);
						}
					}
				}
			}

			/* 外領域追加 */
			grids.push_back(&outofLayers);
			grids.push_back(&allLayers);

			/* グリッドからオブジェクト取得 */
			for (size_t i = 0; i < grids.size(); i++)
			{
				for (size_t j = 0; j < grids[i]->GetObjects().size(); j++)
				{
					Object* o = grids[i]->GetObjects()[j];
					ObjectInternal* o_ = (ObjectInternal*) o;

					if (
						o_->GetNextStatus().Type == OBJECT_SHAPE_TYPE_ALL ||
						IsInView(o_->GetPosition(), o_->GetNextStatus().GetRadius(), facePositions, faceDir))
					{
						objs.push_back(o);
					}
				}
			}

			/* 取得したグリッドを破棄 */
			for (size_t i = 0; i < grids.size(); i++)
			{
				grids[i]->IsScanned = false;
			}

			grids.clear();
		}
		else
		{
			grids.push_back(&allLayers);

			/* グリッドからオブジェクト取得 */
			for (size_t i = 0; i < grids.size(); i++)
			{
				for (size_t j = 0; j < grids[i]->GetObjects().size(); j++)
				{
					Object* o = grids[i]->GetObjects()[j];
					ObjectInternal* o_ = (ObjectInternal*) o;

					if (o_->GetNextStatus().Type == OBJECT_SHAPE_TYPE_ALL)
					{
						objs.push_back(o);
					}
				}
			}

			/* 取得したグリッドを破棄 */
			for (size_t i = 0; i < grids.size(); i++)
			{
				grids[i]->IsScanned = false;
			}

			grids.clear();
		}
		
	}

	bool WorldInternal::Reassign()
	{
		/* 数が少ない */
		if (outofLayers.GetObjects().size() < 10) return false;

		objs.clear();

		for (size_t i = 0; i < layers.size(); i++)
		{
			delete layers[i];
		}

		layers.clear();
		outofLayers.GetObjects().clear();
		allLayers.GetObjects().clear();

		outofLayers.IsScanned = false;
		allLayers.IsScanned = false;

		for (auto& it : containedObjects)
		{
			auto o = (ObjectInternal*) (it);
			o->ObjectIndex = -1;
		}

		float xmin = FLT_MAX;
		float xmax = -FLT_MAX;
		float ymin = FLT_MAX;
		float ymax = -FLT_MAX;
		float zmin = FLT_MAX;
		float zmax = -FLT_MAX;

		for (auto& it : containedObjects)
		{
			ObjectInternal* o_ = (ObjectInternal*) it;
			if (o_->GetNextStatus().Type == OBJECT_SHAPE_TYPE_ALL) continue;

			if (xmin > o_->GetNextStatus().Position.X) xmin = o_->GetNextStatus().Position.X;
			if (xmax < o_->GetNextStatus().Position.X) xmax = o_->GetNextStatus().Position.X;
			if (ymin > o_->GetNextStatus().Position.Y) ymin = o_->GetNextStatus().Position.Y;
			if (ymax < o_->GetNextStatus().Position.Y) ymax = o_->GetNextStatus().Position.Y;
			if (zmin > o_->GetNextStatus().Position.Z) zmin = o_->GetNextStatus().Position.Z;
			if (zmax < o_->GetNextStatus().Position.Z) zmax = o_->GetNextStatus().Position.Z;

		}

		auto xlen = Max(std::abs(xmax), std::abs(xmin)) * 2.0f;
		auto ylen = Max(std::abs(ymax), std::abs(ymin)) * 2.0f;
		auto zlen = Max(std::abs(zmax), std::abs(zmin)) * 2.0f;

		WorldInternal(xlen, ylen, zlen, this->layerCount);

		for (auto& it: containedObjects)
		{
			ObjectInternal* o_ = (ObjectInternal*) (it);
			AddObjectInternal(o_);
		}
		return true;
	}

	void WorldInternal::Dump(const char* path, const Matrix44& cameraProjMat, bool isOpenGL)
	{
		std::ofstream ofs(path);

		/* カメラ情報出力 */
		Matrix44 cameraProjMatInv = cameraProjMat;
		cameraProjMatInv.SetInverted();

		float maxx = 1.0f;
		float minx = -1.0f;

		float maxy = 1.0f;
		float miny = -1.0f;

		float maxz = 1.0f;
		float minz = 0.0f;
		if (isOpenGL) minz = -1.0f;

		Vector3DF eyebox[8];

		eyebox[0 + 0] = Vector3DF(minx, miny, maxz);
		eyebox[1 + 0] = Vector3DF(maxx, miny, maxz);
		eyebox[2 + 0] = Vector3DF(minx, maxy, maxz);
		eyebox[3 + 0] = Vector3DF(maxx, maxy, maxz);

		eyebox[0 + 4] = Vector3DF(minx, miny, minz);
		eyebox[1 + 4] = Vector3DF(maxx, miny, minz);
		eyebox[2 + 4] = Vector3DF(minx, maxy, minz);
		eyebox[3 + 4] = Vector3DF(maxx, maxy, minz);

		for (int32_t i = 0; i < 8; i++)
		{
			eyebox[i] = cameraProjMatInv.Transform3D(eyebox[i]);
		}

		ofs << viewCullingXDiv << "," << viewCullingYDiv << "," << viewCullingZDiv << std::endl;
		for (int32_t i = 0; i < 8; i++)
		{
			ofs << eyebox[i].X << "," << eyebox[i].Y << "," << eyebox[i].Z << std::endl;
		}
		ofs << std::endl;

		for (int32_t z = 0; z < viewCullingZDiv; z++)
		{
			for (int32_t y = 0; y < viewCullingYDiv; y++)
			{
				for (int32_t x = 0; x < viewCullingXDiv; x++)
				{
					Vector3DF eyebox_[8];

					float xsize = 1.0f / (float) viewCullingXDiv;
					float ysize = 1.0f / (float) viewCullingYDiv;
					float zsize = 1.0f / (float) viewCullingZDiv;

					for (int32_t e = 0; e < 8; e++)
					{
						float x_ = 0.0f, y_ = 0.0f, z_ = 0.0f;
						if (e == 0){ x_ = xsize * x; y_ = ysize * y; z_ = zsize * z; }
						if (e == 1){ x_ = xsize * (x + 1); y_ = ysize * y; z_ = zsize * z; }
						if (e == 2){ x_ = xsize * x; y_ = ysize * (y + 1); z_ = zsize * z; }
						if (e == 3){ x_ = xsize * (x + 1); y_ = ysize * (y + 1); z_ = zsize * z; }
						if (e == 4){ x_ = xsize * x; y_ = ysize * y; z_ = zsize * (z + 1); }
						if (e == 5){ x_ = xsize * (x + 1); y_ = ysize * y; z_ = zsize * (z + 1); }
						if (e == 6){ x_ = xsize * x; y_ = ysize * (y + 1); z_ = zsize * (z + 1); }
						if (e == 7){ x_ = xsize * (x + 1); y_ = ysize * (y + 1); z_ = zsize * (z + 1); }

						Vector3DF yzMid[4];
						yzMid[0] = eyebox[0] * x_ + eyebox[1] * (1.0f - x_);
						yzMid[1] = eyebox[2] * x_ + eyebox[3] * (1.0f - x_);
						yzMid[2] = eyebox[4] * x_ + eyebox[5] * (1.0f - x_);
						yzMid[3] = eyebox[6] * x_ + eyebox[7] * (1.0f - x_);

						Vector3DF zMid[2];
						zMid[0] = yzMid[0] * y_ + yzMid[1] * (1.0f - y_);
						zMid[1] = yzMid[2] * y_ + yzMid[3] * (1.0f - y_);

						eyebox_[e] = zMid[0] * z_ + zMid[1] * (1.0f - z_);
					}

					Vector3DF max_(-FLT_MAX, -FLT_MAX, -FLT_MAX);
					Vector3DF min_(FLT_MAX, FLT_MAX, FLT_MAX);

					for (int32_t i = 0; i < 8; i++)
					{
						if (eyebox_[i].X > max_.X) max_.X = eyebox_[i].X;
						if (eyebox_[i].Y > max_.Y) max_.Y = eyebox_[i].Y;
						if (eyebox_[i].Z > max_.Z) max_.Z = eyebox_[i].Z;

						if (eyebox_[i].X < min_.X) min_.X = eyebox_[i].X;
						if (eyebox_[i].Y < min_.Y) min_.Y = eyebox_[i].Y;
						if (eyebox_[i].Z < min_.Z) min_.Z = eyebox_[i].Z;
					}

					ofs << x << "," << y << "," << z << std::endl;
					for (int32_t i = 0; i < 8; i++)
					{
						ofs << eyebox_[i].X << "," << eyebox_[i].Y << "," << eyebox_[i].Z << std::endl;
					}
					ofs << max_.X << "," << max_.Y << "," << max_.Z << std::endl;
					ofs << min_.X << "," << min_.Y << "," << min_.Z << std::endl;
					ofs << std::endl;
				}
			}
		}

		ofs << std::endl;
	
		/* レイヤー情報 */
		ofs << layers.size() << std::endl;

		for (size_t i = 0; i < layers.size(); i++)
		{
			auto& layer = layers[i];
			ofs << layer->GetGridXCount() << "," << layer->GetGridYCount() << "," << layer->GetGridZCount() 
				<< "," << layer->GetOffsetX() << "," << layer->GetOffsetY() << "," << layer->GetOffsetZ() << "," << layer->GetGridSize() << std::endl;
		
			for (size_t j = 0; j < layer->GetGrids().size(); j++)
			{
				auto& grid = layer->GetGrids()[j];

				if (grid.GetObjects().size() > 0)
				{
					ofs << j << "," << grid.GetObjects().size() << std::endl;
				}
			}
		}

		Culling(cameraProjMat, isOpenGL);


	}
}



//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
namespace Effekseer { 
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
Vector2D::Vector2D()
	: X		( 0.0f )
	, Y		( 0.0f )
{

}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
Vector2D::Vector2D( float x, float y )
	: X		( x )
	, Y		( y )
{

}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
Vector2D& Vector2D::operator+=( const Vector2D& value )
{
	X += value.X;
	Y += value.Y;
	return *this;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
 } 
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------


//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
#if defined(_M_X86) && defined(__x86__)
#endif

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
namespace Effekseer {
	
inline float Rsqrt(float x)
{
#if defined(_M_X86) && defined(__x86__)
	_mm_store_ss(&x, _mm_rsqrt_ss(_mm_load_ss(&x)));
	return x;
#else
	float xhalf = 0.5f * x;
	int i = *(int*)&x;
	i = 0x5f3759df - (i >> 1);
	x = *(float*)&i;
	x = x * (1.5f - xhalf * x * x);
	x = x * (1.5f - xhalf * x * x);
	return x;
#endif
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
Vector3D::Vector3D()
	: X		( 0.0f )
	, Y		( 0.0f )
	, Z		( 0.0f )
{

}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
Vector3D::Vector3D( float x, float y, float z )
	: X		( x )
	, Y		( y )
	, Z		( z )
{

}

Vector3D Vector3D::operator-()
{
	return Vector3D(-X, -Y, -Z);
}

Vector3D Vector3D::operator + ( const Vector3D& o ) const
{
	return Vector3D( X + o.X, Y + o.Y, Z + o.Z );
}

//-----------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------
Vector3D Vector3D::operator - ( const Vector3D& o ) const
{
	return Vector3D( X - o.X, Y - o.Y, Z - o.Z );
}

//-----------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------
Vector3D Vector3D::operator * ( const float& o ) const
{
	return Vector3D( X * o, Y * o, Z * o );
}

//-----------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------
Vector3D Vector3D::operator / ( const float& o ) const
{
	return Vector3D( X / o, Y / o, Z / o );
}

Vector3D Vector3D::operator * (const Vector3D& o) const
{
	return Vector3D(X * o.X, Y * o.Y, Z * o.Z);
}

Vector3D Vector3D::operator / (const Vector3D& o) const
{
	return Vector3D(X / o.X, Y / o.Y, Z / o.Z);
}

bool Vector3D::operator == (const Vector3D& o)
{
	return this->X == o.X && this->Y == o.Y && this->Z == o.Z;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void Vector3D::Add( Vector3D* pOut, const Vector3D* pIn1, const Vector3D* pIn2 )
{
	float x = pIn1->X + pIn2->X;
	float y = pIn1->Y + pIn2->Y;
	float z = pIn1->Z + pIn2->Z;
	pOut->X = x;
	pOut->Y = y;
	pOut->Z = z;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
Vector3D& Vector3D::Sub( Vector3D& o, const Vector3D& in1, const Vector3D& in2 )
{
	o.X = in1.X - in2.X;
	o.Y = in1.Y - in2.Y;
	o.Z = in1.Z - in2.Z;
	return o;
}

//-----------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------
Vector3D& Vector3D::operator += ( const Vector3D& o )
{
	X += o.X;
	Y += o.Y;
	Z += o.Z;
	return *this;
}

//-----------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------
Vector3D& Vector3D::operator -= ( const Vector3D& o )
{
	X -= o.X;
	Y -= o.Y;
	Z -= o.Z;
	return *this;
}

//-----------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------
Vector3D& Vector3D::operator *= ( const float& o )
{
	X *= o;
	Y *= o;
	Z *= o;
	return *this;
}

//-----------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------
Vector3D& Vector3D::operator /= ( const float& o )
{
	X /= o;
	Y /= o;
	Z /= o;
	return *this;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
float Vector3D::Length( const Vector3D& in )
{
	return sqrt( in.X * in.X + in.Y * in.Y + in.Z * in.Z );
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
float Vector3D::LengthSq( const Vector3D& in )
{
	return in.X * in.X + in.Y * in.Y + in.Z * in.Z;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
float Vector3D::Dot( const Vector3D& in1, const Vector3D& in2 )
{
	return in1.X * in2.X + in1.Y * in2.Y + in1.Z * in2.Z;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void Vector3D::Normal( Vector3D& o, const Vector3D& in )
{
	float inv = Rsqrt( in.X * in.X + in.Y * in.Y + in.Z * in.Z );
	o.X = in.X * inv;
	o.Y = in.Y * inv;
	o.Z = in.Z * inv;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
Vector3D& Vector3D::Cross( Vector3D& o, const Vector3D& in1, const Vector3D& in2 )
{
	float x = in1.Y * in2.Z - in1.Z * in2.Y;
	float y = in1.Z * in2.X - in1.X * in2.Z;
	float z = in1.X * in2.Y - in1.Y * in2.X;
	o.X = x;
	o.Y = y;
	o.Z = z;
	return o;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
Vector3D& Vector3D::Transform( Vector3D& o, const Vector3D& in, const Matrix43& mat )
{
	float values[4];
	for( int i = 0; i < 3; i++ )
	{
		values[i] = 0;
		values[i] += in.X * mat.Value[0][i];
		values[i] += in.Y * mat.Value[1][i];
		values[i] += in.Z * mat.Value[2][i];
		values[i] += mat.Value[3][i];
	}
	o.X = values[0];
	o.Y = values[1];
	o.Z = values[2];
	return o;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
Vector3D& Vector3D::Transform( Vector3D& o, const Vector3D& in, const Matrix44& mat )
{
	float values[3];

	for( int i = 0; i < 3; i++ )
	{
		values[i] = 0;
		values[i] += in.X * mat.Values[0][i];
		values[i] += in.Y * mat.Values[1][i];
		values[i] += in.Z * mat.Values[2][i];
		values[i] += mat.Values[3][i];
	}

	o.X = values[0];
	o.Y = values[1];
	o.Z = values[2];
	return o;
}

Vector3D& Vector3D::TransformWithW(Vector3D& o, const Vector3D& in, const Matrix44& mat)
{
	float values[4];

	for (int i = 0; i < 4; i++)
	{
		values[i] = 0;
		values[i] += in.X * mat.Values[0][i];
		values[i] += in.Y * mat.Values[1][i];
		values[i] += in.Z * mat.Values[2][i];
		values[i] += mat.Values[3][i];
	}

	o.X = values[0] / values[3];
	o.Y = values[1] / values[3];
	o.Z = values[2] / values[3];
	return o;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
 } 
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------


//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
#if (defined(_M_IX86_FP) && _M_IX86_FP >= 2) || defined(__SSE__)
#define EFK_SSE2
#elif defined(__ARM_NEON__)
#define EFK_NEON
#endif

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
namespace Effekseer
{

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
Color::Color( uint8_t r, uint8_t g, uint8_t b, uint8_t a )
	: R	( r )
	, G	( g )
	, B	( b )
	, A	( a )
{
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
Color Color::Mul( Color in1, Color in2 )
{
#if defined(EFK_SSE2)
	__m128i s1 = _mm_cvtsi32_si128(*(int32_t*)&in1);
	__m128i s2 = _mm_cvtsi32_si128(*(int32_t*)&in2);
	__m128i zero = _mm_setzero_si128();
	__m128i mask = _mm_set1_epi16(1);

	s1 = _mm_unpacklo_epi8(s1, zero);
	s2 = _mm_unpacklo_epi8(s2, zero);

	__m128i r0 = _mm_mullo_epi16(s1, s2);
	__m128i r1 = _mm_srli_epi16(r0, 8);
	__m128i r2 = _mm_and_si128(r0, mask);
	__m128i r3 = _mm_or_si128(r2, r1);
	__m128i res = _mm_packus_epi16(r3, zero);
	
	Color o;
	*(int*)&o = _mm_cvtsi128_si32(res);
	return o;
#elif defined(EFK_NEON)
	uint8x8_t s1 = vreinterpret_u8_u32(vmov_n_u32(*(uint32_t*)&in1));
	uint8x8_t s2 = vreinterpret_u8_u32(vmov_n_u32(*(uint32_t*)&in2));
	uint16x8_t mask = vmovq_n_u16(1);
	uint16x8_t s3 = vmovl_u8(s1);
	uint16x8_t s4 = vmovl_u8(s2);
	uint16x8_t r0 = vmulq_u16(s3, s4);
	uint16x8_t r1 = vshrq_n_u16(r0, 8);
	uint16x8_t r2 = vandq_u16(r0, mask);
	uint16x8_t r3 = vorrq_u16(r2, r1);
	uint8x8_t res = vqmovn_u16(r3);
	
	Color o;
	*(uint32_t*)&o = vget_lane_u32(vreinterpret_u32_u8(res), 0);
	return o;
#else
	Color o;
	o.R = (uint8_t)((float)in1.R * (float)in2.R / 255.0f);
	o.G = (uint8_t)((float)in1.G * (float)in2.G / 255.0f);
	o.B = (uint8_t)((float)in1.B * (float)in2.B / 255.0f);
	o.A = (uint8_t)((float)in1.A * (float)in2.A / 255.0f);
	return o;
#endif
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
Color Color::Mul( Color in1, float in2 )
{
#if defined(EFK_SSE2)
	__m128i s1 = _mm_cvtsi32_si128(*(int32_t*)&in1);
	__m128i s2 = _mm_set1_epi16((int16_t)(in2 * 256));
	__m128i zero = _mm_setzero_si128();
	
	s1 = _mm_unpacklo_epi8(s1, zero);

	__m128i res = _mm_mullo_epi16(s1, s2);
	res = _mm_srli_epi16(res, 8);
	res = _mm_packus_epi16(res, zero);
	
	Color o;
	*(int*)&o = _mm_cvtsi128_si32(res);
	return o;
#elif defined(EFK_NEON)
	uint8x8_t s1 = vreinterpret_u8_u32(vmov_n_u32(*(uint32_t*)&in1));
	uint16x8_t s2 = vmovq_n_u16((uint16_t)(in2 * 256));
	uint16x8_t s3 = vmovl_u8(s1);
	uint16x8_t r0 = vmulq_u16(s3, s2);
	uint16x8_t r1 = vshrq_n_u16(r0, 8);
	uint8x8_t res = vqmovn_u16(r1);
	
	Color o;
	*(uint32_t*)&o = vget_lane_u32(vreinterpret_u32_u8(res), 0);
	return o;
#else
	Color o;
	o.R = (uint8_t)Clamp((int)((float)in1.R * in2), 255, 0);
	o.G = (uint8_t)Clamp((int)((float)in1.G * in2), 255, 0);
	o.B = (uint8_t)Clamp((int)((float)in1.B * in2), 255, 0);
	o.A = (uint8_t)Clamp((int)((float)in1.A * in2), 255, 0);
	return o;
#endif
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
Color Color::Lerp( const Color in1, const Color in2, float t )
{
	/*
#if defined(EFK_SSE2)
	__m128i s1 = _mm_cvtsi32_si128(*(int32_t*)&in1);
	__m128i s2 = _mm_cvtsi32_si128(*(int32_t*)&in2);
	__m128i tm = _mm_set1_epi16((int16_t)(t * 256));
	__m128i zero = _mm_setzero_si128();

	s1 = _mm_unpacklo_epi8(s1, zero);
	s2 = _mm_unpacklo_epi8(s2, zero);
	
	__m128i r0 = _mm_subs_epi16(s2, s1);
	__m128i r1 = _mm_mullo_epi16(r0, tm);
	__m128i r2 = _mm_srai_epi16(r1, 8);
	__m128i r3 = _mm_adds_epi16(s1, r2);
	__m128i res = _mm_packus_epi16(r3, zero);
	
	Color o;
	*(int*)&o = _mm_cvtsi128_si32(res);
	return o;
#elif defined(EFK_NEON)

#ifdef __ANDROID__
	uint8x8_t s1 = vreinterpret_u8_u32(vmov_n_u32(*(uint32_t*)&in1));
	uint8x8_t s2 = vreinterpret_u8_u32(vmov_n_u32(*(uint32_t*)&in2));
	int16x8_t tm = vmovq_n_s16((int16_t)(t * 256));
	uint16x8_t s3 = vmovl_u8(s1);
	uint16x8_t s4 = vmovl_u8(s2);
	int16x8_t r0 = (int16x8_t)vqsubq_s16((int16x8_t)s4, (int16x8_t)s3);
	int16x8_t r1 = vmulq_s16(r0, tm);
	int16x8_t r2 = vrshrq_n_s16(r1, 8);
	int16x8_t r3 = (int16x8_t)vqaddq_s16((int16x8_t)s3, r2);
	uint8x8_t res = (uint8x8_t)vqmovn_u16((uint16x8_t)r3);

	Color o;
	*(uint32_t*)&o = vget_lane_u32(vreinterpret_u32_u8(res), 0);
	return o;
#else
	uint8x8_t s1 = vreinterpret_u8_u32(vmov_n_u32(*(uint32_t*)&in1));
	uint8x8_t s2 = vreinterpret_u8_u32(vmov_n_u32(*(uint32_t*)&in2));
	int16x8_t tm = vmovq_n_s16((int16_t)(t * 256));
	uint16x8_t s3 = vmovl_u8(s1);
	uint16x8_t s4 = vmovl_u8(s2);
	int16x8_t r0 = vqsubq_s16(s4, s3);
	int16x8_t r1 = vmulq_s16(r0, tm);
	int16x8_t r2 = vrshrq_n_s16(r1, 8);
	int16x8_t r3 = vqaddq_s16(s3, r2);
	uint8x8_t res = vqmovn_u16(r3);
	
	Color o;
	*(uint32_t*)&o = vget_lane_u32(vreinterpret_u32_u8(res), 0);
	return o;
#endif

#else
	*/
	Color o;
	o.R = (uint8_t)Clamp( in1.R + (in2.R - in1.R) * t, 255, 0 );
	o.G = (uint8_t)Clamp( in1.G + (in2.G - in1.G) * t, 255, 0 );
	o.B = (uint8_t)Clamp( in1.B + (in2.B - in1.B) * t, 255, 0 );
	o.A = (uint8_t)Clamp( in1.A + (in2.A - in1.A) * t, 255, 0 );
	return o;
//#endif
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
namespace Effekseer { 
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
RectF::RectF()
	: X			( 0.0f )
	, Y			( 0.0f )
	, Width		( 1.0f )
	, Height	( 1.0f )
{
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
RectF::RectF( float x, float y, float width, float height )
	: X			( x )
	, Y			( y )
	, Width		( width )
	, Height	( height )
{
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
Vector2D RectF::Position() const
{
	return Vector2D( X, Y );
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
Vector2D RectF::Size() const
{
	return Vector2D( Width, Height );
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
 } 
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------


//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
#if (defined(_M_IX86_FP) && _M_IX86_FP >= 2) || defined(__SSE__)
#define EFK_SSE2
#elif defined(__ARM_NEON__)
#define EFK_NEON
#endif

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
#if defined(_MSC_VER)
#define EFK_ALIGN_AS(n) __declspec(align(n))
#else
#define EFK_ALIGN_AS(n) alignas(n)
#endif

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
namespace Effekseer {

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void Matrix43::Indentity()
{
	static const Matrix43 indentity = {{
		{1.0f, 0.0f, 0.0f}, 
		{0.0f, 1.0f, 0.0f}, 
		{0.0f, 0.0f, 1.0f},
		{0.0f, 0.0f, 0.0f}
	}};
	memcpy( Value, indentity.Value, sizeof(indentity) );
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void Matrix43::Scaling( float x, float y, float z )
{
	memset( Value, 0, sizeof(float) * 12 );
	Value[0][0] = x;
	Value[1][1] = y;
	Value[2][2] = z;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void Matrix43::RotationX( float angle )
{
	float c, s;
	::Effekseer::SinCos( angle, s, c );

	Value[0][0] = 1.0f;
	Value[0][1] = 0.0f;
	Value[0][2] = 0.0f;

	Value[1][0] = 0.0f;
	Value[1][1] = c;
	Value[1][2] = s;

	Value[2][0] = 0.0f;
	Value[2][1] = -s;
	Value[2][2] = c;

	Value[3][0] = 0.0f;
	Value[3][1] = 0.0f;
	Value[3][2] = 0.0f;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void Matrix43::RotationY( float angle )
{
	float c, s;
	::Effekseer::SinCos( angle, s, c );

	Value[0][0] = c;
	Value[0][1] = 0.0f;
	Value[0][2] = -s;

	Value[1][0] = 0.0f;
	Value[1][1] = 1.0f;
	Value[1][2] = 0.0f;

	Value[2][0] = s;
	Value[2][1] = 0.0f;
	Value[2][2] = c;

	Value[3][0] = 0.0f;
	Value[3][1] = 0.0f;
	Value[3][2] = 0.0f;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void Matrix43::RotationZ( float angle )
{
	float c, s;
	::Effekseer::SinCos( angle, s, c );

	Value[0][0] = c;
	Value[0][1] = s;
	Value[0][2] = 0.0f;

	Value[1][0] = -s;
	Value[1][1] = c;
	Value[1][2] = 0.0f;

	Value[2][0] = 0.0f;
	Value[2][1] = 0.0f;
	Value[2][2] = 1;

	Value[3][0] = 0.0f;
	Value[3][1] = 0.0f;
	Value[3][2] = 0.0f;
}
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void Matrix43::RotationXYZ( float rx, float ry, float rz )
{
	float cx, sx, cy, sy, cz, sz;
	
	if( rx != 0.0f )
	{
		::Effekseer::SinCos( rx, sx, cx );
	}
	else
	{
		sx = 0.0f;
		cx = 1.0f;
	}
	if( ry != 0.0f )
	{
		::Effekseer::SinCos( ry, sy, cy );
	}
	else
	{
		sy = 0.0f;
		cy = 1.0f;
	}
	if( rz != 0.0f )
	{
		::Effekseer::SinCos( rz, sz, cz );
	}
	else
	{
		sz = 0.0f;
		cz = 1.0f;
	}

	Value[0][0] = cy * cz;
	Value[0][1] = cy * sz;
	Value[0][2] = -sy;

	Value[1][0] = sx * sy * -sz + cx * -sz;
	Value[1][1] = sx * sy *  sz + cx *  cz;
	Value[1][2] = sx * cy;

	Value[2][0] = cx * sy * cz + sx * sz;
	Value[2][1] = cx * sy * sz - sx * cz;
	Value[2][2] = cx * cy;

	Value[3][0] = 0.0f;
	Value[3][1] = 0.0f;
	Value[3][2] = 0.0f;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void Matrix43::RotationZXY( float rz, float rx, float ry )
{
	float cx, sx, cy, sy, cz, sz;

	if( rx != 0.0f )
	{
		::Effekseer::SinCos( rx, sx, cx );
	}
	else
	{
		sx = 0.0f;
		cx = 1.0f;
	}
	if( ry != 0.0f )
	{
		::Effekseer::SinCos( ry, sy, cy );
	}
	else
	{
		sy = 0.0f;
		cy = 1.0f;
	}
	if( rz != 0.0f )
	{
		::Effekseer::SinCos( rz, sz, cz );
	}
	else
	{
		sz = 0.0f;
		cz = 1.0f;
	}
	
	Value[0][0] = cz * cy + sz * sx * sy;
	Value[0][1] = sz * cx;
	Value[0][2] = cz * -sy + sz * sx * cy;

	Value[1][0] = -sz * cy + cz * sx * sy;
	Value[1][1] = cz * cx;
	Value[1][2] = -sz * -sy + cz * sx * cy;

	Value[2][0] = cx * sy;
	Value[2][1] = -sx;
	Value[2][2] = cx * cy;
	
	Value[3][0] = 0.0f;
	Value[3][1] = 0.0f;
	Value[3][2] = 0.0f;
}
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void Matrix43::RotationAxis( const Vector3D& axis, float angle )
{
	const float c = cosf( angle );
	const float s = sinf( angle );
	const float cc = 1.0f - c;

	Value[0][0] = cc * (axis.X * axis.X) + c;
	Value[0][1] = cc * (axis.X * axis.Y) + (axis.Z * s);
	Value[0][2] = cc * (axis.Z * axis.X) - (axis.Y * s);

	Value[1][0] = cc * (axis.X * axis.Y) - (axis.Z * s);
	Value[1][1] = cc * (axis.Y * axis.Y) + c;
	Value[1][2] = cc * (axis.Y * axis.Z) + (axis.X * s);

	Value[2][0] = cc * (axis.Z * axis.X) + (axis.Y * s);
	Value[2][1] = cc * (axis.Y * axis.Z) - (axis.X * s);
	Value[2][2] = cc * (axis.Z * axis.Z) + c;

	Value[3][0] = 0.0f;
	Value[3][1] = 0.0f;
	Value[3][2] = 0.0f;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void Matrix43::RotationAxis( const Vector3D& axis, float s, float c )
{
	const float cc = 1.0f - c;

	Value[0][0] = cc * (axis.X * axis.X) + c;
	Value[0][1] = cc * (axis.X * axis.Y) + (axis.Z * s);
	Value[0][2] = cc * (axis.Z * axis.X) - (axis.Y * s);

	Value[1][0] = cc * (axis.X * axis.Y) - (axis.Z * s);
	Value[1][1] = cc * (axis.Y * axis.Y) + c;
	Value[1][2] = cc * (axis.Y * axis.Z) + (axis.X * s);

	Value[2][0] = cc * (axis.Z * axis.X) + (axis.Y * s);
	Value[2][1] = cc * (axis.Y * axis.Z) - (axis.X * s);
	Value[2][2] = cc * (axis.Z * axis.Z) + c;

	Value[3][0] = 0.0f;
	Value[3][1] = 0.0f;
	Value[3][2] = 0.0f;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void Matrix43::Translation( float x, float y, float z )
{
	Indentity();
	Value[3][0] = x;
	Value[3][1] = y;
	Value[3][2] = z;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void Matrix43::GetSRT( Vector3D& s, Matrix43& r, Vector3D& t ) const
{
#if defined(EFK_SSE2)
	t.X = Value[3][0];
	t.Y = Value[3][1];
	t.Z = Value[3][2];

	__m128 v0 = _mm_loadu_ps(&Value[0][0]);
	__m128 v1 = _mm_loadu_ps(&Value[1][0]);
	__m128 v2 = _mm_loadu_ps(&Value[2][0]);
	__m128 m0 = _mm_shuffle_ps(v0, v1, _MM_SHUFFLE(1, 0, 1, 0));
	__m128 m1 = _mm_shuffle_ps(v0, v1, _MM_SHUFFLE(3, 2, 3, 2));
	__m128 s0 = _mm_shuffle_ps(m0, v2, _MM_SHUFFLE(0, 0, 2, 0));
	__m128 s1 = _mm_shuffle_ps(m0, v2, _MM_SHUFFLE(0, 1, 3, 1));
	__m128 s2 = _mm_shuffle_ps(m1, v2, _MM_SHUFFLE(0, 2, 2, 0));
	s0 = _mm_mul_ps(s0, s0);
	s1 = _mm_mul_ps(s1, s1);
	s2 = _mm_mul_ps(s2, s2);
	__m128 vscq = _mm_add_ps(_mm_add_ps(s0, s1), s2);
	__m128 vsc = _mm_sqrt_ps(vscq);
	__m128 vscr = _mm_div_ps(vsc, vscq);
	EFK_ALIGN_AS(16) float sc[4];
	_mm_store_ps(sc, vsc);
	s.X = sc[0];
	s.Y = sc[1];
	s.Z = sc[2];
	v0 = _mm_mul_ps(v0, _mm_shuffle_ps(vscr, vscr, _MM_SHUFFLE(0, 0, 0, 0)));
	v1 = _mm_mul_ps(v1, _mm_shuffle_ps(vscr, vscr, _MM_SHUFFLE(1, 1, 1, 1)));
	v2 = _mm_mul_ps(v2, _mm_shuffle_ps(vscr, vscr, _MM_SHUFFLE(2, 2, 2, 2)));
	_mm_storeu_ps(&r.Value[0][0], v0);
	_mm_storeu_ps(&r.Value[1][0], v1);
	_mm_storeu_ps(&r.Value[2][0], v2);
	r.Value[3][0] = 0.0f;
	r.Value[3][1] = 0.0f;
	r.Value[3][2] = 0.0f;
#elif defined(EFK_NEON)
	t.X = Value[3][0];
	t.Y = Value[3][1];
	t.Z = Value[3][2];

	float32x4x3_t m = vld3q_f32(&Value[0][0]);
	float32x4_t vscq = vmulq_f32(m.val[0], m.val[0]);
	vscq = vmlaq_f32(vscq, m.val[1], m.val[1]);
	vscq = vmlaq_f32(vscq, m.val[2], m.val[2]);
	float32x4_t scr_rep = vrsqrteq_f32(vscq);
	float32x4_t scr_v = vmulq_f32(vrsqrtsq_f32(vmulq_f32(vscq, scr_rep), scr_rep), scr_rep);
	float32x4_t sc_v = vmulq_f32(scr_v, vscq);
	float sc[4];
	vst1q_f32(sc, sc_v);
	s.X = sc[0];
	s.Y = sc[1];
	s.Z = sc[2];
	float32x4_t v0 = vld1q_f32(&Value[0][0]);
	float32x4_t v1 = vld1q_f32(&Value[1][0]);
	float32x4_t v2 = vld1q_f32(&Value[2][0]);
	vst1q_f32(&r.Value[0][0], vmulq_lane_f32(v0, vget_low_f32(scr_v), 0));
	vst1q_f32(&r.Value[1][0], vmulq_lane_f32(v1, vget_low_f32(scr_v), 1));
	vst1q_f32(&r.Value[2][0], vmulq_lane_f32(v2, vget_high_f32(scr_v), 0));
	r.Value[3][0] = 0.0f;
	r.Value[3][1] = 0.0f;
	r.Value[3][2] = 0.0f;
#else
	t.X = Value[3][0];
	t.Y = Value[3][1];
	t.Z = Value[3][2];
	
	float sc[3];
	for( int m = 0; m < 3; m++ )
	{
		sc[m] = sqrt( Value[m][0] * Value[m][0] + Value[m][1] * Value[m][1] + Value[m][2] * Value[m][2] );
	}
	
	s.X = sc[0];
	s.Y = sc[1];
	s.Z = sc[2];
	
	for( int m = 0; m < 3; m++ )
	{
		for( int n = 0; n < 3; n++ )
		{
			r.Value[m][n] = Value[m][n] / sc[m];
		}
	}
	r.Value[3][0] = 0.0f;
	r.Value[3][1] = 0.0f;
	r.Value[3][2] = 0.0f;
#endif
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void Matrix43::GetScale( Vector3D& s ) const
{
#if defined(EFK_SSE2)
	__m128 v0 = _mm_loadu_ps(&Value[0][0]);
	__m128 v1 = _mm_loadu_ps(&Value[1][0]);
	__m128 v2 = _mm_loadu_ps(&Value[2][0]);
	__m128 m0 = _mm_shuffle_ps(v0, v1, _MM_SHUFFLE(1, 0, 1, 0));
	__m128 m1 = _mm_shuffle_ps(v0, v1, _MM_SHUFFLE(3, 2, 3, 2));
	__m128 s0 = _mm_shuffle_ps(m0, v2, _MM_SHUFFLE(0, 0, 2, 0));
	__m128 s1 = _mm_shuffle_ps(m0, v2, _MM_SHUFFLE(0, 1, 3, 1));
	__m128 s2 = _mm_shuffle_ps(m1, v2, _MM_SHUFFLE(0, 2, 2, 0));
	s0 = _mm_mul_ps(s0, s0);
	s1 = _mm_mul_ps(s1, s1);
	s2 = _mm_mul_ps(s2, s2);
	__m128 vscq = _mm_add_ps(_mm_add_ps(s0, s1), s2);
	__m128 sc_v = _mm_sqrt_ps(vscq);
	EFK_ALIGN_AS(16) float sc[4];
	_mm_store_ps(sc, sc_v);
	s.X = sc[0];
	s.Y = sc[1];
	s.Z = sc[2];
#elif defined(EFK_NEON)
	float32x4x3_t m = vld3q_f32(&Value[0][0]);
	float32x4_t vscq = vmulq_f32(m.val[0], m.val[0]);
	vscq = vmlaq_f32(vscq, m.val[1], m.val[1]);
	vscq = vmlaq_f32(vscq, m.val[2], m.val[2]);
	float32x4_t scr_rep = vrsqrteq_f32(vscq);
	float32x4_t scr_v = vmulq_f32(vrsqrtsq_f32(vmulq_f32(vscq, scr_rep), scr_rep), scr_rep);
	float32x4_t sc_v = vmulq_f32(scr_v, vscq);
	float sc[4];
	vst1q_f32(sc, sc_v);
	s.X = sc[0];
	s.Y = sc[1];
	s.Z = sc[2];
#else
	float sc[3];
	for( int m = 0; m < 3; m++ )
	{
		sc[m] = sqrt( Value[m][0] * Value[m][0] + Value[m][1] * Value[m][1] + Value[m][2] * Value[m][2] );
	}
	
	s.X = sc[0];
	s.Y = sc[1];
	s.Z = sc[2];
#endif
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void Matrix43::GetRotation( Matrix43& r ) const
{
#if defined(EFK_SSE2)
	__m128 v0 = _mm_loadu_ps(&Value[0][0]);
	__m128 v1 = _mm_loadu_ps(&Value[1][0]);
	__m128 v2 = _mm_loadu_ps(&Value[2][0]);
	__m128 m0 = _mm_shuffle_ps(v0, v1, _MM_SHUFFLE(1, 0, 1, 0));
	__m128 m1 = _mm_shuffle_ps(v0, v1, _MM_SHUFFLE(3, 2, 3, 2));
	__m128 s0 = _mm_shuffle_ps(m0, v2, _MM_SHUFFLE(0, 0, 2, 0));
	__m128 s1 = _mm_shuffle_ps(m0, v2, _MM_SHUFFLE(0, 1, 3, 1));
	__m128 s2 = _mm_shuffle_ps(m1, v2, _MM_SHUFFLE(0, 2, 2, 0));
	s0 = _mm_mul_ps(s0, s0);
	s1 = _mm_mul_ps(s1, s1);
	s2 = _mm_mul_ps(s2, s2);
	__m128 vscq = _mm_add_ps(_mm_add_ps(s0, s1), s2);
	__m128 vsc = _mm_sqrt_ps(vscq);
	__m128 vscr = _mm_div_ps(vsc, vscq);
	v0 = _mm_mul_ps(v0, _mm_shuffle_ps(vscr, vscr, _MM_SHUFFLE(0, 0, 0, 0)));
	v1 = _mm_mul_ps(v1, _mm_shuffle_ps(vscr, vscr, _MM_SHUFFLE(1, 1, 1, 1)));
	v2 = _mm_mul_ps(v2, _mm_shuffle_ps(vscr, vscr, _MM_SHUFFLE(2, 2, 2, 2)));
	_mm_storeu_ps(&r.Value[0][0], v0);
	_mm_storeu_ps(&r.Value[1][0], v1);
	_mm_storeu_ps(&r.Value[2][0], v2);
	r.Value[3][0] = 0.0f;
	r.Value[3][1] = 0.0f;
	r.Value[3][2] = 0.0f;
#elif defined(EFK_NEON)
	float32x4x3_t m = vld3q_f32(&Value[0][0]);
	float32x4_t vscq = vmulq_f32(m.val[0], m.val[0]);
	vscq = vmlaq_f32(vscq, m.val[1], m.val[1]);
	vscq = vmlaq_f32(vscq, m.val[2], m.val[2]);
	float32x4_t scr_rep = vrsqrteq_f32(vscq);
	float32x4_t scr_v = vmulq_f32(vrsqrtsq_f32(vmulq_f32(vscq, scr_rep), scr_rep), scr_rep);
	float32x4_t v0 = vld1q_f32(&Value[0][0]);
	float32x4_t v1 = vld1q_f32(&Value[1][0]);
	float32x4_t v2 = vld1q_f32(&Value[2][0]);
	vst1q_f32(&r.Value[0][0], vmulq_lane_f32(v0, vget_low_f32(scr_v), 0));
	vst1q_f32(&r.Value[1][0], vmulq_lane_f32(v1, vget_low_f32(scr_v), 1));
	vst1q_f32(&r.Value[2][0], vmulq_lane_f32(v2, vget_high_f32(scr_v), 0));
	r.Value[3][0] = 0.0f;
	r.Value[3][1] = 0.0f;
	r.Value[3][2] = 0.0f;
#else
	float sc[3];
	for( int m = 0; m < 3; m++ )
	{
		sc[m] = sqrt( Value[m][0] * Value[m][0] + Value[m][1] * Value[m][1] + Value[m][2] * Value[m][2] );
	}

	for( int m = 0; m < 3; m++ )
	{
		for( int n = 0; n < 3; n++ )
		{
			r.Value[m][n] = Value[m][n] / sc[m];
		}
	}
	r.Value[3][0] = 0.0f;
	r.Value[3][1] = 0.0f;
	r.Value[3][2] = 0.0f;
#endif
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void Matrix43::GetTranslation( Vector3D& t ) const
{
	t.X = Value[3][0];
	t.Y = Value[3][1];
	t.Z = Value[3][2];
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void Matrix43::SetSRT( const Vector3D& s, const Matrix43& r, const Vector3D& t )
{
	Value[0][0] = s.X * r.Value[0][0];
	Value[0][1] = s.X * r.Value[0][1];
	Value[0][2] = s.X * r.Value[0][2];
	Value[1][0] = s.Y * r.Value[1][0];
	Value[1][1] = s.Y * r.Value[1][1];
	Value[1][2] = s.Y * r.Value[1][2];
	Value[2][0] = s.Z * r.Value[2][0];
	Value[2][1] = s.Z * r.Value[2][1];
	Value[2][2] = s.Z * r.Value[2][2];
	Value[3][0] = t.X;
	Value[3][1] = t.Y;
	Value[3][2] = t.Z;
}

void Matrix43::ToMatrix44(Matrix44& dst)
{
	for (int m = 0; m < 4; m++)
	{
		for (int n = 0; n < 3; n++)
		{
			dst.Values[m][n] = Value[m][n];
		}
		dst.Values[m][3] = 0.0f;
	}

	dst.Values[3][3] = 1.0f;
}

bool Matrix43::IsValid() const
{
	for (int m = 0; m < 4; m++)
	{
		for (int n = 0; n < 3; n++)
		{
			if (isinf(Value[m][n]))
				return false;
			if (isnan(Value[m][n]))
				return false;
		}
	}
	return true;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void Matrix43::Multiple( Matrix43& out, const Matrix43& in1, const Matrix43& in2 )
{
#if defined(EFK_SSE2)
	__m128 s1_v0 = _mm_loadu_ps(&in1.Value[0][0]);
	__m128 s1_v1 = _mm_loadu_ps(&in1.Value[1][0]);
	__m128 s1_v2 = _mm_loadu_ps(&in1.Value[2][0]);
	__m128 s1_v3 = _mm_loadu_ps(&in1.Value[3][0] - 1);
	__m128 s2_v0 = _mm_loadu_ps(&in2.Value[0][0]);
	__m128 s2_v1 = _mm_loadu_ps(&in2.Value[1][0]);
	__m128 s2_v2 = _mm_loadu_ps(&in2.Value[2][0]);
	__m128 s2_v3 = _mm_loadu_ps(&in2.Value[3][0] - 1);
	__m128 o_v3;

	{
		__m128 s1_00 = _mm_shuffle_ps(s1_v0, s1_v0, _MM_SHUFFLE(0, 0, 0, 0));
		__m128 s1_01 = _mm_shuffle_ps(s1_v0, s1_v0, _MM_SHUFFLE(1, 1, 1, 1));
		__m128 s1_02 = _mm_shuffle_ps(s1_v0, s1_v0, _MM_SHUFFLE(2, 2, 2, 2));
		__m128 o_m00 = _mm_mul_ps(s1_00, s2_v0);
		__m128 o_m01 = _mm_mul_ps(s1_01, s2_v1);
		__m128 o_m02 = _mm_mul_ps(s1_02, s2_v2);
		__m128 o_v0 = _mm_add_ps(_mm_add_ps(o_m00, o_m01), o_m02);
		_mm_storeu_ps(&out.Value[0][0], o_v0);
	}
	{
		__m128 s1_10 = _mm_shuffle_ps(s1_v1, s1_v1, _MM_SHUFFLE(0, 0, 0, 0));
		__m128 s1_11 = _mm_shuffle_ps(s1_v1, s1_v1, _MM_SHUFFLE(1, 1, 1, 1));
		__m128 s1_12 = _mm_shuffle_ps(s1_v1, s1_v1, _MM_SHUFFLE(2, 2, 2, 2));
		__m128 o_m10 = _mm_mul_ps(s1_10, s2_v0);
		__m128 o_m11 = _mm_mul_ps(s1_11, s2_v1);
		__m128 o_m12 = _mm_mul_ps(s1_12, s2_v2);
		__m128 o_v1 = _mm_add_ps(_mm_add_ps(o_m10, o_m11), o_m12);
		_mm_storeu_ps(&out.Value[1][0], o_v1);
	}
	{
		__m128 s1_20 = _mm_shuffle_ps(s1_v2, s1_v2, _MM_SHUFFLE(0, 0, 0, 0));
		__m128 s1_21 = _mm_shuffle_ps(s1_v2, s1_v2, _MM_SHUFFLE(1, 1, 1, 1));
		__m128 s1_22 = _mm_shuffle_ps(s1_v2, s1_v2, _MM_SHUFFLE(2, 2, 2, 2));
		__m128 o_m20 = _mm_mul_ps(s1_20, s2_v0);
		__m128 o_m21 = _mm_mul_ps(s1_21, s2_v1);
		__m128 o_m22 = _mm_mul_ps(s1_22, s2_v2);
		__m128 o_v2 = _mm_add_ps(_mm_add_ps(o_m20, o_m21), o_m22);
		_mm_storeu_ps(&out.Value[2][0], o_v2);
		o_v3 = _mm_shuffle_ps(o_v2, o_v2, _MM_SHUFFLE(2, 2, 2, 2));
	}
	{
		EFK_ALIGN_AS(16) const uint32_t mask_u32[4] = {0xffffffff, 0x00000000, 0x00000000, 0x00000000};
		__m128 mask = _mm_load_ps((const float*)mask_u32);
		s2_v0 = _mm_shuffle_ps(s2_v0, s2_v0, _MM_SHUFFLE(2, 1, 0, 0));
		s2_v1 = _mm_shuffle_ps(s2_v1, s2_v1, _MM_SHUFFLE(2, 1, 0, 0));
		s2_v2 = _mm_shuffle_ps(s2_v2, s2_v2, _MM_SHUFFLE(2, 1, 0, 0));
		__m128 s1_30 = _mm_shuffle_ps(s1_v3, s1_v3, _MM_SHUFFLE(1, 1, 1, 1));
		__m128 s1_31 = _mm_shuffle_ps(s1_v3, s1_v3, _MM_SHUFFLE(2, 2, 2, 2));
		__m128 s1_32 = _mm_shuffle_ps(s1_v3, s1_v3, _MM_SHUFFLE(3, 3, 3, 3));
		__m128 o_m30 = _mm_mul_ps(s1_30, s2_v0);
		__m128 o_m31 = _mm_mul_ps(s1_31, s2_v1);
		__m128 o_m32 = _mm_mul_ps(s1_32, s2_v2);
		__m128 o_v3p = _mm_add_ps(_mm_add_ps(o_m30, o_m31), _mm_add_ps(o_m32, s2_v3));
		o_v3 = _mm_or_ps(_mm_and_ps(mask, o_v3), _mm_andnot_ps(mask, o_v3p));
		_mm_storeu_ps(&out.Value[3][0] - 1, o_v3);
	}
#elif defined(EFK_NEON)
	float32x4_t s1_v0 = vld1q_f32(&in1.Value[0][0]);
	float32x4_t s1_v12 = vld1q_f32(&in1.Value[1][1]);
	float32x4_t s1_v3 = vld1q_f32(&in1.Value[2][2]);
	float32x4_t s1_v1 = vextq_f32(s1_v0, s1_v12, 3);
	float32x4_t s1_v2 = vextq_f32(s1_v12, s1_v3, 2);
	float32x4_t s2_v0 = vld1q_f32(&in2.Value[0][0]);
	float32x4_t s2_v12 = vld1q_f32(&in2.Value[1][1]);
	float32x4_t s2_v3 = vld1q_f32(&in2.Value[2][2]);
	float32x4_t s2_v1 = vextq_f32(s2_v0, s2_v12, 3);
	float32x4_t s2_v2 = vextq_f32(s2_v12, s2_v3, 2);
	float o_v3_0;
	{
		float32x4_t o_v0 = vmulq_lane_f32(s2_v0, vget_low_f32(s1_v0), 0);
		float32x4_t o_v1 = vmulq_lane_f32(s2_v0, vget_low_f32(s1_v1), 0);
		float32x4_t o_v2 = vmulq_lane_f32(s2_v0, vget_low_f32(s1_v2), 0);
		o_v0 = vmlaq_lane_f32(o_v0, s2_v1, vget_low_f32(s1_v0), 1);
		o_v1 = vmlaq_lane_f32(o_v1, s2_v1, vget_low_f32(s1_v1), 1);
		o_v2 = vmlaq_lane_f32(o_v2, s2_v1, vget_low_f32(s1_v2), 1);
		o_v0 = vmlaq_lane_f32(o_v0, s2_v2, vget_high_f32(s1_v0), 0);
		o_v1 = vmlaq_lane_f32(o_v1, s2_v2, vget_high_f32(s1_v1), 0);
		o_v2 = vmlaq_lane_f32(o_v2, s2_v2, vget_high_f32(s1_v2), 0);
		vst1q_f32(&out.Value[0][0], o_v0);
		vst1q_f32(&out.Value[1][0], o_v1);
		vst1q_f32(&out.Value[2][0], o_v2);
		o_v3_0 = vgetq_lane_f32(o_v2, 2);
	}
	{
		s2_v0 = vextq_f32(s2_v0, s2_v0, 3);
		s2_v1 = vextq_f32(s2_v1, s2_v1, 3);
		s2_v2 = vextq_f32(s2_v2, s2_v2, 3);
		float32x4_t o_v3 = vmlaq_lane_f32(s2_v3, s2_v0, vget_low_f32(s1_v3), 1);
		o_v3 = vmlaq_lane_f32(o_v3, s2_v1, vget_high_f32(s1_v3), 0);
		o_v3 = vmlaq_lane_f32(o_v3, s2_v2, vget_high_f32(s1_v3), 1);
		vst1q_f32(&out.Value[3][0] - 1, vsetq_lane_f32(o_v3_0, o_v3, 0));
	}
#elif 1
	Matrix43 temp1, temp2;
	// 共通の場合は一時変数にコピー
	const Matrix43& s1 = (&out == &in1) ? (temp1 = in1) : in1;
	const Matrix43& s2 = (&out == &in2) ? (temp2 = in2) : in2;
	
	out.Value[0][0] = s1.Value[0][0] * s2.Value[0][0] + s1.Value[0][1] * s2.Value[1][0] + s1.Value[0][2] * s2.Value[2][0];
	out.Value[0][1] = s1.Value[0][0] * s2.Value[0][1] + s1.Value[0][1] * s2.Value[1][1] + s1.Value[0][2] * s2.Value[2][1];
	out.Value[0][2] = s1.Value[0][0] * s2.Value[0][2] + s1.Value[0][1] * s2.Value[1][2] + s1.Value[0][2] * s2.Value[2][2];
									   					 				 				   				   
	out.Value[1][0] = s1.Value[1][0] * s2.Value[0][0] + s1.Value[1][1] * s2.Value[1][0] + s1.Value[1][2] * s2.Value[2][0];
	out.Value[1][1] = s1.Value[1][0] * s2.Value[0][1] + s1.Value[1][1] * s2.Value[1][1] + s1.Value[1][2] * s2.Value[2][1];
	out.Value[1][2] = s1.Value[1][0] * s2.Value[0][2] + s1.Value[1][1] * s2.Value[1][2] + s1.Value[1][2] * s2.Value[2][2];
									   					 				 				   				   
	out.Value[2][0] = s1.Value[2][0] * s2.Value[0][0] + s1.Value[2][1] * s2.Value[1][0] + s1.Value[2][2] * s2.Value[2][0];
	out.Value[2][1] = s1.Value[2][0] * s2.Value[0][1] + s1.Value[2][1] * s2.Value[1][1] + s1.Value[2][2] * s2.Value[2][1];
	out.Value[2][2] = s1.Value[2][0] * s2.Value[0][2] + s1.Value[2][1] * s2.Value[1][2] + s1.Value[2][2] * s2.Value[2][2];
									   					 				 				   				   
	out.Value[3][0] = s1.Value[3][0] * s2.Value[0][0] + s1.Value[3][1] * s2.Value[1][0] + s1.Value[3][2] * s2.Value[2][0] + s2.Value[3][0];
	out.Value[3][1] = s1.Value[3][0] * s2.Value[0][1] + s1.Value[3][1] * s2.Value[1][1] + s1.Value[3][2] * s2.Value[2][1] + s2.Value[3][1];
	out.Value[3][2] = s1.Value[3][0] * s2.Value[0][2] + s1.Value[3][1] * s2.Value[1][2] + s1.Value[3][2] * s2.Value[2][2] + s2.Value[3][2];
#else
	Matrix43 temp;

	for( int i = 0; i < 4; i++ )
	{
		for( int j = 0; j < 3; j++ )
		{
			float v = 0.0f;
			for( int k = 0; k < 3; k++ )
			{
				v += in1.Value[i][k] * in2.Value[k][j];
			}
			temp.Value[i][j] = v;
		}
	}

	for( int i = 0; i < 3; i++ )
	{
		temp.Value[3][i] += in2.Value[3][i];
	}

	out = temp;
#endif
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
 } 
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------



//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
namespace Effekseer { 
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
Matrix44::Matrix44()
{
	Indentity();
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
Matrix44& Matrix44::Indentity()
{
	memset( Values, 0, sizeof(float) * 16 );
	Values[0][0] = 1.0f;
	Values[1][1] = 1.0f;
	Values[2][2] = 1.0f;
	Values[3][3] = 1.0f;
	return *this;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
Matrix44& Matrix44::Transpose()
{
	for (int32_t c = 0; c < 4; c++)
	{
		for (int32_t r = c; r < 4; r++)
		{
			float v = Values[r][c];
			Values[r][c] = Values[c][r];
			Values[c][r] = v;
		}
	}

	return *this;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
Matrix44& Matrix44::LookAtRH( const Vector3D& eye, const Vector3D& at, const Vector3D& up )
{
	// F=正面、R=右方向、U=上方向
	Vector3D F; 
	Vector3D R;
	Vector3D U;
	Vector3D::Normal( F, Vector3D::Sub( F, eye, at ) );
	Vector3D::Normal( R, Vector3D::Cross( R, up, F ) );
	Vector3D::Normal( U, Vector3D::Cross( U, F, R ) );

	Values[0][0] = R.X;
	Values[1][0] = R.Y;
	Values[2][0] = R.Z;
	Values[3][0] = 0.0f;

	Values[0][1] = U.X;
	Values[1][1] = U.Y;
	Values[2][1] = U.Z;
	Values[3][1] = 0.0f;

	Values[0][2] = F.X;
	Values[1][2] = F.Y;
	Values[2][2] = F.Z;
	Values[3][2] = 0.0f;

	Values[3][0] = - Vector3D::Dot( R, eye );
	Values[3][1] = - Vector3D::Dot( U, eye );
	Values[3][2] = - Vector3D::Dot( F, eye );
	Values[3][3] = 1.0f;
	return *this;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
Matrix44& Matrix44::LookAtLH( const Vector3D& eye, const Vector3D& at, const Vector3D& up )
{
	// F=正面、R=右方向、U=上方向
	Vector3D F; 
	Vector3D R;
	Vector3D U;
	Vector3D::Normal( F, Vector3D::Sub( F, at, eye ) );
	Vector3D::Normal( R, Vector3D::Cross( R, up, F ) );
	Vector3D::Normal( U, Vector3D::Cross( U, F, R ) );

	Values[0][0] = R.X;
	Values[1][0] = R.Y;
	Values[2][0] = R.Z;
	Values[3][0] = 0.0f;

	Values[0][1] = U.X;
	Values[1][1] = U.Y;
	Values[2][1] = U.Z;
	Values[3][1] = 0.0f;

	Values[0][2] = F.X;
	Values[1][2] = F.Y;
	Values[2][2] = F.Z;
	Values[3][2] = 0.0f;

	Values[3][0] = - Vector3D::Dot( R, eye );
	Values[3][1] = - Vector3D::Dot( U, eye );
	Values[3][2] = - Vector3D::Dot( F, eye );
	Values[3][3] = 1.0f;
	return *this;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
Matrix44& Matrix44::PerspectiveFovRH( float ovY, float aspect, float zn, float zf )
{
	float yScale = 1 / tanf( ovY / 2 );
	float xScale = yScale / aspect;

	Values[0][0] = xScale;
	Values[0][1] = 0;
	Values[0][2] = 0;
	Values[0][3] = 0;

	Values[1][0] = 0;
	Values[1][1] = yScale;
	Values[1][2] = 0;
	Values[1][3] = 0;

	Values[2][0] = 0;
	Values[2][1] = 0;
	Values[2][2] = zf / (zn-zf);
	Values[2][3] = -1;

	Values[3][0] = 0;
	Values[3][1] = 0;
	Values[3][2] = zn * zf / (zn-zf);
	Values[3][3] = 0;
	return *this;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
Matrix44& Matrix44::PerspectiveFovRH_OpenGL( float ovY, float aspect, float zn, float zf )
{
	float yScale = 1 / tanf( ovY / 2 );
	float xScale = yScale / aspect;
	float dz = zf - zn;

	Values[0][0] = xScale;
	Values[0][1] = 0;
	Values[0][2] = 0;
	Values[0][3] = 0;

	Values[1][0] = 0;
	Values[1][1] = yScale;
	Values[1][2] = 0;
	Values[1][3] = 0;

	Values[2][0] = 0;
	Values[2][1] = 0;
	Values[2][2] = -(zf + zn) / dz;
	Values[2][3] = -1.0f;

	Values[3][0] = 0;
	Values[3][1] = 0;
	Values[3][2] = -2.0f * zn * zf / dz;
	Values[3][3] = 0.0f;

	return *this;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
Matrix44& Matrix44::PerspectiveFovLH( float ovY, float aspect, float zn, float zf )
{
	float yScale = 1 / tanf( ovY / 2 );
	float xScale = yScale / aspect;

	Values[0][0] = xScale;
	Values[0][1] = 0;
	Values[0][2] = 0;
	Values[0][3] = 0;

	Values[1][0] = 0;
	Values[1][1] = yScale;
	Values[1][2] = 0;
	Values[1][3] = 0;

	Values[2][0] = 0;
	Values[2][1] = 0;
	Values[2][2] = zf / (zf-zn);
	Values[2][3] = 1;

	Values[3][0] = 0;
	Values[3][1] = 0;
	Values[3][2] = -zn * zf / (zf-zn);
	Values[3][3] = 0;
	return *this;
}
	
//----------------------------------------------------------------------------------
//
//---------------------------------------------------------------------------------
Matrix44& Matrix44::PerspectiveFovLH_OpenGL( float ovY, float aspect, float zn, float zf )
{
	float yScale = 1 / tanf( ovY / 2 );
	float xScale = yScale / aspect;
		
	Values[0][0] = xScale;
	Values[0][1] = 0;
	Values[0][2] = 0;
	Values[0][3] = 0;
		
	Values[1][0] = 0;
	Values[1][1] = yScale;
	Values[1][2] = 0;
	Values[1][3] = 0;
		
	Values[2][0] = 0;
	Values[2][1] = 0;
	Values[2][2] = zf / (zf-zn);
	Values[2][3] = 1;
		
	Values[3][0] = 0;
	Values[3][1] = 0;
	Values[3][2] = -2.0f * zn * zf / (zf-zn);
	Values[3][3] = 0;
	return *this;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
Matrix44& Matrix44::OrthographicRH( float width, float height, float zn, float zf )
{
	Values[0][0] = 2 / width;
	Values[0][1] = 0;
	Values[0][2] = 0;
	Values[0][3] = 0;

	Values[1][0] = 0;
	Values[1][1] = 2 / height;
	Values[1][2] = 0;
	Values[1][3] = 0;

	Values[2][0] = 0;
	Values[2][1] = 0;
	Values[2][2] = 1 / (zn-zf);
	Values[2][3] = 0;

	Values[3][0] = 0;
	Values[3][1] = 0;
	Values[3][2] = zn / (zn-zf);
	Values[3][3] = 1;
	return *this;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
Matrix44& Matrix44::OrthographicLH( float width, float height, float zn, float zf )
{
	Values[0][0] = 2 / width;
	Values[0][1] = 0;
	Values[0][2] = 0;
	Values[0][3] = 0;

	Values[1][0] = 0;
	Values[1][1] = 2 / height;
	Values[1][2] = 0;
	Values[1][3] = 0;

	Values[2][0] = 0;
	Values[2][1] = 0;
	Values[2][2] = 1 / (zf-zn);
	Values[2][3] = 0;

	Values[3][0] = 0;
	Values[3][1] = 0;
	Values[3][2] = zn / (zn-zf);
	Values[3][3] = 1;
	return *this;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void Matrix44::Scaling( float x, float y, float z )
{
	memset( Values, 0, sizeof(float) * 16 );
	Values[0][0] = x;
	Values[1][1] = y;
	Values[2][2] = z;
	Values[3][3] = 1.0f;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void Matrix44::RotationX( float angle )
{
	float c, s;
	SinCos( angle, s, c );

	Values[0][0] = 1.0f;
	Values[0][1] = 0.0f;
	Values[0][2] = 0.0f;
	Values[0][3] = 0.0f;

	Values[1][0] = 0.0f;
	Values[1][1] = c;
	Values[1][2] = s;
	Values[1][3] = 0.0f;

	Values[2][0] = 0.0f;
	Values[2][1] = -s;
	Values[2][2] = c;
	Values[2][3] = 0.0f;

	Values[3][0] = 0.0f;
	Values[3][1] = 0.0f;
	Values[3][2] = 0.0f;
	Values[3][3] = 1.0f;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void Matrix44::RotationY( float angle )
{
	float c, s;
	SinCos( angle, s, c );

	Values[0][0] = c;
	Values[0][1] = 0.0f;
	Values[0][2] = -s;
	Values[0][3] = 0.0f;

	Values[1][0] = 0.0f;
	Values[1][1] = 1.0f;
	Values[1][2] = 0.0f;
	Values[1][3] = 0.0f;

	Values[2][0] = s;
	Values[2][1] = 0.0f;
	Values[2][2] = c;
	Values[2][3] = 0.0f;

	Values[3][0] = 0.0f;
	Values[3][1] = 0.0f;
	Values[3][2] = 0.0f;
	Values[3][3] = 1.0f;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void Matrix44::RotationZ( float angle )
{
	float c, s;
	SinCos( angle, s, c );

	Values[0][0] = c;
	Values[0][1] = s;
	Values[0][2] = 0.0f;
	Values[0][3] = 0.0f;

	Values[1][0] = -s;
	Values[1][1] = c;
	Values[1][2] = 0.0f;
	Values[1][3] = 0.0f;

	Values[2][0] = 0.0f;
	Values[2][1] = 0.0f;
	Values[2][2] = 1;
	Values[2][3] = 0.0f;

	Values[3][0] = 0.0f;
	Values[3][1] = 0.0f;
	Values[3][2] = 0.0f;
	Values[3][3] = 1.0f;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void Matrix44::Translation( float x, float y, float z )
{
	Indentity();
	Values[3][0] = x;
	Values[3][1] = y;
	Values[3][2] = z;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void Matrix44::RotationAxis( const Vector3D& axis, float angle )
{
	const float c = cosf( angle );
	const float s = sinf( angle );
	const float cc = 1.0f - c;

	Values[0][0] = cc * (axis.X * axis.X) + c;
	Values[0][1] = cc * (axis.X * axis.Y) + (axis.Z * s);
	Values[0][2] = cc * (axis.Z * axis.X) - (axis.Y * s);

	Values[1][0] = cc * (axis.X * axis.Y) - (axis.Z * s);
	Values[1][1] = cc * (axis.Y * axis.Y) + c;
	Values[1][2] = cc * (axis.Y * axis.Z) + (axis.X * s);

	Values[2][0] = cc * (axis.Z * axis.X) + (axis.Y * s);
	Values[2][1] = cc * (axis.Y * axis.Z) - (axis.X * s);
	Values[2][2] = cc * (axis.Z * axis.Z) + c;

	Values[3][0] = 0.0f;
	Values[3][1] = 0.0f;
	Values[3][2] = 0.0f;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void Matrix44::Quaternion( float x, float y, float z, float w )
{
	float xx = x * x;
	float yy = y * y;
	float zz = z * z;
	float xy = x * y;
	float xz = x * z;
	float yz = y * z;
	float wx = w * x;
	float wy = w * y;
	float wz = w * z;

	Values[0][0] = 1.0f - 2.0f * (yy + zz);
	Values[1][0] = 2.0f * (xy - wz);
	Values[2][0] = 2.0f * (xz + wy);
	Values[3][0] = 0.0f;

	Values[0][1] = 2.0f * (xy + wz);
	Values[1][1] = 1.0f - 2.0f * (xx + zz);
	Values[2][1] = 2.0f * (yz - wx);
	Values[3][1] = 0.0f;

	Values[0][2] = 2.0f * (xz - wy);
	Values[1][2] = 2.0f * (yz + wx);
	Values[2][2] = 1.0f - 2.0f * (xx + yy);
	Values[3][2] = 0.0f;

	Values[0][3] = 0.0f;
	Values[1][3] = 0.0f;
	Values[2][3] = 0.0f;
	Values[3][3] = 1.0f;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
Matrix44& Matrix44::Mul( Matrix44& o, const Matrix44& in1, const Matrix44& in2 )
{
	Matrix44 _in1 = in1;
	Matrix44 _in2 = in2;

	for( int i = 0; i < 4; i++ )
	{
		for( int j = 0; j < 4; j++ )
		{
			float v = 0.0f;
			for( int k = 0; k < 4; k++ )
			{
				v += _in1.Values[i][k] * _in2.Values[k][j];
			}
			o.Values[i][j] = v;
		}
	}
	return o;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
Matrix44& Matrix44::Inverse(  Matrix44& o, const Matrix44& in )
{
	float a11 = in.Values[0][0];
	float a12 = in.Values[0][1];
	float a13 = in.Values[0][2];
	float a14 = in.Values[0][3];
	float a21 = in.Values[1][0];
	float a22 = in.Values[1][1];
	float a23 = in.Values[1][2];
	float a24 = in.Values[1][3];
	float a31 = in.Values[2][0];
	float a32 = in.Values[2][1];
	float a33 = in.Values[2][2];
	float a34 = in.Values[2][3];
	float a41 = in.Values[3][0];
	float a42 = in.Values[3][1];
	float a43 = in.Values[3][2];
	float a44 = in.Values[3][3];

	/* 行列式の計算 */
	float b11 = + a22 * (a33 * a44 - a43 * a34) - a23 * (a32 * a44 - a42 * a34) + a24 * (a32 * a43 - a42 * a33);
	float b12 = - a12 * (a33 * a44 - a43 * a34) + a13 * (a32 * a44 - a42 * a34) - a14 * (a32 * a43 - a42 * a33);
	float b13 = + a12 * (a23 * a44 - a43 * a24) - a13 * (a22 * a44 - a42 * a24) + a14 * (a22 * a43 - a42 * a23);
	float b14 = - a12 * (a23 * a34 - a33 * a24) + a13 * (a22 * a34 - a32 * a24) - a14 * (a22 * a33 - a32 * a23);

	float b21 = - a21 * (a33 * a44 - a43 * a34) + a23 * (a31 * a44 - a41 * a34) - a24 * (a31 * a43 - a41 * a33);
	float b22 = + a11 * (a33 * a44 - a43 * a34) - a13 * (a31 * a44 - a41 * a34) + a14 * (a31 * a43 - a41 * a33);
	float b23 = - a11 * (a23 * a44 - a43 * a24) + a13 * (a21 * a44 - a41 * a24) - a14 * (a21 * a43 - a41 * a23);
	float b24 = + a11 * (a23 * a34 - a33 * a24) - a13 * (a21 * a34 - a31 * a24) + a14 * (a21 * a33 - a31 * a23);

	float b31 = + a21 * (a32 * a44 - a42 * a34) - a22 * (a31 * a44 - a41 * a34) + a24 * (a31 * a42 - a41 * a32);
	float b32 = - a11 * (a32 * a44 - a42 * a34) + a12 * (a31 * a44 - a41 * a34) - a14 * (a31 * a42 - a41 * a32);
	float b33 = + a11 * (a22 * a44 - a42 * a24) - a12 * (a21 * a44 - a41 * a24) + a14 * (a21 * a42 - a41 * a22);
	float b34 = - a11 * (a22 * a34 - a32 * a24) + a12 * (a21 * a34 - a31 * a24) - a14 * (a21 * a32 - a31 * a22);

	float b41 = - a21 * (a32 * a43 - a42 * a33) + a22 * (a31 * a43 - a41 * a33) - a23 * (a31 * a42 - a41 * a32);
	float b42 = + a11 * (a32 * a43 - a42 * a33) - a12 * (a31 * a43 - a41 * a33) + a13 * (a31 * a42 - a41 * a32);
	float b43 = - a11 * (a22 * a43 - a42 * a23) + a12 * (a21 * a43 - a41 * a23) - a13 * (a21 * a42 - a41 * a22);
	float b44 = + a11 * (a22 * a33 - a32 * a23) - a12 * (a21 * a33 - a31 * a23) + a13 * (a21 * a32 - a31 * a22);

	// 行列式の逆数をかける
	float Det = (a11 * b11) + (a12 * b21) + (a13 * b31) + (a14 * b41);
	if ( (-FLT_MIN <= Det) && (Det <= +FLT_MIN) )
	{
		o = in;
		return o;
	}

	float InvDet = 1.0f / Det;

	o.Values[0][0] = b11 * InvDet;
	o.Values[0][1] = b12 * InvDet;
	o.Values[0][2] = b13 * InvDet;
	o.Values[0][3] = b14 * InvDet;
	o.Values[1][0] = b21 * InvDet;
	o.Values[1][1] = b22 * InvDet;
	o.Values[1][2] = b23 * InvDet;
	o.Values[1][3] = b24 * InvDet;
	o.Values[2][0] = b31 * InvDet;
	o.Values[2][1] = b32 * InvDet;
	o.Values[2][2] = b33 * InvDet;
	o.Values[2][3] = b34 * InvDet;
	o.Values[3][0] = b41 * InvDet;
	o.Values[3][1] = b42 * InvDet;
	o.Values[3][2] = b43 * InvDet;
	o.Values[3][3] = b44 * InvDet;

	return o;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
 } 
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------

#ifndef	__EFFEKSEER_INTERNAL_STRUCT_H__
#define	__EFFEKSEER_INTERNAL_STRUCT_H__

/**
	@file
	@brief	内部計算用構造体
	@note
	union等に使用するための構造体。<BR>

*/

//----------------------------------------------------------------------------------
// Include
//----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
namespace Effekseer
{

template <typename T>
void ReadData( T& dst, unsigned char*& pos )
{
	memcpy(&dst, pos, sizeof(T));
	pos += sizeof(T);
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
struct random_float
{
	float	max;
	float	min;

	void reset()
	{
		memset( this, 0 , sizeof(random_float) );
	};

	float getValue(IRandObject& g) const
	{
		float r;
		r = g.GetRand(min, max);
		return r;
	}
};

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
struct random_int
{
	int	max;
	int	min;

	void reset()
	{
		memset( this, 0 , sizeof(random_int) );
	};

	float getValue(IRandObject& g) const
	{
		float r;
		r = g.GetRand((float)min, (float)max);
		return r;
	}
};

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
struct vector2d
{
	float	x;
	float	y;

	void reset()
	{
		assert( sizeof(vector2d) == sizeof(float) * 2 );
		memset( this, 0, sizeof(vector2d) );
	};

	void setValueToArg( Vector2D& v ) const
	{
		v.X = x;
		v.Y = y;
	}

	vector2d operator + ( const vector2d& o ) const
	{
		vector2d ret;
		ret.x = x + o.x;
		ret.y = y + o.y;
		return ret;
	}

	vector2d operator * (const float& o) const
	{
		vector2d ret;
		ret.x = x * o;
		ret.y = y * o;
		return ret;
	}

	vector2d& operator += ( const vector2d& o )
	{
		x += o.x;
		y += o.y;
		return *this;
	}

	vector2d& operator *= ( const float& o )
	{
		x *= o;
		y *= o;
		return *this;
	}
};

struct rectf
{
	float	x;
	float	y;
	float	w;
	float	h;
	void reset()
	{
		assert( sizeof(rectf) == sizeof(float) * 4 );
		memset( this, 0, sizeof(rectf) );
	}
};

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
struct random_vector2d
{
	vector2d	max;
	vector2d	min;

	void reset()
	{
		memset( this, 0 , sizeof(random_vector2d) );
	};

	vector2d getValue(IRandObject& g) const
	{
		vector2d r;
		r.x = g.GetRand(min.x, max.x);
		r.y = g.GetRand(min.y, max.y);
		return r;
	}
};

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
struct easing_float_without_random
{
	float easingA;
	float easingB;
	float easingC;

	void setValueToArg( float& o, const float start_, const float end_, float t ) const
	{
		float df = end_ - start_;
		float d = easingA * t * t * t + easingB * t * t + easingC * t;
		o = start_ + d * df;
	}
};

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
struct easing_float
{
	random_float	start;
	random_float	end;
	float easingA;
	float easingB;
	float easingC;

	void setValueToArg( float& o, const float start_, const float end_, float t ) const
	{
		float df = end_ - start_;
		float d = easingA * t * t * t + easingB * t * t + easingC * t;
		o = start_ + d * df;
	}
};

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
struct easing_vector2d
{
	random_vector2d	start;
	random_vector2d	end;
	float easingA;
	float easingB;
	float easingC;

	void setValueToArg( vector2d& o, const vector2d& start_, const vector2d& end_, float t ) const
	{
		float d_x = end_.x - start_.x;
		float d_y = end_.y - start_.y;
		float d = easingA * t * t * t + easingB * t * t + easingC * t;
		o.x = start_.x + d * d_x;
		o.y = start_.y + d * d_y;
	}

	void setValueToArg( Vector2D& o, const vector2d& start_, const vector2d& end_, float t ) const
	{
		float d_x = end_.x - start_.x;
		float d_y = end_.y - start_.y;
		float d = easingA * t * t * t + easingB * t * t + easingC * t;
		o.X = start_.x + d * d_x;
		o.Y = start_.y + d * d_y;
	}
};

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
struct vector3d
{
	float	x;
	float	y;
	float	z;

	void reset()
	{
		assert( sizeof(vector3d) == sizeof(float) * 3 );
		memset( this, 0, sizeof(vector3d) );
	};

	void normalize()
	{
		float len = sqrtf(x * x + y * y + z * z);
		if (len > 0.0001f)
		{
			x /= len;
			y /= len;
			z /= len;
		}
		else
		{
			x = 1.0;
			y = 0.0;
			z = 0.0;
		}
	}

	void setValueToArg( Vector3D& v ) const
	{
		v.X = x;
		v.Y = y;
		v.Z = z;
	}

	vector3d operator + ( const vector3d& o ) const
	{
		vector3d ret;
		ret.x = x + o.x;
		ret.y = y + o.y;
		ret.z = z + o.z;
		return ret;
	}

	vector3d operator - ( const vector3d& o ) const
	{
		vector3d ret;
		ret.x = x - o.x;
		ret.y = y - o.y;
		ret.z = z - o.z;
		return ret;
	}

	vector3d operator * ( const float& o ) const
	{
		vector3d ret;
		ret.x = x * o;
		ret.y = y * o;
		ret.z = z * o;
		return ret;
	}

	vector3d& operator += ( const vector3d& o )
	{
		x += o.x;
		y += o.y;
		z += o.z;
		return *this;
	}

	vector3d& operator -= ( const vector3d& o )
	{
		x -= o.x;
		y -= o.y;
		z -= o.z;
		return *this;
	}

	vector3d& operator *= ( const float& o )
	{
		x *= o;
		y *= o;
		z *= o;
		return *this;
	}
};

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
struct random_vector3d
{
	vector3d	max;
	vector3d	min;

	void reset()
	{
		memset( this, 0 , sizeof(random_vector3d) );
	};

	vector3d getValue(IRandObject& g) const
	{
		vector3d r;
		r.x = g.GetRand(min.x, max.x);
		r.y = g.GetRand(min.y, max.y);
		r.z = g.GetRand(min.z, max.z);
		return r;
	}
};

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
struct easing_vector3d
{
	random_vector3d	start;
	random_vector3d	end;
	float easingA;
	float easingB;
	float easingC;

	void setValueToArg( vector3d& o, const vector3d& start_, const vector3d& end_, float t ) const
	{
		float d_x = end_.x - start_.x;
		float d_y = end_.y - start_.y;
		float d_z = end_.z - start_.z;
		float d = easingA * t * t * t + easingB * t * t + easingC * t;
		o.x = start_.x + d * d_x;
		o.y = start_.y + d * d_y;
		o.z = start_.z + d * d_z;
	}

	void setValueToArg( Vector3D& o, const vector3d& start_, const vector3d& end_, float t ) const
	{
		float d_x = end_.x - start_.x;
		float d_y = end_.y - start_.y;
		float d_z = end_.z - start_.z;
		float d = easingA * t * t * t + easingB * t * t + easingC * t;
		o.X = start_.x + d * d_x;
		o.Y = start_.y + d * d_y;
		o.Z = start_.z + d * d_z;
	}
};

inline Color HSVToRGB(Color hsv) {
	int H = hsv.R, S = hsv.G, V = hsv.B;
	int Hi, R=0, G=0, B=0, p, q, t;
	float f, s;

	if (H >= 252) H = 252;
	Hi = (H / 42);
	f = H / 42.0f - Hi;
	Hi = Hi % 6;
	s = S / 255.0f;
	p = (int)((V * (1 - s)));
	q = (int)((V * (1 - f * s)));
	t = (int)((V * (1 - (1 - f) * s)));

	switch (Hi) {
	case 0: R = V; G = t; B = p; break;
	case 1: R = q; G = V; B = p; break;
	case 2: R = p; G = V; B = t; break;
	case 3: R = p; G = q; B = V; break;
	case 4: R = t; G = p; B = V; break;
	case 5: R = V; G = p; B = q; break;
	}
	Color result;
	result.R = R;
	result.G = G;
	result.B = B;
	result.A = hsv.A;
	return result;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
struct random_color
{
	ColorMode mode;
	Color	max;
	Color	min;

	void reset()
	{
		assert( sizeof(random_color) == 12 );
		mode = COLOR_MODE_RGBA;
		max = {255, 255, 255, 255};
		min = {255, 255, 255, 255};
	};

	Color getValue(IRandObject& g) const
	{
		Color r = getDirectValue( g );
		if( mode == COLOR_MODE_HSVA )
		{
			r = HSVToRGB( r );
		}
		return r;
	}
	
	Color getDirectValue(IRandObject& g) const
	{
		Color r;
		r.R = (uint8_t) (g.GetRand(min.R, max.R));
		r.G = (uint8_t) (g.GetRand(min.G, max.G));
		r.B = (uint8_t) (g.GetRand(min.B, max.B));
		r.A = (uint8_t) (g.GetRand(min.A, max.A));
		return r;
	}

	void load( int version, unsigned char*& pos )
	{
		if( version >= 4 )
		{
			uint8_t mode_ = 0;
			ReadData<uint8_t>(mode_, pos);
			mode = static_cast<ColorMode>(mode_);
			pos++;	// reserved
		}
		else
		{
			mode = COLOR_MODE_RGBA;
		}
		ReadData<Color>(max, pos );
		ReadData<Color>(min, pos );
	}
};

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
struct easing_color
{
	random_color	start;
	random_color	end;
	float easingA;
	float easingB;
	float easingC;

	void setValueToArg( Color& o, const Color& start_, const Color& end_, float t ) const
	{
		assert( start.mode == end.mode );
		float d = easingA * t * t * t + easingB * t * t + easingC * t;
		o = Color::Lerp(start_, end_, d);
		if( start.mode == COLOR_MODE_HSVA )
		{
			o = HSVToRGB( o );
		}
	}

	Color getStartValue(IRandObject& g) const
	{
		return start.getDirectValue( g );
	}
	
	Color getEndValue(IRandObject& g) const
	{
		return end.getDirectValue( g);
	}

	void load( int version, unsigned char*& pos )
	{
		start.load( version, pos );
		end.load( version, pos );
		ReadData<float>(easingA, pos );
		ReadData<float>(easingB, pos );
		ReadData<float>(easingC, pos );
	}
};

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
}
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
#endif	// __EFFEKSEER_INTERNAL_STRUCT_H__

#ifndef __EFFEKSEER_INTERNAL_SCRIPT_H__
#define __EFFEKSEER_INTERNAL_SCRIPT_H__


namespace Effekseer
{

typedef float(RandFuncCallback)(void* userData);

typedef float(RandWithSeedFuncCallback)(void* userData, float seed);

class InternalScript
{
public:
	enum class RunningPhaseType : int32_t
	{
		Global,
		Local,
	};

private:
	enum class OperatorType : int32_t
	{
		Constant = 0,
		Add = 1,
		Sub = 2,
		Mul = 3,
		Div = 4,

		UnaryAdd = 11,
		UnarySub = 12,

		Sine = 21,
		Cos = 22,

		Rand = 31,
		Rand_WithSeed = 32,
	};

private:
	RunningPhaseType runningPhase = RunningPhaseType::Local;
	std::vector<float> registers;
	std::vector<uint8_t> operators;
	int32_t version_ = 0;
	int32_t operatorCount_ = 0;
	std::array<int32_t, 4> outputRegisters_;
	bool isValid_ = false;

	bool IsValidOperator(int value) const;
	bool IsValidRegister(int index) const;
	float GetRegisterValue(int index,
						   const std::array<float, 4>& externals,
						   const std::array<float, 1>& globals,
						   const std::array<float, 5>& locals) const;

public:
	InternalScript();
	virtual ~InternalScript();
	bool Load(uint8_t* data, int size);
	std::array<float, 4> Execute(const std::array<float, 4>& externals,
								 const std::array<float, 1>& globals,
								 const std::array<float, 5>& locals,
								 RandFuncCallback* randFuncCallback,
								 RandWithSeedFuncCallback* randSeedFuncCallback,
								 void* userData);
	RunningPhaseType GetRunningPhase() const { return runningPhase; }
};

} // namespace Effekseer

#endif


#ifndef	__EFFEKSEER_DEFAULTEFFECTLOADER_H__
#define	__EFFEKSEER_DEFAULTEFFECTLOADER_H__

//----------------------------------------------------------------------------------
// Include
//----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
namespace Effekseer { 
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
/**
	@brief	標準のエフェクトファイル読み込み破棄関数指定クラス
*/
class DefaultEffectLoader
	: public EffectLoader
{
	DefaultFileInterface m_defaultFileInterface;
	FileInterface* m_fileInterface;
public:

	DefaultEffectLoader( FileInterface* fileInterface = NULL );

	virtual ~DefaultEffectLoader();

	bool Load( const EFK_CHAR* path, void*& data, int32_t& size );

	void Unload( void* data, int32_t size );
};

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
 } 
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
#endif	// __EFFEKSEER_DEFAULTEFFECTLOADER_H__


//----------------------------------------------------------------------------------
// Include
//----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
namespace Effekseer { 
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
DefaultEffectLoader::DefaultEffectLoader( FileInterface* fileInterface )
	: m_fileInterface( fileInterface )
{
	if( m_fileInterface == NULL )
	{
		m_fileInterface = &m_defaultFileInterface;
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
DefaultEffectLoader::~DefaultEffectLoader()
{

}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
bool DefaultEffectLoader::Load( const EFK_CHAR* path, void*& data, int32_t& size )
{
	assert( path != NULL );

	data = NULL;
	size = 0;

	std::unique_ptr<FileReader> 
		reader( m_fileInterface->OpenRead( path ) );
	if( reader.get() == NULL ) return false;

	size = (int32_t)reader->GetLength();
	data = new uint8_t[size];
	reader->Read( data, size );

	return true;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void DefaultEffectLoader::Unload( void* data, int32_t size )
{
	uint8_t* data8 = (uint8_t*)data;
	ES_SAFE_DELETE_ARRAY( data8 );
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
 } 
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------


#ifndef	__EFFEKSEER_DEFAULT_FILE_H__
#define	__EFFEKSEER_DEFAULT_FILE_H__

//----------------------------------------------------------------------------------
// Include
//----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
namespace Effekseer { 
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
/**
	@brief	標準のファイル読み込みクラス
*/

class DefaultFileReader : public FileReader
{
private:
	FILE* m_filePtr;

public:
	DefaultFileReader( FILE* filePtr );

	~DefaultFileReader();

	size_t Read( void* buffer, size_t size );

	void Seek( int position );

	int GetPosition();

	size_t GetLength();
};

class DefaultFileWriter : public FileWriter
{
private:
	FILE* m_filePtr;

public:
	DefaultFileWriter( FILE* filePtr );

	~DefaultFileWriter();

	size_t Write( const void* buffer, size_t size );

	void Flush();

	void Seek( int position );

	int GetPosition();

	size_t GetLength();
};

class DefaultFileInterface : public FileInterface
{
private:

public:
	FileReader* OpenRead( const EFK_CHAR* path );

	FileWriter* OpenWrite( const EFK_CHAR* path );
};


//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
 } 
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
#endif	// __EFFEKSEER_DEFAULT_FILE_H__


//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
namespace Effekseer { 
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
DefaultFileReader::DefaultFileReader( FILE* filePtr )
	: m_filePtr( filePtr )
{
	assert( filePtr != NULL );
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
DefaultFileReader::~DefaultFileReader()
{
	fclose( m_filePtr );
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
size_t DefaultFileReader::Read( void* buffer, size_t size )
{
	return fread( buffer, 1, size, m_filePtr );
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void DefaultFileReader::Seek( int position )
{
	fseek( m_filePtr, (size_t)position, SEEK_SET );
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
int DefaultFileReader::GetPosition()
{
	return (int)ftell( m_filePtr );
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
size_t DefaultFileReader::GetLength()
{
	long position = ftell( m_filePtr );
	fseek( m_filePtr, 0, SEEK_END );
	long length = ftell( m_filePtr );
	fseek( m_filePtr, position, SEEK_SET );
	return (size_t)length;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
DefaultFileWriter::DefaultFileWriter( FILE* filePtr )
	: m_filePtr( filePtr )
{
	assert( filePtr != NULL );
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
DefaultFileWriter::~DefaultFileWriter()
{
	fclose( m_filePtr );
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
size_t DefaultFileWriter::Write( const void* buffer, size_t size )
{
	return fwrite( buffer, 1, size, m_filePtr );
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void DefaultFileWriter::Flush()
{
	fflush( m_filePtr );
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void DefaultFileWriter::Seek( int position )
{
	fseek( m_filePtr, (size_t)position, SEEK_SET );
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
int DefaultFileWriter::GetPosition()
{
	return (int)ftell( m_filePtr );
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
size_t DefaultFileWriter::GetLength()
{
	long position = ftell( m_filePtr );
	fseek( m_filePtr, 0, SEEK_END );
	long length = ftell( m_filePtr );
	fseek( m_filePtr, position, SEEK_SET );
	return (size_t)length;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
FileReader* DefaultFileInterface::OpenRead( const EFK_CHAR* path )
{
	FILE* filePtr = NULL;
#ifdef _WIN32
	_wfopen_s( &filePtr, (const wchar_t*)path, L"rb" );
#else
	int8_t path8[256];
	ConvertUtf16ToUtf8( path8, 256, (const int16_t*)path );
	filePtr = fopen( (const char*)path8, "rb" );
#endif
	
	if( filePtr == NULL )
	{
		return NULL;
	}

	return new DefaultFileReader( filePtr );
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
FileWriter* DefaultFileInterface::OpenWrite( const EFK_CHAR* path )
{
	FILE* filePtr = NULL;
#ifdef _WIN32
	_wfopen_s( &filePtr, (const wchar_t*)path, L"wb" );
#else
	int8_t path8[256];
	ConvertUtf16ToUtf8( path8, 256, (const int16_t*)path );
	filePtr = fopen( (const char*)path8, "wb" );
#endif
	
	if( filePtr == NULL )
	{
		return NULL;
	}

	return new DefaultFileWriter( filePtr );
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
 } 
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------

#ifndef __EFFEKSEER_FCURVES_H__
#define __EFFEKSEER_FCURVES_H__


namespace Effekseer
{

enum class FCurveTimelineType : int32_t
{
	Time = 0,
	Percent = 1,
};

class FCurve
{
private:
	enum class FCurveEdge : int32_t
	{
		Constant = 0,
		Loop = 1,
		LoopInversely = 2,
	};

private:
	int32_t offset_ = 0;
	int32_t len_ = 0;
	int32_t freq_ = 0;
	FCurveEdge start_ = FCurveEdge::Constant;
	FCurveEdge end_ = FCurveEdge::Constant;
	std::vector<float> keys_;

	float defaultValue_ = 0;
	float offsetMax_ = 0;
	float offsetMin_ = 0;

public:
	FCurve(float defaultValue);
	int32_t Load(void* data, int32_t version);

	float GetValue(float living, float life, FCurveTimelineType type) const;

	float GetOffset(InstanceGlobal& g) const;

	void SetDefaultValue(float value) { defaultValue_ = value; }

	void ChangeCoordinate();

	void Maginify(float value);
};

class FCurveVector2D
{
public:
	FCurveTimelineType Timeline = FCurveTimelineType::Time;
	FCurve X = FCurve(0);
	FCurve Y = FCurve(0);

	int32_t Load(void* data, int32_t version);

	std::array<float, 2> GetValues(float living, float life) const;
	std::array<float, 2> GetOffsets(InstanceGlobal& g) const;
};

class FCurveVector3D
{
public:
	FCurveTimelineType Timeline = FCurveTimelineType::Time;
	FCurve X = FCurve(0);
	FCurve Y = FCurve(0);
	FCurve Z = FCurve(0);

	int32_t Load(void* data, int32_t version);

	std::array<float, 3> GetValues(float living, float life) const;
	std::array<float, 3> GetOffsets(InstanceGlobal& g) const;
};

class FCurveVectorColor
{
public:
	FCurveTimelineType Timeline = FCurveTimelineType::Time;
	FCurve R = FCurve(255);
	FCurve G = FCurve(255);
	FCurve B = FCurve(255);
	FCurve A = FCurve(255);

	int32_t Load(void* data, int32_t version);

	std::array<float, 4> GetValues(float living, float life) const;
	std::array<float, 4> GetOffsets(InstanceGlobal& g) const;
};

} // namespace Effekseer

#endif // __EFFEKSEER_FCURVES_H__


#ifndef	__EFFEKSEER_EFFECTNODE_H__
#define	__EFFEKSEER_EFFECTNODE_H__

//----------------------------------------------------------------------------------
// Include
//----------------------------------------------------------------------------------


//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
namespace Effekseer
{

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
enum class BindType : int32_t
{
	NotBind = 0,
	NotBind_Root = 3,
	WhenCreating = 1,
	Always = 2,
};

/**!
	@brief indexes of dynamic parameter
*/
struct RefMinMax
{
	int32_t Max = -1;
	int32_t Min = -1;
};

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
class StandardColorParameter
{
public:
	enum
	{
		Fixed = 0,
		Random = 1,
		Easing = 2,
		FCurve_RGBA = 3,
		Parameter_DWORD = 0x7fffffff,
	} type;

	union
	{
		struct
		{
			Color all;
		} fixed;

		struct
		{
			random_color all;
		} random;

		struct
		{
			easing_color all;
		} easing;

		struct
		{
			FCurveVectorColor* FCurve;
		} fcurve_rgba;
	};

	StandardColorParameter()
	{
		type = Fixed;
	}

	~StandardColorParameter()
	{
		if (type == FCurve_RGBA)
		{
			ES_SAFE_DELETE(fcurve_rgba.FCurve);
		}
	}

	void load(uint8_t*& pos, int32_t version)
	{
		memcpy(&type, pos, sizeof(int));
		pos += sizeof(int);

		if (type == Fixed)
		{
			memcpy(&fixed, pos, sizeof(fixed));
			pos += sizeof(fixed);
		}
		else if (type == Random)
		{
			random.all.load(version, pos);
		}
		else if (type == Easing)
		{
			easing.all.load(version, pos);
		}
		else if (type == FCurve_RGBA)
		{
			fcurve_rgba.FCurve = new FCurveVectorColor();
			int32_t size = fcurve_rgba.FCurve->Load(pos, version);
			pos += size;
		}
	}
};
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
struct ParameterCommonValues_8
{
	int MaxGeneration;
	BindType TranslationBindType;
	BindType RotationBindType;
	BindType ScalingBindType;
	int		RemoveWhenLifeIsExtinct;
	int		RemoveWhenParentIsRemoved;
	int		RemoveWhenChildrenIsExtinct;
	random_int	life;
	float GenerationTime;
	float GenerationTimeOffset;
};

struct ParameterCommonValues
{
	int32_t RefEqMaxGeneration = -1;
	RefMinMax RefEqLife;
	RefMinMax RefEqGenerationTime;
	RefMinMax RefEqGenerationTimeOffset;

	int MaxGeneration = 1;
	BindType TranslationBindType = BindType::Always;
	BindType RotationBindType = BindType::Always;
	BindType ScalingBindType = BindType::Always;
	int RemoveWhenLifeIsExtinct = 1;
	int RemoveWhenParentIsRemoved = 0;
	int RemoveWhenChildrenIsExtinct = 0;
	random_int	life;
	random_float GenerationTime;
	random_float GenerationTimeOffset;

	ParameterCommonValues()
	{
		life.max = 1;
		life.min = 1;
		GenerationTime.max = 1;
		GenerationTime.min = 1;
		GenerationTimeOffset.max = 0;
		GenerationTimeOffset.min = 0;
	}
};

struct ParameterDepthValues
{
	float	DepthOffset;
	bool	IsDepthOffsetScaledWithCamera;
	bool	IsDepthOffsetScaledWithParticleScale;
	ZSortType	ZSort;
	int32_t	DrawingPriority;
	float	SoftParticle;

	NodeRendererDepthParameter DepthParameter;

	ParameterDepthValues()
	{
		DepthOffset = 0;
		IsDepthOffsetScaledWithCamera = false;
		IsDepthOffsetScaledWithParticleScale = false;
		ZSort = ZSortType::None;
		DrawingPriority = 0;
		SoftParticle = 0.0f;
	}
};

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
enum ParameterTranslationType
{
	ParameterTranslationType_Fixed = 0,
	ParameterTranslationType_PVA = 1,
	ParameterTranslationType_Easing = 2,
	ParameterTranslationType_FCurve = 3,

	ParameterTranslationType_None = 0x7fffffff - 1,

	ParameterTranslationType_DWORD = 0x7fffffff,
};

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
struct ParameterTranslationFixed
{
	int32_t RefEq = -1;

	Vector3D Position;
};

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
struct ParameterTranslationPVA
{
	RefMinMax RefEqP;
	RefMinMax RefEqV;
	RefMinMax RefEqA;
	random_vector3d	location;
	random_vector3d	velocity;
	random_vector3d	acceleration;
};

struct ParameterTranslationEasing
{
	RefMinMax RefEqS;
	RefMinMax RefEqE;
	easing_vector3d location;
};

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
enum class LocationAbsType : int32_t
{
	None = 0,
	Gravity = 1,
	AttractiveForce = 2,
};

struct LocationAbsParameter
{
	LocationAbsType type = LocationAbsType::None;

	union
	{
		struct
		{

		} none;

		vector3d	gravity;

		struct {
			float	force;
			float	control;
			float	minRange;
			float	maxRange;
		} attractiveForce;
	};
};

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
enum ParameterRotationType
{
	ParameterRotationType_Fixed = 0,
	ParameterRotationType_PVA = 1,
	ParameterRotationType_Easing = 2,
	ParameterRotationType_AxisPVA = 3,
	ParameterRotationType_AxisEasing = 4,

	ParameterRotationType_FCurve = 5,

	ParameterRotationType_None = 0x7fffffff - 1,

	ParameterRotationType_DWORD = 0x7fffffff,
};

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
struct ParameterRotationFixed
{
	int32_t RefEq = -1;
	Vector3D Position;
};

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
struct ParameterRotationPVA
{
	RefMinMax RefEqP;
	RefMinMax RefEqV;
	RefMinMax RefEqA;
	random_vector3d	rotation;
	random_vector3d	velocity;
	random_vector3d	acceleration;
};

struct ParameterRotationEasing
{
	RefMinMax RefEqS;
	RefMinMax RefEqE;
	easing_vector3d rotation;
};

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
struct ParameterRotationAxisPVA
{
	random_vector3d	axis;
	random_float	rotation;
	random_float	velocity;
	random_float	acceleration;
};

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
struct ParameterRotationAxisEasing
{
	random_vector3d	axis;
	easing_float	easing;
};

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
enum ParameterScalingType
{
	ParameterScalingType_Fixed = 0,
	ParameterScalingType_PVA = 1,
	ParameterScalingType_Easing = 2,
	ParameterScalingType_SinglePVA = 3,
	ParameterScalingType_SingleEasing = 4,
	ParameterScalingType_FCurve = 5,

	ParameterScalingType_None = 0x7fffffff - 1,

	ParameterScalingType_DWORD = 0x7fffffff,
};

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
struct ParameterScalingFixed
{
	int32_t RefEq = -1;

	Vector3D Position;
};

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
struct ParameterScalingPVA
{
	RefMinMax RefEqP;
	RefMinMax RefEqV;
	RefMinMax RefEqA;

	random_vector3d Position;
	random_vector3d Velocity;
	random_vector3d Acceleration;
};

struct ParameterScalingEasing
{
	RefMinMax RefEqS;
	RefMinMax RefEqE;
	easing_vector3d Position;
};

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
struct ParameterScalingSinglePVA
{
	random_float Position;
	random_float Velocity;
	random_float Acceleration;
};

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
struct ParameterGenerationLocation
{
	int	EffectsRotation;

	enum class AxisType : int32_t
	{
		X,
		Y,
		Z,
	};

	enum
	{
		TYPE_POINT = 0,
		TYPE_SPHERE = 1,
		TYPE_MODEL = 2,
		TYPE_CIRCLE = 3,
		TYPE_LINE = 4,

		TYPE_DWORD = 0x7fffffff,
	} type;

	enum eModelType
	{
		MODELTYPE_RANDOM = 0,
		MODELTYPE_VERTEX = 1,
		MODELTYPE_VERTEX_RANDOM = 2,
		MODELTYPE_FACE = 3,
		MODELTYPE_FACE_RANDOM = 4,

		MODELTYPE_DWORD = 0x7fffffff,
	};

	enum eCircleType
	{
		CIRCLE_TYPE_RANDOM = 0,
		CIRCLE_TYPE_ORDER = 1,
		CIRCLE_TYPE_REVERSE_ORDER = 2,
	};

	enum class LineType : int32_t
	{
		Random = 0,
		Order = 1,
	};

	union
	{
		struct
		{
			random_vector3d location;
		} point;

		struct
		{
			random_float radius;
			random_float rotation_x;
			random_float rotation_y;
		} sphere;

		struct
		{
			int32_t		index;
			eModelType	type;
		} model;

		struct
		{
			int32_t			division;
			random_float	radius;
			random_float	angle_start;
			random_float	angle_end;
			eCircleType		type;
			AxisType		axisDirection;
			random_float	angle_noize;
		} circle;

		struct
		{
			int32_t			division;
			random_vector3d	position_start;
			random_vector3d	position_end;
			random_float	position_noize;
			LineType		type;
		} line;
	};

	void load(uint8_t*& pos, int32_t version)
	{
		memcpy(&EffectsRotation, pos, sizeof(int));
		pos += sizeof(int);

		memcpy(&type, pos, sizeof(int));
		pos += sizeof(int);

		if (type == TYPE_POINT)
		{
			memcpy(&point, pos, sizeof(point));
			pos += sizeof(point);
		}
		else if (type == TYPE_SPHERE)
		{
			memcpy(&sphere, pos, sizeof(sphere));
			pos += sizeof(sphere);
		}
		else if (type == TYPE_MODEL)
		{
			memcpy(&model, pos, sizeof(model));
			pos += sizeof(model);
		}
		else if (type == TYPE_CIRCLE)
		{
			if (version < 10)
			{
				memcpy(&circle, pos, sizeof(circle) - sizeof(circle.axisDirection) - sizeof(circle.angle_noize));
				pos += sizeof(circle) - sizeof(circle.axisDirection) - sizeof(circle.angle_noize);
				circle.axisDirection = AxisType::Z;
				circle.angle_noize.max = 0;
				circle.angle_noize.min = 0;
			}
			else
			{
				memcpy(&circle, pos, sizeof(circle));
				pos += sizeof(circle);
			}
		}
		else if (type == TYPE_LINE)
		{
			memcpy(&line, pos, sizeof(line));
			pos += sizeof(line);
		}
	}
};

enum ParameterCustomDataType : int32_t
{
	None = 0,
	Fixed2D = 20,
	Easing2D = 22,
	FCurve2D = 23,
	Fixed4D = 40,
	FCurveColor = 53,
	Unknown,
};

struct ParameterCustomDataFixed
{
	vector2d Values;
};

struct ParameterCustomDataEasing
{
	easing_vector2d Values;
};

struct ParameterCustomDataFCurve
{
	FCurveVector2D* Values;
};

struct ParameterCustomDataFCurveColor
{
	FCurveVectorColor* Values;
};

struct ParameterCustomData
{
	ParameterCustomDataType Type = ParameterCustomDataType::None;

	union {
		ParameterCustomDataFixed Fixed;
		ParameterCustomDataEasing Easing;
		ParameterCustomDataFCurve FCurve;
		std::array<float, 4> Fixed4D;
		ParameterCustomDataFCurveColor FCurveColor;
	};

	ParameterCustomData() = default;

	~ParameterCustomData()
	{
		if (Type == ParameterCustomDataType::FCurve2D)
		{
			ES_SAFE_DELETE(FCurve.Values);
		}

		if (Type == ParameterCustomDataType::FCurveColor)
		{
			ES_SAFE_DELETE(FCurveColor.Values);
		}
	}

	void load(uint8_t*& pos, int32_t version)
	{
		memcpy(&Type, pos, sizeof(int));
		pos += sizeof(int);

		if (Type == ParameterCustomDataType::None)
		{
		}
		else if (Type == ParameterCustomDataType::Fixed2D)
		{
			memcpy(&Fixed.Values, pos, sizeof(Fixed));
			pos += sizeof(Fixed);
		}
		else if (Type == ParameterCustomDataType::Easing2D)
		{
			memcpy(&Easing.Values, pos, sizeof(Easing));
			pos += sizeof(Easing);
		}
		else if (Type == ParameterCustomDataType::FCurve2D)
		{
			FCurve.Values = new FCurveVector2D();
			pos += FCurve.Values->Load(pos, version);
		}
		else if (Type == ParameterCustomDataType::Fixed4D)
		{
			memcpy(Fixed4D.data(), pos, sizeof(float) * 4);
			pos += sizeof(float) * 4;
		}
		else if (Type == ParameterCustomDataType::FCurveColor)
		{
			FCurveColor.Values = new FCurveVectorColor();
			pos += FCurveColor.Values->Load(pos, version);
		}
		else
		{
			assert(0);
		}
	}
};


struct ParameterRendererCommon
{

	RendererMaterialType MaterialType = RendererMaterialType::Default;

	//! texture index except a file
	int32_t				ColorTextureIndex = -1;

	//! texture index except a file
	int32_t Texture2Index = -1;

	//! material index in MaterialType::File
	MaterialParameter Material;

	AlphaBlendType AlphaBlend = AlphaBlendType::Opacity;

	TextureFilterType FilterType = TextureFilterType::Nearest;

	TextureWrapType WrapType = TextureWrapType::Repeat;

	TextureFilterType Filter2Type = TextureFilterType::Nearest;

	TextureWrapType Wrap2Type = TextureWrapType::Repeat;

	bool				ZWrite = false;

	bool				ZTest = false;

	//! this value is not unused
	bool				Distortion = false;

	float				DistortionIntensity = 0;

	BindType ColorBindType = BindType::NotBind;

	//! pass into a renderer (to make easy to send parameters, it should be refactored)
	NodeRendererBasicParameter BasicParameter;

	ParameterCustomData CustomData1;
	ParameterCustomData CustomData2;

	enum
	{
		FADEIN_ON = 1,
		FADEIN_OFF = 0,

		FADEIN_DWORD = 0x7fffffff,
	} FadeInType;

	struct
	{
		float	Frame;
		easing_float_without_random	Value;
	} FadeIn;

	enum
	{
		FADEOUT_ON = 1,
		FADEOUT_OFF = 0,

		FADEOUT_DWORD = 0x7fffffff,
	} FadeOutType;

	struct
	{
		float	Frame;
		easing_float_without_random	Value;
	} FadeOut;

	enum
	{
		UV_DEFAULT = 0,
		UV_FIXED = 1,
		UV_ANIMATION = 2,
		UV_SCROLL = 3,
		UV_FCURVE = 4,

		UV_DWORD = 0x7fffffff,
	} UVType;


	/**
	@brief	UV Parameter
	@note
	for Compatibility
	*/
	struct UVScroll_09
	{
		rectf		Position;
		vector2d	Speed;
	};

	union
	{
		struct
		{
		} Default;

		struct
		{
			rectf	Position;
		} Fixed;

		struct
		{
			rectf	Position;
			int32_t	FrameLength;
			int32_t	FrameCountX;
			int32_t	FrameCountY;

			enum
			{
				LOOPTYPE_ONCE = 0,
				LOOPTYPE_LOOP = 1,
				LOOPTYPE_REVERSELOOP = 2,

				LOOPTYPE_DWORD = 0x7fffffff,
			} LoopType;

			random_int	StartFrame;

		} Animation;

		struct
		{
			random_vector2d	Position;
			random_vector2d	Size;
			random_vector2d	Speed;
		} Scroll;

		struct
		{
			FCurveVector2D* Position;
			FCurveVector2D* Size;
		} FCurve;

	} UV;

	ParameterRendererCommon()
	{
		FadeInType = FADEIN_OFF;
		FadeOutType = FADEOUT_OFF;
		UVType = UV_DEFAULT;
	}

	~ParameterRendererCommon()
	{
		if (UVType == UV_FCURVE)
		{
			ES_SAFE_DELETE(UV.FCurve.Position);
			ES_SAFE_DELETE(UV.FCurve.Size);
		}
	}

	void reset()
	{
		// with constructor
		//memset(this, 0, sizeof(ParameterRendererCommon));
	}

	void load(uint8_t*& pos, int32_t version)
	{
		//memset(this, 0, sizeof(ParameterRendererCommon));

		if (version >= 15)
		{
			memcpy(&MaterialType, pos, sizeof(int));
			pos += sizeof(int);

			if (MaterialType == RendererMaterialType::Default || 
				MaterialType == RendererMaterialType::BackDistortion ||
				MaterialType == RendererMaterialType::Lighting)
			{
				memcpy(&ColorTextureIndex, pos, sizeof(int));
				pos += sizeof(int);

				memcpy(&Texture2Index, pos, sizeof(int));
				pos += sizeof(int);
			}
			else
			{
				memcpy(&Material.MaterialIndex, pos, sizeof(int));
				pos += sizeof(int);

				int32_t textures = 0;
				int32_t uniforms = 0;

				memcpy(&textures, pos, sizeof(int));
				pos += sizeof(int);

				
				Material.MaterialTextures.resize(textures);
				memcpy(Material.MaterialTextures.data(), pos, sizeof(MaterialTextureParameter) * textures);
				pos += (sizeof(MaterialTextureParameter) * textures);

				memcpy(&uniforms, pos, sizeof(int));
				pos += sizeof(int);

				Material.MaterialUniforms.resize(uniforms);
				memcpy(Material.MaterialUniforms.data(), pos, sizeof(float) * 4 * uniforms);
				pos += (sizeof(float) * 4 * uniforms);
			}
		}
		else
		{
			memcpy(&ColorTextureIndex, pos, sizeof(int));
			pos += sizeof(int);
		}
		
		memcpy(&AlphaBlend, pos, sizeof(int));
		pos += sizeof(int);

		memcpy(&FilterType, pos, sizeof(int));
		pos += sizeof(int);

		memcpy(&WrapType, pos, sizeof(int));
		pos += sizeof(int);

		if (version >= 15)
		{
			memcpy(&Filter2Type, pos, sizeof(int));
			pos += sizeof(int);

			memcpy(&Wrap2Type, pos, sizeof(int));
			pos += sizeof(int);
		}
		else
		{
			Filter2Type = FilterType;
			Wrap2Type = WrapType;
		}

		if (version >= 5)
		{
			int32_t zwrite, ztest = 0;

			memcpy(&ztest, pos, sizeof(int32_t));
			pos += sizeof(int32_t);

			memcpy(&zwrite, pos, sizeof(int32_t));
			pos += sizeof(int32_t);

			ZWrite = zwrite != 0;
			ZTest = ztest != 0;
		}
		else
		{
			ZWrite = false;
			ZTest = true;
		}

		memcpy(&FadeInType, pos, sizeof(int));
		pos += sizeof(int);

		if (FadeInType == FADEIN_ON)
		{
			memcpy(&FadeIn, pos, sizeof(FadeIn));
			pos += sizeof(FadeIn);
		}

		memcpy(&FadeOutType, pos, sizeof(int));
		pos += sizeof(int);

		if (FadeOutType == FADEOUT_ON)
		{
			memcpy(&FadeOut, pos, sizeof(FadeOut));
			pos += sizeof(FadeOut);
		}

		memcpy(&UVType, pos, sizeof(int));
		pos += sizeof(int);

		if (UVType == UV_DEFAULT)
		{
		}
		else if (UVType == UV_FIXED)
		{
			memcpy(&UV.Fixed, pos, sizeof(UV.Fixed));
			pos += sizeof(UV.Fixed);
		}
		else if (UVType == UV_ANIMATION)
		{
			if (version < 10)
			{
				// without start frame
				memcpy(&UV.Animation, pos, sizeof(UV.Animation) - sizeof(UV.Animation.StartFrame));
				pos += sizeof(UV.Animation) - sizeof(UV.Animation.StartFrame);
				UV.Animation.StartFrame.max = 0;
				UV.Animation.StartFrame.min = 0;
			}
			else
			{
				memcpy(&UV.Animation, pos, sizeof(UV.Animation));
				pos += sizeof(UV.Animation);
			}
		}
		else if (UVType == UV_SCROLL)
		{
			if (version < 10)
			{
				// compatibility
				UVScroll_09 values;
				memcpy(&values, pos, sizeof(values));
				pos += sizeof(values);
				UV.Scroll.Position.max.x = values.Position.x;
				UV.Scroll.Position.max.y = values.Position.y;
				UV.Scroll.Position.min = UV.Scroll.Position.max;

				UV.Scroll.Size.max.x = values.Position.w;
				UV.Scroll.Size.max.y = values.Position.h;
				UV.Scroll.Size.min = UV.Scroll.Size.max;

				UV.Scroll.Speed.max.x = values.Speed.x;
				UV.Scroll.Speed.max.y = values.Speed.y;
				UV.Scroll.Speed.min = UV.Scroll.Speed.max;

			}
			else
			{
				memcpy(&UV.Scroll, pos, sizeof(UV.Scroll));
				pos += sizeof(UV.Scroll);
			}
		}
		else if (UVType == UV_FCURVE)
		{
			UV.FCurve.Position = new FCurveVector2D();
			UV.FCurve.Size = new FCurveVector2D();
			pos += UV.FCurve.Position->Load(pos, version);
			pos += UV.FCurve.Size->Load(pos, version);
		}

		if (version >= 10)
		{
			memcpy(&ColorBindType, pos, sizeof(int32_t));
			pos += sizeof(int32_t);
		}
		else
		{
			ColorBindType = BindType::NotBind;
		}

		if (version >= 9)
		{
			if (version < 15)
			{
				int32_t distortion = 0;

				memcpy(&distortion, pos, sizeof(int32_t));
				pos += sizeof(int32_t);

				Distortion = distortion > 0;

				if (Distortion)
				{
					MaterialType = RendererMaterialType::BackDistortion;
				}
			}

			memcpy(&DistortionIntensity, pos, sizeof(float));
			pos += sizeof(float);
		}

		if (version >= 15)
		{
			CustomData1.load(pos, version);
			CustomData2.load(pos, version);
		}

		// copy to basic parameter
		BasicParameter.AlphaBlend = AlphaBlend;
		BasicParameter.TextureFilter1 = FilterType;
		BasicParameter.TextureFilter2 = Filter2Type;
		BasicParameter.TextureWrap1 = WrapType;
		BasicParameter.TextureWrap2 = Wrap2Type;

		BasicParameter.DistortionIntensity = DistortionIntensity;
		BasicParameter.MaterialType = MaterialType;
		BasicParameter.Texture1Index = ColorTextureIndex;
		BasicParameter.Texture2Index = Texture2Index;

		if (BasicParameter.MaterialType == RendererMaterialType::File)
		{
			BasicParameter.MaterialParameterPtr = &Material;
		}
		else
		{
			BasicParameter.MaterialParameterPtr = nullptr;
		}

		if (BasicParameter.MaterialType != RendererMaterialType::Lighting)
		{
			BasicParameter.TextureFilter2 = TextureFilterType::Nearest;
			BasicParameter.TextureWrap2 = TextureWrapType::Clamp;
		}
	}
};

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
enum ParameterSoundType
{
	ParameterSoundType_None = 0,
	ParameterSoundType_Use = 1,

	ParameterSoundType_DWORD = 0x7fffffff,
};

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
enum ParameterSoundPanType
{
	ParameterSoundPanType_2D = 0,
	ParameterSoundPanType_3D = 1,

	ParameterSoundPanType_DWORD = 0x7fffffff,
};

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
struct ParameterSound
{
	int32_t			WaveId;
	random_float	Volume;
	random_float	Pitch;
	ParameterSoundPanType PanType;
	random_float	Pan;
	float			Distance;
	random_int		Delay;
};

/**
	@brief	a factor to calculate original parameter for dynamic parameter
*/
struct DynamicFactorParameter
{
	std::array<float, 3> Tra;
	std::array<float, 3> TraInv;
	std::array<float, 3> Rot;
	std::array<float, 3> RotInv;
	std::array<float, 3> Scale;
	std::array<float, 3> ScaleInv;

	DynamicFactorParameter()
	{ 
		Tra.fill(1.0f);
		TraInv.fill(1.0f);
		Rot.fill(1.0f);
		RotInv.fill(1.0f);
		Scale.fill(1.0f);
		ScaleInv.fill(1.0f);
	}
};


//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
enum eRenderingOrder
{
	RenderingOrder_FirstCreatedInstanceIsFirst = 0,
	RenderingOrder_FirstCreatedInstanceIsLast = 1,

	RenderingOrder_DWORD = 0x7fffffff,
};


/**
@brief	ノードインスタンス生成クラス
@note
エフェクトのノードの実体を生成する。
*/
class EffectNodeImplemented
	: public EffectNode
{
	friend class Manager;
	friend class EffectImplemented;
	friend class Instance;

protected:
	// 所属しているパラメーター
	Effect*	m_effect;
	
	//! a generation in the node tree
	int generation_;

	// 子ノード
	std::vector<EffectNodeImplemented*>	m_Nodes;

	// ユーザーデータ
	void* m_userData;

	// コンストラクタ
	EffectNodeImplemented(Effect* effect, unsigned char*& pos);

	// デストラクタ
	virtual ~EffectNodeImplemented();

	// 読込
	void LoadParameter(unsigned char*& pos, EffectNode* parent, Setting* setting);

	// 初期化
	void Initialize();

	//! calculate custom data
	void CalcCustomData(const Instance* instance, std::array<float, 4>& customData1, std::array<float, 4>& customData2);

public:

	/**
	@brief	\~english Whether to draw the node.
	\~japanese このノードを描画するか?

	@note
	\~english 普通は描画されないノードは、描画の種類が変更されて、描画しないノードになる。ただし、色の継承をする場合、描画のみを行わないノードになる。
	\~japanese For nodes that are not normally rendered, the rendering type is changed to become a node that does not render. However, when color inheritance is done, it becomes a node which does not perform drawing only.
	*/
	bool IsRendered;

	ParameterCommonValues		CommonValues;

	ParameterTranslationType	TranslationType;
	ParameterTranslationFixed	TranslationFixed;
	ParameterTranslationPVA		TranslationPVA;
	ParameterTranslationEasing TranslationEasing;
	FCurveVector3D*				TranslationFCurve;

	LocationAbsParameter		LocationAbs;

	ParameterRotationType		RotationType;
	ParameterRotationFixed		RotationFixed;
	ParameterRotationPVA		RotationPVA;
	ParameterRotationEasing RotationEasing;
	FCurveVector3D*				RotationFCurve;

	ParameterRotationAxisPVA	RotationAxisPVA;
	ParameterRotationAxisEasing	RotationAxisEasing;

	ParameterScalingType		ScalingType;
	ParameterScalingFixed		ScalingFixed;
	ParameterScalingPVA			ScalingPVA;
	ParameterScalingEasing ScalingEasing;
	ParameterScalingSinglePVA	ScalingSinglePVA;
	easing_float				ScalingSingleEasing;
	FCurveVector3D*				ScalingFCurve;

	ParameterGenerationLocation	GenerationLocation;

	ParameterDepthValues		DepthValues;

	ParameterRendererCommon		RendererCommon;

	ParameterSoundType			SoundType;
	ParameterSound				Sound;

	eRenderingOrder				RenderingOrder;

	int32_t						RenderingPriority = -1;

	DynamicFactorParameter DynamicFactor;

	Effect* GetEffect() const override;

	int GetGeneration() const override;

	int GetChildrenCount() const override;

	EffectNode* GetChild(int index) const override;

	EffectBasicRenderParameter GetBasicRenderParameter() override;

	void SetBasicRenderParameter(EffectBasicRenderParameter param) override;

	EffectModelParameter GetEffectModelParameter() override;

	/**
	@brief	描画部分の読込
	*/
	virtual void LoadRendererParameter(unsigned char*& pos, Setting* setting);

	/**
	@brief	描画開始
	*/
	virtual void BeginRendering(int32_t count, Manager* manager);

	/**
	@brief	グループ描画開始
	*/
	virtual void BeginRenderingGroup(InstanceGroup* group, Manager* manager);

	virtual void EndRenderingGroup(InstanceGroup* group, Manager* manager);

	/**
	@brief	描画
	*/
	virtual void Rendering(const Instance& instance, const Instance* next_instance, Manager* manager);

	/**
	@brief	描画終了
	*/
	virtual void EndRendering(Manager* manager);

	/**
	@brief	インスタンスグループ描画時初期化
	*/
	virtual void InitializeRenderedInstanceGroup(InstanceGroup& instanceGroup, Manager* manager);

	/**
	@brief	描画部分初期化
	*/
	virtual void InitializeRenderedInstance(Instance& instance, Manager* manager);

	/**
	@brief	描画部分更新
	*/
	virtual void UpdateRenderedInstance(Instance& instance, Manager* manager);

	/**
	@brief	描画部分更新
	*/
	virtual float GetFadeAlpha(const Instance& instance);

	/**
	@brief	サウンド再生
	*/
	virtual void PlaySound_(Instance& instance, SoundTag tag, Manager* manager);

	EffectInstanceTerm CalculateInstanceTerm(EffectInstanceTerm& parentTerm) const override;

	/**
	@brief	エフェクトノード生成
	*/
	static EffectNodeImplemented* Create(Effect* effect, EffectNode* parent, unsigned char*& pos);

	/**
	@brief	ノードの種類取得
	*/
	virtual eEffectNodeType GetType() const { return EFFECT_NODE_TYPE_NONE; }
};
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------

}
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
#endif	// __EFFEKSEER_EFFECTNODE_H__

#ifndef	__EFFEKSEER_ParameterNODE_MODEL_H__
#define	__EFFEKSEER_ParameterNODE_MODEL_H__

//----------------------------------------------------------------------------------
// Include
//----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
namespace Effekseer
{
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
class EffectNodeModel
	: public EffectNodeImplemented
{
	friend class Manager;
	friend class Effect;
	friend class Instance;

public:

	struct InstanceValues
	{
		// 色
		Color _color;
		Color _original;

		union 
		{
			struct
			{
				Color _color;
			} fixed;

			struct
			{
				Color _color;
			} random;

			struct
			{
				Color start;
				Color  end;

			} easing;

			struct
			{
				std::array<float, 4> offset;
			} fcurve_rgba;

		} allColorValues;
	};

public:
	AlphaBlendType		AlphaBlend;
	int32_t			ModelIndex;

	//! this value is not used
	int32_t			NormalTextureIndex;

	BillboardType	Billboard;

	//! this value is not used
	bool			Lighting;
	CullingType	Culling;

	StandardColorParameter	AllColor;

	EffectNodeModel( Effect* effect, unsigned char*& pos )
		: EffectNodeImplemented(effect, pos)
	{
	}

	~EffectNodeModel()
	{
	}

	void LoadRendererParameter(unsigned char*& pos, Setting* setting) override;

	void BeginRendering(int32_t count, Manager* manager) override;

	void Rendering(const Instance& instance, const Instance* next_instance, Manager* manager) override;

	void EndRendering(Manager* manager) override;

	void InitializeRenderedInstance(Instance& instance, Manager* manager) override;

	void UpdateRenderedInstance(Instance& instance, Manager* manager) override;

	eEffectNodeType GetType() const override { return EFFECT_NODE_TYPE_MODEL; }
};

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
}
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
#endif	// __EFFEKSEER_ParameterNODE_MODEL_H__


#ifndef	__EFFEKSEER_ParameterNODE_RIBBON_H__
#define	__EFFEKSEER_ParameterNODE_RIBBON_H__

//----------------------------------------------------------------------------------
// Include
//----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
namespace Effekseer
{
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
struct RibbonAllColorParameter
{
	enum
	{
		Fixed = 0,
		Random = 1,
		Easing = 2,

		Parameter_DWORD = 0x7fffffff,
	} type;

	union
	{
		struct
		{
			Color all;
		} fixed;

		struct
		{
			random_color all;
		} random;

		struct
		{
			easing_color all;
		} easing;
	};
};

struct RibbonColorParameter
{
	enum
	{
		Default = 0,
		Fixed = 1,

		Parameter_DWORD = 0x7fffffff,
	} type;

	union
	{
		struct
		{

		} def;

		struct
		{
			Color l;
			Color r;
		} fixed;
	};
};

struct RibbonPositionParameter
{
	enum
	{
		Default = 0,
		Fixed = 1,

		Parameter_DWORD = 0x7fffffff,
	} type;

	union
	{
		struct
		{

		} def;

		struct
		{
			float l;
			float r;
		} fixed;
	};
};

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
class EffectNodeRibbon
	: public EffectNodeImplemented
{
public:

	struct InstanceValues
	{
		// 色
		Color _color;
		Color _original;

		union
		{
			struct
			{
				Color _color;
			} fixed;

			struct
			{
				Color _color;
			} random;

			struct
			{
				Color start;
				Color  end;

			} easing;

		} allColorValues;

		union
		{

		} colorValues;

		union
		{

		} positionValues;
	};

	RibbonRenderer::NodeParameter	m_nodeParameter;
	RibbonRenderer::InstanceParameter m_instanceParameter;
public:

	AlphaBlendType		AlphaBlend;

	int				ViewpointDependent;

	RibbonAllColorParameter	RibbonAllColor;

	RibbonColorParameter RibbonColor;
	RibbonPositionParameter RibbonPosition;

	int RibbonTexture;

	int32_t	SplineDivision = 1;

	NodeRendererTextureUVTypeParameter TextureUVType;

	EffectNodeRibbon(Effect* effect, unsigned char*& pos)
		: EffectNodeImplemented(effect, pos)
	{
	}

	~EffectNodeRibbon()
	{
	}

	void LoadRendererParameter(unsigned char*& pos, Setting* setting) override;

	void BeginRendering(int32_t count, Manager* manager) override;

	void BeginRenderingGroup(InstanceGroup* group, Manager* manager) override;

	void EndRenderingGroup(InstanceGroup* group, Manager* manager) override;

	void Rendering(const Instance& instance, const Instance* next_instance, Manager* manager) override;

	void EndRendering(Manager* manager) override;

	void InitializeRenderedInstance(Instance& instance, Manager* manager) override;

	void UpdateRenderedInstance(Instance& instance, Manager* manager) override;

	eEffectNodeType GetType() const override { return EFFECT_NODE_TYPE_RIBBON; }
};

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
}
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
#endif	// __EFFEKSEER_ParameterNODE_RIBBON_H__


#ifndef	__EFFEKSEER_ParameterNODE_RING_H__
#define	__EFFEKSEER_ParameterNODE_RING_H__

//----------------------------------------------------------------------------------
// Include
//----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
namespace Effekseer
{
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
struct RingSingleParameter
{
	enum
	{
		Fixed = 0,
		Random = 1,
		Easing = 2,

		Parameter_DWORD = 0x7fffffff,
	} type;

	union
	{
		float fixed;
		random_float random;
		easing_float easing;
	};
};

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
struct RingLocationParameter
{
	enum
	{
		Fixed = 0,
		PVA = 1,
		Easing = 2,

		Parameter_DWORD = 0x7fffffff,
	} type;
	
	union
	{
		struct
		{
			vector2d location;
		} fixed;
	
		struct
		{
			random_vector2d	location;
			random_vector2d	velocity;
			random_vector2d	acceleration;
		} pva;
		
		easing_vector2d easing;
	};
};

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
struct RingColorParameter
{
	enum
	{
		Fixed = 0,
		Random = 1,
		Easing = 2,

		Parameter_DWORD = 0x7fffffff,
	} type;

	union
	{	
		Color fixed;
		random_color random;
		easing_color easing;
	};
};

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
struct RingSingleValues
{
	float	current;
	union
	{
		struct
		{
			
		} fixed;

		struct
		{

		} random;

		struct
		{
			float  start;
			float  end;
		} easing;
	};
};

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
struct RingLocationValues
{
	vector2d	current;
	union
	{
		struct
		{
	
		} fixed;

		struct
		{
			vector2d  start;
			vector2d  velocity;
			vector2d  acceleration;
		} pva;

		struct
		{
			vector2d  start;
			vector2d  end;
		} easing;
	};
};

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
struct RingColorValues
{
	Color	current;
	Color	original;

	union
	{
		struct
		{
			Color _color;
		} fixed;

		struct
		{
			Color _color;
		} random;

		struct
		{
			Color  start;
			Color  end;
		} easing;
	};
};

enum class RingShapeType : int32_t
{
	Dount,
	Cresient,
};

struct RingShapeParameter
{
	RingShapeType Type = RingShapeType::Dount;
	float StartingFade = 0.0f;
	float EndingFade = 0.0f;
	RingSingleParameter StartingAngle;
	RingSingleParameter EndingAngle;
};

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
class EffectNodeRing
	: public EffectNodeImplemented
{
	friend class Manager;
	friend class Effect;
	friend class Instance;

public:

	struct InstanceValues
	{
		RingSingleValues startingAngle;
		RingSingleValues endingAngle;
		RingLocationValues outerLocation;
		RingLocationValues innerLocation;
		RingSingleValues centerRatio;
		RingColorValues outerColor;
		RingColorValues centerColor;
		RingColorValues innerColor;
	};

public:

	AlphaBlendType		AlphaBlend;
	BillboardType	Billboard;

	int32_t	VertexCount;

	RingShapeParameter Shape;
	//RingSingleParameter	ViewingAngle;

	RingLocationParameter	OuterLocation;
	RingLocationParameter	InnerLocation;
	
	RingSingleParameter	CenterRatio;
	
	RingColorParameter OuterColor;
	RingColorParameter CenterColor;
	RingColorParameter InnerColor;

	int RingTexture;

	RingRenderer::NodeParameter nodeParameter;

	EffectNodeRing( Effect* effect, unsigned char*& pos )
		: EffectNodeImplemented(effect, pos)
	{
	}

	~EffectNodeRing()
	{
	}

	void LoadRendererParameter(unsigned char*& pos, Setting* setting) override;

	void BeginRendering(int32_t count, Manager* manager) override;

	void Rendering(const Instance& instance, const Instance* next_instance, Manager* manager) override;

	void EndRendering(Manager* manager) override;

	void InitializeRenderedInstance(Instance& instance, Manager* manager) override;

	void UpdateRenderedInstance(Instance& instance, Manager* manager) override;

	eEffectNodeType GetType() const override { return EFFECT_NODE_TYPE_RING; }

private:
	void LoadSingleParameter( unsigned char*& pos, RingSingleParameter& param );

	void LoadLocationParameter( unsigned char*& pos, RingLocationParameter& param );
	
	void LoadColorParameter( unsigned char*& pos, RingColorParameter& param );
	
	void InitializeSingleValues(const RingSingleParameter& param, RingSingleValues& values, Manager* manager, InstanceGlobal* instanceGlobal);

	void InitializeLocationValues(const RingLocationParameter& param, RingLocationValues& values, Manager* manager, InstanceGlobal* instanceGlobal);
	
	void InitializeColorValues(const RingColorParameter& param, RingColorValues& values, Manager* manager, InstanceGlobal* instanceGlobal);
	
	void UpdateSingleValues( Instance& instance, const RingSingleParameter& param, RingSingleValues& values );
	
	void UpdateLocationValues( Instance& instance, const RingLocationParameter& param, RingLocationValues& values );
	
	void UpdateColorValues( Instance& instance, const RingColorParameter& param, RingColorValues& values );
};

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
}
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
#endif	// __EFFEKSEER_ParameterNODE_RING_H__


#ifndef	__EFFEKSEER_ParameterNODE_ROOT_H__
#define	__EFFEKSEER_ParameterNODE_ROOT_H__

//----------------------------------------------------------------------------------
// Include
//----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
namespace Effekseer
{
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
	
class EffectNodeRoot
	: public EffectNodeImplemented
{
	friend class Manager;
	friend class Effect;
	friend class Instance;

protected:

	

public:
	EffectNodeRoot( Effect* effect, unsigned char*& pos )
		: EffectNodeImplemented(effect, pos)
	{
	}

	~EffectNodeRoot()
	{
	
	}

	eEffectNodeType GetType() const { return EFFECT_NODE_TYPE_ROOT; }
};

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
}
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
#endif	// __EFFEKSEER_ParameterNODE_ROOT_H__


#ifndef	__EFFEKSEER_ParameterNODE_SPRITE_H__
#define	__EFFEKSEER_ParameterNODE_SPRITE_H__

//----------------------------------------------------------------------------------
// Include
//----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
namespace Effekseer
{
struct SpriteColorParameter
{
	enum
	{
		Default = 0,
		Fixed = 1,

		Parameter_DWORD = 0x7fffffff,
	} type;

	union
	{
		struct
		{
		
		} def;

		struct
		{
			Color ll;
			Color lr;
			Color ul;
			Color ur;
		} fixed;
	};
};

struct SpritePositionParameter
{
	enum
	{
		Default = 0,
		Fixed = 1,

		Parameter_DWORD = 0x7fffffff,
	} type;

	union
	{
		struct
		{
		
		} def;

		struct
		{
			vector2d ll;
			vector2d lr;
			vector2d ul;
			vector2d ur;
		} fixed;
	};
};

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
class EffectNodeSprite
	: public EffectNodeImplemented
{
	friend class Manager;
	friend class Effect;
	friend class Instance;

public:

	struct InstanceValues
	{
		// 色
		Color _color;

		Color _originalColor;
		
		union 
		{
			struct
			{
				Color _color;
			} fixed;

			struct
			{
				Color _color;
			} random;

			struct
			{
				Color start;
				Color  end;

			} easing;

			struct
			{
				std::array<float, 4> offset;
			} fcurve_rgba;

		} allColorValues;

		union
		{
	
		} colorValues;

		union
		{
	
		} positionValues;
	};

public:

	AlphaBlendType		AlphaBlend;
	BillboardType	Billboard;

	StandardColorParameter	SpriteAllColor;

	SpriteColorParameter SpriteColor;
	SpritePositionParameter SpritePosition;

	int SpriteTexture;

	EffectNodeSprite( Effect* effect, unsigned char*& pos )
		: EffectNodeImplemented(effect, pos)
	{
	}

	void LoadRendererParameter(unsigned char*& pos, Setting* setting) override;

	void BeginRendering(int32_t count, Manager* manager) override;

	void Rendering(const Instance& instance, const Instance* next_instance, Manager* manager) override;

	void EndRendering(Manager* manager) override;

	void InitializeRenderedInstance(Instance& instance, Manager* manager) override;

	void UpdateRenderedInstance(Instance& instance, Manager* manager) override;

	eEffectNodeType GetType() const override { return EFFECT_NODE_TYPE_SPRITE; }
};

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
}
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
#endif	// __EFFEKSEER_ParameterNODE_SPRITE_H__


#ifndef	__EFFEKSEER_ParameterNODE_TRACK_H__
#define	__EFFEKSEER_ParameterNODE_TRACK_H__

//----------------------------------------------------------------------------------
// Include
//----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
namespace Effekseer
{
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
struct TrackSizeParameter
{
	enum
	{
		Fixed = 0,

		Parameter_DWORD = 0x7fffffff,
	} type;

	union
	{
		struct
		{
			float size;
		} fixed;
	};
};

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
class EffectNodeTrack
	: public EffectNodeImplemented
{
public:

	struct InstanceGroupValues
	{
		struct Color
		{
			union
			{
				struct
				{
					Effekseer::Color color_;
				} fixed;

				struct
				{
					Effekseer::Color color_;
				} random;

				struct
				{
					Effekseer::Color start;
					Effekseer::Color  end;
				} easing;

				struct
				{
					std::array<float, 4> offset;
				} fcurve_rgba;

			} color;
		};

		struct Size
		{
			union
			{
				struct
				{
					float	size_;
				} fixed;
			} size;
		};

		Color	ColorLeft;
		Color	ColorCenter;
		Color	ColorRight;

		Color	ColorLeftMiddle;
		Color	ColorCenterMiddle;
		Color	ColorRightMiddle;

		Size	SizeFor;
		Size	SizeMiddle;
		Size	SizeBack;

	};

	struct InstanceValues
	{
		Color	colorLeft;
		Color	colorCenter;
		Color	colorRight;

		Color	colorLeftMiddle;
		Color	colorCenterMiddle;
		Color	colorRightMiddle;

		Color	_colorLeft;
		Color	_colorCenter;
		Color	_colorRight;

		Color	_colorLeftMiddle;
		Color	_colorCenterMiddle;
		Color	_colorRightMiddle;

		float	SizeFor;
		float	SizeMiddle;
		float	SizeBack;
	};

	TrackRenderer::NodeParameter	m_nodeParameter;
	TrackRenderer::InstanceParameter m_instanceParameter;

	InstanceGroupValues		m_currentGroupValues;

public:

	AlphaBlendType		AlphaBlend;

	StandardColorParameter	TrackColorLeft;
	StandardColorParameter	TrackColorCenter;
	StandardColorParameter	TrackColorRight;

	StandardColorParameter	TrackColorLeftMiddle;
	StandardColorParameter	TrackColorCenterMiddle;
	StandardColorParameter	TrackColorRightMiddle;

	TrackSizeParameter	TrackSizeFor;
	TrackSizeParameter	TrackSizeMiddle;
	TrackSizeParameter	TrackSizeBack;

	int TrackTexture;

	int32_t	SplineDivision = 1;

	NodeRendererTextureUVTypeParameter TextureUVType;

	EffectNodeTrack(Effect* effect, unsigned char*& pos)
		: EffectNodeImplemented(effect, pos)
		, TrackTexture(-1)
	{
	}

	~EffectNodeTrack()
	{
	}

	void LoadRendererParameter(unsigned char*& pos, Setting* setting) override;

	void BeginRendering(int32_t count, Manager* manager) override;

	void BeginRenderingGroup(InstanceGroup* group, Manager* manager) override;

	void EndRenderingGroup(InstanceGroup* group, Manager* manager) override;

	void Rendering(const Instance& instance, const Instance* next_instance, Manager* manager) override;

	void EndRendering(Manager* manager) override;

	void InitializeRenderedInstanceGroup(InstanceGroup& instanceGroup, Manager* manager) override;

	void InitializeRenderedInstance(Instance& instance, Manager* manager) override;

	void UpdateRenderedInstance(Instance& instance, Manager* manager) override;

	eEffectNodeType GetType() const override { return EFFECT_NODE_TYPE_TRACK; }

	void InitializeValues(InstanceGroupValues::Color& value, StandardColorParameter& param, InstanceGlobal* instanceGlobal);
	void InitializeValues(InstanceGroupValues::Size& value, TrackSizeParameter& param, Manager* manager);
	void SetValues(Color& c, const Instance& instance, InstanceGroupValues::Color& value, StandardColorParameter& param, int32_t time, int32_t livedTime);
	void SetValues(float& s, InstanceGroupValues::Size& value, TrackSizeParameter& param, float time);
	void LoadValues(TrackSizeParameter& param, unsigned char*& pos);
};

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
}
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
#endif	// __EFFEKSEER_ParameterNODE_TRACK_H__


#ifndef __EFFEKSEER_EFFECT_IMPLEMENTED_H__
#define __EFFEKSEER_EFFECT_IMPLEMENTED_H__

//----------------------------------------------------------------------------------
// Include
//----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
namespace Effekseer
{

/**
	@brief	A class to backup resorces when effect is reloaded
*/
class EffectReloadingBackup
{
public:
	template <class T> class Holder
	{
	public:
		T value;
		int counter = 0;
	};

	template <class T> class HolderCollection
	{
		std::map<std::u16string, Holder<T>> collection;

	public:
		void Push(const char16_t* key, T value)
		{
			auto key_ = std::u16string(key);
			auto it = collection.find(key_);

			if (it == collection.end())
			{
				collection[key_].value = value;
				collection[key_].counter = 1;
			}
			else
			{
				assert(it->second.value == value);
				it->second.counter++;
			}
		}

		bool Pop(const char16_t* key, T& value)
		{
			auto key_ = std::u16string(key);
			auto it = collection.find(key_);

			if (it == collection.end())
			{
				return false;
			}
			else
			{
				it->second.counter--;
				value = it->second.value;
				if (it->second.counter == 0)
				{
					collection.erase(it);
				}
				return true;
			}
		}

		std::map<std::u16string, Holder<T>>& GetCollection() { return collection; }
	};

	HolderCollection<TextureData*> images;
	HolderCollection<TextureData*> normalImages;
	HolderCollection<TextureData*> distortionImages;
	HolderCollection<void*> sounds;
	HolderCollection<void*> models;
	HolderCollection<MaterialData*> materials;
};

/**
	@brief	Effect parameter
*/
class EffectImplemented : public Effect, public ReferenceObject
{
	friend class ManagerImplemented;
	friend class EffectNodeImplemented;
	friend class EffectFactory;
	friend class Instance;

protected:
	ManagerImplemented* m_pManager;

	Setting* m_setting;

	mutable std::atomic<int32_t> m_reference;

	EffectFactory* factory = nullptr;

	int m_version;

	int m_ImageCount;
	EFK_CHAR** m_ImagePaths;
	TextureData** m_pImages;

	int m_normalImageCount;
	EFK_CHAR** m_normalImagePaths;
	TextureData** m_normalImages;

	int m_distortionImageCount;
	EFK_CHAR** m_distortionImagePaths;
	TextureData** m_distortionImages;

	int m_WaveCount = 0;
	EFK_CHAR** m_WavePaths = nullptr;
	void** m_pWaves = nullptr;

	int32_t modelCount_ = 0;
	EFK_CHAR** modelPaths_ = nullptr;
	void** models_ = nullptr;

	int32_t materialCount_ = 0;
	EFK_CHAR** materialPaths_ = nullptr;
	MaterialData** materials_ = nullptr;

	std::u16string name_;
	std::basic_string<EFK_CHAR> m_materialPath;

	//! dynamic inputs
	std::array<float, 4> defaultDynamicInputs;

	//! dynamic parameters
	std::vector<InternalScript> dynamicEquation;

	int32_t renderingNodesCount = 0;
	int32_t renderingNodesThreshold = 0;

	//! scaling of this effect
	float m_maginification = 1.0f;

	float m_maginificationExternal = 1.0f;

	// default random seed
	int32_t m_defaultRandomSeed;

	//! child root node
	EffectNode* m_pRoot = nullptr;

	// culling
	struct
	{
		CullingShape Shape;
		Vector3D Location;

		union {
			struct
			{
			} None;

			struct
			{
				float Radius;
			} Sphere;
		};

	} Culling;

	//! a flag to reload
	bool isReloadingOnRenderingThread = false;

	//! backup to reload on rendering thread
	std::unique_ptr<EffectReloadingBackup> reloadingBackup;

	ReferenceObject* loadingObject = nullptr;

	bool LoadBody(const uint8_t* data, int32_t size, float mag);

	void ResetReloadingBackup();

public:
	/**
		@brief	生成
	*/
	static Effect* Create(Manager* pManager, void* pData, int size, float magnification, const EFK_CHAR* materialPath = NULL);

	/**
		@brief	生成
	*/
	static Effect* Create(Setting* setting, void* pData, int size, float magnification, const EFK_CHAR* materialPath = NULL);

	// コンストラクタ
	EffectImplemented(Manager* pManager, void* pData, int size);

	// コンストラクタ
	EffectImplemented(Setting* setting, void* pData, int size);

	// デストラクタ
	virtual ~EffectImplemented();

	// Rootの取得
	EffectNode* GetRoot() const override;

	float GetMaginification() const override;

	bool Load(void* pData, int size, float mag, const EFK_CHAR* materialPath, ReloadingThreadType reloadingThreadType);

	/**
		@breif	何も読み込まれていない状態に戻す
	*/
	void Reset();

	/**
		@brief	Compatibility for magnification.
	*/
	bool IsDyanamicMagnificationValid() const;

	ReferenceObject* GetLoadingParameter() const override;

	void SetLoadingParameter(ReferenceObject* obj);

private:
	/**
		@brief	マネージャー取得
	*/
	Manager* GetManager() const;

public:
	const char16_t* GetName() const override;

	void SetName(const char16_t* name) override;

	/**
	@brief	設定取得
	*/
	Setting* GetSetting() const override;

	/**
		@brief	エフェクトデータのバージョン取得
	*/
	int GetVersion() const override;

	/**
		@brief	格納されている画像のポインタを取得する。
	*/
	TextureData* GetColorImage(int n) const override;

	int32_t GetColorImageCount() const override;

	const EFK_CHAR* GetColorImagePath(int n) const override;

	/**
	@brief	格納されている画像のポインタを取得する。
	*/
	TextureData* GetNormalImage(int n) const override;

	int32_t GetNormalImageCount() const override;

	const EFK_CHAR* GetNormalImagePath(int n) const override;

	TextureData* GetDistortionImage(int n) const override;

	int32_t GetDistortionImageCount() const override;

	const EFK_CHAR* GetDistortionImagePath(int n) const override;

	void* GetWave(int n) const override;

	int32_t GetWaveCount() const override;

	const EFK_CHAR* GetWavePath(int n) const override;

	void* GetModel(int n) const override;

	int32_t GetModelCount() const override;

	const EFK_CHAR* GetModelPath(int n) const override;

	MaterialData* GetMaterial(int n) const override;

	int32_t GetMaterialCount() const override;

	const EFK_CHAR* GetMaterialPath(int n) const override;

	/**
		@brief	エフェクトのリロードを行う。
	*/
	bool Reload(void* data, int32_t size, const EFK_CHAR* materialPath, ReloadingThreadType reloadingThreadType) override;

	/**
		@brief	エフェクトのリロードを行う。
	*/
	bool Reload(const EFK_CHAR* path, const EFK_CHAR* materialPath, ReloadingThreadType reloadingThreadType) override;

	/**
		@brief	エフェクトのリロードを行う。
	*/
	bool Reload(Manager** managers,
				int32_t managersCount,
				void* data,
				int32_t size,
				const EFK_CHAR* materialPath,
				ReloadingThreadType reloadingThreadType) override;

	/**
		@brief	エフェクトのリロードを行う。
	*/
	bool Reload(Manager** managers,
				int32_t managersCount,
				const EFK_CHAR* path,
				const EFK_CHAR* materialPath,
				ReloadingThreadType reloadingThreadType) override;

	/**
		@brief	画像等リソースの再読み込みを行う。
	*/
	void ReloadResources(const void* data, int32_t size, const EFK_CHAR* materialPath) override;

	void UnloadResources(const EFK_CHAR* materialPath);

	void UnloadResources() override;

	EffectTerm CalculateTerm() const override;

	virtual int GetRef() override { return ReferenceObject::GetRef(); }
	virtual int AddRef() override { return ReferenceObject::AddRef(); }
	virtual int Release() override { return ReferenceObject::Release(); }
};
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
} // namespace Effekseer
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
#endif // __EFFEKSEER_EFFECT_IMPLEMENTED_H__


#ifndef	__EFFEKSEER_MANAGER_IMPLEMENTED_H__
#define	__EFFEKSEER_MANAGER_IMPLEMENTED_H__

//----------------------------------------------------------------------------------
// Include
//----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
namespace Effekseer
{
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------

/**
	@brief エフェクト管理クラス(実装)
*/
class ManagerImplemented
	: public Manager
	, public ReferenceObject
{
	friend class Effect;
	friend class EffectNode;
	friend class InstanceContainer;
	friend class InstanceGroup;

private:

	/**
		@brief	再生オブジェクトの組み合わせ
	*/
	class DrawSet
	{
	public:
		Effect*			ParameterPointer;
		InstanceContainer*	InstanceContainerPointer;
		InstanceGlobal*		GlobalPointer;
		Culling3D::Object*	CullingObjectPointer;
		bool				IsPaused;
		bool				IsShown;
		bool				IsAutoDrawing;
		bool				IsRemoving;
		bool				IsParameterChanged;
		bool				DoUseBaseMatrix;
		bool				GoingToStop;
		bool				GoingToStopRoot;
		EffectInstanceRemovingCallback	RemovingCallback;
		
		Matrix43			BaseMatrix;

		Matrix43			GlobalMatrix;

		float				Speed;

		Handle				Self;

		bool				IsPreupdated = false;
		int32_t				StartFrame = 0;

		int32_t Layer = 0;

		DrawSet( Effect* effect, InstanceContainer* pContainer, InstanceGlobal* pGlobal )
			: ParameterPointer			( effect )
			, InstanceContainerPointer	( pContainer )
			, GlobalPointer				( pGlobal )
			, CullingObjectPointer		( NULL )
			, IsPaused					( false )
			, IsShown					( true )
			, IsAutoDrawing				( true )
			, IsRemoving				( false )
			, IsParameterChanged		( false )
			, DoUseBaseMatrix			( false )
			, GoingToStop				( false )
			, GoingToStopRoot			( false )
			, RemovingCallback			( NULL )
			, Speed						( 1.0f )
			, Self						( -1 )
		{
		
		}

		DrawSet()
			: ParameterPointer			( NULL )
			, InstanceContainerPointer	( NULL )
			, GlobalPointer				( NULL )
			, CullingObjectPointer		( NULL )
			, IsPaused					( false )
			, IsShown					( true )
			, IsRemoving				( false )
			, IsParameterChanged		( false )
			, DoUseBaseMatrix			( false )
			, RemovingCallback			(NULL)
			, Speed						( 1.0f )
			, Self						( -1 )
		{
		
		}

		Matrix43* GetEnabledGlobalMatrix();

		void CopyMatrixFromInstanceToRoot();
	};

	struct CullingParameter
	{
		float		SizeX;
		float		SizeY;
		float		SizeZ;
		int32_t		LayerCount;

		CullingParameter()
		{
			SizeX = 0.0f;
			SizeY = 0.0f;
			SizeZ = 0.0f;
			LayerCount = 0;
		}

	} cullingCurrent, cullingNext;

private:
	/* 自動データ入れ替えフラグ */
	bool m_autoFlip;

	// 次のHandle
	Handle		m_NextHandle;

	// 確保済みインスタンス数
	int m_instance_max;

	// buffers which is allocated while initializing
	// 初期化中に確保されたバッファ
	std::unique_ptr<InstanceChunk[]> reservedChunksBuffer_;
	std::unique_ptr<uint8_t[]> reservedGroupBuffer_;
	std::unique_ptr<uint8_t[]> reservedContainerBuffer_;

	// pooled instances. Thease are not used and waiting to be used.
	// プールされたインスタンス。使用されておらず、使用されてるのを待っている。
	std::queue<InstanceChunk*> pooledChunks_;
	std::queue<InstanceGroup*> pooledGroups_;
	std::queue<InstanceContainer*> pooledContainers_;

	// instance chunks by generations
	// 世代ごとのインスタンスチャンク
	static const size_t GenerationsMax = 20;
	std::array<std::vector<InstanceChunk*>, GenerationsMax> instanceChunks_;
	std::array<int32_t, GenerationsMax> creatableChunkOffsets_;

	// 再生中オブジェクトの組み合わせ集合体
	std::map<Handle,DrawSet>	m_DrawSets;

	// 破棄待ちオブジェクト
	std::map<Handle,DrawSet>	m_RemovingDrawSets[2];

	//! objects on rendering
	CustomVector<DrawSet> m_renderingDrawSets;

	//! objects on rendering
	CustomMap<Handle,DrawSet> m_renderingDrawSetMaps;

	// mutex for rendering
	std::mutex					m_renderingMutex;
	bool						m_isLockedWithRenderingMutex = false;

	/* 設定インスタンス */
	Setting*					m_setting;

	int							m_updateTime;
	int							m_drawTime;

	/* 更新回数カウント */
	uint32_t					m_sequenceNumber;
	
	/* カリング */
	Culling3D::World*			m_cullingWorld;

	/* カリング */
	std::vector<DrawSet*>	m_culledObjects;
	std::set<Handle>		m_culledObjectSets;
	bool					m_culled;

	/* スプライト描画機能用インスタンス */
	SpriteRenderer*				m_spriteRenderer;

	/* リボン描画機能用インスタンス */
	RibbonRenderer*				m_ribbonRenderer;

	/* リング描画機能用インスタンス */
	RingRenderer*				m_ringRenderer;

	/* モデル描画機能用インスタンス */
	ModelRenderer*				m_modelRenderer;

	/* トラック描画機能用インスタンス */
	TrackRenderer*				m_trackRenderer;

	/* サウンド再生用インスタンス */
	SoundPlayer*				m_soundPlayer;

	// メモリ確保関数
	MallocFunc	m_MallocFunc;

	// メモリ破棄関数
	FreeFunc	m_FreeFunc;

	// ランダム関数
	RandFunc	m_randFunc;

	// ランダム関数最大値
	int			m_randMax;

	// 描画オブジェクト追加
	Handle AddDrawSet( Effect* effect, InstanceContainer* pInstanceContainer, InstanceGlobal* pGlobalPointer );

	// 描画オブジェクト破棄処理
	void GCDrawSet( bool isRemovingManager );

	// メモリ確保関数
	static void* EFK_STDCALL Malloc( unsigned int size );

	// メモリ破棄関数
	static void EFK_STDCALL Free( void* p, unsigned int size );

	// ランダム関数
	static int EFK_STDCALL Rand();

	// 破棄等のイベントを実際に実行
	void ExecuteEvents();
public:

	// コンストラクタ
	ManagerImplemented( int instance_max, bool autoFlip );

	// デストラクタ
	virtual ~ManagerImplemented();

	/* Root以外のインスタンスバッファ生成(Flip,Update,終了時からのみ呼ばれる) */
	Instance* CreateInstance( EffectNode* pEffectNode, InstanceContainer* pContainer, InstanceGroup* pGroup );

	InstanceGroup* CreateInstanceGroup( EffectNode* pEffectNode, InstanceContainer* pContainer, InstanceGlobal* pGlobal );
	void ReleaseGroup( InstanceGroup* group );

	InstanceContainer* CreateInstanceContainer( EffectNode* pEffectNode, InstanceGlobal* pGlobal, bool isRoot, const Matrix43& rootMatrix, Instance* pParent );
	void ReleaseInstanceContainer( InstanceContainer* container );

	/**
		@brief マネージャー破棄
		@note
		このマネージャーから生成されたエフェクトは全て強制的に破棄されます。
	*/
	void Destroy() override;

	/**
		@brief	更新番号を取得する。
	*/
	uint32_t GetSequenceNumber() const;

	MallocFunc GetMallocFunc() const override;

	void SetMallocFunc( MallocFunc func ) override;

	FreeFunc GetFreeFunc() const override;

	void SetFreeFunc( FreeFunc func ) override;

	RandFunc GetRandFunc() const override;

	void SetRandFunc( RandFunc func ) override;

	/**
		@brief	ランダム最大値取得
	*/
	int GetRandMax() const override;

	/**
		@brief	ランダム関数設定
	*/
	void SetRandMax( int max_ ) override;

	/**
		@brief	座標系を取得する。
	*/
	CoordinateSystem GetCoordinateSystem() const override;

	/**
		@brief	座標系を設定する。
	*/
	void SetCoordinateSystem( CoordinateSystem coordinateSystem ) override;

	/**
		@brief	スプライト描画機能取得
	*/
	SpriteRenderer* GetSpriteRenderer() override;

	/**
		@brief	スプライト描画機能設定
	*/
	void SetSpriteRenderer( SpriteRenderer* renderer ) override;

	/**
		@brief	リボン描画機能取得
	*/
	RibbonRenderer* GetRibbonRenderer() override;

	/**
		@brief	リボン描画機能設定
	*/
	void SetRibbonRenderer( RibbonRenderer* renderer ) override;

	/**
		@brief	リング描画機能取得
	*/
	RingRenderer* GetRingRenderer() override;

	/**
		@brief	リング描画機能設定
	*/
	void SetRingRenderer( RingRenderer* renderer ) override;

	/**
		@brief	モデル描画機能取得
	*/
	ModelRenderer* GetModelRenderer() override;

	/**
		@brief	モデル描画機能設定
	*/
	void SetModelRenderer( ModelRenderer* renderer ) override;

	/**
		@brief	軌跡描画機能取得
	*/
	TrackRenderer* GetTrackRenderer() override;

	/**
		@brief	軌跡描画機能設定
	*/
	void SetTrackRenderer( TrackRenderer* renderer ) override;

	/**
		@brief	設定クラスを取得する。
	*/
	Setting* GetSetting() override;

	/**
		@brief	設定クラスを設定する。
		@param	setting	[in]	設定
	*/
	void SetSetting(Setting* setting) override;

	/**
		@brief	エフェクト読込クラスを取得する。
	*/
	EffectLoader* GetEffectLoader() override;

	/**
		@brief	エフェクト読込クラスを設定する。
	*/
	void SetEffectLoader( EffectLoader* effectLoader ) override;

	/**
		@brief	テクスチャ読込クラスを取得する。
	*/
	TextureLoader* GetTextureLoader() override;

	/**
		@brief	テクスチャ読込クラスを設定する。
	*/
	void SetTextureLoader( TextureLoader* textureLoader ) override;
	
	/**
		@brief	サウンド再生取得
	*/
	SoundPlayer* GetSoundPlayer() override;

	/**
		@brief	サウンド再生機能設定
	*/
	void SetSoundPlayer( SoundPlayer* soundPlayer ) override;
	
	/**
		@brief	サウンド再生取得
	*/
	SoundLoader* GetSoundLoader() override;

	/**
		@brief	サウンド再生機能設定
	*/
	void SetSoundLoader( SoundLoader* soundLoader ) override;

	/**
		@brief	モデル読込クラスを取得する。
	*/
	ModelLoader* GetModelLoader() override;

	/**
		@brief	モデル読込クラスを設定する。
	*/
	void SetModelLoader( ModelLoader* modelLoader ) override;
	
	MaterialLoader* GetMaterialLoader() override;

	void SetMaterialLoader(MaterialLoader* loader) override;

	/**
		@brief	エフェクト停止
	*/
	void StopEffect( Handle handle ) override;

	/**
		@brief	全てのエフェクト停止
	*/
	void StopAllEffects() override;

	/**
		@brief	エフェクト停止
	*/
	void StopRoot( Handle handle ) override;

	/**
		@brief	エフェクト停止
	*/
	void StopRoot( Effect* effect ) override;

	/**
		@brief	エフェクトのインスタンスが存在しているか取得する。
		@param	handle	[in]	インスタンスのハンドル
		@return	存在してるか?
	*/
	bool Exists( Handle handle ) override;

	int32_t GetInstanceCount( Handle handle ) override;

	int32_t GetTotalInstanceCount() const override;

	/**
		@brief	エフェクトのインスタンスに設定されている行列を取得する。
		@param	handle	[in]	インスタンスのハンドル
		@return	行列
	*/
	Matrix43 GetMatrix( Handle handle ) override;

	/**
		@brief	エフェクトのインスタンスに変換行列を設定する。
		@param	handle	[in]	インスタンスのハンドル
		@param	mat		[in]	変換行列
	*/
	void SetMatrix( Handle handle, const Matrix43& mat ) override;

	Vector3D GetLocation( Handle handle ) override;
	void SetLocation( Handle handle, float x, float y, float z ) override;
	void SetLocation( Handle handle, const Vector3D& location ) override;
	void AddLocation( Handle handle, const Vector3D& location ) override;

	/**
		@brief	エフェクトのインスタンスの回転角度を指定する。(ラジアン)
	*/
	void SetRotation( Handle handle, float x, float y, float z ) override;

	/**
		@brief	エフェクトのインスタンスの任意軸周りの反時計周りの回転角度を指定する。
		@param	handle	[in]	インスタンスのハンドル
		@param	axis	[in]	軸
		@param	angle	[in]	角度(ラジアン)
	*/
	void SetRotation( Handle handle, const Vector3D& axis, float angle ) override;

	/**
		@brief	エフェクトのインスタンスの拡大率を指定する。
		@param	handle	[in]	インスタンスのハンドル
		@param	x		[in]	X方向拡大率
		@param	y		[in]	Y方向拡大率
		@param	z		[in]	Z方向拡大率
	*/
	void SetScale( Handle handle, float x, float y, float z ) override;

	void SetAllColor(Handle handle, Color color) override;

	// エフェクトのターゲット位置を指定する。
	void SetTargetLocation( Handle handle, float x, float y, float z ) override;
	void SetTargetLocation( Handle handle, const Vector3D& location ) override;

	float GetDynamicInput(Handle handle, int32_t index) override;

	void SetDynamicInput(Handle handle, int32_t index, float value) override;

	Matrix43 GetBaseMatrix( Handle handle ) override;

	void SetBaseMatrix( Handle handle, const Matrix43& mat ) override;

	/**
		@brief	エフェクトのインスタンスに廃棄時のコールバックを設定する。
		@param	handle	[in]	インスタンスのハンドル
		@param	callback	[in]	コールバック
	*/
	void SetRemovingCallback( Handle handle, EffectInstanceRemovingCallback callback ) override;

	bool GetShown(Handle handle) override;

	void SetShown( Handle handle, bool shown ) override;
	
	bool GetPaused(Handle handle) override;

	void SetPaused( Handle handle, bool paused ) override;

	void SetPausedToAllEffects(bool paused) override;

	int GetLayer(Handle handle) override;

	void SetLayer(Handle handle, int32_t layer) override;

	float GetSpeed(Handle handle) const override;

	void SetSpeed( Handle handle, float speed ) override;
	
	void SetAutoDrawing( Handle handle, bool autoDraw ) override;
	
	void Flip() override;

	void Update( float deltaFrame ) override;

	void BeginUpdate() override;

	void EndUpdate() override;

	void UpdateHandle( Handle handle, float deltaFrame = 1.0f ) override;

private:
	void UpdateHandle( DrawSet& drawSet, float deltaFrame );

	void Preupdate(DrawSet& drawSet);

	//! whether container is disabled while rendering because of a distance between the effect and a camera
	bool IsClippedWithDepth(DrawSet& drawSet, InstanceContainer* container, const Manager::DrawParameter& drawParameter);
 public:

	void Draw(const Manager::DrawParameter& drawParameter) override;
	
	void DrawBack(const Manager::DrawParameter& drawParameter) override;

	void DrawFront(const Manager::DrawParameter& drawParameter) override;

	void DrawHandle(Handle handle, const Manager::DrawParameter& drawParameter) override;

	void DrawHandleBack(Handle handle, const Manager::DrawParameter& drawParameter) override;

	void DrawHandleFront(Handle handle, const Manager::DrawParameter& drawParameter) override;

	Handle Play( Effect* effect, float x, float y, float z ) override;

	Handle Play(Effect* effect, const Vector3D& position, int32_t startFrame) override;
	
	int GetCameraCullingMaskToShowAllEffects() override;

	int GetUpdateTime() const override;
	
	int GetDrawTime() const override;

	int32_t GetRestInstancesCount() const override;

	/**
		@brief	start reload
	*/
	void BeginReloadEffect( Effect* effect, bool doLockThread );

	/**
		@brief	end reload
	*/
	void EndReloadEffect( Effect* effect, bool doLockThread);

	/**
		@brief	エフェクトをカリングし描画負荷を減らすための空間を生成する。
		@param	xsize	X方向幅
		@param	ysize	Y方向幅
		@param	zsize	Z方向幅
		@param	layerCount	層数(大きいほどカリングの効率は上がるがメモリも大量に使用する)
	*/
	void CreateCullingWorld( float xsize, float ysize, float zsize, int32_t layerCount) override;

	/**
		@brief	カリングを行い、カリングされたオブジェクトのみを描画するようにする。
		@param	cameraProjMat	カメラプロジェクション行列
		@param	isOpenGL		OpenGLによる描画か?
	*/
	void CalcCulling(const Matrix44& cameraProjMat, bool isOpenGL) override;

	/**
		@brief	現在存在するエフェクトのハンドルからカリングの空間を配置しなおす。
	*/
	void RessignCulling() override;

	virtual int GetRef() override { return ReferenceObject::GetRef(); }
	virtual int AddRef() override { return ReferenceObject::AddRef(); }
	virtual int Release() override { return ReferenceObject::Release(); }
};
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
}
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
#endif	// __EFFEKSEER_MANAGER_IMPLEMENTED_H__


#ifndef	__EFFEKSEER_INTRUSIVE_LIST_H__
#define	__EFFEKSEER_INTRUSIVE_LIST_H__

//----------------------------------------------------------------------------------
// Include
//----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
namespace Effekseer { 
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
/**
	@brief	Intrusive List
	@code
	class Instance : public IntrusiveList<Instance> {...};
	@endcode
*/
template <typename T>
class IntrusiveList final
{
public:
	typedef T Type;
	
	class Iterator
	{
		Type* m_Node = nullptr;
	public:
		Iterator() = default;
		Iterator( const Iterator& it ) = default;
		Iterator( Type* node ): m_Node(node) {}
		Type* operator*() const { assert( m_Node != nullptr ); return m_Node; }
		Type* operator->() const { assert( m_Node != nullptr ); return m_Node; }
		Iterator& operator++() { assert( m_Node != nullptr ); m_Node = m_Node->m_NextNode; return *this; }
		Iterator operator++(int) { assert( m_Node != nullptr ); Iterator it(m_Node); m_Node = m_Node->m_NextNode; return it; }
		bool operator==( const Iterator& rhs ) const { return m_Node == rhs.m_Node; }
		bool operator!=( const Iterator& rhs ) const { return m_Node != rhs.m_Node; }
	};
	
	class ReverseIterator
	{
		Type* m_Node = nullptr;
	public:
		ReverseIterator() = default;
		ReverseIterator( const ReverseIterator& it ) = default;
		ReverseIterator( Type* node ): m_Node(node) {}
		Type* operator*() const { assert( m_Node != nullptr ); return m_Node; }
		Type* operator->() const { assert( m_Node != nullptr ); return m_Node; }
		ReverseIterator& operator++() { assert( m_Node != nullptr ); m_Node = m_Node->m_PrevNode; return *this; }
		ReverseIterator operator++(int) { assert( m_Node != nullptr ); ReverseIterator it(m_Node); m_Node = m_Node->m_PrevNode; return it; }
		bool operator==( const ReverseIterator& rhs ) const { return m_Node == rhs.m_Node; }
		bool operator!=( const ReverseIterator& rhs ) const { return m_Node != rhs.m_Node; }
	};
	
	class Node
	{
		friend class IntrusiveList<Type>;
		friend class IntrusiveList<Type>::Iterator;
		
	private:
		Type* m_PrevNode = nullptr;
		Type* m_NextNode = nullptr;
	};

private:
	Type* m_HeadNode = nullptr;
	Type* m_TailNode = nullptr;
	size_t m_Count = 0;

public:
	IntrusiveList() = default;
	IntrusiveList( const IntrusiveList<T>& rhs ) = delete;
	IntrusiveList<T>& operator=( const IntrusiveList<T>& rhs ) = delete;
	IntrusiveList( IntrusiveList<T>&& rhs );
	IntrusiveList<T>& operator=( IntrusiveList<T>&& rhs );
	~IntrusiveList();

	void push_back( Type* newObject );
	void pop_back();
	void push_front( Type* newObject );
	void pop_front();
		
	Iterator insert( Iterator it, Type* newObject );
	Iterator erase( Iterator it );
	void clear();
		
	Type* front() const;
	Type* back() const;

	bool empty() const { return m_Count == 0; }
	size_t size() const { return m_Count; }

	Iterator begin() const { return Iterator( m_HeadNode ); }
	Iterator end() const { return Iterator( nullptr ); }
	ReverseIterator rbegin() const { return ReverseIterator( m_TailNode ); }
	ReverseIterator rend() const { return ReverseIterator( nullptr ); }
};

template <typename T>
IntrusiveList<T>::IntrusiveList( IntrusiveList<T>&& rhs )
{
	m_HeadNode = rhs.m_HeadNode;
	m_TailNode = rhs.m_TailNode;
	m_Count = rhs.m_Count;
	rhs.m_HeadNode = nullptr;
	rhs.m_TailNode = nullptr;
	rhs.m_Count = 0;
}

template <typename T>
IntrusiveList<T>& IntrusiveList<T>::operator=( IntrusiveList<T>&& rhs )
{
	m_HeadNode = rhs.m_HeadNode;
	m_TailNode = rhs.m_TailNode;
	m_Count = rhs.m_Count;
	rhs.m_HeadNode = nullptr;
	rhs.m_TailNode = nullptr;
	rhs.m_Count = 0;
}

template <typename T>
IntrusiveList<T>::~IntrusiveList()
{
	clear();
}

template <typename T>
inline void IntrusiveList<T>::push_back( typename IntrusiveList<T>::Type* newObject )
{
	assert( newObject != nullptr );
	assert( newObject->m_PrevNode == nullptr );
	assert( newObject->m_NextNode == nullptr );

	if( m_TailNode )
	{
		newObject->m_PrevNode = m_TailNode;
		m_TailNode->m_NextNode = newObject;
		m_TailNode = newObject;
	}
	else
	{
		m_HeadNode = newObject;
		m_TailNode = newObject;
	}
	m_Count++;
}
	
template <typename T>
inline void IntrusiveList<T>::pop_back()
{
	assert( m_TailNode != nullptr );
	if( m_TailNode )
	{
		auto prev = m_TailNode->m_PrevNode;
		m_TailNode->m_PrevNode = nullptr;
		m_TailNode->m_NextNode = nullptr;
		if( prev ){ prev->m_NextNode = nullptr; }
		m_TailNode = prev;
		m_Count--;
	}
}
	
template <typename T>
inline void IntrusiveList<T>::push_front( typename IntrusiveList<T>::Type* newObject )
{
	assert( newObject != nullptr );
	assert( newObject->m_PrevNode == nullptr );
	assert( newObject->m_NextNode == nullptr );

	if( m_HeadNode )
	{
		newObject->m_NextNode = m_HeadNode;
		m_HeadNode->m_PrevNode = newObject;
		m_HeadNode = newObject;
	}
	else
	{
		m_HeadNode = newObject;
		m_TailNode = newObject;
	}
	m_Count++;
}
	
template <typename T>
inline void IntrusiveList<T>::pop_front()
{
	assert( m_HeadNode != nullptr );
	if( m_HeadNode )
	{
		auto next = m_HeadNode->m_NextNode;
		m_HeadNode->m_PrevNode = nullptr;
		m_HeadNode->m_NextNode = nullptr;
		if( next ){ next->m_PrevNode = nullptr; }
		m_HeadNode = next;
		m_Count--;
	}
}
	
template <typename T>
inline typename IntrusiveList<T>::Iterator
IntrusiveList<T>::insert( typename IntrusiveList<T>::Iterator it, Type* newObject )
{
	assert( newObject != nullptr );
	assert( newObject->m_PrevNode == nullptr );
	assert( newObject->m_NextNode == nullptr );
	auto prev = it->m_PrevNode;
	newObject->m_PrevNode = prev;
	newObject->m_NextNode = *it;
	if( prev ){ prev->m_NextNode = newObject; }
	else{ m_HeadNode = newObject; }
	m_Count++;
	return IntrusiveList<T>::Iterator( newObject );
}
	
template <typename T>
inline typename IntrusiveList<T>::Iterator 
IntrusiveList<T>::erase( typename IntrusiveList<T>::Iterator it )
{
	auto prev = it->m_PrevNode;
	auto next = it->m_NextNode;
	it->m_PrevNode = nullptr;
	it->m_NextNode = nullptr;
	if( prev ) prev->m_NextNode = next;
	else m_HeadNode = next;
	if( next ){ next->m_PrevNode = prev; }
	else{ m_TailNode = prev; }
	m_Count--;
	return IntrusiveList<T>::Iterator( next );
}
	
template <typename T>
inline void IntrusiveList<T>::clear()
{
	for( Type* it = m_HeadNode; it != nullptr; )
	{
		Type* next = it->m_NextNode;
		it->m_PrevNode = nullptr;
		it->m_NextNode = nullptr;
		it = next;
	}
	m_HeadNode = nullptr;
	m_TailNode = nullptr;
	m_Count = 0;
}

template <typename T>
T* IntrusiveList<T>::front() const
{
	assert( m_HeadNode != nullptr );
	return m_HeadNode;
}

template <typename T>
T* IntrusiveList<T>::back() const
{
	assert( m_TailNode != nullptr );
	return m_TailNode;
}
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
 } 
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
#endif	// __EFFEKSEER_INTRUSIVE_LIST_H__


#ifndef	__EFFEKSEER_INSTANCECONTAINER_H__
#define	__EFFEKSEER_INSTANCECONTAINER_H__

//----------------------------------------------------------------------------------
// Include
//----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
namespace Effekseer
{
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------

/**
	@brief	
	@note

*/
class InstanceContainer : public IntrusiveList<InstanceContainer>::Node
{
	friend class ManagerImplemented;

private:

	// マネージャ
	ManagerImplemented*	m_pManager;

	// パラメーター
	EffectNodeImplemented* m_pEffectNode;

	// グローバル
	InstanceGlobal*	m_pGlobal;

	// 子のコンテナ
	IntrusiveList<InstanceContainer>	m_Children;

	// グループの連結リストの先頭
	InstanceGroup*	m_headGroups;

	// グループの連結リストの最後
	InstanceGroup*	m_tailGroups;

	// コンストラクタ
	InstanceContainer( ManagerImplemented* pManager, EffectNode* pEffectNode, InstanceGlobal* pGlobal );

	// デストラクタ
	virtual ~InstanceContainer();

	// 無効なグループの破棄
	void RemoveInvalidGroups();

public:
	/**
		@brief	グループの作成
	*/
	InstanceGroup* CreateInstanceGroup();

	/**
		@brief	グループの先頭取得
	*/
	InstanceGroup* GetFirstGroup() const;

	void Update( bool recursive, float deltaFrame, bool shown );

	void SetBaseMatrix( bool recursive, const Matrix43& mat );

	void RemoveForcibly( bool recursive );

	void Draw( bool recursive );

	void KillAllInstances(  bool recursive );

	InstanceGlobal* GetRootInstance();

	void AddChild( InstanceContainer* pContainter );

	InstanceContainer* GetChild( int index );
};

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
}
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
#endif	// __EFFEKSEER_INSTANCECONTAINER_H__


#ifndef	__EFFEKSEER_INSTANCE_H__
#define	__EFFEKSEER_INSTANCE_H__

//----------------------------------------------------------------------------------
// Include
//----------------------------------------------------------------------------------



//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
namespace Effekseer
{

struct InstanceCustomData
{
	union {
		struct
		{
			vector2d start;
			vector2d end;
		} easing;

		struct
		{
			vector2d offset;
		} fcruve;

		struct
		{
			std::array<float, 4> offset;
		} fcurveColor;
	};
};

/**
	@brief	エフェクトの実体
*/
class alignas(16) Instance : public IntrusiveList<Instance>::Node
{
	friend class Manager;
	friend class InstanceContainer;

protected:

	//! custom data
	InstanceCustomData customDataValues1;
	InstanceCustomData customDataValues2;

public:
	static const int32_t ChildrenMax = 16;

	// マネージャ
	Manager*	m_pManager;

	// パラメーター
	EffectNodeImplemented* m_pEffectNode;

	// コンテナ
	InstanceContainer*	m_pContainer;

	// a group which the instance belongs to
	// 自分が所属するグループ
	InstanceGroup* ownGroup_;

	// a head of list in children group
	// 子グループの連結リストの先頭
	InstanceGroup* childrenGroups_;

	// 親
	Instance*	m_pParent;
	
	// グローバル位置
	Vector3D	m_GlobalPosition;
	Vector3D	m_GlobalVelocity;
	
	// グローバル位置補正
	Vector3D	m_GlobalRevisionLocation;
	Vector3D	m_GlobalRevisionVelocity;
	
	// Color for binding
	Color		ColorInheritance;

	// Parent color
	Color		ColorParent;

	union 
	{
		struct
		{
		
		} fixed;

		struct
		{
			vector3d location;
			vector3d velocity;
			vector3d acceleration;
		} random;

		struct
		{
			vector3d	start;
			vector3d	end;
		} easing;

		struct
		{
			vector3d	offset;
		} fcruve;

	} translation_values;

	union 
	{
		struct
		{
		
		} fixed;

		struct
		{
			vector3d rotation;
			vector3d velocity;
			vector3d acceleration;
		} random;

		struct
		{
			vector3d start;
			vector3d end;
		} easing;
		
		struct
		{
			float rotation;
			vector3d axis;

			union
			{
				struct
				{
					float rotation;
					float velocity;
					float acceleration;
				} random;

				struct
				{
					float start;
					float end;
				} easing;
			};
		} axis;

		struct
		{
			vector3d offset;
		} fcruve;

	} rotation_values;

	union 
	{
		struct
		{
		
		} fixed;

		struct
		{
			vector3d  scale;
			vector3d  velocity;
			vector3d  acceleration;
		} random;

		struct
		{
			vector3d  start;
			vector3d  end;
		} easing;
		
		struct
		{
			float  scale;
			float  velocity;
			float  acceleration;
		} single_random;

		struct
		{
			float  start;
			float  end;
		} single_easing;

		struct
		{
			vector3d offset;
		} fcruve;

	} scaling_values;

	// 描画
	union
	{
		EffectNodeSprite::InstanceValues	sprite;
		EffectNodeRibbon::InstanceValues	ribbon;
		EffectNodeRing::InstanceValues		ring;
		EffectNodeModel::InstanceValues		model;
		EffectNodeTrack::InstanceValues		track;
	} rendererValues;
	
	// 音
	union
	{
		int32_t		delay;
	} soundValues;

	// 状態
	eInstanceState	m_State;

	// 生存時間
	float		m_LivedTime;

	// 生成されてからの時間
	float		m_LivingTime;

	//! The time offset for UV animation
	int32_t uvTimeOffset = 0;

	// Scroll, FCurve area for UV
	RectF		uvAreaOffset;

	// Scroll speed for UV
	Vector2D	uvScrollSpeed;

	// The number of generated chiledren. (fixed size)
	int32_t		m_fixedGeneratedChildrenCount[ChildrenMax];

	// The number of maximum generated chiledren. (fixed size)
	int32_t fixedMaxGenerationChildrenCount_[ChildrenMax];

	// The time to generate next child.  (fixed size)
	float		m_fixedNextGenerationTime[ChildrenMax];

	// The number of generated chiledren. (flexible size)
	int32_t*		m_flexibleGeneratedChildrenCount;

	// The number of maximum generated chiledren. (flexible size)
	int32_t* flexibleMaxGenerationChildrenCount_ = nullptr;

	// The time to generate next child.  (flexible size)
	float*		m_flexibleNextGenerationTime;

	// The number of generated chiledren. (actually used)
	int32_t*		m_generatedChildrenCount;

	// The number of maximum generated chiledren. (actually used)
	int32_t* maxGenerationChildrenCount = nullptr;

	// The time to generate next child.  (actually used)
	float*			m_nextGenerationTime;

	// Spawning Method matrix
	Matrix43		m_GenerationLocation;

	// 変換用行列
	Matrix43		m_GlobalMatrix43;

	// 親の変換用行列
	Matrix43		m_ParentMatrix;

	// 変換用行列が計算済かどうか
	bool			m_GlobalMatrix43Calculated;

	// 親の変換用行列が計算済かどうか
	bool			m_ParentMatrix43Calculated;

	//! whether a time is allowed to pass
	bool			is_time_step_allowed;

	/* 更新番号 */
	uint32_t		m_sequenceNumber;

	//! calculate dynamic equation and assign a result
	template <typename T, typename U>
	void ApplyEq(T& dstParam, Effect* e, InstanceGlobal* instg, int dpInd, const U& originalParam);

	//! calculate dynamic equation and return a result
	template <typename S> 
	Vector3D ApplyEq(const int& dpInd, Vector3D originalParam, const S& scale, const S& scaleInv);

	//! calculate dynamic equation and return a result
	random_float ApplyEq(const RefMinMax& dpInd, random_float originalParam);

	//! calculate dynamic equation and return a result
	template <typename S> 
	random_vector3d ApplyEq(const RefMinMax& dpInd, random_vector3d originalParam, const S& scale, const S& scaleInv);

	//! calculate dynamic equation and return a result
	random_int ApplyEq(const RefMinMax& dpInd, random_int originalParam);

	// コンストラクタ
	Instance( Manager* pManager, EffectNode* pEffectNode, InstanceContainer* pContainer, InstanceGroup* pGroup );

	// デストラクタ
	virtual ~Instance();

	bool IsRequiredToCreateChildren(float currentTime);

	void GenerateChildrenInRequired(float currentTime);

	void UpdateChildrenGroupMatrix();

	InstanceGlobal* GetInstanceGlobal();

public:
	/**
		@brief	状態の取得
	*/
	eInstanceState GetState() const;

	/**
		@brief	行列の取得
	*/
	const Matrix43& GetGlobalMatrix43() const;

	/**
		@brief	初期化
	*/
	void Initialize( Instance* parent, int32_t instanceNumber, int32_t parentTime, const Matrix43& globalMatrix);

	/**
		@brief	更新
	*/
	void Update( float deltaFrame, bool shown );

	/**
		@brief	Draw instance
	*/
	void Draw(Instance* next);

	/**
		@brief	破棄
	*/
	void Kill();

	/**
		@brief	UVの位置取得
	*/
	RectF GetUV() const;

	//! get custom data
	std::array<float,4> GetCustomData(int32_t index) const;

private:
	/**
		@brief	行列の更新
	*/
	void CalculateMatrix( float deltaFrame );
	
	/**
		@brief	行列の更新
	*/
	void CalculateParentMatrix( float deltaFrame );
	
	/**
		@brief	絶対パラメータの反映
	*/
	void ModifyMatrixFromLocationAbs( float deltaFrame );
	
};

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
}
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
#endif	// __EFFEKSEER_INSTANCE_H__


#ifndef __EFFEKSEER_INSTANCECHUNK_H__
#define __EFFEKSEER_INSTANCECHUNK_H__


namespace Effekseer
{

/**
	@brief	a group of allocated instances
	@note
	instances are allocated as a group because of memory optimization
*/
class alignas(32) InstanceChunk
{
public:
	static const int32_t InstancesOfChunk = 16;

	InstanceChunk();

	~InstanceChunk();

	void UpdateInstances(float deltaFrame);

	void UpdateInstancesByInstanceGlobal(InstanceGlobal* global, float deltaFrame);

	Instance* CreateInstance(Manager* pManager, EffectNode* pEffectNode, InstanceContainer* pContainer, InstanceGroup* pGroup);

	int32_t GetAliveCount() const { return aliveCount_; }

	bool IsInstanceCreatable() const { return aliveCount_ < InstancesOfChunk; }

private:
	std::array<uint8_t[sizeof(Instance)], InstancesOfChunk> instances_;

	//! flags whether are instances alive
	std::array<bool, InstancesOfChunk> instancesAlive_;

	//! the number of living instances
	int32_t aliveCount_ = 0;
};

} // namespace Effekseer

#endif // __EFFEKSEER_INSTANCECHUNK_H__


#ifndef	__EFFEKSEER_INSTANCEGLOBAL_H__
#define	__EFFEKSEER_INSTANCEGLOBAL_H__

//----------------------------------------------------------------------------------
// Include
//----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
namespace Effekseer
{
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------

/**
	@brief	インスタンス共通部分
	@note
	生成されたインスタンスの全てから参照できる部分
*/
class InstanceGlobal
	: public IRandObject
{
	friend class ManagerImplemented;
	friend class Instance;


private:
	/* このエフェクトで使用しているインスタンス数 */
	int			m_instanceCount;
	
	/* 更新されたフレーム数 */
	float		m_updatedFrame;

	InstanceContainer*	m_rootContainer;
	Vector3D			m_targetLocation;

	int64_t				m_seed = 0;

	std::array<float, 4> dynamicInputParameters;

	//! placement new
	static void* operator new(size_t size);

	//! placement delete
	static void operator delete(void* p);

	InstanceGlobal();

	virtual ~InstanceGlobal();

public:

	bool		IsGlobalColorSet = false;
	Color		GlobalColor = Color(255, 255, 255, 255);

	std::array<std::array<float, 4>, 16> dynamicEqResults;

	std::vector<InstanceContainer*>	RenderedInstanceContainers;

	std::array<float, 4> GetDynamicEquationResult(int32_t index);
	void SetSeed(int64_t seed);

	virtual float GetRand() override;

	virtual float GetRand(float min_, float max_) override;

	void IncInstanceCount();

	void DecInstanceCount();

	void AddUpdatedFrame( float frame );

	/**
		@brief	全てのインスタンス数を取得
	*/
	int GetInstanceCount();

	/**
		@brief	更新されたフレーム数を取得する。
	*/
	float GetUpdatedFrame();

	InstanceContainer* GetRootContainer() const;
	void SetRootContainer( InstanceContainer* container );

	const Vector3D& GetTargetLocation() const;
	void SetTargetLocation( const Vector3D& location );

	static float Rand(void* userData);

	static float RandSeed(void* userData, float randSeed);
};
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
}
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
#endif	// __EFFEKSEER_INSTANCEGLOBAL_H__


#ifndef	__EFFEKSEER_INSTANCEGROUP_H__
#define	__EFFEKSEER_INSTANCEGROUP_H__

//----------------------------------------------------------------------------------
// Include
//----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
namespace Effekseer
{
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------

/**
	@brief	インスタンスグループ
	@note
	インスタンスコンテナ内でさらにインスタンスをグループ化するクラス
*/
class InstanceGroup
{
friend class InstanceContainer;
friend class ManagerImplemented;

private:
	ManagerImplemented*		m_manager;
	EffectNodeImplemented*	m_effectNode;
	InstanceContainer*	m_container;
	InstanceGlobal*		m_global;
	int32_t				m_time;

	Matrix43 parentMatrix_;
	Matrix43 parentRotation_;
	Vector3D parentTranslation_;
	Vector3D parentScale_;

	// インスタンスの実体
	IntrusiveList<Instance> m_instances;
	IntrusiveList<Instance> m_removingInstances;

	InstanceGroup( Manager* manager, EffectNode* effectNode, InstanceContainer* container, InstanceGlobal* global );

	~InstanceGroup();

public:

	/** 
		@brief	描画に必要なパラメータ
	*/
	union
	{
		EffectNodeTrack::InstanceGroupValues		track;
	} rendererValues;

	/**
		@brief	インスタンスの生成
	*/
	Instance* CreateInstance();

	Instance* GetFirst();

	int GetInstanceCount() const;

	void Update( float deltaFrame, bool shown );

	void SetBaseMatrix( const Matrix43& mat );

	void SetParentMatrix( const Matrix43& mat );

	void RemoveForcibly();

	void KillAllInstances();

	int32_t GetTime() const { return m_time; }

	/**
		@brief	グループを生成したインスタンスからの参照が残っているか?
	*/
	bool IsReferencedFromInstance;

	/**
		@brief	インスタンスから利用する連結リストの次のオブジェクトへのポインタ
	*/
	InstanceGroup*	NextUsedByInstance;

	/**
		@brief	コンテナから利用する連結リストの次のオブジェクトへのポインタ
	*/
	InstanceGroup*	NextUsedByContainer;

	InstanceGlobal* GetRootInstance() const { return m_global; }

	const Matrix43& GetParentMatrix() const { return parentMatrix_; }
	const Vector3D& GetParentTranslation() const { return parentTranslation_; }
	const Matrix43& GetParentRotation() const { return parentRotation_; }
	const Vector3D& GetParentScale() const { return parentScale_; }
};
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
}
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
#endif	// __EFFEKSEER_INSTANCEGROUP_H__




namespace Effekseer
{

FCurve::FCurve(float defaultValue) : defaultValue_(defaultValue) {}

int32_t FCurve::Load(void* data, int32_t version)
{
	int32_t size = 0;
	uint8_t* p = (uint8_t*)data;

	memcpy(&start_, p, sizeof(int32_t));
	p += sizeof(int32_t);
	size += sizeof(int32_t);

	memcpy(&end_, p, sizeof(int32_t));
	p += sizeof(int32_t);
	size += sizeof(int32_t);

	memcpy(&offsetMax_, p, sizeof(float));
	p += sizeof(float);
	size += sizeof(float);

	memcpy(&offsetMin_, p, sizeof(float));
	p += sizeof(float);
	size += sizeof(float);

	memcpy(&offset_, p, sizeof(int32_t));
	p += sizeof(int32_t);
	size += sizeof(int32_t);

	memcpy(&len_, p, sizeof(int32_t));
	p += sizeof(int32_t);
	size += sizeof(int32_t);

	memcpy(&freq_, p, sizeof(int32_t));
	p += sizeof(int32_t);
	size += sizeof(int32_t);

	int32_t count = 0;
	memcpy(&count, p, sizeof(int32_t));
	p += sizeof(int32_t);
	size += sizeof(int32_t);

	for (int32_t i = 0; i < count; i++)
	{
		float value = 0;

		memcpy(&value, p, sizeof(float));
		p += sizeof(float);
		size += sizeof(float);

		keys_.push_back(value);
	}

	return size;
}

float FCurve::GetValue(float living, float life, FCurveTimelineType type) const
{
	if (keys_.size() == 0)
		return defaultValue_;

	float frame = 0;
	if (type == FCurveTimelineType::Time)
	{
		frame = living;
	}
	else
	{
		frame = living / life * 100.0f;
	}

	frame -= offset_;
	auto flen = static_cast<float>(len_);

	if (frame < 0)
	{
		if (start_ == FCurveEdge::Constant)
		{
			return keys_[0];
		}
		else if (start_ == FCurveEdge::Loop)
		{
			frame = len_ - fmodf(-frame, flen);
		}
		else if (start_ == FCurveEdge::LoopInversely)
		{
			frame = fmodf(-frame, flen);
		}
	}

	if (len_ < frame)
	{
		if (end_ == FCurveEdge::Constant)
		{
			return keys_[keys_.size() - 1];
		}
		else if (end_ == FCurveEdge::Loop)
		{
			frame = fmodf(frame - flen, flen);
		}
		else if (end_ == FCurveEdge::LoopInversely)
		{
			frame = flen - fmodf(frame - flen, flen);
		}
	}

	assert(frame / freq_ >= 0.0f);
	uint32_t ind = static_cast<uint32_t>(frame / freq_);
	auto ep = 0.0001f;
	if (std::abs(frame - flen) < ep)
	{
		return keys_[keys_.size() - 1];
	}
	else if (ind == keys_.size() - 1)
	{
		float subF = (float)(len_ - ind * freq_);
		float subV = keys_[ind + 1] - keys_[ind];
		return subV / (float)(subF) * (float)(frame - ind * freq_) + keys_[ind];
	}
	else
	{
		float subF = (float)(freq_);
		float subV = keys_[ind + 1] - keys_[ind];
		return subV / (float)(subF) * (float)(frame - ind * freq_) + keys_[ind];
	}
}

float FCurve::GetOffset(InstanceGlobal& g) const { return g.GetRand(offsetMin_, offsetMax_); }

void FCurve::ChangeCoordinate()
{
	offsetMax_ *= -1.0f;
	offsetMin_ *= -1.0f;

	for (size_t i = 0; i < keys_.size(); i++)
	{
		keys_[i] *= -1.0f;
	}
}

void FCurve::Maginify(float value)
{
	offsetMax_ *= value;
	offsetMin_ *= value;

	for (size_t i = 0; i < keys_.size(); i++)
	{
		keys_[i] *= value;
	}
}

int32_t FCurveVector2D::Load(void* data, int32_t version)
{
	int32_t size = 0;
	uint8_t* p = (uint8_t*)data;

	if (version >= 15)
	{
		memcpy(&Timeline, p, sizeof(int32_t));
		size += sizeof(int);
		p += sizeof(int);
	}

	int32_t x_size = X.Load(p, version);
	size += x_size;
	p += x_size;

	int32_t y_size = Y.Load(p, version);
	size += y_size;
	p += y_size;

	return size;
}

std::array<float, 2> FCurveVector2D::GetValues(float living, float life) const
{
	auto x = X.GetValue(living, life, Timeline);
	auto y = Y.GetValue(living, life, Timeline);
	return std::array<float, 2>{x, y};
}

std::array<float, 2> FCurveVector2D::GetOffsets(InstanceGlobal& g) const
{
	auto x = X.GetOffset(g);
	auto y = Y.GetOffset(g);
	return std::array<float, 2>{x, y};
}

int32_t FCurveVector3D::Load(void* data, int32_t version)
{
	int32_t size = 0;
	uint8_t* p = (uint8_t*)data;

	if (version >= 15)
	{
		memcpy(&Timeline, p, sizeof(int32_t));
		size += sizeof(int);
		p += sizeof(int);
	}

	int32_t x_size = X.Load(p, version);
	size += x_size;
	p += x_size;

	int32_t y_size = Y.Load(p, version);
	size += y_size;
	p += y_size;

	int32_t z_size = Z.Load(p, version);
	size += z_size;
	p += z_size;

	return size;
}

std::array<float, 3> FCurveVector3D::GetValues(float living, float life) const
{
	auto x = X.GetValue(living, life, Timeline);
	auto y = Y.GetValue(living, life, Timeline);
	auto z = Z.GetValue(living, life, Timeline);
	return std::array<float, 3>{x, y, z};
}

std::array<float, 3> FCurveVector3D::GetOffsets(InstanceGlobal& g) const
{
	auto x = X.GetOffset(g);
	auto y = Y.GetOffset(g);
	auto z = Z.GetOffset(g);
	return std::array<float, 3>{x, y, z};
}

int32_t FCurveVectorColor::Load(void* data, int32_t version)
{
	int32_t size = 0;
	uint8_t* p = (uint8_t*)data;

	if (version >= 15)
	{
		memcpy(&Timeline, p, sizeof(int32_t));
		size += sizeof(int);
		p += sizeof(int);
	}

	int32_t x_size = R.Load(p, version);
	size += x_size;
	p += x_size;

	int32_t y_size = G.Load(p, version);
	size += y_size;
	p += y_size;

	int32_t z_size = B.Load(p, version);
	size += z_size;
	p += z_size;

	int32_t w_size = A.Load(p, version);
	size += w_size;
	p += w_size;

	return size;
}

std::array<float, 4> FCurveVectorColor::GetValues(float living, float life) const
{
	auto r = R.GetValue(living, life, Timeline);
	auto g = G.GetValue(living, life, Timeline);
	auto b = B.GetValue(living, life, Timeline);
	auto a = A.GetValue(living, life, Timeline);

	return std::array<float, 4>{r, g, b, a};
}

std::array<float, 4> FCurveVectorColor::GetOffsets(InstanceGlobal& gl) const
{
	auto r = R.GetOffset(gl);
	auto g = G.GetOffset(gl);
	auto b = B.GetOffset(gl);
	auto a = A.GetOffset(gl);
	return std::array<float, 4>{r, g, b, a};
}

} // namespace Effekseer



//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------





//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
namespace Effekseer
{

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
EffectNodeImplemented::EffectNodeImplemented(Effect* effect, unsigned char*& pos)
	: m_effect(effect)
	, generation_(0)
	, m_userData(NULL)
	, IsRendered(true)
	, TranslationFCurve(NULL)
	, RotationFCurve(NULL)
	, ScalingFCurve(NULL)
	, SoundType(ParameterSoundType_None)
	, RenderingOrder(RenderingOrder_FirstCreatedInstanceIsFirst)
{
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void EffectNodeImplemented::LoadParameter(unsigned char*& pos, EffectNode* parent, Setting* setting)
{
	int size = 0;
	int node_type = 0;
	auto ef = (EffectImplemented*)m_effect;

	if (parent) {
		generation_ = parent->GetGeneration() + 1;
	} else {
		generation_ = 0;
	}

	memcpy(&node_type, pos, sizeof(int));
	pos += sizeof(int);

	if (node_type == -1)
	{
		TranslationType = ParameterTranslationType_None;
		LocationAbs.type = LocationAbsType::None;
		RotationType = ParameterRotationType_None;
		ScalingType = ParameterScalingType_None;
		CommonValues.MaxGeneration = 1;

		GenerationLocation.EffectsRotation = 0;
		GenerationLocation.type = ParameterGenerationLocation::TYPE_POINT;
		GenerationLocation.point.location.reset();

		RenderingPriority = -1;
	}
	else
	{
		if (m_effect->GetVersion() >= 10)
		{
			int32_t rendered = 0;
			memcpy(&rendered, pos, sizeof(int32_t));
			pos += sizeof(int32_t);

			IsRendered = rendered != 0;
		}

		// To render with priority, nodes are assigned a list.
		if (m_effect->GetVersion() >= 13)
		{
			memcpy(&RenderingPriority, pos, sizeof(int32_t));
			pos += sizeof(int32_t);
		}
		else
		{
			RenderingPriority = -1;
		}

		memcpy(&size, pos, sizeof(int));
		pos += sizeof(int);

		if (ef->GetVersion() >= 14)
		{
			assert(size == sizeof(ParameterCommonValues));
			memcpy(&CommonValues, pos, size);
			pos += size;
		}
		else if (m_effect->GetVersion() >= 9)
		{
			memcpy(&CommonValues.MaxGeneration, pos, size);
			pos += size;
		}
		else
		{
			assert(size == sizeof(ParameterCommonValues_8));
			ParameterCommonValues_8 param_8;
			memcpy(&param_8, pos, size);
			pos += size;

			CommonValues.MaxGeneration = param_8.MaxGeneration;
			CommonValues.TranslationBindType = param_8.TranslationBindType;
			CommonValues.RotationBindType = param_8.RotationBindType;
			CommonValues.ScalingBindType = param_8.ScalingBindType;
			CommonValues.RemoveWhenLifeIsExtinct = param_8.RemoveWhenLifeIsExtinct;
			CommonValues.RemoveWhenParentIsRemoved = param_8.RemoveWhenParentIsRemoved;
			CommonValues.RemoveWhenChildrenIsExtinct = param_8.RemoveWhenChildrenIsExtinct;
			CommonValues.life = param_8.life;
			CommonValues.GenerationTime.max = param_8.GenerationTime;
			CommonValues.GenerationTime.min = param_8.GenerationTime;
			CommonValues.GenerationTimeOffset.max = param_8.GenerationTimeOffset;
			CommonValues.GenerationTimeOffset.min = param_8.GenerationTimeOffset;
		}

		memcpy(&TranslationType, pos, sizeof(int));
		pos += sizeof(int);

		if (TranslationType == ParameterTranslationType_Fixed)
		{
			int32_t translationSize = 0;
			memcpy(&translationSize, pos, sizeof(int));
			pos += sizeof(int);

			if (ef->GetVersion() >= 14)
			{
				memcpy(&TranslationFixed, pos, sizeof(ParameterTranslationFixed));
			}
			else
			{
				memcpy(&(TranslationFixed.Position), pos, sizeof(float) * 3);

				// make invalid
				if (TranslationFixed.Position.X == 0.0f && TranslationFixed.Position.Y == 0.0f && TranslationFixed.Position.Z == 0.0f)
				{
					TranslationType = ParameterTranslationType_None;
					EffekseerPrintDebug("LocationType Change None\n");
				}
			}

			pos += translationSize;
		}
		else if (TranslationType == ParameterTranslationType_PVA)
		{
			if (ef->GetVersion() >= 14)
			{
				memcpy(&size, pos, sizeof(int));
				pos += sizeof(int);
				assert(size == sizeof(ParameterTranslationPVA));
				memcpy(&TranslationPVA, pos, size);
				pos += size;
			}
			else
			{
				memcpy(&size, pos, sizeof(int));
				pos += sizeof(int);
				memcpy(&TranslationPVA.location, pos, size);
				pos += size;
			}
		}
		else if (TranslationType == ParameterTranslationType_Easing)
		{
			memcpy(&size, pos, sizeof(int));
			pos += sizeof(int);

			if (ef->GetVersion() >= 14)
			{
				assert(size == sizeof(ParameterTranslationEasing));
				memcpy(&TranslationEasing, pos, size);
				pos += size;
			}
			else
			{
				assert(size == sizeof(easing_vector3d));
				memcpy(&TranslationEasing.location, pos, size);
				pos += size;
			}
		}
		else if (TranslationType == ParameterTranslationType_FCurve)
		{
			memcpy(&size, pos, sizeof(int));
			pos += sizeof(int);

			TranslationFCurve = new FCurveVector3D();
			pos += TranslationFCurve->Load(pos, m_effect->GetVersion());
		}

		/* 位置拡大処理 */
		if (ef->IsDyanamicMagnificationValid())
		{
			DynamicFactor.Tra[0] *= m_effect->GetMaginification();
			DynamicFactor.Tra[1] *= m_effect->GetMaginification();
			DynamicFactor.Tra[2] *= m_effect->GetMaginification();

			if (TranslationType == ParameterTranslationType_Fixed)
			{
				TranslationFixed.Position *= m_effect->GetMaginification();
			}
			else if (TranslationType == ParameterTranslationType_PVA)
			{
				TranslationPVA.location.min *= m_effect->GetMaginification();
				TranslationPVA.location.max *= m_effect->GetMaginification();
				TranslationPVA.velocity.min *= m_effect->GetMaginification();
				TranslationPVA.velocity.max *= m_effect->GetMaginification();
				TranslationPVA.acceleration.min *= m_effect->GetMaginification();
				TranslationPVA.acceleration.max *= m_effect->GetMaginification();
			}
			else if (TranslationType == ParameterTranslationType_Easing)
			{
				TranslationEasing.location.start.min *= m_effect->GetMaginification();
				TranslationEasing.location.start.max *= m_effect->GetMaginification();
				TranslationEasing.location.end.min *= m_effect->GetMaginification();
				TranslationEasing.location.end.max *= m_effect->GetMaginification();
			}
			else if (TranslationType == ParameterTranslationType_FCurve)
			{
				TranslationFCurve->X.Maginify(m_effect->GetMaginification());
				TranslationFCurve->Y.Maginify(m_effect->GetMaginification());
				TranslationFCurve->Z.Maginify(m_effect->GetMaginification());
			}
		}

		memcpy(&LocationAbs.type, pos, sizeof(int));
		pos += sizeof(int);

		// Calc attraction forces
		if (LocationAbs.type == LocationAbsType::None)
		{
			memcpy(&size, pos, sizeof(int));
			pos += sizeof(int);
			assert(size == 0);
			memcpy(&LocationAbs.none, pos, size);
			pos += size;
		}
		else if (LocationAbs.type == LocationAbsType::Gravity)
		{
			memcpy(&size, pos, sizeof(int));
			pos += sizeof(int);
			assert(size == sizeof(vector3d));
			memcpy(&LocationAbs.gravity, pos, size);
			pos += size;
		}
		else if (LocationAbs.type == LocationAbsType::AttractiveForce)
		{
			memcpy(&size, pos, sizeof(int));
			pos += sizeof(int);
			assert(size == sizeof(LocationAbs.attractiveForce));
			memcpy(&LocationAbs.attractiveForce, pos, size);
			pos += size;
		}

		// Magnify attraction forces
		if (ef->IsDyanamicMagnificationValid())
		{
			if (LocationAbs.type == LocationAbsType::None)
			{
			}
			else if (LocationAbs.type == LocationAbsType::Gravity)
			{
				LocationAbs.gravity *= m_effect->GetMaginification();
			}
			else if (LocationAbs.type == LocationAbsType::AttractiveForce)
			{
				LocationAbs.attractiveForce.control *= m_effect->GetMaginification();
				LocationAbs.attractiveForce.force *= m_effect->GetMaginification();
				LocationAbs.attractiveForce.minRange *= m_effect->GetMaginification();
				LocationAbs.attractiveForce.maxRange *= m_effect->GetMaginification();
			}
		}

		memcpy(&RotationType, pos, sizeof(int));
		pos += sizeof(int);
		EffekseerPrintDebug("RotationType %d\n", RotationType);
		if (RotationType == ParameterRotationType_Fixed)
		{
			memcpy(&size, pos, sizeof(int));
			pos += sizeof(int);

			if (ef->GetVersion() >= 14)
			{
				assert(size == sizeof(ParameterRotationFixed));
				memcpy(&RotationFixed, pos, size);
			}
			else
			{
				memcpy(&RotationFixed.Position, pos, size);
			}
			pos += size;

			// make invalid
			if (RotationFixed.RefEq < 0 && RotationFixed.Position.X == 0.0f && RotationFixed.Position.Y == 0.0f &&
				RotationFixed.Position.Z == 0.0f)
			{
				RotationType = ParameterRotationType_None;
				EffekseerPrintDebug("RotationType Change None\n");
			}
		}
		else if (RotationType == ParameterRotationType_PVA)
		{
			memcpy(&size, pos, sizeof(int));
			pos += sizeof(int);
			if (ef->GetVersion() >= 14)
			{
				assert(size == sizeof(ParameterRotationPVA));
				memcpy(&RotationPVA, pos, size);
			}
			else
			{
				memcpy(&RotationPVA.rotation, pos, size);
			}
			pos += size;
		}
		else if (RotationType == ParameterRotationType_Easing)
		{
			memcpy(&size, pos, sizeof(int));
			pos += sizeof(int);
			if (ef->GetVersion() >= 14)
			{
				assert(size == sizeof(RotationEasing));
				memcpy(&RotationEasing, pos, size);
			}
			else
			{
				assert(size == sizeof(easing_vector3d));
				memcpy(&RotationEasing.rotation, pos, size);
			}

			pos += size;
		}
		else if (RotationType == ParameterRotationType_AxisPVA)
		{
			memcpy(&size, pos, sizeof(int));
			pos += sizeof(int);
			assert(size == sizeof(ParameterRotationAxisPVA));
			memcpy(&RotationAxisPVA, pos, size);
			pos += size;
		}
		else if (RotationType == ParameterRotationType_AxisEasing)
		{
			memcpy(&size, pos, sizeof(int));
			pos += sizeof(int);
			assert(size == sizeof(ParameterRotationAxisEasing));
			memcpy(&RotationAxisEasing, pos, size);
			pos += size;
		}
		else if (RotationType == ParameterRotationType_FCurve)
		{
			memcpy(&size, pos, sizeof(int));
			pos += sizeof(int);

			RotationFCurve = new FCurveVector3D();
			pos += RotationFCurve->Load(pos, m_effect->GetVersion());
		}

		memcpy(&ScalingType, pos, sizeof(int));
		pos += sizeof(int);
		EffekseerPrintDebug("ScalingType %d\n", ScalingType);
		if (ScalingType == ParameterScalingType_Fixed)
		{
			memcpy(&size, pos, sizeof(int));
			pos += sizeof(int);

			if (ef->GetVersion() >= 14)
			{
				assert(size == sizeof(ParameterScalingFixed));
				memcpy(&ScalingFixed, pos, size);
				pos += size;
			}
			else
			{
				memcpy(&ScalingFixed.Position, pos, size);
				pos += size;
			}

			// make invalid
			if (ScalingFixed.RefEq < 0 && ScalingFixed.Position.X == 1.0f && ScalingFixed.Position.Y == 1.0f &&
				ScalingFixed.Position.Z == 1.0f)
			{
				ScalingType = ParameterScalingType_None;
				EffekseerPrintDebug("ScalingType Change None\n");
			}
		}
		else if (ScalingType == ParameterScalingType_PVA)
		{
			memcpy(&size, pos, sizeof(int));
			pos += sizeof(int);
			if (ef->GetVersion() >= 14)
			{
				assert(size == sizeof(ParameterScalingPVA));
				memcpy(&ScalingPVA, pos, size);
			}
			else
			{
				memcpy(&ScalingPVA.Position, pos, size);
			}
			pos += size;
		}
		else if (ScalingType == ParameterScalingType_Easing)
		{
			memcpy(&size, pos, sizeof(int));
			pos += sizeof(int);
			if (ef->GetVersion() >= 14)
			{
				assert(size == sizeof(ParameterScalingEasing));
				memcpy(&ScalingEasing, pos, size);
			}
			else
			{
				memcpy(&ScalingEasing.Position, pos, size);
			}
			pos += size;
		}
		else if (ScalingType == ParameterScalingType_SinglePVA)
		{
			memcpy(&size, pos, sizeof(int));
			pos += sizeof(int);
			assert(size == sizeof(ParameterScalingSinglePVA));
			memcpy(&ScalingSinglePVA, pos, size);
			pos += size;
		}
		else if (ScalingType == ParameterScalingType_SingleEasing)
		{
			memcpy(&size, pos, sizeof(int));
			pos += sizeof(int);
			assert(size == sizeof(easing_float));
			memcpy(&ScalingSingleEasing, pos, size);
			pos += size;
		}
		else if (ScalingType == ParameterScalingType_FCurve)
		{
			memcpy(&size, pos, sizeof(int));
			pos += sizeof(int);

			ScalingFCurve = new FCurveVector3D();
			pos += ScalingFCurve->Load(pos, m_effect->GetVersion());
			ScalingFCurve->X.SetDefaultValue(1.0f);
			ScalingFCurve->Y.SetDefaultValue(1.0f);
			ScalingFCurve->Z.SetDefaultValue(1.0f);
		}

		/* Spawning Method */
		GenerationLocation.load(pos, m_effect->GetVersion());

		/* Spawning Method 拡大処理*/
		if (ef->IsDyanamicMagnificationValid()
			/* && (this->CommonValues.ScalingBindType == BindType::NotBind || parent->GetType() == EFFECT_NODE_TYPE_ROOT)*/)
		{
			if (GenerationLocation.type == ParameterGenerationLocation::TYPE_POINT)
			{
				GenerationLocation.point.location.min *= m_effect->GetMaginification();
				GenerationLocation.point.location.max *= m_effect->GetMaginification();
			}
			else if (GenerationLocation.type == ParameterGenerationLocation::TYPE_LINE)
			{
				GenerationLocation.line.position_end.min *= m_effect->GetMaginification();
				GenerationLocation.line.position_end.max *= m_effect->GetMaginification();
				GenerationLocation.line.position_start.min *= m_effect->GetMaginification();
				GenerationLocation.line.position_start.max *= m_effect->GetMaginification();
				GenerationLocation.line.position_noize.min *= m_effect->GetMaginification();
				GenerationLocation.line.position_noize.max *= m_effect->GetMaginification();
			}
			else if (GenerationLocation.type == ParameterGenerationLocation::TYPE_SPHERE)
			{
				GenerationLocation.sphere.radius.min *= m_effect->GetMaginification();
				GenerationLocation.sphere.radius.max *= m_effect->GetMaginification();
			}
			else if (GenerationLocation.type == ParameterGenerationLocation::TYPE_CIRCLE)
			{
				GenerationLocation.circle.radius.min *= m_effect->GetMaginification();
				GenerationLocation.circle.radius.max *= m_effect->GetMaginification();
			}
		}

		// Load depth values
		if (m_effect->GetVersion() >= 12)
		{
			memcpy(&DepthValues.DepthOffset, pos, sizeof(float));
			pos += sizeof(float);

			auto IsDepthOffsetScaledWithCamera = 0;
			memcpy(&IsDepthOffsetScaledWithCamera, pos, sizeof(int32_t));
			pos += sizeof(int32_t);

			DepthValues.IsDepthOffsetScaledWithCamera = IsDepthOffsetScaledWithCamera > 0;

			auto IsDepthOffsetScaledWithParticleScale = 0;
			memcpy(&IsDepthOffsetScaledWithParticleScale, pos, sizeof(int32_t));
			pos += sizeof(int32_t);

			DepthValues.IsDepthOffsetScaledWithParticleScale = IsDepthOffsetScaledWithParticleScale > 0;

			if (m_effect->GetVersion() >= 15)
			{
				memcpy(&DepthValues.DepthParameter.SuppressionOfScalingByDepth, pos, sizeof(float));
				pos += sizeof(float);

				memcpy(&DepthValues.DepthParameter.DepthClipping, pos, sizeof(float));
				pos += sizeof(float);
			}

			if (m_effect->GetVersion() >= 13)
			{
				memcpy(&DepthValues.ZSort, pos, sizeof(int32_t));
				pos += sizeof(int32_t);

				memcpy(&DepthValues.DrawingPriority, pos, sizeof(int32_t));
				pos += sizeof(int32_t);
			}

			memcpy(&DepthValues.SoftParticle, pos, sizeof(float));
			pos += sizeof(float);

			DepthValues.DepthOffset *= m_effect->GetMaginification();
			DepthValues.SoftParticle *= m_effect->GetMaginification();

			if (DepthValues.DepthParameter.DepthClipping < FLT_MAX / 10)
			{
				DepthValues.DepthParameter.DepthClipping *= m_effect->GetMaginification();
			}

			DepthValues.DepthParameter.DepthOffset = DepthValues.DepthOffset;
			DepthValues.DepthParameter.IsDepthOffsetScaledWithCamera = DepthValues.IsDepthOffsetScaledWithCamera;
			DepthValues.DepthParameter.IsDepthOffsetScaledWithParticleScale = DepthValues.IsDepthOffsetScaledWithParticleScale;
			DepthValues.DepthParameter.ZSort = DepthValues.ZSort;
		}

		// Convert right handle coordinate system into left handle coordinate system
		if (setting->GetCoordinateSystem() == CoordinateSystem::LH)
		{
			// Translation
			DynamicFactor.Tra[2] *= -1.0f;

			if (TranslationType == ParameterTranslationType_Fixed)
			{
				TranslationFixed.Position.Z *= -1.0f;
			}
			else if (TranslationType == ParameterTranslationType_PVA)
			{
				TranslationPVA.location.max.z *= -1.0f;
				TranslationPVA.location.min.z *= -1.0f;
				TranslationPVA.velocity.max.z *= -1.0f;
				TranslationPVA.velocity.min.z *= -1.0f;
				TranslationPVA.acceleration.max.z *= -1.0f;
				TranslationPVA.acceleration.min.z *= -1.0f;
			}
			else if (TranslationType == ParameterTranslationType_Easing)
			{
				TranslationEasing.location.start.max.z *= -1.0f;
				TranslationEasing.location.start.min.z *= -1.0f;
				TranslationEasing.location.end.max.z *= -1.0f;
				TranslationEasing.location.end.min.z *= -1.0f;
			}

			// Rotation
			DynamicFactor.Rot[0] *= -1.0f;
			DynamicFactor.Rot[1] *= -1.0f;

			if (RotationType == ParameterRotationType_Fixed)
			{
				RotationFixed.Position.X *= -1.0f;
				RotationFixed.Position.Y *= -1.0f;
			}
			else if (RotationType == ParameterRotationType_PVA)
			{
				RotationPVA.rotation.max.x *= -1.0f;
				RotationPVA.rotation.min.x *= -1.0f;
				RotationPVA.rotation.max.y *= -1.0f;
				RotationPVA.rotation.min.y *= -1.0f;
				RotationPVA.velocity.max.x *= -1.0f;
				RotationPVA.velocity.min.x *= -1.0f;
				RotationPVA.velocity.max.y *= -1.0f;
				RotationPVA.velocity.min.y *= -1.0f;
				RotationPVA.acceleration.max.x *= -1.0f;
				RotationPVA.acceleration.min.x *= -1.0f;
				RotationPVA.acceleration.max.y *= -1.0f;
				RotationPVA.acceleration.min.y *= -1.0f;
			}
			else if (RotationType == ParameterRotationType_Easing)
			{
				RotationEasing.rotation.start.max.x *= -1.0f;
				RotationEasing.rotation.start.min.x *= -1.0f;
				RotationEasing.rotation.start.max.y *= -1.0f;
				RotationEasing.rotation.start.min.y *= -1.0f;
				RotationEasing.rotation.end.max.x *= -1.0f;
				RotationEasing.rotation.end.min.x *= -1.0f;
				RotationEasing.rotation.end.max.y *= -1.0f;
				RotationEasing.rotation.end.min.y *= -1.0f;
			}
			else if (RotationType == ParameterRotationType_AxisPVA)
			{
				RotationAxisPVA.axis.max.z *= -1.0f;
				RotationAxisPVA.axis.min.z *= -1.0f;
			}
			else if (RotationType == ParameterRotationType_AxisEasing)
			{
				RotationAxisEasing.axis.max.z *= -1.0f;
				RotationAxisEasing.axis.min.z *= -1.0f;
			}
			else if (RotationType == ParameterRotationType_FCurve)
			{
				RotationFCurve->X.ChangeCoordinate();
				RotationFCurve->Y.ChangeCoordinate();
			}

			// GenerationLocation
			if (GenerationLocation.type == ParameterGenerationLocation::TYPE_POINT)
			{
			}
			else if (GenerationLocation.type == ParameterGenerationLocation::TYPE_SPHERE)
			{
				GenerationLocation.sphere.rotation_x.max *= -1.0f;
				GenerationLocation.sphere.rotation_x.min *= -1.0f;
				GenerationLocation.sphere.rotation_y.max *= -1.0f;
				GenerationLocation.sphere.rotation_y.min *= -1.0f;
			}
		}

		// generate inversed parameter
		for (size_t i = 0; i < DynamicFactor.Tra.size(); i++)
		{
			DynamicFactor.TraInv[i] = 1.0f / DynamicFactor.Tra[i];
		}

		for (size_t i = 0; i < DynamicFactor.Rot.size(); i++)
		{
			DynamicFactor.RotInv[i] = 1.0f / DynamicFactor.Rot[i];
		}

		for (size_t i = 0; i < DynamicFactor.Scale.size(); i++)
		{
			DynamicFactor.ScaleInv[i] = 1.0f / DynamicFactor.Scale[i];
		}

		if (m_effect->GetVersion() >= 3)
		{
			RendererCommon.load(pos, m_effect->GetVersion());
		}
		else
		{
			RendererCommon.reset();
		}

		LoadRendererParameter(pos, m_effect->GetSetting());

		// rescale intensity after 1.5
		RendererCommon.BasicParameter.DistortionIntensity *= m_effect->GetMaginification();
		RendererCommon.DistortionIntensity *= m_effect->GetMaginification();

		if (m_effect->GetVersion() >= 1)
		{
			// Sound
			memcpy(&SoundType, pos, sizeof(int));
			pos += sizeof(int);
			if (SoundType == ParameterSoundType_Use)
			{
				memcpy(&Sound.WaveId, pos, sizeof(int32_t));
				pos += sizeof(int32_t);
				memcpy(&Sound.Volume, pos, sizeof(random_float));
				pos += sizeof(random_float);
				memcpy(&Sound.Pitch, pos, sizeof(random_float));
				pos += sizeof(random_float);
				memcpy(&Sound.PanType, pos, sizeof(ParameterSoundPanType));
				pos += sizeof(ParameterSoundPanType);
				memcpy(&Sound.Pan, pos, sizeof(random_float));
				pos += sizeof(random_float);
				memcpy(&Sound.Distance, pos, sizeof(float));
				pos += sizeof(float);
				memcpy(&Sound.Delay, pos, sizeof(random_int));
				pos += sizeof(random_int);
			}
		}
	}

	// ノード
	int nodeCount = 0;
	memcpy(&nodeCount, pos, sizeof(int));
	pos += sizeof(int);
	EffekseerPrintDebug("ChildrenCount : %d\n", nodeCount);
	m_Nodes.resize(nodeCount);
	for (size_t i = 0; i < m_Nodes.size(); i++)
	{
		m_Nodes[i] = EffectNodeImplemented::Create(m_effect, this, pos);
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
EffectNodeImplemented::~EffectNodeImplemented()
{
	for (size_t i = 0; i < m_Nodes.size(); i++)
	{
		ES_SAFE_DELETE(m_Nodes[i]);
	}

	ES_SAFE_DELETE(TranslationFCurve);
	ES_SAFE_DELETE(RotationFCurve);
	ES_SAFE_DELETE(ScalingFCurve);
}

void EffectNodeImplemented::CalcCustomData(const Instance* instance, std::array<float, 4>& customData1, std::array<float, 4>& customData2)
{
	if (this->RendererCommon.BasicParameter.MaterialParameterPtr != nullptr)
	{
		if (this->RendererCommon.BasicParameter.MaterialParameterPtr->MaterialIndex >= 0)
		{
			auto material = m_effect->GetMaterial(this->RendererCommon.BasicParameter.MaterialParameterPtr->MaterialIndex);

			if (material != nullptr)
			{
				if (material->CustomData1 > 0)
				{
					customData1 = instance->GetCustomData(0);
				}
				if (material->CustomData2 > 0)
				{
					customData2 = instance->GetCustomData(1);
				}
			}
		}
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
Effect* EffectNodeImplemented::GetEffect() const { return m_effect; }

int EffectNodeImplemented::GetGeneration() const { return generation_; }

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
int EffectNodeImplemented::GetChildrenCount() const { return (int)m_Nodes.size(); }

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
EffectNode* EffectNodeImplemented::GetChild(int index) const
{
	if (index >= GetChildrenCount())
		return NULL;
	return m_Nodes[index];
}

EffectBasicRenderParameter EffectNodeImplemented::GetBasicRenderParameter()
{
	EffectBasicRenderParameter param;
	param.ColorTextureIndex = RendererCommon.ColorTextureIndex;
	param.AlphaBlend = RendererCommon.AlphaBlend;
	param.Distortion = RendererCommon.Distortion;
	param.DistortionIntensity = RendererCommon.DistortionIntensity;
	param.FilterType = RendererCommon.FilterType;
	param.WrapType = RendererCommon.WrapType;
	param.ZTest = RendererCommon.ZTest;
	param.ZWrite = RendererCommon.ZWrite;
	return param;
}

void EffectNodeImplemented::SetBasicRenderParameter(EffectBasicRenderParameter param)
{
	RendererCommon.ColorTextureIndex = param.ColorTextureIndex;
	RendererCommon.AlphaBlend = param.AlphaBlend;
	RendererCommon.Distortion = param.Distortion;
	RendererCommon.DistortionIntensity = param.DistortionIntensity;
	RendererCommon.FilterType = param.FilterType;
	RendererCommon.WrapType = param.WrapType;
	RendererCommon.ZTest = param.ZTest;
	RendererCommon.ZWrite = param.ZWrite;
}

EffectModelParameter EffectNodeImplemented::GetEffectModelParameter()
{
	EffectModelParameter param;
	param.Lighting = false;

	if (GetType() == EFFECT_NODE_TYPE_MODEL)
	{
		auto t = (EffectNodeModel*)this;
		param.Lighting = t->Lighting;
	}

	return param;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void EffectNodeImplemented::LoadRendererParameter(unsigned char*& pos, Setting* setting)
{
	int32_t type = 0;
	memcpy(&type, pos, sizeof(int));
	pos += sizeof(int);
	assert(type == GetType());
	EffekseerPrintDebug("Renderer : None\n");
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void EffectNodeImplemented::BeginRendering(int32_t count, Manager* manager) {}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void EffectNodeImplemented::BeginRenderingGroup(InstanceGroup* group, Manager* manager) {}

void EffectNodeImplemented::EndRenderingGroup(InstanceGroup* group, Manager* manager) {}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void EffectNodeImplemented::Rendering(const Instance& instance, const Instance* next_instance, Manager* manager) {}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void EffectNodeImplemented::EndRendering(Manager* manager) {}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void EffectNodeImplemented::InitializeRenderedInstanceGroup(InstanceGroup& instanceGroup, Manager* manager) {}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void EffectNodeImplemented::InitializeRenderedInstance(Instance& instance, Manager* manager) {}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void EffectNodeImplemented::UpdateRenderedInstance(Instance& instance, Manager* manager) {}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
float EffectNodeImplemented::GetFadeAlpha(const Instance& instance)
{
	float alpha = 1.0f;

	if (RendererCommon.FadeInType == ParameterRendererCommon::FADEIN_ON && instance.m_LivingTime < RendererCommon.FadeIn.Frame)
	{
		float v = 1.0f;
		RendererCommon.FadeIn.Value.setValueToArg(v, 0.0f, 1.0f, (float)instance.m_LivingTime / (float)RendererCommon.FadeIn.Frame);

		alpha *= v;
	}

	if (RendererCommon.FadeOutType == ParameterRendererCommon::FADEOUT_ON &&
		instance.m_LivingTime + RendererCommon.FadeOut.Frame > instance.m_LivedTime)
	{
		float v = 1.0f;
		RendererCommon.FadeOut.Value.setValueToArg(v,
												   1.0f,
												   0.0f,
												   (float)(instance.m_LivingTime + RendererCommon.FadeOut.Frame - instance.m_LivedTime) /
													   (float)RendererCommon.FadeOut.Frame);

		alpha *= v;
	}

	return alpha;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void EffectNodeImplemented::PlaySound_(Instance& instance, SoundTag tag, Manager* manager)
{
	auto instanceGlobal = instance.m_pContainer->GetRootInstance();

	SoundPlayer* player = manager->GetSoundPlayer();
	if (player == NULL)
	{
		return;
	}

	if (Sound.WaveId >= 0)
	{
		SoundPlayer::InstanceParameter parameter;
		parameter.Data = m_effect->GetWave(Sound.WaveId);
		parameter.Volume = Sound.Volume.getValue(*instanceGlobal);
		parameter.Pitch = Sound.Pitch.getValue(*instanceGlobal);
		parameter.Pan = Sound.Pan.getValue(*instanceGlobal);

		parameter.Mode3D = (Sound.PanType == ParameterSoundPanType_3D);
		Vector3D::Transform(parameter.Position, Vector3D(0.0f, 0.0f, 0.0f), instance.GetGlobalMatrix43());
		parameter.Distance = Sound.Distance;

		player->Play(tag, parameter);
	}
}

EffectInstanceTerm EffectNodeImplemented::CalculateInstanceTerm(EffectInstanceTerm& parentTerm) const
{
	EffectInstanceTerm ret;

	auto addWithClip = [](int v1, int v2) -> int {
		v1 = Max(v1, 0);
		v2 = Max(v2, 0);

		if (v1 >= INT_MAX / 2)
			return INT_MAX;

		if (v2 >= INT_MAX / 2)
			return INT_MAX;

		return v1 + v2;
	};

	int lifeMin = CommonValues.life.min;
	int lifeMax = CommonValues.life.max;

	if (CommonValues.RemoveWhenLifeIsExtinct <= 0)
	{
		lifeMin = INT_MAX;
		lifeMax = INT_MAX;
	}

	auto firstBeginMin = CommonValues.GenerationTimeOffset.min;
	auto firstBeginMax = CommonValues.GenerationTimeOffset.max;
	auto firstEndMin = addWithClip(firstBeginMin, lifeMin);
	auto firstEndMax = addWithClip(firstBeginMax, lifeMax);

	auto lastBeginMin = 0;
	auto lastBeginMax = 0;
	if (CommonValues.MaxGeneration > INT_MAX / 2)
	{
		lastBeginMin = INT_MAX / 2;
	}
	else
	{
		lastBeginMin = CommonValues.GenerationTimeOffset.min + (CommonValues.MaxGeneration - 1) * (CommonValues.GenerationTime.min);
	}

	if (CommonValues.MaxGeneration > INT_MAX / 2)
	{
		lastBeginMax = INT_MAX / 2;
	}
	else
	{
		lastBeginMax = CommonValues.GenerationTimeOffset.max + (CommonValues.MaxGeneration - 1) * (CommonValues.GenerationTime.max);
	}

	auto lastEndMin = addWithClip(lastBeginMin, lifeMin);
	auto lastEndMax = addWithClip(lastBeginMax, lifeMax);

	auto parentFirstTermMin = parentTerm.FirstInstanceEndMin - parentTerm.FirstInstanceStartMin;
	auto parentFirstTermMax = parentTerm.FirstInstanceEndMax - parentTerm.FirstInstanceStartMax;
	auto parentLastTermMin = parentTerm.LastInstanceEndMin - parentTerm.LastInstanceStartMin;
	auto parentLastTermMax = parentTerm.LastInstanceEndMax - parentTerm.LastInstanceStartMax;

	if (CommonValues.RemoveWhenParentIsRemoved > 0)
	{
		if (firstEndMin - firstBeginMin > parentFirstTermMin)
			firstEndMin = firstBeginMin + parentFirstTermMin;

		if (firstEndMax - firstBeginMax > parentFirstTermMax)
			firstEndMax = firstBeginMax + parentFirstTermMax;

		if (lastEndMin > INT_MAX / 2)
		{
			lastBeginMin = parentLastTermMin;
			lastEndMin = parentLastTermMin;
		}
		else if (lastEndMin - lastBeginMin > parentLastTermMin)
		{
			lastEndMin = lastBeginMin + parentLastTermMin;
		}

		if (lastEndMax > INT_MAX / 2)
		{
			lastBeginMax = parentLastTermMax;
			lastEndMax = parentLastTermMax;
		}
		else if (lastEndMax - lastBeginMax > parentLastTermMax)
		{
			lastEndMax = lastBeginMax + parentLastTermMax;
		}
	}

	ret.FirstInstanceStartMin = addWithClip(parentTerm.FirstInstanceStartMin, firstBeginMin);
	ret.FirstInstanceStartMax = addWithClip(parentTerm.FirstInstanceStartMax, firstBeginMax);
	ret.FirstInstanceEndMin = addWithClip(parentTerm.FirstInstanceStartMin, firstEndMin);
	ret.FirstInstanceEndMax = addWithClip(parentTerm.FirstInstanceStartMax, firstEndMax);

	ret.LastInstanceStartMin = addWithClip(parentTerm.LastInstanceStartMin, lastBeginMin);
	ret.LastInstanceStartMax = addWithClip(parentTerm.LastInstanceStartMax, lastBeginMax);
	ret.LastInstanceEndMin = addWithClip(parentTerm.LastInstanceStartMin, lastEndMin);
	ret.LastInstanceEndMax = addWithClip(parentTerm.LastInstanceStartMax, lastEndMax);

	// check children
	if (CommonValues.RemoveWhenChildrenIsExtinct > 0)
	{
		int childFirstEndMin = 0;
		int childFirstEndMax = 0;
		int childLastEndMin = 0;
		int childLastEndMax = 0;

		for (int32_t i = 0; i < GetChildrenCount(); i++)
		{
			auto child = static_cast<EffectNodeImplemented*>(GetChild(i));
			auto childTerm = child->CalculateInstanceTerm(ret);
			childFirstEndMin = Max(childTerm.FirstInstanceEndMin, childFirstEndMin);
			childFirstEndMax = Max(childTerm.FirstInstanceEndMax, childFirstEndMax);
			childLastEndMin = Max(childTerm.LastInstanceEndMin, childLastEndMin);
			childLastEndMax = Max(childTerm.LastInstanceEndMax, childLastEndMax);
		}

		ret.FirstInstanceEndMin = Min(ret.FirstInstanceEndMin, childFirstEndMin);
		ret.FirstInstanceEndMax = Min(ret.FirstInstanceEndMax, childFirstEndMax);
		ret.LastInstanceEndMin = Min(ret.LastInstanceEndMin, childLastEndMin);
		ret.LastInstanceEndMax = Min(ret.LastInstanceEndMax, childLastEndMax);
	}

	return ret;
}

EffectNodeImplemented* EffectNodeImplemented::Create(Effect* effect, EffectNode* parent, unsigned char*& pos)
{
	EffectNodeImplemented* effectnode = NULL;

	int node_type = 0;
	memcpy(&node_type, pos, sizeof(int));

	if (node_type == EFFECT_NODE_TYPE_ROOT)
	{
		EffekseerPrintDebug("* Create : EffectNodeRoot\n");
		effectnode = new EffectNodeRoot(effect, pos);
	}
	else if (node_type == EFFECT_NODE_TYPE_NONE)
	{
		EffekseerPrintDebug("* Create : EffectNodeNone\n");
		effectnode = new EffectNodeImplemented(effect, pos);
	}
	else if (node_type == EFFECT_NODE_TYPE_SPRITE)
	{
		EffekseerPrintDebug("* Create : EffectNodeSprite\n");
		effectnode = new EffectNodeSprite(effect, pos);
	}
	else if (node_type == EFFECT_NODE_TYPE_RIBBON)
	{
		EffekseerPrintDebug("* Create : EffectNodeRibbon\n");
		effectnode = new EffectNodeRibbon(effect, pos);
	}
	else if (node_type == EFFECT_NODE_TYPE_RING)
	{
		EffekseerPrintDebug("* Create : EffectNodeRing\n");
		effectnode = new EffectNodeRing(effect, pos);
	}
	else if (node_type == EFFECT_NODE_TYPE_MODEL)
	{
		EffekseerPrintDebug("* Create : EffectNodeModel\n");
		effectnode = new EffectNodeModel(effect, pos);
	}
	else if (node_type == EFFECT_NODE_TYPE_TRACK)
	{
		EffekseerPrintDebug("* Create : EffectNodeTrack\n");
		effectnode = new EffectNodeTrack(effect, pos);
	}
	else
	{
		assert(0);
	}

	effectnode->LoadParameter(pos, parent, effect->GetSetting());

	return effectnode;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------

} // namespace Effekseer

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------


//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------





//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
namespace Effekseer
{
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
	void EffectNodeModel::LoadRendererParameter(unsigned char*& pos, Setting* setting)
{
	int32_t type = 0;
	memcpy( &type, pos, sizeof(int) );
	pos += sizeof(int);
	assert( type == GetType() );
	EffekseerPrintDebug("Renderer : Model\n");

	AlphaBlend = RendererCommon.AlphaBlend;

	if( m_effect->GetVersion() >= 7 )
	{
		float Magnification;
		memcpy( &Magnification, pos, sizeof(float) );
		pos += sizeof(float);
	}

	memcpy( &ModelIndex, pos, sizeof(int) );
	pos += sizeof(int);

	if (m_effect->GetVersion() < 15)
	{
		memcpy(&NormalTextureIndex, pos, sizeof(int));
		pos += sizeof(int);
		EffekseerPrintDebug("NormalTextureIndex : %d\n", NormalTextureIndex);
		RendererCommon.Texture2Index = NormalTextureIndex;
		RendererCommon.BasicParameter.Texture2Index = NormalTextureIndex;
		RendererCommon.BasicParameter.TextureFilter2 = RendererCommon.BasicParameter.TextureFilter1;
		RendererCommon.BasicParameter.TextureWrap2 = RendererCommon.BasicParameter.TextureWrap1;
	}

	if (m_effect->GetVersion() >= 12)
	{
		memcpy(&Billboard, pos, sizeof(int));
		pos += sizeof(int);
	}
	else
	{
		Billboard = BillboardType::Fixed;
	}

	if (m_effect->GetVersion() < 15)
	{
		int32_t lighting;
		memcpy(&lighting, pos, sizeof(int));
		pos += sizeof(int);
		Lighting = lighting > 0;

		if (Lighting && !RendererCommon.Distortion)
		{
			RendererCommon.MaterialType = RendererMaterialType::Lighting;
			RendererCommon.BasicParameter.MaterialType = RendererMaterialType::Lighting;
		}
	}

	memcpy( &Culling, pos, sizeof(int) );
	pos += sizeof(int);

	AllColor.load( pos, m_effect->GetVersion() );
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void EffectNodeModel::BeginRendering(int32_t count, Manager* manager)
{
	ModelRenderer* renderer = manager->GetModelRenderer();
	if (renderer != NULL)
	{
		ModelRenderer::NodeParameter nodeParameter;
		// nodeParameter.TextureFilter = RendererCommon.FilterType;
		// nodeParameter.TextureWrap = RendererCommon.WrapType;
		nodeParameter.ZTest = RendererCommon.ZTest;
		nodeParameter.ZWrite = RendererCommon.ZWrite;
		nodeParameter.EffectPointer = GetEffect();
		nodeParameter.ModelIndex = ModelIndex;
		nodeParameter.Culling = Culling;
		nodeParameter.Billboard = Billboard;
		nodeParameter.Magnification = m_effect->GetMaginification();
		nodeParameter.IsRightHand = manager->GetCoordinateSystem() ==
			CoordinateSystem::RH;

		nodeParameter.DepthParameterPtr = &DepthValues.DepthParameter;
		//nodeParameter.DepthOffset = DepthValues.DepthOffset;
		//nodeParameter.IsDepthOffsetScaledWithCamera = DepthValues.IsDepthOffsetScaledWithCamera;
		//nodeParameter.IsDepthOffsetScaledWithParticleScale = DepthValues.IsDepthOffsetScaledWithParticleScale;

		nodeParameter.BasicParameterPtr = &RendererCommon.BasicParameter;
		renderer->BeginRendering(nodeParameter, count, m_userData);
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void EffectNodeModel::Rendering(const Instance& instance, const Instance* next_instance, Manager* manager)
{
	const InstanceValues& instValues = instance.rendererValues.model;
	ModelRenderer* renderer = manager->GetModelRenderer();
	if( renderer != NULL )
	{
		ModelRenderer::NodeParameter nodeParameter;
		//nodeParameter.TextureFilter = RendererCommon.FilterType;
		//nodeParameter.TextureWrap = RendererCommon.WrapType;
		nodeParameter.ZTest = RendererCommon.ZTest;
		nodeParameter.ZWrite = RendererCommon.ZWrite;
		nodeParameter.EffectPointer = GetEffect();
		nodeParameter.ModelIndex = ModelIndex;
		nodeParameter.Culling = Culling;
		nodeParameter.Billboard = Billboard;
		nodeParameter.Magnification = m_effect->GetMaginification();
		nodeParameter.IsRightHand = manager->GetCoordinateSystem() ==
			CoordinateSystem::RH;

		nodeParameter.DepthParameterPtr = &DepthValues.DepthParameter;
		//nodeParameter.DepthOffset = DepthValues.DepthOffset;
		//nodeParameter.IsDepthOffsetScaledWithCamera = DepthValues.IsDepthOffsetScaledWithCamera;
		//nodeParameter.IsDepthOffsetScaledWithParticleScale = DepthValues.IsDepthOffsetScaledWithParticleScale;
		nodeParameter.BasicParameterPtr = &RendererCommon.BasicParameter;

		ModelRenderer::InstanceParameter instanceParameter;
		instanceParameter.SRTMatrix43 = instance.GetGlobalMatrix43();
		instanceParameter.Time = (int32_t)instance.m_LivingTime;

		instanceParameter.UV = instance.GetUV();
		CalcCustomData(&instance, instanceParameter.CustomData1, instanceParameter.CustomData2);

		Color _color;
		if (RendererCommon.ColorBindType == BindType::Always || RendererCommon.ColorBindType == BindType::WhenCreating)
		{
			_color = Color::Mul(instValues._original, instance.ColorParent);
		}
		else
		{
			_color = instValues._original;
		}
		instanceParameter.AllColor = _color;
		
		if (instance.m_pContainer->GetRootInstance()->IsGlobalColorSet)
		{
			instanceParameter.AllColor = Color::Mul(instanceParameter.AllColor, instance.m_pContainer->GetRootInstance()->GlobalColor);
		}

		nodeParameter.BasicParameterPtr = &RendererCommon.BasicParameter;
		renderer->Rendering( nodeParameter, instanceParameter, m_userData );
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void EffectNodeModel::EndRendering(Manager* manager)
{
	ModelRenderer* renderer = manager->GetModelRenderer();
	if( renderer != NULL )
	{
		ModelRenderer::NodeParameter nodeParameter;
		//nodeParameter.TextureFilter = RendererCommon.FilterType;
		//nodeParameter.TextureWrap = RendererCommon.WrapType;
		nodeParameter.ZTest = RendererCommon.ZTest;
		nodeParameter.ZWrite = RendererCommon.ZWrite;
		nodeParameter.EffectPointer = GetEffect();
		nodeParameter.ModelIndex = ModelIndex;
		nodeParameter.Culling = Culling;
		nodeParameter.Billboard = Billboard;
		nodeParameter.Magnification = m_effect->GetMaginification();
		nodeParameter.IsRightHand = manager->GetSetting()->GetCoordinateSystem() ==
			CoordinateSystem::RH;

		nodeParameter.DepthParameterPtr = &DepthValues.DepthParameter;
		//nodeParameter.DepthOffset = DepthValues.DepthOffset;
		//nodeParameter.IsDepthOffsetScaledWithCamera = DepthValues.IsDepthOffsetScaledWithCamera;
		//nodeParameter.IsDepthOffsetScaledWithParticleScale = DepthValues.IsDepthOffsetScaledWithParticleScale;

		nodeParameter.BasicParameterPtr = &RendererCommon.BasicParameter;
		renderer->EndRendering( nodeParameter, m_userData );
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void EffectNodeModel::InitializeRenderedInstance(Instance& instance, Manager* manager)
{
	InstanceValues& instValues = instance.rendererValues.model;
	auto instanceGlobal = instance.m_pContainer->GetRootInstance();

	if( AllColor.type == StandardColorParameter::Fixed )
	{
		instValues._original = AllColor.fixed.all;
		instValues.allColorValues.fixed._color = instValues._original;
	}
	else if( AllColor.type == StandardColorParameter::Random )
	{
		instValues._original = AllColor.random.all.getValue(*instanceGlobal);
		instValues.allColorValues.random._color = instValues._original;
	}
	else if( AllColor.type == StandardColorParameter::Easing )
	{
		instValues.allColorValues.easing.start = AllColor.easing.all.getStartValue(*instanceGlobal);
		instValues.allColorValues.easing.end = AllColor.easing.all.getEndValue(*instanceGlobal);

		float t = instance.m_LivingTime / instance.m_LivedTime;

		AllColor.easing.all.setValueToArg(
			instValues._original,
			instValues.allColorValues.easing.start,
			instValues.allColorValues.easing.end,
			t );
	}
	else if( AllColor.type == StandardColorParameter::FCurve_RGBA )
	{
		instValues.allColorValues.fcurve_rgba.offset = AllColor.fcurve_rgba.FCurve->GetOffsets(*instanceGlobal);
		auto fcurveColors = AllColor.fcurve_rgba.FCurve->GetValues(instance.m_LivingTime, instance.m_LivedTime);
		instValues._original.R = (uint8_t)Clamp( (instValues.allColorValues.fcurve_rgba.offset[0] + fcurveColors[0]), 255, 0);
		instValues._original.G = (uint8_t)Clamp( (instValues.allColorValues.fcurve_rgba.offset[1] + fcurveColors[1]), 255, 0);
		instValues._original.B = (uint8_t)Clamp( (instValues.allColorValues.fcurve_rgba.offset[2] + fcurveColors[2]), 255, 0);
		instValues._original.A = (uint8_t)Clamp( (instValues.allColorValues.fcurve_rgba.offset[3] + fcurveColors[3]), 255, 0);
	}

	if (RendererCommon.ColorBindType == BindType::Always || RendererCommon.ColorBindType == BindType::WhenCreating)
	{
		instValues._color = Color::Mul(instValues._original, instance.ColorParent);
	}
	else
	{
		instValues._color = instValues._original;
	}

	instance.ColorInheritance = instValues._color;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void EffectNodeModel::UpdateRenderedInstance(Instance& instance, Manager* manager)
{
	InstanceValues& instValues = instance.rendererValues.model;

	if (AllColor.type == StandardColorParameter::Fixed)
	{
		instValues._original = instValues.allColorValues.fixed._color;
	}
	else if (AllColor.type == StandardColorParameter::Random)
	{
		instValues._original = instValues.allColorValues.random._color;
	}
	else if( AllColor.type == StandardColorParameter::Easing )
	{
		float t = instance.m_LivingTime / instance.m_LivedTime;

		AllColor.easing.all.setValueToArg(
			instValues._original,
			instValues.allColorValues.easing.start,
			instValues.allColorValues.easing.end,
			t );
	}
	else if( AllColor.type == StandardColorParameter::FCurve_RGBA )
	{
		auto fcurveColors = AllColor.fcurve_rgba.FCurve->GetValues(instance.m_LivingTime, instance.m_LivedTime);
		instValues._original.R = (uint8_t)Clamp( (instValues.allColorValues.fcurve_rgba.offset[0] + fcurveColors[0]), 255, 0);
		instValues._original.G = (uint8_t)Clamp( (instValues.allColorValues.fcurve_rgba.offset[1] + fcurveColors[1]), 255, 0);
		instValues._original.B = (uint8_t)Clamp( (instValues.allColorValues.fcurve_rgba.offset[2] + fcurveColors[2]), 255, 0);
		instValues._original.A = (uint8_t)Clamp( (instValues.allColorValues.fcurve_rgba.offset[3] + fcurveColors[3]), 255, 0);
	}

	float fadeAlpha = GetFadeAlpha(instance);
	if (fadeAlpha != 1.0f)
	{
		instValues._original.A = (uint8_t)(instValues._original.A * fadeAlpha);
	}

	if (RendererCommon.ColorBindType == BindType::Always || RendererCommon.ColorBindType == BindType::WhenCreating)
	{
		instValues._color = Color::Mul(instValues._original, instance.ColorParent);
	}
	else
	{
		instValues._color = instValues._original;
	}

	instance.ColorInheritance = instValues._color;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------



//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------




//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
namespace Effekseer
{
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void EffectNodeRibbon::LoadRendererParameter(unsigned char*& pos, Setting* setting)
{
	int32_t type = 0;
	memcpy(&type, pos, sizeof(int));
	pos += sizeof(int);
	assert(type == GetType());
	EffekseerPrintDebug("Renderer : Ribbon\n");

	if (m_effect->GetVersion() >= 15)
	{
		TextureUVType.Load(pos, m_effect->GetVersion());
	}

	if (m_effect->GetVersion() >= 3)
	{
		AlphaBlend = RendererCommon.AlphaBlend;
	}
	else
	{
		memcpy(&AlphaBlend, pos, sizeof(int));
		pos += sizeof(int);
	}

	memcpy(&ViewpointDependent, pos, sizeof(int));
	pos += sizeof(int);

	memcpy(&RibbonAllColor.type, pos, sizeof(int));
	pos += sizeof(int);
	EffekseerPrintDebug("RibbonColorAllType : %d\n", RibbonAllColor.type);

	if (RibbonAllColor.type == RibbonAllColorParameter::Fixed)
	{
		memcpy(&RibbonAllColor.fixed, pos, sizeof(RibbonAllColor.fixed));
		pos += sizeof(RibbonAllColor.fixed);
	}
	else if (RibbonAllColor.type == RibbonAllColorParameter::Random)
	{
		RibbonAllColor.random.all.load(m_effect->GetVersion(), pos);
	}
	else if (RibbonAllColor.type == RibbonAllColorParameter::Easing)
	{
		RibbonAllColor.easing.all.load(m_effect->GetVersion(), pos);
	}

	memcpy(&RibbonColor.type, pos, sizeof(int));
	pos += sizeof(int);
	EffekseerPrintDebug("RibbonColorType : %d\n", RibbonColor.type);

	if (RibbonColor.type == RibbonColor.Default)
	{
	}
	else if (RibbonColor.type == RibbonColor.Fixed)
	{
		memcpy(&RibbonColor.fixed, pos, sizeof(RibbonColor.fixed));
		pos += sizeof(RibbonColor.fixed);
	}

	memcpy(&RibbonPosition.type, pos, sizeof(int));
	pos += sizeof(int);
	EffekseerPrintDebug("RibbonPosition : %d\n", RibbonPosition.type);

	if (RibbonPosition.type == RibbonPosition.Default)
	{
		if (m_effect->GetVersion() >= 8)
		{
			memcpy(&RibbonPosition.fixed, pos, sizeof(RibbonPosition.fixed));
			pos += sizeof(RibbonPosition.fixed);
			RibbonPosition.type = RibbonPosition.Fixed;
		}
	}
	else if (RibbonPosition.type == RibbonPosition.Fixed)
	{
		memcpy(&RibbonPosition.fixed, pos, sizeof(RibbonPosition.fixed));
		pos += sizeof(RibbonPosition.fixed);
	}

	if (m_effect->GetVersion() >= 13)
	{
		memcpy(&SplineDivision, pos, sizeof(int32_t));
		pos += sizeof(int32_t);
	}

	if (m_effect->GetVersion() >= 3)
	{
		RibbonTexture = RendererCommon.ColorTextureIndex;
	}
	else
	{
		memcpy(&RibbonTexture, pos, sizeof(int));
		pos += sizeof(int);
	}

	// 右手系左手系変換
	if (setting->GetCoordinateSystem() == CoordinateSystem::LH)
	{
	}

	/* 位置拡大処理 */
	if (m_effect->GetVersion() >= 8)
	{
		if (RibbonPosition.type == RibbonPosition.Default)
		{
		}
		else if (RibbonPosition.type == RibbonPosition.Fixed)
		{
			RibbonPosition.fixed.l *= m_effect->GetMaginification();
			RibbonPosition.fixed.r *= m_effect->GetMaginification();
		}
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void EffectNodeRibbon::BeginRendering(int32_t count, Manager* manager)
{
	RibbonRenderer* renderer = manager->GetRibbonRenderer();
	if (renderer != NULL)
	{
		//m_nodeParameter.TextureFilter = RendererCommon.FilterType;
		//m_nodeParameter.TextureWrap = RendererCommon.WrapType;
		m_nodeParameter.ZTest = RendererCommon.ZTest;
		m_nodeParameter.ZWrite = RendererCommon.ZWrite;
		m_nodeParameter.ViewpointDependent = ViewpointDependent != 0;
		m_nodeParameter.EffectPointer = GetEffect();

		m_nodeParameter.SplineDivision = SplineDivision;
		m_nodeParameter.DepthParameterPtr = &DepthValues.DepthParameter;
		m_nodeParameter.BasicParameterPtr = &RendererCommon.BasicParameter;
		m_nodeParameter.TextureUVTypeParameterPtr = &TextureUVType;
		m_nodeParameter.IsRightHand = manager->GetCoordinateSystem() == CoordinateSystem::RH;
		renderer->BeginRendering(m_nodeParameter, count, m_userData);
	}
}

void EffectNodeRibbon::BeginRenderingGroup(InstanceGroup* group, Manager* manager)
{
	RibbonRenderer* renderer = manager->GetRibbonRenderer();
	if (renderer != NULL)
	{
		m_instanceParameter.InstanceCount = group->GetInstanceCount();
		m_instanceParameter.InstanceIndex = 0;

		if (group->GetFirst() != nullptr)
		{
			m_instanceParameter.UV = group->GetFirst()->GetUV();
			CalcCustomData(group->GetFirst(), m_instanceParameter.CustomData1, m_instanceParameter.CustomData2);
		}

		renderer->BeginRenderingGroup(m_nodeParameter, m_instanceParameter.InstanceCount, m_userData);
	}
}

void EffectNodeRibbon::EndRenderingGroup(InstanceGroup* group, Manager* manager)
{
	RibbonRenderer* renderer = manager->GetRibbonRenderer();
	if (renderer != NULL)
	{
		renderer->EndRenderingGroup(m_nodeParameter, m_instanceParameter.InstanceCount, m_userData);
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void EffectNodeRibbon::Rendering(const Instance& instance, const Instance* next_instance, Manager* manager)
{
	const InstanceValues& instValues = instance.rendererValues.ribbon;
	RibbonRenderer* renderer = manager->GetRibbonRenderer();
	if (renderer != NULL)
	{
		Color _color;
		if (RendererCommon.ColorBindType == BindType::Always || RendererCommon.ColorBindType == BindType::WhenCreating)
		{
			_color = Color::Mul(instValues._original, instance.ColorParent);
		}
		else
		{
			_color = instValues._original;
		}

		m_instanceParameter.AllColor = _color;
		m_instanceParameter.SRTMatrix43 = instance.GetGlobalMatrix43();

		Color color_l = _color;
		Color color_r = _color;
		Color color_nl = _color;
		Color color_nr = _color;

		if (next_instance != nullptr)
		{
			const InstanceValues& instValues_next = next_instance->rendererValues.ribbon;
			Color _color_next;
			if (RendererCommon.ColorBindType == BindType::Always || RendererCommon.ColorBindType == BindType::WhenCreating)
			{
				_color_next = Color::Mul(instValues_next._original, next_instance->ColorParent);
			}
			else
			{
				_color_next = instValues_next._original;
			}

			color_nl = _color_next;
			color_nr = _color_next;
		}

		if (RibbonColor.type == RibbonColorParameter::Default)
		{

		}
		else if (RibbonColor.type == RibbonColorParameter::Fixed)
		{
			color_l = Color::Mul(color_l, RibbonColor.fixed.l);
			color_r = Color::Mul(color_r, RibbonColor.fixed.r);
			color_nl = Color::Mul(color_nl, RibbonColor.fixed.l);
			color_nr = Color::Mul(color_nr, RibbonColor.fixed.r);
		}

		m_instanceParameter.Colors[0] = color_l;
		m_instanceParameter.Colors[1] = color_r;
		m_instanceParameter.Colors[2] = color_nl;
		m_instanceParameter.Colors[3] = color_nr;

		// Apply global Color
		if (instance.m_pContainer->GetRootInstance()->IsGlobalColorSet)
		{
			m_instanceParameter.Colors[0] = Color::Mul(m_instanceParameter.Colors[0], instance.m_pContainer->GetRootInstance()->GlobalColor);
			m_instanceParameter.Colors[1] = Color::Mul(m_instanceParameter.Colors[1], instance.m_pContainer->GetRootInstance()->GlobalColor);
		}

		if (RibbonPosition.type == RibbonPositionParameter::Default)
		{
			m_instanceParameter.Positions[0] = -0.5f;
			m_instanceParameter.Positions[1] = 0.5f;
		}
		else if (RibbonPosition.type == RibbonPositionParameter::Fixed)
		{
			m_instanceParameter.Positions[0] = RibbonPosition.fixed.l;
			m_instanceParameter.Positions[1] = RibbonPosition.fixed.r;
		}

		renderer->Rendering(m_nodeParameter, m_instanceParameter, m_userData);

		m_instanceParameter.InstanceIndex++;
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void EffectNodeRibbon::EndRendering(Manager* manager)
{
	RibbonRenderer* renderer = manager->GetRibbonRenderer();
	if (renderer != NULL)
	{
		renderer->EndRendering(m_nodeParameter, m_userData);
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void EffectNodeRibbon::InitializeRenderedInstance(Instance& instance, Manager* manager)
{
	InstanceValues& instValues = instance.rendererValues.ribbon;
	auto instanceGlobal = instance.m_pContainer->GetRootInstance();

	if (RibbonAllColor.type == RibbonAllColorParameter::Fixed)
	{
		instValues._original = RibbonAllColor.fixed.all;
		instValues.allColorValues.fixed._color = instValues._original;
	}
	else if (RibbonAllColor.type == RibbonAllColorParameter::Random)
	{
		instValues._original = RibbonAllColor.random.all.getValue(*instanceGlobal);
		instValues.allColorValues.random._color = instValues._original;
	}
	else if (RibbonAllColor.type == RibbonAllColorParameter::Easing)
	{
		instValues.allColorValues.easing.start = RibbonAllColor.easing.all.getStartValue(*instanceGlobal);
		instValues.allColorValues.easing.end = RibbonAllColor.easing.all.getEndValue(*instanceGlobal);
	}

	if (RendererCommon.ColorBindType == BindType::Always || RendererCommon.ColorBindType == BindType::WhenCreating)
	{
		instValues._color = Color::Mul(instValues._original, instance.ColorParent);
	}
	else
	{
		instValues._color = instValues._original;
	}

	instance.ColorInheritance = instValues._color;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void EffectNodeRibbon::UpdateRenderedInstance(Instance& instance, Manager* manager)
{
	InstanceValues& instValues = instance.rendererValues.ribbon;

	if (RibbonAllColor.type == RibbonAllColorParameter::Fixed)
	{
		instValues._original = instValues.allColorValues.fixed._color;
	}
	else if (RibbonAllColor.type == RibbonAllColorParameter::Random)
	{
		instValues._original = instValues.allColorValues.random._color;
	}
	else if (RibbonAllColor.type == RibbonAllColorParameter::Easing)
	{
		float t = instance.m_LivingTime / instance.m_LivedTime;

		RibbonAllColor.easing.all.setValueToArg(
			instValues._original,
			instValues.allColorValues.easing.start,
			instValues.allColorValues.easing.end,
			t);
	}

	float fadeAlpha = GetFadeAlpha(instance);
	if (fadeAlpha != 1.0f)
	{
		instValues._original.A = (uint8_t)(instValues._original.A * fadeAlpha);
	}

	if (RendererCommon.ColorBindType == BindType::Always || RendererCommon.ColorBindType == BindType::WhenCreating)
	{
		instValues._color = Color::Mul(instValues._original, instance.ColorParent);
	}
	else
	{
		instValues._color = instValues._original;
	}

	instance.ColorInheritance = instValues._color;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------



//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------





//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
namespace Effekseer
{

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void EffectNodeRing::LoadRendererParameter(unsigned char*& pos, Setting* setting)
{
	int32_t type = 0;
	memcpy( &type, pos, sizeof(int) );
	pos += sizeof(int);
	assert( type == GetType() );
	EffekseerPrintDebug("Renderer : Ring\n");

	memcpy( &RenderingOrder, pos, sizeof(int) );
	pos += sizeof(int);

	if( m_effect->GetVersion() >= 3)
	{
		AlphaBlend = RendererCommon.AlphaBlend;
	}
	else
	{
		memcpy( &AlphaBlend, pos, sizeof(int) );
		pos += sizeof(int);
	}

	memcpy( &Billboard, pos, sizeof(int) );
	pos += sizeof(int);
	
	if (m_effect->GetVersion() >= 15)
	{
		int32_t ringShape = 0;
		memcpy(&ringShape, pos, sizeof(int));
		pos += sizeof(int);

		Shape.Type = static_cast<RingShapeType>(ringShape);

		if (Shape.Type == RingShapeType::Dount)
		{
			Shape.StartingAngle.type = RingSingleParameter::Fixed;
			Shape.EndingAngle.type = RingSingleParameter::Fixed;
			Shape.StartingAngle.fixed = 0;
			Shape.EndingAngle.fixed = 360;
		}
		else if (Shape.Type == RingShapeType::Cresient)
		{
			memcpy(&Shape.StartingFade, pos, sizeof(float));
			pos += sizeof(float);
			memcpy(&Shape.EndingFade, pos, sizeof(float));
			pos += sizeof(float);

			LoadSingleParameter(pos, Shape.StartingAngle);
			LoadSingleParameter(pos, Shape.EndingAngle);
		}
	}

	memcpy( &VertexCount, pos, sizeof(int) );
	pos += sizeof(int);
	
	// compatiblity
	{
		RingSingleParameter viewingAngle;
		LoadSingleParameter(pos, viewingAngle);
		if (m_effect->GetVersion() < 15)
		{
			Shape.Type = RingShapeType::Cresient;
			Shape.StartingAngle = viewingAngle;
			Shape.EndingAngle = viewingAngle;

			if (viewingAngle.type == RingSingleParameter::Fixed)
			{
				Shape.StartingAngle.fixed = (360 - viewingAngle.fixed) / 2.0f + 90.0f;
				Shape.EndingAngle.fixed = 360.0f - (360 - viewingAngle.fixed) / 2.0f + 90.0f;
			}

			if (viewingAngle.type == RingSingleParameter::Random)
			{
				Shape.StartingAngle.random.max = (360 - viewingAngle.random.min) / 2.0f + 90.0f;
				Shape.StartingAngle.random.min = (360 - viewingAngle.random.max) / 2.0f + 90.0f;
				Shape.EndingAngle.random.max = 360.0f - (360 - viewingAngle.random.max) / 2.0f + 90.0f;
				Shape.EndingAngle.random.min = 360.0f - (360 - viewingAngle.random.min) / 2.0f + 90.0f;
			}

			if (viewingAngle.type == RingSingleParameter::Easing)
			{
				Shape.StartingAngle.easing.start.max = (360 - viewingAngle.easing.start.min) / 2.0f + 90.0f;
				Shape.StartingAngle.easing.start.min = (360 - viewingAngle.easing.start.max) / 2.0f + 90.0f;
				Shape.StartingAngle.easing.end.max = (360 - viewingAngle.easing.end.min) / 2.0f + 90.0f;
				Shape.StartingAngle.easing.end.min = (360 - viewingAngle.easing.end.max) / 2.0f + 90.0f;
				Shape.EndingAngle.easing.start.max = 360.0f - (360 - viewingAngle.easing.start.max) / 2.0f + 90.0f;
				Shape.EndingAngle.easing.start.min = 360.0f - (360 - viewingAngle.easing.start.min) / 2.0f + 90.0f;
				Shape.EndingAngle.easing.end.max = 360.0f - (360 - viewingAngle.easing.end.max) / 2.0f + 90.0f;
				Shape.EndingAngle.easing.end.min = 360.0f - (360 - viewingAngle.easing.end.min) / 2.0f + 90.0f;
			}
		}
	}

	LoadLocationParameter( pos, OuterLocation );
	
	LoadLocationParameter( pos, InnerLocation );
	
	LoadSingleParameter( pos, CenterRatio );

	LoadColorParameter( pos, OuterColor);

	LoadColorParameter( pos, CenterColor);

	LoadColorParameter( pos, InnerColor);

	if( m_effect->GetVersion() >= 3)
	{
		RingTexture = RendererCommon.ColorTextureIndex;
	}
	else
	{
		memcpy( &RingTexture, pos, sizeof(int) );
		pos += sizeof(int);
	}
	
	// 右手系左手系変換
	if (setting->GetCoordinateSystem() == CoordinateSystem::LH)
	{
		if( OuterLocation.type == RingLocationParameter::Fixed )
		{
			OuterLocation.fixed.location.y *= -1;
		}
		else if( OuterLocation.type == RingLocationParameter::PVA )
		{
			OuterLocation.pva.location.min.y *= -1;
			OuterLocation.pva.location.max.y *= -1;
			OuterLocation.pva.velocity.min.y *= -1;
			OuterLocation.pva.velocity.max.y *= -1;
			OuterLocation.pva.acceleration.min.y *= -1;
			OuterLocation.pva.acceleration.max.y *= -1;
		}
		else if( OuterLocation.type == RingLocationParameter::Easing )
		{
			OuterLocation.easing.start.min.y *= -1;
			OuterLocation.easing.start.max.y *= -1;
			OuterLocation.easing.end.min.y *= -1;
			OuterLocation.easing.end.max.y *= -1;
		}

		if( InnerLocation.type == RingLocationParameter::Fixed )
		{
			InnerLocation.fixed.location.y *= -1;
		}
		else if( InnerLocation.type == RingLocationParameter::PVA )
		{
			InnerLocation.pva.location.min.y *= -1;
			InnerLocation.pva.location.max.y *= -1;
			InnerLocation.pva.velocity.min.y *= -1;
			InnerLocation.pva.velocity.max.y *= -1;
			InnerLocation.pva.acceleration.min.y *= -1;
			InnerLocation.pva.acceleration.max.y *= -1;
		}
		else if( InnerLocation.type == RingLocationParameter::Easing )
		{
			InnerLocation.easing.start.min.y *= -1;
			InnerLocation.easing.start.max.y *= -1;
			InnerLocation.easing.end.min.y *= -1;
			InnerLocation.easing.end.max.y *= -1;
		}
	}

	/* 位置拡大処理 */
	if( m_effect->GetVersion() >= 8 )
	{
		if( OuterLocation.type == RingLocationParameter::Fixed )
		{
			OuterLocation.fixed.location *= m_effect->GetMaginification();
		}
		else if( OuterLocation.type == RingLocationParameter::PVA )
		{
			OuterLocation.pva.location.min *= m_effect->GetMaginification();
			OuterLocation.pva.location.max *= m_effect->GetMaginification();
			OuterLocation.pva.velocity.min *= m_effect->GetMaginification();
			OuterLocation.pva.velocity.max *= m_effect->GetMaginification();
			OuterLocation.pva.acceleration.min *= m_effect->GetMaginification();
			OuterLocation.pva.acceleration.max *= m_effect->GetMaginification();
		}
		else if( OuterLocation.type == RingLocationParameter::Easing )
		{
			OuterLocation.easing.start.min *= m_effect->GetMaginification();
			OuterLocation.easing.start.max *= m_effect->GetMaginification();
			OuterLocation.easing.end.min *= m_effect->GetMaginification();
			OuterLocation.easing.end.max *= m_effect->GetMaginification();
		}

		if( InnerLocation.type == RingLocationParameter::Fixed )
		{
			InnerLocation.fixed.location *= m_effect->GetMaginification();
		}
		else if( InnerLocation.type == RingLocationParameter::PVA )
		{
			InnerLocation.pva.location.min *= m_effect->GetMaginification();
			InnerLocation.pva.location.max *= m_effect->GetMaginification();
			InnerLocation.pva.velocity.min *= m_effect->GetMaginification();
			InnerLocation.pva.velocity.max *= m_effect->GetMaginification();
			InnerLocation.pva.acceleration.min *= m_effect->GetMaginification();
			InnerLocation.pva.acceleration.max *= m_effect->GetMaginification();
		}
		else if( InnerLocation.type == RingLocationParameter::Easing )
		{
			InnerLocation.easing.start.min *= m_effect->GetMaginification();
			InnerLocation.easing.start.max *= m_effect->GetMaginification();
			InnerLocation.easing.end.min *= m_effect->GetMaginification();
			InnerLocation.easing.end.max *= m_effect->GetMaginification();
		}
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void EffectNodeRing::BeginRendering(int32_t count, Manager* manager)
{
	RingRenderer* renderer = manager->GetRingRenderer();
	if( renderer != NULL )
	{
		nodeParameter.EffectPointer = GetEffect();
		nodeParameter.ZTest = RendererCommon.ZTest;
		nodeParameter.ZWrite = RendererCommon.ZWrite;
		nodeParameter.Billboard = Billboard;
		nodeParameter.VertexCount = VertexCount;
		nodeParameter.IsRightHand = manager->GetCoordinateSystem() == CoordinateSystem::RH;

		nodeParameter.DepthParameterPtr = &DepthValues.DepthParameter;
		nodeParameter.BasicParameterPtr = &RendererCommon.BasicParameter;
		nodeParameter.StartingFade = Shape.StartingFade;
		nodeParameter.EndingFade = Shape.EndingFade;

		renderer->BeginRendering( nodeParameter, count, m_userData );
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void EffectNodeRing::Rendering(const Instance& instance, const Instance* next_instance, Manager* manager)
{
	const InstanceValues& instValues = instance.rendererValues.ring;
	RingRenderer* renderer = manager->GetRingRenderer();
	if( renderer != NULL )
	{
		nodeParameter.EffectPointer = GetEffect();
		nodeParameter.ZTest = RendererCommon.ZTest;
		nodeParameter.ZWrite = RendererCommon.ZWrite;
		nodeParameter.Billboard = Billboard;
		nodeParameter.VertexCount = VertexCount;
		nodeParameter.IsRightHand = manager->GetCoordinateSystem() ==
			CoordinateSystem::RH;

		nodeParameter.DepthParameterPtr = &DepthValues.DepthParameter;
		nodeParameter.BasicParameterPtr = &RendererCommon.BasicParameter;
		nodeParameter.StartingFade = Shape.StartingFade;
		nodeParameter.EndingFade = Shape.EndingFade;

		Color _outerColor;
		Color _centerColor;
		Color _innerColor;

		if (RendererCommon.ColorBindType == BindType::Always || RendererCommon.ColorBindType == BindType::WhenCreating)
		{
			_outerColor = Color::Mul(instValues.outerColor.original, instance.ColorParent);
			_centerColor = Color::Mul(instValues.centerColor.original, instance.ColorParent);
			_innerColor = Color::Mul(instValues.innerColor.original, instance.ColorParent);
		}
		else
		{
			_outerColor = instValues.outerColor.original;
			_centerColor = instValues.centerColor.original;
			_innerColor = instValues.innerColor.original;
		}

		RingRenderer::InstanceParameter instanceParameter;
		instanceParameter.SRTMatrix43 = instance.GetGlobalMatrix43();

		instanceParameter.ViewingAngleStart = instValues.startingAngle.current;
		instanceParameter.ViewingAngleEnd = instValues.endingAngle.current;
		
		instValues.outerLocation.current.setValueToArg( instanceParameter.OuterLocation );
		instValues.innerLocation.current.setValueToArg( instanceParameter.InnerLocation );

		instanceParameter.CenterRatio = instValues.centerRatio.current;

		// Apply global Color
		if (instance.m_pContainer->GetRootInstance()->IsGlobalColorSet)
		{
			_outerColor = Color::Mul(_outerColor, instance.m_pContainer->GetRootInstance()->GlobalColor);
			_centerColor = Color::Mul(_centerColor, instance.m_pContainer->GetRootInstance()->GlobalColor);
			_innerColor = Color::Mul(_innerColor, instance.m_pContainer->GetRootInstance()->GlobalColor);
		}

		instanceParameter.OuterColor  = _outerColor;
		instanceParameter.CenterColor = _centerColor;
		instanceParameter.InnerColor  = _innerColor;
		
		instanceParameter.UV = instance.GetUV();
		
		CalcCustomData(&instance, instanceParameter.CustomData1, instanceParameter.CustomData2);

		renderer->Rendering( nodeParameter, instanceParameter, m_userData );
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void EffectNodeRing::EndRendering(Manager* manager)
{
	RingRenderer* renderer = manager->GetRingRenderer();
	if( renderer != NULL )
	{
		renderer->EndRendering( nodeParameter, m_userData );
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void EffectNodeRing::InitializeRenderedInstance(Instance& instance, Manager* manager)
{
	auto instanceGlobal = instance.m_pContainer->GetRootInstance();

	InstanceValues& instValues = instance.rendererValues.ring;

	InitializeSingleValues(Shape.StartingAngle, instValues.startingAngle, manager, instanceGlobal);
	InitializeSingleValues(Shape.EndingAngle, instValues.endingAngle, manager, instanceGlobal);

	InitializeLocationValues(OuterLocation, instValues.outerLocation, manager, instanceGlobal);
	InitializeLocationValues(InnerLocation, instValues.innerLocation, manager, instanceGlobal);
	
	InitializeSingleValues(CenterRatio, instValues.centerRatio, manager, instanceGlobal);

	InitializeColorValues(OuterColor, instValues.outerColor, manager, instanceGlobal);
	InitializeColorValues(CenterColor, instValues.centerColor, manager, instanceGlobal);
	InitializeColorValues(InnerColor, instValues.innerColor, manager, instanceGlobal);

	if (RendererCommon.ColorBindType == BindType::Always || RendererCommon.ColorBindType == BindType::WhenCreating)
	{
		instValues.outerColor.current = Color::Mul(instValues.outerColor.original, instance.ColorParent);
		instValues.centerColor.current = Color::Mul(instValues.centerColor.original, instance.ColorParent);
		instValues.innerColor.current = Color::Mul(instValues.innerColor.original, instance.ColorParent);
	}
	else
	{
		instValues.outerColor.current = instValues.outerColor.original;
		instValues.centerColor.current = instValues.centerColor.original;
		instValues.innerColor.current = instValues.innerColor.original;
	}

	instance.ColorInheritance = instValues.centerColor.current;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void EffectNodeRing::UpdateRenderedInstance(Instance& instance, Manager* manager)
{
	InstanceValues& instValues = instance.rendererValues.ring;
	
	UpdateSingleValues(instance, Shape.StartingAngle, instValues.startingAngle);
	UpdateSingleValues(instance, Shape.EndingAngle, instValues.endingAngle);

	UpdateLocationValues( instance, OuterLocation, instValues.outerLocation );
	UpdateLocationValues( instance, InnerLocation, instValues.innerLocation );
	
	UpdateSingleValues( instance, CenterRatio, instValues.centerRatio );

	UpdateColorValues( instance, OuterColor, instValues.outerColor );
	UpdateColorValues( instance, CenterColor, instValues.centerColor );
	UpdateColorValues( instance, InnerColor, instValues.innerColor );

	if (RendererCommon.ColorBindType == BindType::Always || RendererCommon.ColorBindType == BindType::WhenCreating)
	{
		instValues.outerColor.current = Color::Mul(instValues.outerColor.original, instance.ColorParent);
		instValues.centerColor.current = Color::Mul(instValues.centerColor.original, instance.ColorParent);
		instValues.innerColor.current = Color::Mul(instValues.innerColor.original, instance.ColorParent);
	}
	else
	{
		instValues.outerColor.current = instValues.outerColor.original;
		instValues.centerColor.current = instValues.centerColor.original;
		instValues.innerColor.current = instValues.innerColor.original;
	}

	instance.ColorInheritance = instValues.centerColor.current;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void EffectNodeRing::LoadSingleParameter( unsigned char*& pos, RingSingleParameter& param )
{
	memcpy( &param.type, pos, sizeof(int) );
	pos += sizeof(int);

	if( param.type == RingSingleParameter::Fixed )
	{
		memcpy( &param.fixed, pos, sizeof(float) );
		pos += sizeof(float);
	}
	else if( param.type == RingSingleParameter::Random )
	{
		memcpy( &param.random, pos, sizeof(param.random) );
		pos += sizeof(param.random);
	}
	else if( param.type == RingSingleParameter::Easing )
	{
		memcpy( &param.easing, pos, sizeof(param.easing) );
		pos += sizeof(param.easing);
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void EffectNodeRing::LoadLocationParameter( unsigned char*& pos, RingLocationParameter& param )
{
	memcpy( &param.type, pos, sizeof(int) );
	pos += sizeof(int);
	
	if( param.type == RingLocationParameter::Fixed )
	{
		memcpy( &param.fixed, pos, sizeof(param.fixed) );
		pos += sizeof(param.fixed);
	}
	else if( param.type == RingLocationParameter::PVA )
	{
		memcpy( &param.pva, pos, sizeof(param.pva) );
		pos += sizeof(param.pva);
	}
	else if( param.type == RingLocationParameter::Easing )
	{
		memcpy( &param.easing, pos, sizeof(param.easing) );
		pos += sizeof(param.easing);
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void EffectNodeRing::LoadColorParameter( unsigned char*& pos, RingColorParameter& param )
{
	memcpy( &param.type, pos, sizeof(int) );
	pos += sizeof(int);
	
	if( param.type == RingColorParameter::Fixed )
	{
		memcpy( &param.fixed, pos, sizeof(param.fixed) );
		pos += sizeof(param.fixed);
	}
	else if( param.type == RingColorParameter::Random )
	{
		param.random.load( m_effect->GetVersion(), pos );
	}
	else if( param.type == RingColorParameter::Easing )
	{
		param.easing.load( m_effect->GetVersion(), pos );
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void EffectNodeRing::InitializeSingleValues(const RingSingleParameter& param, RingSingleValues& values, Manager* manager, InstanceGlobal* instanceGlobal)
{
	switch( param.type )
	{
		case RingSingleParameter::Fixed:
			values.current = param.fixed;
			break;
		case RingSingleParameter::Random:
			values.current = param.random.getValue(*instanceGlobal);
			break;
		case RingSingleParameter::Easing:
			values.easing.start = param.easing.start.getValue(*instanceGlobal);
			values.easing.end = param.easing.end.getValue(*instanceGlobal);
			values.current = values.easing.start;
			break;
		default:
			break;
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void EffectNodeRing::InitializeLocationValues(const RingLocationParameter& param, RingLocationValues& values, Manager* manager, InstanceGlobal* instanceGlobal)
{
	switch( param.type )
	{
		case RingLocationParameter::Fixed:
			values.current = param.fixed.location;
			break;
		case RingLocationParameter::PVA:
			values.pva.start = param.pva.location.getValue(*instanceGlobal);
			values.pva.velocity = param.pva.velocity.getValue(*instanceGlobal);
			values.pva.acceleration = param.pva.acceleration.getValue(*instanceGlobal);
			values.current = values.pva.start;
			break;
		case RingLocationParameter::Easing:
			values.easing.start = param.easing.start.getValue(*instanceGlobal);
			values.easing.end = param.easing.end.getValue(*instanceGlobal);
			values.current = values.easing.start;
			break;
		default:
			break;
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void EffectNodeRing::InitializeColorValues(const RingColorParameter& param, RingColorValues& values, Manager* manager, InstanceGlobal* instanceGlobal)
{
	switch( param.type )
	{
		case RingColorParameter::Fixed:
			values.original = param.fixed;
			values.fixed._color = values.original;
			break;
		case RingColorParameter::Random:
			values.original = param.random.getValue(*instanceGlobal);
			values.random._color = values.original;
			break;
		case RingColorParameter::Easing:
			values.easing.start = param.easing.getStartValue(*instanceGlobal);
			values.easing.end = param.easing.getEndValue(*instanceGlobal);
			values.original = values.easing.start;
			break;
		default:
			break;
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void EffectNodeRing::UpdateSingleValues( Instance& instance, const RingSingleParameter& param, RingSingleValues& values )
{
	if( param.type == RingSingleParameter::Easing )
	{
		param.easing.setValueToArg(
			values.current,
			values.easing.start,
			values.easing.end,
			instance.m_LivingTime / instance.m_LivedTime );
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void EffectNodeRing::UpdateLocationValues( Instance& instance, const RingLocationParameter& param, RingLocationValues& values )
{
	if( param.type == RingLocationParameter::PVA )
	{
		values.current = values.pva.start +
			values.pva.velocity * instance.m_LivingTime +
			values.pva.acceleration * instance.m_LivingTime * instance.m_LivingTime * 0.5f;
	}
	else if( param.type == RingLocationParameter::Easing )
	{
		param.easing.setValueToArg(
			values.current,
			values.easing.start,
			values.easing.end,
			instance.m_LivingTime / instance.m_LivedTime );
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void EffectNodeRing::UpdateColorValues( Instance& instance, const RingColorParameter& param, RingColorValues& values )
{
	if (param.type == RingColorParameter::Fixed)
	{
		values.original = values.fixed._color;
	}
	else if (param.type == RingColorParameter::Random)
	{
		values.original = values.random._color;
	}
	else if( param.type == RingColorParameter::Easing )
	{
		param.easing.setValueToArg(
			values.original, 
			values.easing.start,
			values.easing.end,
			instance.m_LivingTime / instance.m_LivedTime );
	}

	float fadeAlpha = GetFadeAlpha(instance);
	if (fadeAlpha != 1.0f)
	{
		values.original.A = (uint8_t)(values.original.A * fadeAlpha);
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------



//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
namespace Effekseer
{
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------


//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------



//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------







//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
namespace Effekseer
{
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
	void EffectNodeSprite::LoadRendererParameter(unsigned char*& pos, Setting* setting)
{
	int32_t type = 0;
	memcpy(&type, pos, sizeof(int));
	pos += sizeof(int);
	assert(type == GetType());
	EffekseerPrintDebug("Renderer : Sprite\n");

	auto ef = (EffectImplemented*) m_effect;

	memcpy(&RenderingOrder, pos, sizeof(int));
	pos += sizeof(int);

	if (m_effect->GetVersion() >= 3)
	{
		AlphaBlend = RendererCommon.AlphaBlend;
	}
	else
	{
		memcpy(&AlphaBlend, pos, sizeof(int));
		pos += sizeof(int);
		RendererCommon.AlphaBlend = AlphaBlend;
		RendererCommon.BasicParameter.AlphaBlend = AlphaBlend;
	}

	memcpy(&Billboard, pos, sizeof(int));
	pos += sizeof(int);

	SpriteAllColor.load(pos, m_effect->GetVersion());
	EffekseerPrintDebug("SpriteColorAllType : %d\n", SpriteAllColor.type);

	memcpy(&SpriteColor.type, pos, sizeof(int));
	pos += sizeof(int);
	EffekseerPrintDebug("SpriteColorType : %d\n", SpriteColor.type);

	if (SpriteColor.type == SpriteColor.Default)
	{
	}
	else if (SpriteColor.type == SpriteColor.Fixed)
	{
		memcpy(&SpriteColor.fixed, pos, sizeof(SpriteColor.fixed));
		pos += sizeof(SpriteColor.fixed);
	}

	memcpy(&SpritePosition.type, pos, sizeof(int));
	pos += sizeof(int);
	EffekseerPrintDebug("SpritePosition : %d\n", SpritePosition.type);

	if (SpritePosition.type == SpritePosition.Default)
	{
		if (m_effect->GetVersion() >= 8)
		{
			memcpy(&SpritePosition.fixed, pos, sizeof(SpritePosition.fixed));
			pos += sizeof(SpritePosition.fixed);
			SpritePosition.type = SpritePosition.Fixed;
		}
		else
		{
			SpritePosition.fixed.ll.x = -0.5f;
			SpritePosition.fixed.ll.y = -0.5f;
			SpritePosition.fixed.lr.x = 0.5f;
			SpritePosition.fixed.lr.y = -0.5f;
			SpritePosition.fixed.ul.x = -0.5f;
			SpritePosition.fixed.ul.y = 0.5f;
			SpritePosition.fixed.ur.x = 0.5f;
			SpritePosition.fixed.ur.y = 0.5f;
			SpritePosition.type = SpritePosition.Fixed;
		}
	}
	else if (SpritePosition.type == SpritePosition.Fixed)
	{
		memcpy(&SpritePosition.fixed, pos, sizeof(SpritePosition.fixed));
		pos += sizeof(SpritePosition.fixed);
	}

	if (m_effect->GetVersion() >= 3)
	{
		SpriteTexture = RendererCommon.ColorTextureIndex;
	}
	else
	{
		memcpy(&SpriteTexture, pos, sizeof(int));
		pos += sizeof(int);
		RendererCommon.ColorTextureIndex = SpriteTexture;
		RendererCommon.BasicParameter.Texture1Index = SpriteTexture;
	}

	// 右手系左手系変換
	if (setting->GetCoordinateSystem() == CoordinateSystem::LH)
	{
	}

	/* 位置拡大処理 */
	if (ef->IsDyanamicMagnificationValid())
	{
		if (SpritePosition.type == SpritePosition.Default)
		{
		}
		else if (SpritePosition.type == SpritePosition.Fixed)
		{
			SpritePosition.fixed.ll *= m_effect->GetMaginification();
			SpritePosition.fixed.lr *= m_effect->GetMaginification();
			SpritePosition.fixed.ul *= m_effect->GetMaginification();
			SpritePosition.fixed.ur *= m_effect->GetMaginification();
		}
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void EffectNodeSprite::BeginRendering(int32_t count, Manager* manager)
{
	SpriteRenderer* renderer = manager->GetSpriteRenderer();
	if( renderer != NULL )
	{
		SpriteRenderer::NodeParameter nodeParameter;
		//nodeParameter.TextureFilter = RendererCommon.FilterType;
		//nodeParameter.TextureWrap = RendererCommon.WrapType;
		nodeParameter.ZTest = RendererCommon.ZTest;
		nodeParameter.ZWrite = RendererCommon.ZWrite;
		nodeParameter.Billboard = Billboard;
		nodeParameter.EffectPointer = GetEffect();
		nodeParameter.IsRightHand = manager->GetCoordinateSystem() ==
			CoordinateSystem::RH;

		nodeParameter.DepthParameterPtr = &DepthValues.DepthParameter;
		nodeParameter.BasicParameterPtr = &RendererCommon.BasicParameter;

		nodeParameter.ZSort = DepthValues.ZSort;

		renderer->BeginRendering( nodeParameter, count, m_userData );
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void EffectNodeSprite::Rendering(const Instance& instance, const Instance* next_instance, Manager* manager)
{
	const InstanceValues& instValues = instance.rendererValues.sprite;
	SpriteRenderer* renderer = manager->GetSpriteRenderer();
	if( renderer != NULL )
	{
		SpriteRenderer::NodeParameter nodeParameter;
		//nodeParameter.TextureFilter = RendererCommon.FilterType;
		//nodeParameter.TextureWrap = RendererCommon.WrapType;
		nodeParameter.ZTest = RendererCommon.ZTest;
		nodeParameter.ZWrite = RendererCommon.ZWrite;
		nodeParameter.Billboard = Billboard;
		nodeParameter.EffectPointer = GetEffect();
		nodeParameter.IsRightHand = manager->GetCoordinateSystem() ==
			CoordinateSystem::RH;

		nodeParameter.DepthParameterPtr = &DepthValues.DepthParameter;
		nodeParameter.BasicParameterPtr = &RendererCommon.BasicParameter;

		nodeParameter.ZSort = DepthValues.ZSort;

		SpriteRenderer::InstanceParameter instanceParameter;
		instanceParameter.AllColor = instValues._color;

		instanceParameter.SRTMatrix43 = instance.GetGlobalMatrix43();

		// Inherit Color
		Color _color;
		if (RendererCommon.ColorBindType == BindType::Always || RendererCommon.ColorBindType == BindType::WhenCreating)
		{
			_color = Color::Mul(instValues._originalColor, instance.ColorParent);
		}
		else
		{
			_color = instValues._originalColor;
		}

		Color color_ll = _color;
		Color color_lr = _color;
		Color color_ul = _color;
		Color color_ur = _color;

		if( SpriteColor.type == SpriteColorParameter::Default )
		{
		}
		else if( SpriteColor.type == SpriteColorParameter::Fixed )
		{
			color_ll = Color::Mul( color_ll, SpriteColor.fixed.ll );
			color_lr = Color::Mul( color_lr, SpriteColor.fixed.lr );
			color_ul = Color::Mul( color_ul, SpriteColor.fixed.ul );
			color_ur = Color::Mul( color_ur, SpriteColor.fixed.ur );
		}

		instanceParameter.Colors[0] = color_ll;
		instanceParameter.Colors[1] = color_lr;
		instanceParameter.Colors[2] = color_ul;
		instanceParameter.Colors[3] = color_ur;
		
		// Apply global Color
		if (instance.m_pContainer->GetRootInstance()->IsGlobalColorSet)
		{
			instanceParameter.Colors[0] = Color::Mul(instanceParameter.Colors[0], instance.m_pContainer->GetRootInstance()->GlobalColor);
			instanceParameter.Colors[1] = Color::Mul(instanceParameter.Colors[1], instance.m_pContainer->GetRootInstance()->GlobalColor);
			instanceParameter.Colors[2] = Color::Mul(instanceParameter.Colors[2], instance.m_pContainer->GetRootInstance()->GlobalColor);
			instanceParameter.Colors[3] = Color::Mul(instanceParameter.Colors[3], instance.m_pContainer->GetRootInstance()->GlobalColor);
		}

		if( SpritePosition.type == SpritePosition.Default )
		{
			instanceParameter.Positions[0].X = -0.5f;
			instanceParameter.Positions[0].Y = -0.5f;
			instanceParameter.Positions[1].X = 0.5f;
			instanceParameter.Positions[1].Y = -0.5f;
			instanceParameter.Positions[2].X = -0.5f;
			instanceParameter.Positions[2].Y = 0.5f;
			instanceParameter.Positions[3].X = 0.5f;
			instanceParameter.Positions[3].Y = 0.5f;
		}
		else if( SpritePosition.type == SpritePosition.Fixed )
		{
			SpritePosition.fixed.ll.setValueToArg( instanceParameter.Positions[0] );
			SpritePosition.fixed.lr.setValueToArg( instanceParameter.Positions[1] );
			SpritePosition.fixed.ul.setValueToArg( instanceParameter.Positions[2] );
			SpritePosition.fixed.ur.setValueToArg( instanceParameter.Positions[3] );
		}

		instanceParameter.UV = instance.GetUV();
		CalcCustomData(&instance, instanceParameter.CustomData1, instanceParameter.CustomData2);

		renderer->Rendering( nodeParameter, instanceParameter, m_userData );
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void EffectNodeSprite::EndRendering(Manager* manager)
{
	SpriteRenderer* renderer = manager->GetSpriteRenderer();
	if( renderer != NULL )
	{
		SpriteRenderer::NodeParameter nodeParameter;
		//nodeParameter.TextureFilter = RendererCommon.FilterType;
		//nodeParameter.TextureWrap = RendererCommon.WrapType;
		nodeParameter.ZTest = RendererCommon.ZTest;
		nodeParameter.ZWrite = RendererCommon.ZWrite;
		nodeParameter.Billboard = Billboard;
		nodeParameter.EffectPointer = GetEffect();
		nodeParameter.IsRightHand = manager->GetCoordinateSystem() ==
			CoordinateSystem::RH;

		nodeParameter.ZSort = DepthValues.ZSort;

		nodeParameter.DepthParameterPtr = &DepthValues.DepthParameter;
		nodeParameter.BasicParameterPtr = &RendererCommon.BasicParameter;

		renderer->EndRendering( nodeParameter, m_userData );
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void EffectNodeSprite::InitializeRenderedInstance(Instance& instance, Manager* manager)
{
	InstanceValues& instValues = instance.rendererValues.sprite;
	auto instanceGlobal = instance.m_pContainer->GetRootInstance();

	if( SpriteAllColor.type == StandardColorParameter::Fixed )
	{
		instValues.allColorValues.fixed._color = SpriteAllColor.fixed.all;
		instValues._originalColor = instValues.allColorValues.fixed._color;
	}
	else if( SpriteAllColor.type == StandardColorParameter::Random )
	{
		instValues.allColorValues.random._color = SpriteAllColor.random.all.getValue(*instanceGlobal);
		instValues._originalColor = instValues.allColorValues.random._color;
	}
	else if( SpriteAllColor.type == StandardColorParameter::Easing )
	{
		instValues.allColorValues.easing.start = SpriteAllColor.easing.all.getStartValue(*instanceGlobal);
		instValues.allColorValues.easing.end = SpriteAllColor.easing.all.getEndValue(*instanceGlobal);

		float t = instance.m_LivingTime / instance.m_LivedTime;

		SpriteAllColor.easing.all.setValueToArg(
			instValues._originalColor,
			instValues.allColorValues.easing.start,
			instValues.allColorValues.easing.end,
			t );
	}
	else if (SpriteAllColor.type == StandardColorParameter::FCurve_RGBA)
	{
		instValues.allColorValues.fcurve_rgba.offset = SpriteAllColor.fcurve_rgba.FCurve->GetOffsets(*instanceGlobal);
		auto fcurveColor = SpriteAllColor.fcurve_rgba.FCurve->GetValues(instance.m_LivingTime, instance.m_LivedTime);
		instValues._originalColor.R = (uint8_t)Clamp((instValues.allColorValues.fcurve_rgba.offset[0] + fcurveColor[0]), 255, 0);
		instValues._originalColor.G = (uint8_t)Clamp((instValues.allColorValues.fcurve_rgba.offset[1] + fcurveColor[1]), 255, 0);
		instValues._originalColor.B = (uint8_t)Clamp((instValues.allColorValues.fcurve_rgba.offset[2] + fcurveColor[2]), 255, 0);
		instValues._originalColor.A = (uint8_t)Clamp((instValues.allColorValues.fcurve_rgba.offset[3] + fcurveColor[3]), 255, 0);
	}

	if (RendererCommon.ColorBindType == BindType::Always || RendererCommon.ColorBindType == BindType::WhenCreating)
	{
		instValues._color = Color::Mul(instValues._originalColor, instance.ColorParent);
	}
	else
	{
		instValues._color = instValues._originalColor;
	}
	
	instance.ColorInheritance = instValues._color;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void EffectNodeSprite::UpdateRenderedInstance(Instance& instance, Manager* manager)
{
	InstanceValues& instValues = instance.rendererValues.sprite;

	if (SpriteAllColor.type == StandardColorParameter::Fixed)
	{
		instValues._originalColor = instValues.allColorValues.fixed._color;
	}
	else if (SpriteAllColor.type == StandardColorParameter::Random)
	{
		instValues._originalColor = instValues.allColorValues.random._color;
	}
	if( SpriteAllColor.type == StandardColorParameter::Easing )
	{
		float t = instance.m_LivingTime / instance.m_LivedTime;

		SpriteAllColor.easing.all.setValueToArg(
			instValues._originalColor, 
			instValues.allColorValues.easing.start,
			instValues.allColorValues.easing.end,
			t );
	}
	else if (SpriteAllColor.type == StandardColorParameter::FCurve_RGBA)
	{
		auto fcurveColor = SpriteAllColor.fcurve_rgba.FCurve->GetValues(instance.m_LivingTime, instance.m_LivedTime);
		instValues._originalColor.R = (uint8_t)Clamp((instValues.allColorValues.fcurve_rgba.offset[0] + fcurveColor[0]), 255, 0);
		instValues._originalColor.G = (uint8_t)Clamp((instValues.allColorValues.fcurve_rgba.offset[1] + fcurveColor[1]), 255, 0);
		instValues._originalColor.B = (uint8_t)Clamp((instValues.allColorValues.fcurve_rgba.offset[2] + fcurveColor[2]), 255, 0);
		instValues._originalColor.A = (uint8_t)Clamp((instValues.allColorValues.fcurve_rgba.offset[3] + fcurveColor[3]), 255, 0);
	}

	float fadeAlpha = GetFadeAlpha(instance);
	if (fadeAlpha != 1.0f)
	{
		instValues._originalColor.A = (uint8_t)(instValues._originalColor.A * fadeAlpha);
	}

	if (RendererCommon.ColorBindType == BindType::Always || RendererCommon.ColorBindType == BindType::WhenCreating)
	{
		instValues._color = Color::Mul(instValues._originalColor, instance.ColorParent);
	}
	else
	{
		instValues._color = instValues._originalColor;
	}

	instance.ColorInheritance = instValues._color;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------



//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------




//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
namespace Effekseer
{
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void EffectNodeTrack::LoadRendererParameter(unsigned char*& pos, Setting* setting)
{
	int32_t type = 0;
	memcpy(&type, pos, sizeof(int));
	pos += sizeof(int);
	assert(type == GetType());
	EffekseerPrintDebug("Renderer : Track\n");

	if (m_effect->GetVersion() >= 15)
	{
		TextureUVType.Load(pos, m_effect->GetVersion());
	}

	LoadValues(TrackSizeFor, pos);
	LoadValues(TrackSizeMiddle, pos);
	LoadValues(TrackSizeBack, pos);

	if (m_effect->GetVersion() >= 13)
	{
		memcpy(&SplineDivision, pos, sizeof(int32_t));
		pos += sizeof(int32_t);
	}

	TrackColorLeft.load(pos, m_effect->GetVersion());
	TrackColorLeftMiddle.load(pos, m_effect->GetVersion());

	TrackColorCenter.load(pos, m_effect->GetVersion());
	TrackColorCenterMiddle.load(pos, m_effect->GetVersion());

	TrackColorRight.load(pos, m_effect->GetVersion());
	TrackColorRightMiddle.load(pos, m_effect->GetVersion());

	AlphaBlend = RendererCommon.AlphaBlend;
	TrackTexture = RendererCommon.ColorTextureIndex;

	EffekseerPrintDebug("TrackColorLeft : %d\n", TrackColorLeft.type);
	EffekseerPrintDebug("TrackColorLeftMiddle : %d\n", TrackColorLeftMiddle.type);
	EffekseerPrintDebug("TrackColorCenter : %d\n", TrackColorCenter.type);
	EffekseerPrintDebug("TrackColorCenterMiddle : %d\n", TrackColorCenterMiddle.type);
	EffekseerPrintDebug("TrackColorRight : %d\n", TrackColorRight.type);
	EffekseerPrintDebug("TrackColorRightMiddle : %d\n", TrackColorRightMiddle.type);

	// 右手系左手系変換
	if (setting->GetCoordinateSystem() == CoordinateSystem::LH)
	{
	}

	/* 位置拡大処理 */
	if (m_effect->GetVersion() >= 8)
	{
		TrackSizeFor.fixed.size *= m_effect->GetMaginification();
		TrackSizeMiddle.fixed.size *= m_effect->GetMaginification();
		TrackSizeBack.fixed.size *= m_effect->GetMaginification();
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void EffectNodeTrack::BeginRendering(int32_t count, Manager* manager)
{
	TrackRenderer* renderer = manager->GetTrackRenderer();
	if (renderer != NULL)
	{
		//m_nodeParameter.TextureFilter = RendererCommon.FilterType;
		//m_nodeParameter.TextureWrap = RendererCommon.WrapType;
		m_nodeParameter.ZTest = RendererCommon.ZTest;
		m_nodeParameter.ZWrite = RendererCommon.ZWrite;
		m_nodeParameter.EffectPointer = GetEffect();

		m_nodeParameter.DepthParameterPtr = &DepthValues.DepthParameter;

		m_nodeParameter.SplineDivision = SplineDivision;
		m_nodeParameter.BasicParameterPtr = &RendererCommon.BasicParameter;
		m_nodeParameter.TextureUVTypeParameterPtr = &TextureUVType;
		m_nodeParameter.IsRightHand = manager->GetCoordinateSystem() == CoordinateSystem::RH;
		renderer->BeginRendering(m_nodeParameter, count, m_userData);
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void EffectNodeTrack::BeginRenderingGroup(InstanceGroup* group, Manager* manager)
{
	TrackRenderer* renderer = manager->GetTrackRenderer();
	if (renderer != nullptr)
	{
		m_currentGroupValues = group->rendererValues.track;

		m_instanceParameter.InstanceCount = group->GetInstanceCount();
		m_instanceParameter.InstanceIndex = 0;
		
		if (group->GetFirst() != nullptr)
		{
			m_instanceParameter.UV = group->GetFirst()->GetUV();
			CalcCustomData(group->GetFirst(), m_instanceParameter.CustomData1, m_instanceParameter.CustomData2);
		}

		renderer->BeginRenderingGroup(m_nodeParameter, group->GetInstanceCount(), m_userData);
	}
}

void EffectNodeTrack::EndRenderingGroup(InstanceGroup* group, Manager* manager)
{
	TrackRenderer* renderer = manager->GetTrackRenderer();
	if (renderer != NULL)
	{
		renderer->EndRenderingGroup(m_nodeParameter, group->GetInstanceCount(), m_userData);
	}
}

void EffectNodeTrack::Rendering(const Instance& instance, const Instance* next_instance, Manager* manager)
{
	TrackRenderer* renderer = manager->GetTrackRenderer();
	if (renderer != NULL)
	{
		float t = (float)instance.m_LivingTime / (float)instance.m_LivedTime;
		int32_t time = (int32_t)instance.m_LivingTime;
		int32_t livedTime = (int32_t)instance.m_LivedTime;

		SetValues(m_instanceParameter.ColorLeft, instance, m_currentGroupValues.ColorLeft, TrackColorLeft, time, livedTime);
		SetValues(m_instanceParameter.ColorCenter, instance, m_currentGroupValues.ColorCenter, TrackColorCenter, time, livedTime);
		SetValues(m_instanceParameter.ColorRight, instance, m_currentGroupValues.ColorRight, TrackColorRight, time, livedTime);

		SetValues(m_instanceParameter.ColorLeftMiddle, instance, m_currentGroupValues.ColorLeftMiddle, TrackColorLeftMiddle, time, livedTime);
		SetValues(m_instanceParameter.ColorCenterMiddle, instance, m_currentGroupValues.ColorCenterMiddle, TrackColorCenterMiddle, time, livedTime);
		SetValues(m_instanceParameter.ColorRightMiddle, instance, m_currentGroupValues.ColorRightMiddle, TrackColorRightMiddle, time, livedTime);

		SetValues(m_instanceParameter.SizeFor, m_currentGroupValues.SizeFor, TrackSizeFor, t);
		SetValues(m_instanceParameter.SizeMiddle, m_currentGroupValues.SizeMiddle, TrackSizeMiddle, t);
		SetValues(m_instanceParameter.SizeBack, m_currentGroupValues.SizeBack, TrackSizeBack, t);

		m_instanceParameter.SRTMatrix43 = instance.GetGlobalMatrix43();

		renderer->Rendering(m_nodeParameter, m_instanceParameter, m_userData);
		m_instanceParameter.InstanceIndex++;
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void EffectNodeTrack::EndRendering(Manager* manager)
{
	TrackRenderer* renderer = manager->GetTrackRenderer();
	if (renderer != NULL)
	{
		renderer->EndRendering(m_nodeParameter, m_userData);
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void EffectNodeTrack::InitializeRenderedInstanceGroup(InstanceGroup& instanceGroup, Manager* manager)
{
	InstanceGroupValues& instValues = instanceGroup.rendererValues.track;
	auto instanceGlobal = instanceGroup.GetRootInstance();

	InitializeValues(instValues.ColorLeft, TrackColorLeft, instanceGlobal);
	InitializeValues(instValues.ColorCenter, TrackColorCenter, instanceGlobal);
	InitializeValues(instValues.ColorRight, TrackColorRight, instanceGlobal);

	InitializeValues(instValues.ColorLeftMiddle, TrackColorLeftMiddle, instanceGlobal);
	InitializeValues(instValues.ColorCenterMiddle, TrackColorCenterMiddle, instanceGlobal);
	InitializeValues(instValues.ColorRightMiddle, TrackColorRightMiddle, instanceGlobal);

	InitializeValues(instValues.SizeFor, TrackSizeFor, manager);
	InitializeValues(instValues.SizeBack, TrackSizeBack, manager);
	InitializeValues(instValues.SizeMiddle, TrackSizeMiddle, manager);
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void EffectNodeTrack::InitializeRenderedInstance(Instance& instance, Manager* manager)
{
	// Calculate only center
	int32_t time = (int32_t)instance.m_LivingTime;
	int32_t livedTime = (int32_t)instance.m_LivedTime;

	Color c;
	SetValues(c, instance, m_currentGroupValues.ColorCenterMiddle, TrackColorCenterMiddle, time, livedTime);

	if (RendererCommon.ColorBindType == BindType::Always || RendererCommon.ColorBindType == BindType::WhenCreating)
	{
		c = Color::Mul(c, instance.ColorParent);
	}

	instance.ColorInheritance = c;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void EffectNodeTrack::UpdateRenderedInstance(Instance& instance, Manager* manager)
{
	// Calculate only center
	int32_t time = (int32_t)instance.m_LivingTime;
	int32_t livedTime = (int32_t)instance.m_LivedTime;

	Color c;
	SetValues(c, instance, m_currentGroupValues.ColorCenterMiddle, TrackColorCenterMiddle, time, livedTime);

	instance.ColorInheritance = c;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void EffectNodeTrack::InitializeValues(InstanceGroupValues::Color& value, StandardColorParameter& param, InstanceGlobal* instanceGlobal)
{
	if (param.type == StandardColorParameter::Fixed)
	{
		value.color.fixed.color_ = param.fixed.all;
	}
	else if (param.type == StandardColorParameter::Random)
	{
		value.color.random.color_ = param.random.all.getValue(*(instanceGlobal));
	}
	else if (param.type == StandardColorParameter::Easing)
	{
		value.color.easing.start = param.easing.all.getStartValue(*(instanceGlobal));
		value.color.easing.end = param.easing.all.getEndValue(*(instanceGlobal));
	}
	else if (param.type == StandardColorParameter::FCurve_RGBA)
	{
		value.color.fcurve_rgba.offset = param.fcurve_rgba.FCurve->GetOffsets(*instanceGlobal);
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void EffectNodeTrack::InitializeValues(InstanceGroupValues::Size& value, TrackSizeParameter& param, Manager* manager)
{
	if (param.type == TrackSizeParameter::Fixed)
	{
		value.size.fixed.size_ = param.fixed.size;
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void EffectNodeTrack::SetValues(Color& c, const Instance& instance, InstanceGroupValues::Color& value, StandardColorParameter& param, int32_t time, int32_t livedTime)
{
	if (param.type == StandardColorParameter::Fixed)
	{
		c = value.color.fixed.color_;
	}
	else if (param.type == StandardColorParameter::Random)
	{
		c = value.color.random.color_;
	}
	else if (param.type == StandardColorParameter::Easing)
	{
		float t = (float)time / (float)livedTime;
		param.easing.all.setValueToArg(
			c,
			value.color.easing.start,
			value.color.easing.end,
			t);
	}
	else if (param.type == StandardColorParameter::FCurve_RGBA)
	{
		auto fcurveColors = param.fcurve_rgba.FCurve->GetValues(time, livedTime);
		c.R = (uint8_t)Clamp((value.color.fcurve_rgba.offset[0] + fcurveColors[0]), 255, 0);
		c.G = (uint8_t)Clamp((value.color.fcurve_rgba.offset[1] + fcurveColors[1]), 255, 0);
		c.B = (uint8_t)Clamp((value.color.fcurve_rgba.offset[2] + fcurveColors[2]), 255, 0);
		c.A = (uint8_t)Clamp((value.color.fcurve_rgba.offset[3] + fcurveColors[3]), 255, 0);
	}

	if (RendererCommon.ColorBindType == BindType::Always || RendererCommon.ColorBindType == BindType::WhenCreating)
	{
		c = Color::Mul(c, instance.ColorParent);
	}

	float fadeAlpha = GetFadeAlpha(instance);
	if (fadeAlpha != 1.0f)
	{
		c.A = (uint8_t)(c.A * fadeAlpha);
	}

	// Apply global Color
	if (instance.m_pContainer->GetRootInstance()->IsGlobalColorSet)
	{
		c = Color::Mul(c, instance.m_pContainer->GetRootInstance()->GlobalColor);
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void EffectNodeTrack::SetValues(float& s, InstanceGroupValues::Size& value, TrackSizeParameter& param, float time)
{
	if (param.type == TrackSizeParameter::Fixed)
	{
		s = value.size.fixed.size_;
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void EffectNodeTrack::LoadValues(TrackSizeParameter& param, unsigned char*& pos)
{
	memcpy(&param.type, pos, sizeof(int));
	pos += sizeof(int);

	if (param.type == TrackSizeParameter::Fixed)
	{
		memcpy(&param.fixed, pos, sizeof(param.fixed));
		pos += sizeof(param.fixed);
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------



//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------


//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
namespace Effekseer
{

static void PathCombine(EFK_CHAR* dst, const EFK_CHAR* src1, const EFK_CHAR* src2)
{
	int len1 = 0, len2 = 0;
	if( src1 != NULL )
	{
		for( len1 = 0; src1[len1] != L'\0'; len1++ ) {}
		memcpy( dst, src1, len1 * sizeof(EFK_CHAR) );
		if( len1 > 0 && src1[len1 - 1] != L'/' && src1[len1 - 1] != L'\\' )
		{
			dst[len1++] = L'/';
		}
	}
	if( src2 != NULL)
	{
		for( len2 = 0; src2[len2] != L'\0'; len2++ ) {}
		memcpy( &dst[len1], src2, len2 * sizeof(EFK_CHAR) );
	}
	dst[len1 + len2] = L'\0';
}

static void GetParentDir(EFK_CHAR* dst, const EFK_CHAR* src)
{
	int i, last = -1;
	for( i = 0; src[i] != L'\0'; i++ )
	{
		if( src[i] == L'/' || src[i] == L'\\' )
			last = i;
	}
	if( last >= 0 )
	{
		memcpy( dst, src, last * sizeof(EFK_CHAR) );
		dst[last] = L'\0';
	}
	else
	{
		dst[0] = L'\0';
	}
}

static std::u16string getFilenameWithoutExt(const char16_t* path)
{
	int start = 0;
	int end = 0;

	for (int i = 0; path[i] != 0; i++)
	{
		if (path[i] == u'/' || path[i] == u'\\')
		{
			start = i;
		}
	}

	for (int i = start; path[i] != 0; i++)
	{
		if (path[i] == u'.')
		{
			end = i;
		}
	}

	std::vector<char16_t> ret;

	for (int i = start; i < end; i++)
	{
		ret.push_back(path[i]);
	}
	ret.push_back(0);

	return std::u16string(ret.data());
}

bool EffectFactory::LoadBody(Effect* effect, const void* data, int32_t size, float magnification, const EFK_CHAR* materialPath)
{
	auto effect_ = static_cast<EffectImplemented*>(effect);
	auto data_ = static_cast<const uint8_t*>(data);
	return effect_->LoadBody(data_, size, magnification);
}

void EffectFactory::SetTexture(Effect* effect, int32_t index, TextureType type, TextureData* data)
{
	auto effect_ = static_cast<EffectImplemented*>(effect);

	if (type == TextureType::Color)
	{
		assert(0 <= index && index < effect_->m_ImageCount);
		effect_->m_pImages[index] = data;
	}

	if (type == TextureType::Normal)
	{
		assert(0 <= index && index < effect_->m_normalImageCount);
		effect_->m_normalImages[index] = data;
	}

	if (type == TextureType::Distortion)
	{
		assert(0 <= index && index < effect_->m_distortionImageCount);
		effect_->m_distortionImages[index] = data;
	}
}

void EffectFactory::SetSound(Effect* effect, int32_t index, void* data)
{ 
	auto effect_ = static_cast<EffectImplemented*>(effect); 

	assert(0 <= index && index < effect_->m_WaveCount);
	effect_->m_pWaves[index] = data;
	
}

void EffectFactory::SetModel(Effect* effect, int32_t index, void* data)
{ 
	auto effect_ = static_cast<EffectImplemented*>(effect);
	assert(0 <= index && index < effect_->modelCount_);
	effect_->models_[index] = data;
}

void EffectFactory::SetMaterial(Effect* effect, int32_t index, MaterialData* data)
{
	auto effect_ = static_cast<EffectImplemented*>(effect);
	assert(0 <= index && index < effect_->materialCount_);
	effect_->materials_[index] = data;
}

void EffectFactory::SetLoadingParameter(Effect* effect, ReferenceObject* parameter) {
	auto effect_ = static_cast<EffectImplemented*>(effect);
	effect_->SetLoadingParameter(parameter);
}


bool EffectFactory::OnCheckIsBinarySupported(const void* data, int32_t size)
{ 
	// EFKS
	int head = 0;
	memcpy(&head, data, sizeof(int));
	if (memcmp(&head, "SKFE", 4) != 0)
		return false;
	return true; 
}

bool EffectFactory::OnCheckIsReloadSupported() { return true; }

bool EffectFactory::OnLoading(Effect* effect, const void* data, int32_t size, float magnification, const EFK_CHAR* materialPath)
{ 
	return this->LoadBody(effect, data, size, magnification, materialPath);
}

void EffectFactory::OnLoadingResource(Effect* effect, const void* data, int32_t size, const EFK_CHAR* materialPath)
{
	auto textureLoader = effect->GetSetting()->GetTextureLoader();
	auto soundLoader = effect->GetSetting()->GetSoundLoader();
	auto modelLoader = effect->GetSetting()->GetModelLoader();
	auto materialLoader = effect->GetSetting()->GetMaterialLoader();

	if (textureLoader != nullptr)
	{
		for (auto i = 0; i < effect->GetColorImageCount(); i++)
		{
			EFK_CHAR fullPath[512];
			PathCombine(fullPath, materialPath, effect->GetColorImagePath(i));

			auto resource = textureLoader->Load(fullPath, TextureType::Color);
			SetTexture(effect, i, TextureType::Color, resource);
		}

		for (auto i = 0; i < effect->GetNormalImageCount(); i++)
		{
			EFK_CHAR fullPath[512];
			PathCombine(fullPath, materialPath, effect->GetNormalImagePath(i));

			auto resource = textureLoader->Load(fullPath, TextureType::Normal);
			SetTexture(effect, i, TextureType::Normal, resource);
		}

		for (auto i = 0; i < effect->GetDistortionImageCount(); i++)
		{
			EFK_CHAR fullPath[512];
			PathCombine(fullPath, materialPath, effect->GetDistortionImagePath(i));

			auto resource = textureLoader->Load(fullPath, TextureType::Distortion);
			SetTexture(effect, i, TextureType::Distortion, resource);
		}
	}

	if (soundLoader != nullptr)
	{
		for (auto i = 0; i < effect->GetWaveCount(); i++)
		{
			EFK_CHAR fullPath[512];
			PathCombine(fullPath, materialPath, effect->GetWavePath(i));

			auto resource = soundLoader->Load(fullPath);
			SetSound(effect, i, resource);
		}
	}

	if (modelLoader != nullptr)
	{
		for (auto i = 0; i < effect->GetModelCount(); i++)
		{
			EFK_CHAR fullPath[512];
			PathCombine(fullPath, materialPath, effect->GetModelPath(i));

			auto resource = modelLoader->Load(fullPath);
			SetModel(effect, i, resource);
		}
	}

	if (materialLoader != nullptr)
	{
		for (auto i = 0; i < effect->GetMaterialCount(); i++)
		{
			EFK_CHAR fullPath[512];
			PathCombine(fullPath, materialPath, effect->GetMaterialPath(i));

			auto resource = materialLoader->Load(fullPath);
			SetMaterial(effect, i, resource);
		}
	}
}

void EffectFactory::OnUnloadingResource(Effect* effect) { 
	auto textureLoader = effect->GetSetting()->GetTextureLoader(); 
	auto soundLoader = effect->GetSetting()->GetSoundLoader();
	auto modelLoader = effect->GetSetting()->GetModelLoader();
	auto materialLoader = effect->GetSetting()->GetMaterialLoader();

	if (textureLoader != nullptr)
	{
		for (auto i = 0; i < effect->GetColorImageCount(); i++)
		{
			textureLoader->Unload(effect->GetColorImage(i));
			SetTexture(effect, i, TextureType::Color, nullptr);
		}

		for (auto i = 0; i < effect->GetNormalImageCount(); i++)
		{
			textureLoader->Unload(effect->GetNormalImage(i));
			SetTexture(effect, i, TextureType::Normal, nullptr);
		}

		for (auto i = 0; i < effect->GetDistortionImageCount(); i++)
		{
			textureLoader->Unload(effect->GetDistortionImage(i));
			SetTexture(effect, i, TextureType::Distortion, nullptr);
		}
	}

	if (soundLoader != nullptr)
	{
		for (auto i = 0; i < effect->GetWaveCount(); i++)
		{
			soundLoader->Unload(effect->GetWave(i));
			SetSound(effect, i, nullptr);
		}
	}

	if (modelLoader != nullptr)
	{
		for (auto i = 0; i < effect->GetModelCount(); i++)
		{
			modelLoader->Unload(effect->GetModel(i));
			SetModel(effect, i, nullptr);
		}
	}
	
	if (materialLoader != nullptr)
	{
		for (auto i = 0; i < effect->GetMaterialCount(); i++)
		{
			materialLoader->Unload(effect->GetMaterial(i));
			SetMaterial(effect, i, nullptr);
		}
	}
}

const char* EffectFactory::GetName() const 
{ 
	static const char* name = "Default";
	return name;
}

 bool EffectFactory::GetIsResourcesLoadedAutomatically() const { return true; }

EffectFactory::EffectFactory() {
}

EffectFactory::~EffectFactory()
{
}

Effect* Effect::Create(Manager * manager, void* data, int32_t size, float magnification, const EFK_CHAR* materialPath)
{
	return EffectImplemented::Create( manager, data, size, magnification, materialPath );
}

Effect* Effect::Create(Manager* manager, const EFK_CHAR* path, float magnification, const EFK_CHAR* materialPath)
{
	Setting* setting = manager->GetSetting();

	EffectLoader* eLoader = setting->GetEffectLoader();

	if (setting == NULL) return NULL;

	void* data = NULL;
	int32_t size = 0;

	if (!eLoader->Load(path, data, size)) return NULL;

	EFK_CHAR parentDir[512];
	if (materialPath == NULL)
	{
		GetParentDir(parentDir, path);
		materialPath = parentDir;
	}

	Effect* effect = EffectImplemented::Create(manager, data, size, magnification, materialPath);

	eLoader->Unload(data, size);

	effect->SetName(getFilenameWithoutExt(path).c_str());

	return effect;
}

bool EffectImplemented::LoadBody(const uint8_t* data, int32_t size, float mag)
{
	// TODO share with an editor
	const int32_t elementCountMax = 1024;
	const int32_t dynamicBinaryCountMax = 102400;

	uint8_t* pos = const_cast<uint8_t*>(data);

	BinaryReader<true> binaryReader(const_cast<uint8_t*>(data), size);

	// EFKS
	int head = 0;
	binaryReader.Read(head);
	if (memcmp(&head, "SKFE", 4) != 0)
		return false;

	binaryReader.Read(m_version);

	// Image
	binaryReader.Read(m_ImageCount, 0, elementCountMax);

	if (m_ImageCount > 0)
	{
		m_ImagePaths = new EFK_CHAR*[m_ImageCount];
		m_pImages = new TextureData*[m_ImageCount];

		for (int i = 0; i < m_ImageCount; i++)
		{
			int length = 0;
			binaryReader.Read(length, 0, elementCountMax);

			m_ImagePaths[i] = new EFK_CHAR[length];
			binaryReader.Read(m_ImagePaths[i], length);

			m_pImages[i] = nullptr;
		}
	}

	if (m_version >= 9)
	{
		// Image
		binaryReader.Read(m_normalImageCount, 0, elementCountMax);

		if (m_normalImageCount > 0)
		{
			m_normalImagePaths = new EFK_CHAR*[m_normalImageCount];
			m_normalImages = new TextureData*[m_normalImageCount];

			for (int i = 0; i < m_normalImageCount; i++)
			{
				int length = 0;
				binaryReader.Read(length, 0, elementCountMax);

				m_normalImagePaths[i] = new EFK_CHAR[length];
				binaryReader.Read(m_normalImagePaths[i], length);

				m_normalImages[i] = nullptr;
			}
		}

		// Image
		binaryReader.Read(m_distortionImageCount, 0, elementCountMax);

		if (m_distortionImageCount > 0)
		{
			m_distortionImagePaths = new EFK_CHAR*[m_distortionImageCount];
			m_distortionImages = new TextureData*[m_distortionImageCount];

			for (int i = 0; i < m_distortionImageCount; i++)
			{
				int length = 0;
				binaryReader.Read(length, 0, elementCountMax);

				m_distortionImagePaths[i] = new EFK_CHAR[length];
				binaryReader.Read(m_distortionImagePaths[i], length);

				m_distortionImages[i] = nullptr;
			}
		}
	}

	if (m_version >= 1)
	{
		// Sound
		binaryReader.Read(m_WaveCount, 0, elementCountMax);

		if (m_WaveCount > 0)
		{
			m_WavePaths = new EFK_CHAR*[m_WaveCount];
			m_pWaves = new void*[m_WaveCount];

			for (int i = 0; i < m_WaveCount; i++)
			{
				int length = 0;
				binaryReader.Read(length, 0, elementCountMax);

				m_WavePaths[i] = new EFK_CHAR[length];
				binaryReader.Read(m_WavePaths[i], length);

				m_pWaves[i] = nullptr;
			}
		}
	}

	if (m_version >= 6)
	{
		// Model
		binaryReader.Read(modelCount_, 0, elementCountMax);

		if (modelCount_ > 0)
		{
			modelPaths_ = new EFK_CHAR*[modelCount_];
			models_ = new void*[modelCount_];

			for (int i = 0; i < modelCount_; i++)
			{
				int length = 0;
				binaryReader.Read(length, 0, elementCountMax);

				modelPaths_[i] = new EFK_CHAR[length];
				binaryReader.Read(modelPaths_[i], length);

				models_[i] = nullptr;
			}
		}
	}

	if (m_version >= 15)
	{
		// material
		binaryReader.Read(materialCount_, 0, elementCountMax);
		
		if (materialCount_ > 0)
		{
			materialPaths_ = new EFK_CHAR*[materialCount_];
			materials_ = new MaterialData*[materialCount_];

			for (int i = 0; i < materialCount_; i++)
			{
				int length = 0;
				binaryReader.Read(length, 0, elementCountMax);

				materialPaths_[i] = new EFK_CHAR[length];
				binaryReader.Read(materialPaths_[i], length);

				materials_[i] = nullptr;
			}
		}
	}

	if (m_version >= 14)
	{
		// inputs
		defaultDynamicInputs.fill(0);
		int32_t dynamicInputCount = 0;
		binaryReader.Read(dynamicInputCount, 0, elementCountMax);

		for (size_t i = 0; i < dynamicInputCount; i++)
		{
			float param = 0.0f;
			binaryReader.Read(param);

			if (i < defaultDynamicInputs.size())
			{
				defaultDynamicInputs[i] = param;
			}
		}

		// dynamic parameter
		int32_t dynamicEquationCount = 0;
		binaryReader.Read(dynamicEquationCount, 0, elementCountMax);

		if (dynamicEquationCount > 0)
		{
			dynamicEquation.resize(dynamicEquationCount);

			for (size_t dp = 0; dp < dynamicEquation.size(); dp++)
			{
				int size_ = 0;
				binaryReader.Read(size_, 0, dynamicBinaryCountMax);

				auto data_ = pos + binaryReader.GetOffset();
				dynamicEquation[dp].Load(data_, size_);

				binaryReader.AddOffset(size_);
			}
		}
	}
	else
	{
		defaultDynamicInputs.fill(0);
	}

	if (m_version >= 13)
	{
		binaryReader.Read(renderingNodesCount, 0, elementCountMax);
		binaryReader.Read(renderingNodesThreshold, 0, elementCountMax);
	}

	// magnification
	if (m_version >= 2)
	{
		binaryReader.Read(m_maginification);
	}

	m_maginification *= mag;
	m_maginificationExternal = mag;

	if (m_version >= 11)
	{
		binaryReader.Read(m_defaultRandomSeed);
	}
	else
	{
		m_defaultRandomSeed = -1;
	}

	// Culling
	if (m_version >= 9)
	{
		binaryReader.Read(Culling.Shape);
		if (Culling.Shape == CullingShape::Sphere)
		{
			binaryReader.Read(Culling.Sphere.Radius);
			binaryReader.Read(Culling.Location.X);
			binaryReader.Read(Culling.Location.Y);
			binaryReader.Read(Culling.Location.Z);
		}
	}

	// Check
	if (binaryReader.GetStatus() == BinaryReaderStatus::Failed)
		return false;

	// Nodes
	auto nodeData = pos + binaryReader.GetOffset();
	m_pRoot = EffectNodeImplemented::Create(this, nullptr, nodeData);
	
	return true;
}

void EffectImplemented::ResetReloadingBackup()
{
	if(reloadingBackup == nullptr) return;

	Setting* loader = GetSetting();

	TextureLoader* textureLoader = loader->GetTextureLoader();
	if (textureLoader != NULL)
	{
		for (auto it: reloadingBackup->images.GetCollection())
		{
			textureLoader->Unload(it.second.value);
		}
	
		for (auto it : reloadingBackup->normalImages.GetCollection())
		{
			textureLoader->Unload(it.second.value);
		}

		for (auto it : reloadingBackup->distortionImages.GetCollection())
		{
			textureLoader->Unload(it.second.value);
		}
	}

	SoundLoader* soundLoader = loader->GetSoundLoader();
	if (soundLoader != NULL)
	{
		for (auto it : reloadingBackup->sounds.GetCollection())
		{
			soundLoader->Unload(it.second.value);
		}
	}

	{
		ModelLoader* modelLoader = loader->GetModelLoader();
		if (modelLoader != NULL)
		{
			for (auto it : reloadingBackup->models.GetCollection())
			{
				modelLoader->Unload(it.second.value);
			}
		}
	}

	reloadingBackup.reset();
}


Effect* EffectImplemented::Create( Manager* pManager, void* pData, int size, float magnification, const EFK_CHAR* materialPath )
{
	if( pData == NULL || size == 0 ) return NULL;

	EffectImplemented* effect = new EffectImplemented( pManager, pData, size );
	if ( !effect->Load( pData, size, magnification, materialPath, ReloadingThreadType::Main) )
	{
		effect->Release();
		effect = NULL;
	}
	return effect;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
Effect* Effect::Create( Setting* setting, void* data, int32_t size, float magnification, const EFK_CHAR* materialPath )
{
	return EffectImplemented::Create(setting, data, size, magnification, materialPath );
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
Effect* Effect::Create( Setting* setting, const EFK_CHAR* path, float magnification, const EFK_CHAR* materialPath)
{
	if(setting == NULL) return NULL;
	EffectLoader* eLoader = setting->GetEffectLoader();

	if (setting == NULL) return NULL;

	void* data = NULL;
	int32_t size = 0;

	if (!eLoader->Load(path, data, size)) return NULL;

	EFK_CHAR parentDir[512];
	if (materialPath == NULL)
	{
		GetParentDir(parentDir, path);
		materialPath = parentDir;
	}

	Effect* effect = EffectImplemented::Create(setting, data, size, magnification, materialPath);

	eLoader->Unload(data, size);

	effect->SetName(getFilenameWithoutExt(path).c_str());

	return effect;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
Effect* EffectImplemented::Create( Setting* setting, void* pData, int size, float magnification, const EFK_CHAR* materialPath )
{
	if( pData == NULL || size == 0 ) return NULL;

	EffectImplemented* effect = new EffectImplemented( setting, pData, size );
	if ( !effect->Load( pData, size, magnification, materialPath, ReloadingThreadType::Main) )
	{
		effect->Release();
		effect = NULL;
	}
	return effect;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
::Effekseer::EffectLoader* Effect::CreateEffectLoader(::Effekseer::FileInterface* fileInterface)
{
	return new ::Effekseer::DefaultEffectLoader(fileInterface);
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
EffectImplemented::EffectImplemented( Manager* pManager, void* pData, int size )
	: m_pManager		( (ManagerImplemented*)pManager )
	, m_setting			(NULL)
	, m_reference		( 1 )
	, m_version			( 0 )
	, m_ImageCount		( 0 )
	, m_ImagePaths		( NULL )
	, m_pImages			( NULL )
	, m_normalImageCount(0)
	, m_normalImagePaths(nullptr)
	, m_normalImages(nullptr)
	, m_distortionImageCount(0)
	, m_distortionImagePaths(nullptr)
	, m_distortionImages(nullptr)
	, m_defaultRandomSeed	(-1)

{
	ES_SAFE_ADDREF( m_pManager );

	Culling.Shape = CullingShape::NoneShape;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
EffectImplemented::EffectImplemented( Setting* setting, void* pData, int size )
	: m_pManager		( NULL )
	, m_setting			(setting)
	, m_reference		( 1 )
	, m_version			( 0 )
	, m_ImageCount		( 0 )
	, m_ImagePaths		( NULL )
	, m_pImages			( NULL )
	, m_normalImageCount(0)
	, m_normalImagePaths(nullptr)
	, m_normalImages(nullptr)
	, m_distortionImageCount(0)
	, m_distortionImagePaths(nullptr)
	, m_distortionImages(nullptr)
{
	ES_SAFE_ADDREF( m_setting );
	
	Culling.Shape = CullingShape::NoneShape;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
EffectImplemented::~EffectImplemented()
{
	ResetReloadingBackup();
	Reset();
	SetLoadingParameter(nullptr);

	ES_SAFE_RELEASE( m_setting );
	ES_SAFE_RELEASE( m_pManager );

	ES_SAFE_RELEASE(factory);
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
EffectNode* EffectImplemented::GetRoot() const
{
	return m_pRoot;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
float EffectImplemented::GetMaginification() const
{
	return m_maginification;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
bool EffectImplemented::Load( void* pData, int size, float mag, const EFK_CHAR* materialPath, ReloadingThreadType reloadingThreadType)
{
	if(m_setting != nullptr)
	{
		for (int i = 0; i < m_setting->GetEffectFactoryCount(); i++)
		{
			auto f = m_setting->GetEffectFactory(i);

			if(f->OnCheckIsBinarySupported(pData, size))
			{
				ES_SAFE_ADDREF(f);
				factory = f;
				break;
			}
		}
	}

	if (m_pManager != nullptr)
	{
		for (int i = 0; i < m_pManager->GetSetting()->GetEffectFactoryCount(); i++)
		{
			auto f = m_pManager->GetSetting()->GetEffectFactory(i);

			if (f->OnCheckIsBinarySupported(pData, size))
			{
				ES_SAFE_ADDREF(f);
				factory = f;
				break;
			}
		}
	}

	if (factory == nullptr)
		return false;

	// if reladingThreadType == ReloadingThreadType::Main, this function was regarded as loading function actually

	if (!factory->OnCheckIsBinarySupported(pData, size))
	{
		return false;
	}

	EffekseerPrintDebug("** Create : Effect\n");

	if(!factory->OnLoading(this, pData, size, mag, materialPath))
	{
		return false;
	}

	// save materialPath for reloading
    if (materialPath != nullptr) m_materialPath = materialPath;

	if (factory->GetIsResourcesLoadedAutomatically())
	{
		ReloadResources(pData, size, materialPath);
	}

	return true;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void EffectImplemented::Reset()
{
	UnloadResources();

	for( int i = 0; i < m_ImageCount; i++ )
	{
		if( m_ImagePaths[i] != NULL ) delete [] m_ImagePaths[i];
	}

	m_ImageCount = 0;

	ES_SAFE_DELETE_ARRAY( m_ImagePaths );
	ES_SAFE_DELETE_ARRAY(m_pImages);

	{
		for (int i = 0; i < m_normalImageCount; i++)
		{
			if (m_normalImagePaths[i] != NULL) delete [] m_normalImagePaths[i];
		}

		m_normalImageCount = 0;

		ES_SAFE_DELETE_ARRAY(m_normalImagePaths);
		ES_SAFE_DELETE_ARRAY(m_normalImages);
	}

	{
		for (int i = 0; i < m_distortionImageCount; i++)
		{
			if (m_distortionImagePaths[i] != NULL) delete [] m_distortionImagePaths[i];
		}

		m_distortionImageCount = 0;

		ES_SAFE_DELETE_ARRAY(m_distortionImagePaths);
		ES_SAFE_DELETE_ARRAY(m_distortionImages);
	}

	for( int i = 0; i < m_WaveCount; i++ )
	{
		if( m_WavePaths[i] != NULL ) delete [] m_WavePaths[i];
	}
	m_WaveCount = 0;

	ES_SAFE_DELETE_ARRAY( m_WavePaths );
	ES_SAFE_DELETE_ARRAY( m_pWaves );

	for( int i = 0; i < modelCount_; i++ )
	{
		if( modelPaths_[i] != NULL ) delete [] modelPaths_[i];
	}
	modelCount_ = 0;

	ES_SAFE_DELETE_ARRAY( modelPaths_ );
	ES_SAFE_DELETE_ARRAY( models_ );

	for (int i = 0; i < materialCount_; i++)
	{
		if (materialPaths_[i] != NULL)
			delete[] materialPaths_[i];
	}
	materialCount_ = 0;

	ES_SAFE_DELETE_ARRAY(materialPaths_);
	ES_SAFE_DELETE_ARRAY(materials_);

	ES_SAFE_DELETE( m_pRoot );
}

bool EffectImplemented::IsDyanamicMagnificationValid() const
{
	return GetVersion() >= 8 || GetVersion() < 2;
}

ReferenceObject* EffectImplemented::GetLoadingParameter() const {
	return loadingObject;
}

void EffectImplemented::SetLoadingParameter(ReferenceObject* obj)
{ 
	ES_SAFE_ADDREF(obj);
	ES_SAFE_RELEASE(loadingObject);
	loadingObject = obj;
}

Manager* EffectImplemented::GetManager() const
{
	return m_pManager;	
}

const char16_t* EffectImplemented::GetName() const
{
	return name_.c_str();
}

void EffectImplemented::SetName(const char16_t* name)
{
	name_ = name;
}

Setting* EffectImplemented::GetSetting() const
{
	if(m_setting != NULL) return m_setting;
	return m_pManager->GetSetting();
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
int EffectImplemented::GetVersion() const
{
	return m_version;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
TextureData* EffectImplemented::GetColorImage( int n ) const
{
	return m_pImages[ n ];
}

int32_t EffectImplemented::GetColorImageCount() const
{
	return m_ImageCount;
}

const EFK_CHAR* EffectImplemented::GetColorImagePath(int n) const { return m_ImagePaths[n]; }

TextureData* EffectImplemented::GetNormalImage(int n) const
{
	/* 強制的に互換をとる */
	if (this->m_version <= 8)
	{
		return m_pImages[n];
	}

	return m_normalImages[n];
}

int32_t EffectImplemented::GetNormalImageCount() const
{
	return m_normalImageCount;
}

const EFK_CHAR* EffectImplemented::GetNormalImagePath(int n) const { return m_normalImagePaths[n]; }

TextureData* EffectImplemented::GetDistortionImage(int n) const
{
	/* 強制的に互換をとる */
	if (this->m_version <= 8)
	{
		return m_pImages[n];
	}

	return m_distortionImages[n];
}

int32_t EffectImplemented::GetDistortionImageCount() const
{
	return m_distortionImageCount;
}

const EFK_CHAR* EffectImplemented::GetDistortionImagePath(int n) const { return m_distortionImagePaths[n]; }

void* EffectImplemented::GetWave( int n ) const
{
	return m_pWaves[ n ];
}

int32_t EffectImplemented::GetWaveCount() const
{
	return m_WaveCount;
}

const EFK_CHAR* EffectImplemented::GetWavePath(int n) const { return m_WavePaths[n]; }

void* EffectImplemented::GetModel( int n ) const
{
	return models_[ n ];
}

int32_t EffectImplemented::GetModelCount() const
{
	return modelCount_;
}

const EFK_CHAR* EffectImplemented::GetModelPath(int n) const { return modelPaths_[n]; }

MaterialData* EffectImplemented::GetMaterial(int n) const { return materials_[n]; }

int32_t EffectImplemented::GetMaterialCount() const { return materialCount_; }

const EFK_CHAR* EffectImplemented::GetMaterialPath(int n) const { return materialPaths_[n]; }


bool EffectImplemented::Reload( void* data, int32_t size, const EFK_CHAR* materialPath, ReloadingThreadType reloadingThreadType)
{
	if(m_pManager == NULL ) return false;

	std::array<Manager*, 1> managers;
	managers[0] = m_pManager;

	return Reload( managers.data(), managers.size(), data, size, materialPath, reloadingThreadType);
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
bool EffectImplemented::Reload( const EFK_CHAR* path, const EFK_CHAR* materialPath, ReloadingThreadType reloadingThreadType)
{
	if(m_pManager == NULL ) return false;

	std::array<Manager*, 1> managers;
	managers[0] = m_pManager;

	return Reload(managers.data(), managers.size(), path, materialPath, reloadingThreadType);
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
bool EffectImplemented::Reload( Manager** managers, int32_t managersCount, void* data, int32_t size, const EFK_CHAR* materialPath, ReloadingThreadType reloadingThreadType)
{
	if (!factory->OnCheckIsReloadSupported())
		return false;

	const EFK_CHAR* matPath = materialPath != NULL ? materialPath : m_materialPath.c_str();
	
	int lockCount = 0;

	for( int32_t i = 0; i < managersCount; i++)
	{
		((ManagerImplemented*)managers[i])->BeginReloadEffect( this, lockCount == 0);
		lockCount++;
	}

	// HACK for scale
	auto originalMag = this->GetMaginification() / this->m_maginificationExternal;
	auto originalMagExt = this->m_maginificationExternal;

	isReloadingOnRenderingThread = true;
	Reset();
	Load( data, size, originalMag * originalMagExt, matPath, reloadingThreadType);

	// HACK for scale
	m_maginification = originalMag * originalMagExt;
	m_maginificationExternal = originalMagExt;

	isReloadingOnRenderingThread = false;

	for( int32_t i = 0; i < managersCount; i++)
	{
		lockCount--;
		((ManagerImplemented*)managers[i])->EndReloadEffect( this, lockCount == 0);
	}

	return false;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
bool EffectImplemented::Reload( Manager** managers, int32_t managersCount, const EFK_CHAR* path, const EFK_CHAR* materialPath, ReloadingThreadType reloadingThreadType)
{
	if (!factory->OnCheckIsReloadSupported())
		return false;

	Setting* loader = GetSetting();
	
	EffectLoader* eLoader = loader->GetEffectLoader();
	if( loader == NULL ) return false;

	void* data = NULL;
	int32_t size = 0;

	if( !eLoader->Load( path, data, size ) ) return false;

	EFK_CHAR parentDir[512];
	if( materialPath == NULL )
	{
		GetParentDir(parentDir, path);
		materialPath = parentDir;
	}

	int lockCount = 0;

	for( int32_t i = 0; i < managersCount; i++)
	{
		((ManagerImplemented*)&(managers[i]))->BeginReloadEffect( this, lockCount == 0);
		lockCount++;
	}

	isReloadingOnRenderingThread = true;
	Reset();
	Load( data, size, m_maginificationExternal, materialPath, reloadingThreadType);
	isReloadingOnRenderingThread = false;

	for( int32_t i = 0; i < managersCount; i++)
	{
		lockCount--;
		((ManagerImplemented*)&(managers[i]))->EndReloadEffect( this, lockCount == 0);
	}

	return false;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void EffectImplemented::ReloadResources(const void* data, int32_t size, const EFK_CHAR* materialPath)
{
	UnloadResources();

	const EFK_CHAR* matPath = materialPath != NULL ? materialPath : m_materialPath.c_str();
	
	Setting* loader = GetSetting();

	// reloading on render thread
	if (isReloadingOnRenderingThread)
	{
		assert(reloadingBackup != nullptr);

		for (int32_t ind = 0; ind < m_ImageCount; ind++)
		{
			EFK_CHAR fullPath[512];
			PathCombine(fullPath, matPath, m_ImagePaths[ind]);

			TextureData* value = nullptr;
			if (reloadingBackup->images.Pop(fullPath, value))
			{
				m_pImages[ind] = value;
			}
		}
	
		for (int32_t ind = 0; ind < m_normalImageCount; ind++)
		{
			EFK_CHAR fullPath[512];
			PathCombine(fullPath, matPath, m_normalImagePaths[ind]);
			
			TextureData* value = nullptr;
			if (reloadingBackup->normalImages.Pop(fullPath, value))
			{
				m_normalImages[ind] = value;
			}
		}
				
		for (int32_t ind = 0; ind < m_distortionImageCount; ind++)
		{
			EFK_CHAR fullPath[512];
			PathCombine(fullPath, matPath, m_distortionImagePaths[ind]);
			
			TextureData* value = nullptr;
			if (reloadingBackup->distortionImages.Pop(fullPath, value))
			{
				m_distortionImages[ind] = value;
			}
		}
				
		for (int32_t ind = 0; ind < m_WaveCount; ind++)
		{
			EFK_CHAR fullPath[512];
			PathCombine(fullPath, matPath, m_WavePaths[ind]);

			void* value = nullptr;
			if (reloadingBackup->sounds.Pop(fullPath, value))
			{
				m_pWaves[ind] = value;
			}
		}
		
		
		
		for (int32_t ind = 0; ind < modelCount_; ind++)
		{
			EFK_CHAR fullPath[512];
			PathCombine(fullPath, matPath, modelPaths_[ind]);
			
			void* value = nullptr;
			if (reloadingBackup->models.Pop(fullPath, value))
			{
				models_[ind] = value;
			}
		}

		for (int32_t ind = 0; ind < materialCount_; ind++)
		{
			EFK_CHAR fullPath[512];
			PathCombine(fullPath, matPath, materialPaths_[ind]);

			MaterialData* value = nullptr;
			if (reloadingBackup->materials.Pop(fullPath, value))
			{
				materials_[ind] = value;
			}
		}
			
		return;
	}

	factory->OnLoadingResource(this, data, size, matPath);
}

void EffectImplemented::UnloadResources(const EFK_CHAR* materialPath)
{
	Setting* loader = GetSetting();

	// reloading on render thread
	if (isReloadingOnRenderingThread)
	{
		if (reloadingBackup == nullptr)
		{
			reloadingBackup = std::unique_ptr<EffectReloadingBackup>(new EffectReloadingBackup());
		}

		const EFK_CHAR* matPath = materialPath != nullptr ? materialPath : m_materialPath.c_str();

		for (int32_t ind = 0; ind < m_ImageCount; ind++)
		{
			if (m_pImages[ind] == nullptr) continue;

			EFK_CHAR fullPath[512];
			PathCombine(fullPath, matPath, m_ImagePaths[ind]);
			reloadingBackup->images.Push(fullPath, m_pImages[ind]);
		}

		for (int32_t ind = 0; ind < m_normalImageCount; ind++)
		{
			if (m_normalImages[ind] == nullptr) continue;

			EFK_CHAR fullPath[512];
			PathCombine(fullPath, matPath, m_normalImagePaths[ind]);
			reloadingBackup->normalImages.Push(fullPath, m_normalImages[ind]);
		}

		for (int32_t ind = 0; ind < m_distortionImageCount; ind++)
		{
			if (m_distortionImagePaths[ind] == nullptr) continue;

			EFK_CHAR fullPath[512];
			PathCombine(fullPath, matPath, m_distortionImagePaths[ind]);
			reloadingBackup->distortionImages.Push(fullPath, m_distortionImages[ind]);
		}

		for (int32_t ind = 0; ind < m_WaveCount; ind++)
		{
			if (m_pWaves[ind] == nullptr) continue;

			EFK_CHAR fullPath[512];
			PathCombine(fullPath, matPath, m_WavePaths[ind]);
			reloadingBackup->sounds.Push(fullPath, m_pWaves[ind]);
		}

		for (int32_t ind = 0; ind < modelCount_; ind++)
		{
			if (models_[ind] == nullptr) continue;

			EFK_CHAR fullPath[512];
			PathCombine(fullPath, matPath, modelPaths_[ind]);
			reloadingBackup->models.Push(fullPath, models_[ind]);
		}

		for (int32_t ind = 0; ind < materialCount_; ind++)
		{
			if (materials_[ind] == nullptr)
				continue;

			EFK_CHAR fullPath[512];
			PathCombine(fullPath, matPath, materialPaths_[ind]);
			reloadingBackup->materials.Push(fullPath, materials_[ind]);
		}

		return;
	}
	else
	{
		ResetReloadingBackup();
	}

	factory->OnUnloadingResource(this);
}

void EffectImplemented::UnloadResources()
{
	UnloadResources(nullptr);
}

EffectTerm EffectImplemented::CalculateTerm() const
{ 
	
	EffectTerm effectTerm;
	effectTerm.TermMin = 0;
	effectTerm.TermMax = 0;

	auto root = GetRoot(); 
	EffectInstanceTerm rootTerm;

	std::function<void(EffectNode*, EffectInstanceTerm&)> recurse;
	recurse = [&effectTerm, &recurse](EffectNode* node, EffectInstanceTerm& term) -> void
	{
		for (int i = 0; i < node->GetChildrenCount(); i++)
		{
			auto cterm = node->GetChild(i)->CalculateInstanceTerm(term);
			effectTerm.TermMin = Max(effectTerm.TermMin, cterm.LastInstanceEndMin);
			effectTerm.TermMax = Max(effectTerm.TermMax, cterm.LastInstanceEndMax);

			recurse(node->GetChild(i), cterm);
		}
	};

	recurse(root, rootTerm);

	return effectTerm;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
}




//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------








//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
namespace Effekseer
{

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
static int64_t GetTime(void)
{
	return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

Manager::DrawParameter::DrawParameter()
{ 
	CameraCullingMask = 1; 
}

Manager* Manager::Create( int instance_max, bool autoFlip )
{
	return new ManagerImplemented( instance_max, autoFlip );
}

Matrix43* ManagerImplemented::DrawSet::GetEnabledGlobalMatrix()
{
	if (IsPreupdated)
	{
		InstanceContainer* pContainer = InstanceContainerPointer;
		if (pContainer == nullptr) return nullptr;

		auto firstGroup = pContainer->GetFirstGroup();
		if (firstGroup == nullptr) return nullptr;

		Instance* pInstance = pContainer->GetFirstGroup()->GetFirst();
		if (pInstance == nullptr) return nullptr;

		return &(pInstance->m_GlobalMatrix43);
	}
	else
	{
		return &(GlobalMatrix);
	}
}


void ManagerImplemented::DrawSet::CopyMatrixFromInstanceToRoot()
{
	if (IsPreupdated)
	{
		InstanceContainer* pContainer = InstanceContainerPointer;
		if (pContainer == nullptr) return;

		auto firstGroup = pContainer->GetFirstGroup();
		if (firstGroup == nullptr) return;

		Instance* pInstance = pContainer->GetFirstGroup()->GetFirst();
		if (pInstance == nullptr) return;

		GlobalMatrix = pInstance->m_GlobalMatrix43;
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
Handle ManagerImplemented::AddDrawSet( Effect* effect, InstanceContainer* pInstanceContainer, InstanceGlobal* pGlobalPointer )
{
	Handle Temp = m_NextHandle;

	if( ++m_NextHandle < 0 )
	{
		// オーバーフローしたらリセットする
		m_NextHandle = 0;
	}

	DrawSet drawset( effect, pInstanceContainer, pGlobalPointer );
	drawset.Self = Temp;

	ES_SAFE_ADDREF( effect );

	m_DrawSets[Temp] = drawset;

	return Temp;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void ManagerImplemented::GCDrawSet( bool isRemovingManager )
{
	// インスタンスグループ自体の削除処理
	{
		std::map<Handle,DrawSet>::iterator it = m_RemovingDrawSets[1].begin();
		while( it != m_RemovingDrawSets[1].end() )
		{
			DrawSet& drawset = (*it).second;

			// dispose all instances
			if (drawset.InstanceContainerPointer != nullptr)
			{
				drawset.InstanceContainerPointer->RemoveForcibly(true);
				ReleaseInstanceContainer( drawset.InstanceContainerPointer );
			}

			ES_SAFE_RELEASE( drawset.ParameterPointer );
			ES_SAFE_DELETE( drawset.GlobalPointer );

			if(m_cullingWorld != NULL)
			{
				m_cullingWorld->RemoveObject(drawset.CullingObjectPointer);
				Culling3D::SafeRelease(drawset.CullingObjectPointer);
			}

			m_RemovingDrawSets[1].erase( it++ );
		}
		m_RemovingDrawSets[1].clear();
	}

	{
		std::map<Handle,DrawSet>::iterator it = m_RemovingDrawSets[0].begin();
		while( it != m_RemovingDrawSets[0].end() )
		{
			m_RemovingDrawSets[1][ (*it).first ] = (*it).second;
			m_RemovingDrawSets[0].erase( it++ );
		}
		m_RemovingDrawSets[0].clear();
	}

	{
		std::map<Handle,DrawSet>::iterator it = m_DrawSets.begin();
		while( it != m_DrawSets.end() )
		{
			DrawSet& draw_set = (*it).second;

			// 削除フラグが立っている時
			bool isRemoving = draw_set.IsRemoving;

			// 何も存在しない時
			if( !isRemoving && draw_set.GlobalPointer->GetInstanceCount() == 0 )
			{
				isRemoving = true;
			}

			// ルートのみ存在し、既に新しく生成する見込みがないとき
			if( !isRemoving && draw_set.GlobalPointer->GetInstanceCount() == 1 )
			{
				InstanceContainer* pRootContainer = draw_set.InstanceContainerPointer;
				InstanceGroup* group = pRootContainer->GetFirstGroup();

				if( group )
				{
					Instance* pRootInstance = group->GetFirst();

					if( pRootInstance && pRootInstance->GetState() == INSTANCE_STATE_ACTIVE )
					{
						int maxcreate_count = 0;
						bool canRemoved = true;
						for( int i = 0; i < pRootInstance->m_pEffectNode->GetChildrenCount(); i++ )
						{
							auto child = (EffectNodeImplemented*) pRootInstance->m_pEffectNode->GetChild(i);

							if (pRootInstance->maxGenerationChildrenCount[i] > pRootInstance->m_generatedChildrenCount[i])
							{
								canRemoved = false;
								break;
							}
						}
					
						if (canRemoved)
						{
							// when a sound is not playing.
							if (!GetSoundPlayer() || !GetSoundPlayer()->CheckPlayingTag(draw_set.GlobalPointer))
							{
								isRemoving = true;
							}
						}
					}
				}
			}

			if( isRemoving )
			{
				// 消去処理
				StopEffect( (*it).first );

				if( (*it).second.RemovingCallback != NULL )
				{
					(*it).second.RemovingCallback( this, (*it).first, isRemovingManager );
				}

				m_RemovingDrawSets[0][ (*it).first ] = (*it).second;
				m_DrawSets.erase( it++ );
			}
			else
			{
				++it;
			}			
		}
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
InstanceContainer* ManagerImplemented::CreateInstanceContainer( EffectNode* pEffectNode, InstanceGlobal* pGlobal, bool isRoot, const Matrix43& rootMatrix, Instance* pParent )
{
	if( pooledContainers_.empty() )
	{
		return nullptr;
	}
	InstanceContainer* memory = pooledContainers_.front();
	pooledContainers_.pop();
	InstanceContainer* pContainer = new(memory) InstanceContainer( this, pEffectNode, pGlobal );

	for (int i = 0; i < pEffectNode->GetChildrenCount(); i++)
	{
		pContainer->AddChild(CreateInstanceContainer(pEffectNode->GetChild(i), pGlobal, false, Matrix43(), nullptr));
	}

	if( isRoot )
	{
		pGlobal->SetRootContainer(pContainer);

		InstanceGroup* group = pContainer->CreateInstanceGroup();
		Instance* instance = group->CreateInstance();
		instance->Initialize( NULL, 0, 0, rootMatrix);

		// This group is not generated by an instance, so changed a flag
		group->IsReferencedFromInstance = false;
	}

	return pContainer;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void ManagerImplemented::ReleaseInstanceContainer( InstanceContainer* container )
{
	container->~InstanceContainer();
	pooledContainers_.push( container );
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void* EFK_STDCALL ManagerImplemented::Malloc( unsigned int size )
{
	return (void*)new char*[size];
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void EFK_STDCALL ManagerImplemented::Free( void* p, unsigned int size )
{
	char* pData = (char*)p;
	delete [] pData;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
int EFK_STDCALL ManagerImplemented::Rand()
{
	return rand();
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void ManagerImplemented::ExecuteEvents()
{	
	for (auto& ds : m_DrawSets)
	{
		if( ds.second.GoingToStop )
		{
			InstanceContainer* pContainer = ds.second.InstanceContainerPointer;

			if (pContainer != nullptr)
			{
				pContainer->KillAllInstances(true);
			}

			ds.second.IsRemoving = true;
			if (GetSoundPlayer() != NULL)
			{
				GetSoundPlayer()->StopTag(ds.second.GlobalPointer);
			}
		}

		if(ds.second.GoingToStopRoot )
		{
			InstanceContainer* pContainer = ds.second.InstanceContainerPointer;

			if (pContainer != nullptr)
			{
				pContainer->KillAllInstances(false);
			}
		}
	}
}

ManagerImplemented::ManagerImplemented( int instance_max, bool autoFlip )
	: m_autoFlip	( autoFlip )
	, m_NextHandle	( 0 )
	, m_instance_max	( instance_max )
	, m_setting			( NULL )
	, m_sequenceNumber	( 0 )

	, m_cullingWorld	(NULL)
	, m_culled(false)

	, m_spriteRenderer(NULL)
	, m_ribbonRenderer(NULL)
	, m_ringRenderer(NULL)
	, m_modelRenderer(NULL)
	, m_trackRenderer(NULL)

	, m_soundPlayer(NULL)

	, m_MallocFunc(NULL)
	, m_FreeFunc(NULL)
	, m_randFunc(NULL)
	, m_randMax(0)
{
	m_setting = Setting::Create();

	SetMallocFunc( Malloc );
	SetFreeFunc( Free );
	SetRandFunc( Rand );
	SetRandMax(RAND_MAX);

	m_renderingDrawSets.reserve( 64 );

	int chunk_max = (m_instance_max + InstanceChunk::InstancesOfChunk - 1) / InstanceChunk::InstancesOfChunk;
	reservedChunksBuffer_.reset(new InstanceChunk[chunk_max]);
	for (int i = 0; i < chunk_max; i++)
	{
		pooledChunks_.push(&reservedChunksBuffer_[i]);
	}
	for (auto& chunks : instanceChunks_)
	{
		chunks.reserve(chunk_max);
	}
	std::fill(creatableChunkOffsets_.begin(), creatableChunkOffsets_.end(), 0);

	// Pooling InstanceGroup
	reservedGroupBuffer_.reset( new uint8_t[instance_max * sizeof(InstanceGroup)] );
	for( int i = 0; i < instance_max; i++ )
	{
		pooledGroups_.push( (InstanceGroup*)&reservedGroupBuffer_[i * sizeof(InstanceGroup)] );
	}

	// Pooling InstanceGroup
	reservedContainerBuffer_.reset( new uint8_t[instance_max * sizeof(InstanceGroup)] );
	for( int i = 0; i < instance_max; i++ )
	{
		pooledContainers_.push( (InstanceContainer*)&reservedContainerBuffer_[i * sizeof(InstanceGroup)] );
	}

	m_setting->SetEffectLoader(new DefaultEffectLoader());
	EffekseerPrintDebug("*** Create : Manager\n");
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
ManagerImplemented::~ManagerImplemented()
{
	StopAllEffects();

	ExecuteEvents();

	for( int i = 0; i < 5; i++ )
	{
		GCDrawSet( true );
	}

	//assert( m_reserved_instances.size() == m_instance_max ); 
	//ES_SAFE_DELETE_ARRAY( m_reserved_instances_buffer );

	Culling3D::SafeRelease(m_cullingWorld);

	ES_SAFE_DELETE(m_spriteRenderer);
	ES_SAFE_DELETE(m_ribbonRenderer);
	ES_SAFE_DELETE(m_modelRenderer);
	ES_SAFE_DELETE(m_trackRenderer);
	ES_SAFE_DELETE(m_ringRenderer);

	ES_SAFE_DELETE(m_soundPlayer);

	ES_SAFE_RELEASE( m_setting );
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
Instance* ManagerImplemented::CreateInstance( EffectNode* pEffectNode, InstanceContainer* pContainer, InstanceGroup* pGroup )
{
	int32_t generationNumber = pEffectNode->GetGeneration();
	assert(generationNumber < GenerationsMax);

	auto& chunks = instanceChunks_[generationNumber];

	int32_t offset = creatableChunkOffsets_[generationNumber];

	auto it = std::find_if(chunks.begin() + offset, chunks.end(), [](const InstanceChunk* chunk) { return chunk->IsInstanceCreatable(); });

	creatableChunkOffsets_[generationNumber] = (int32_t)std::distance(chunks.begin(), it);

	if (it != chunks.end())
	{
		auto chunk = *it;
		return chunk->CreateInstance(this, pEffectNode, pContainer, pGroup);
	}

	if (!pooledChunks_.empty())
	{
		auto chunk = pooledChunks_.front();
		pooledChunks_.pop();
		chunks.push_back(chunk);
		return chunk->CreateInstance(this, pEffectNode, pContainer, pGroup);
	}

	return nullptr;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
InstanceGroup* ManagerImplemented::CreateInstanceGroup( EffectNode* pEffectNode, InstanceContainer* pContainer, InstanceGlobal* pGlobal )
{
	if( pooledGroups_.empty() )
	{
		return nullptr;
	}
	InstanceGroup* memory = pooledGroups_.front();
	pooledGroups_.pop();
	return new(memory) InstanceGroup( this, pEffectNode, pContainer, pGlobal );
}

void ManagerImplemented::ReleaseGroup(InstanceGroup* group)
{
	group->~InstanceGroup();
	pooledGroups_.push(group);
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void ManagerImplemented::Destroy()
{
	StopAllEffects();

	ExecuteEvents();

	for( int i = 0; i < 5; i++ )
	{
		GCDrawSet( true );
	}

	Release();
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
uint32_t ManagerImplemented::GetSequenceNumber() const
{
	return m_sequenceNumber;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
MallocFunc ManagerImplemented::GetMallocFunc() const
{
	return m_MallocFunc;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void ManagerImplemented::SetMallocFunc(MallocFunc func)
{
	m_MallocFunc = func;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
FreeFunc ManagerImplemented::GetFreeFunc() const
{
	return m_FreeFunc;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void ManagerImplemented::SetFreeFunc(FreeFunc func)
{
	m_FreeFunc = func;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
RandFunc ManagerImplemented::GetRandFunc() const
{
	return m_randFunc;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void ManagerImplemented::SetRandFunc(RandFunc func)
{
	m_randFunc = func;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
int ManagerImplemented::GetRandMax() const
{
	return m_randMax;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void ManagerImplemented::SetRandMax(int max_)
{
	m_randMax = max_;
}


//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
CoordinateSystem ManagerImplemented::GetCoordinateSystem() const
{
	return m_setting->GetCoordinateSystem();
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void ManagerImplemented::SetCoordinateSystem( CoordinateSystem coordinateSystem )
{
	m_setting->SetCoordinateSystem(coordinateSystem);
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
SpriteRenderer* ManagerImplemented::GetSpriteRenderer()
{
	return m_spriteRenderer;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void ManagerImplemented::SetSpriteRenderer(SpriteRenderer* renderer)
{
	ES_SAFE_DELETE(m_spriteRenderer);
	m_spriteRenderer = renderer;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
RibbonRenderer* ManagerImplemented::GetRibbonRenderer()
{
	return m_ribbonRenderer;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void ManagerImplemented::SetRibbonRenderer(RibbonRenderer* renderer)
{
	ES_SAFE_DELETE(m_ribbonRenderer);
	m_ribbonRenderer = renderer;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
RingRenderer* ManagerImplemented::GetRingRenderer()
{
	return m_ringRenderer;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void ManagerImplemented::SetRingRenderer(RingRenderer* renderer)
{
	ES_SAFE_DELETE(m_ringRenderer);
	m_ringRenderer = renderer;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
ModelRenderer* ManagerImplemented::GetModelRenderer()
{
	return m_modelRenderer;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void ManagerImplemented::SetModelRenderer(ModelRenderer* renderer)
{
	ES_SAFE_DELETE(m_modelRenderer);
	m_modelRenderer = renderer;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
TrackRenderer* ManagerImplemented::GetTrackRenderer()
{
	return m_trackRenderer;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void ManagerImplemented::SetTrackRenderer(TrackRenderer* renderer)
{
	ES_SAFE_DELETE(m_trackRenderer);
	m_trackRenderer = renderer;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
SoundPlayer* ManagerImplemented::GetSoundPlayer()
{
	return m_soundPlayer;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void ManagerImplemented::SetSoundPlayer(SoundPlayer* soundPlayer)
{
	ES_SAFE_DELETE(m_soundPlayer);
	m_soundPlayer = soundPlayer;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
Setting* ManagerImplemented::GetSetting()
{
	return m_setting;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void ManagerImplemented::SetSetting(Setting* setting)
{
	ES_SAFE_RELEASE(m_setting);
	m_setting = setting;
	ES_SAFE_ADDREF(m_setting);
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
EffectLoader* ManagerImplemented::GetEffectLoader()
{
	return m_setting->GetEffectLoader();
}
	
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void ManagerImplemented::SetEffectLoader( EffectLoader* effectLoader )
{
	m_setting->SetEffectLoader(effectLoader);
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
TextureLoader* ManagerImplemented::GetTextureLoader()
{
	return m_setting->GetTextureLoader();
}
	
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void ManagerImplemented::SetTextureLoader( TextureLoader* textureLoader )
{
	m_setting->SetTextureLoader(textureLoader);
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
SoundLoader* ManagerImplemented::GetSoundLoader()
{
	return m_setting->GetSoundLoader();
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void ManagerImplemented::SetSoundLoader( SoundLoader* soundLoader )
{
	m_setting->SetSoundLoader(soundLoader);
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
ModelLoader* ManagerImplemented::GetModelLoader()
{
	return m_setting->GetModelLoader();
}
	
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void ManagerImplemented::SetModelLoader( ModelLoader* modelLoader )
{
	m_setting->SetModelLoader(modelLoader);
}

MaterialLoader* ManagerImplemented::GetMaterialLoader() { return m_setting->GetMaterialLoader(); }

void ManagerImplemented::SetMaterialLoader(MaterialLoader* loader) { m_setting->SetMaterialLoader(loader); }

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void ManagerImplemented::StopEffect( Handle handle )
{
	if( m_DrawSets.count( handle ) > 0 )
	{
		DrawSet& drawSet = m_DrawSets[handle];
		drawSet.GoingToStop = true;
		drawSet.IsRemoving = true;
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void ManagerImplemented::StopAllEffects()
{
	std::map<Handle,DrawSet>::iterator it = m_DrawSets.begin();
	while( it != m_DrawSets.end() )
	{
		(*it).second.GoingToStop = true;
		(*it).second.IsRemoving = true;
		++it;		
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void ManagerImplemented::StopRoot( Handle handle )
{
	if( m_DrawSets.count( handle ) > 0 )
	{
		m_DrawSets[handle].GoingToStopRoot = true;
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void ManagerImplemented::StopRoot( Effect* effect )
{
	std::map<Handle,DrawSet>::iterator it = m_DrawSets.begin();
	std::map<Handle,DrawSet>::iterator it_end = m_DrawSets.end();

	while( it != it_end )
	{
		if( (*it).second.ParameterPointer == effect )
		{
			(*it).second.GoingToStopRoot = true;
		}
		++it;
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
bool ManagerImplemented::Exists( Handle handle )
{
	if( m_DrawSets.count( handle ) > 0 )
	{
		// always exists before update
		if (!m_DrawSets[handle].IsPreupdated) return true;

		if( m_DrawSets[handle].IsRemoving ) return false;
		return true;
	}
	return false;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
int32_t ManagerImplemented::GetInstanceCount( Handle handle )
{
	if( m_DrawSets.count( handle ) > 0 )
	{
		return m_DrawSets[handle].GlobalPointer->GetInstanceCount();
	}
	return 0;
}

int32_t ManagerImplemented::GetTotalInstanceCount() const
{
	int32_t instanceCount = 0;
	for (auto pair : m_DrawSets)
	{
		const DrawSet& drawSet = pair.second;
		instanceCount += drawSet.GlobalPointer->GetInstanceCount();
	}
	return instanceCount;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
Matrix43 ManagerImplemented::GetMatrix( Handle handle )
{
	if( m_DrawSets.count( handle ) > 0 )
	{
		DrawSet& drawSet = m_DrawSets[handle];

		auto mat = drawSet.GetEnabledGlobalMatrix();

		if (mat != nullptr)
		{
			return *mat;
		}
	}

	return Matrix43();
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void ManagerImplemented::SetMatrix( Handle handle, const Matrix43& mat )
{
	if( m_DrawSets.count( handle ) > 0 )
	{
		DrawSet& drawSet = m_DrawSets[handle];
		auto mat_ = drawSet.GetEnabledGlobalMatrix();

		if (mat_ != nullptr)
		{
			(*mat_) = mat;
			drawSet.CopyMatrixFromInstanceToRoot();
			drawSet.IsParameterChanged = true;
		}
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
Vector3D ManagerImplemented::GetLocation( Handle handle )
{
	Vector3D location;

	if( m_DrawSets.count( handle ) > 0 )
	{
		DrawSet& drawSet = m_DrawSets[handle];
		auto mat_ = drawSet.GetEnabledGlobalMatrix();

		if (mat_ != nullptr)
		{
			location.X = mat_->Value[3][0];
			location.Y = mat_->Value[3][1];
			location.Z = mat_->Value[3][2];
		}
	}

	return location;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void ManagerImplemented::SetLocation( Handle handle, float x, float y, float z )
{
	if( m_DrawSets.count( handle ) > 0 )
	{
		DrawSet& drawSet = m_DrawSets[handle];
		auto mat_ = drawSet.GetEnabledGlobalMatrix();

		if (mat_ != nullptr)
		{
			mat_->Value[3][0] = x;
			mat_->Value[3][1] = y;
			mat_->Value[3][2] = z;

			drawSet.CopyMatrixFromInstanceToRoot();
			drawSet.IsParameterChanged = true;
		}
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void ManagerImplemented::SetLocation( Handle handle, const Vector3D& location )
{
	SetLocation( handle, location.X, location.Y, location.Z );
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void ManagerImplemented::AddLocation( Handle handle, const Vector3D& location )
{
	if( m_DrawSets.count( handle ) > 0 )
	{
		DrawSet& drawSet = m_DrawSets[handle];
		auto mat_ = drawSet.GetEnabledGlobalMatrix();

		if (mat_ != nullptr)
		{
			mat_->Value[3][0] += location.X;
			mat_->Value[3][1] += location.Y;
			mat_->Value[3][2] += location.Z;
			drawSet.CopyMatrixFromInstanceToRoot();
			drawSet.IsParameterChanged = true;
		}
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void ManagerImplemented::SetRotation( Handle handle, float x, float y, float z )
{
	if( m_DrawSets.count( handle ) > 0 )
	{
		DrawSet& drawSet = m_DrawSets[handle];

		auto mat_ = drawSet.GetEnabledGlobalMatrix();

		if (mat_ != nullptr)
		{
			Matrix43 MatRotX, MatRotY, MatRotZ;

			MatRotX.RotationX(x);
			MatRotY.RotationY(y);
			MatRotZ.RotationZ(z);

			Matrix43 r;
			Vector3D s, t;

			mat_->GetSRT(s, r, t);

			r.Indentity();
			Matrix43::Multiple(r, r, MatRotZ);
			Matrix43::Multiple(r, r, MatRotX);
			Matrix43::Multiple(r, r, MatRotY);

			mat_->SetSRT(s, r, t);

			drawSet.CopyMatrixFromInstanceToRoot();
			drawSet.IsParameterChanged = true;
		}		
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void ManagerImplemented::SetRotation( Handle handle, const Vector3D& axis, float angle )
{
	if( m_DrawSets.count( handle ) > 0 )
	{
		DrawSet& drawSet = m_DrawSets[handle];

		auto mat_ = drawSet.GetEnabledGlobalMatrix();

		if (mat_ != nullptr)
		{
			Matrix43 r;
			Vector3D s, t;

			mat_->GetSRT(s, r, t);

			r.RotationAxis(axis, angle);

			mat_->SetSRT(s, r, t);

			drawSet.CopyMatrixFromInstanceToRoot();
			drawSet.IsParameterChanged = true;
		}
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void ManagerImplemented::SetScale( Handle handle, float x, float y, float z )
{
	if( m_DrawSets.count( handle ) > 0 )
	{
		DrawSet& drawSet = m_DrawSets[handle];

		auto mat_ = drawSet.GetEnabledGlobalMatrix();

		if (mat_ != nullptr)
		{
			Matrix43 r;
			Vector3D s, t;

			mat_->GetSRT(s, r, t);

			s.X = x;
			s.Y = y;
			s.Z = z;

			mat_->SetSRT(s, r, t);

			drawSet.CopyMatrixFromInstanceToRoot();
			drawSet.IsParameterChanged = true;
		}
	}
}

void ManagerImplemented::SetAllColor(Handle handle, Color color)
{
	if (m_DrawSets.count(handle) > 0)
	{
		auto& drawSet = m_DrawSets[handle];

		drawSet.GlobalPointer->IsGlobalColorSet = true;
		drawSet.GlobalPointer->GlobalColor = color;
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void ManagerImplemented::SetTargetLocation( Handle handle, float x, float y, float z )
{
	SetTargetLocation( handle, Vector3D( x, y, z ) );
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void ManagerImplemented::SetTargetLocation( Handle handle, const Vector3D& location )
{
	if( m_DrawSets.count( handle ) > 0 )
	{
		DrawSet& drawSet = m_DrawSets[handle];

		InstanceGlobal* instanceGlobal = drawSet.GlobalPointer;
		instanceGlobal->SetTargetLocation( location );

		drawSet.IsParameterChanged = true;
	}
}

float ManagerImplemented::GetDynamicInput(Handle handle, int32_t index) 
{
	auto it = m_DrawSets.find(handle);
	if (it != m_DrawSets.end())
	{
		auto globalPtr = it->second.GlobalPointer;
		if (index < 0 || globalPtr->dynamicInputParameters.size() <= index)
			return 0.0f;

		return globalPtr->dynamicInputParameters[index];
	}

	return 0.0f;
}

void ManagerImplemented::SetDynamicInput(Handle handle, int32_t index, float value) {
	if (m_DrawSets.count(handle) > 0)
	{
		DrawSet& drawSet = m_DrawSets[handle];

		InstanceGlobal* instanceGlobal = drawSet.GlobalPointer;

		if (index < 0 || instanceGlobal->dynamicInputParameters.size() <= index)
			return;

		instanceGlobal->dynamicInputParameters[index] = value;

		drawSet.IsParameterChanged = true;
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
Matrix43 ManagerImplemented::GetBaseMatrix( Handle handle )
{
	if( m_DrawSets.count( handle ) > 0 )
	{
		return m_DrawSets[handle].BaseMatrix;
	}

	return Matrix43();
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void ManagerImplemented::SetBaseMatrix( Handle handle, const Matrix43& mat )
{
	if( m_DrawSets.count( handle ) > 0 )
	{
		m_DrawSets[handle].BaseMatrix = mat;
		m_DrawSets[handle].DoUseBaseMatrix = true;
		m_DrawSets[handle].IsParameterChanged = true;
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void ManagerImplemented::SetRemovingCallback( Handle handle, EffectInstanceRemovingCallback callback )
{
	if( m_DrawSets.count( handle ) > 0 )
	{
		m_DrawSets[handle].RemovingCallback = callback;
	}
}

bool ManagerImplemented::GetShown(Handle handle)
{
	if (m_DrawSets.count(handle) > 0)
	{
		return m_DrawSets[handle].IsShown;
	}

	return false;
}

void ManagerImplemented::SetShown( Handle handle, bool shown )
{
	if( m_DrawSets.count( handle ) > 0 )
	{
		m_DrawSets[handle].IsShown = shown;
	}
}

bool ManagerImplemented::GetPaused(Handle handle)
{
	if (m_DrawSets.count(handle) > 0)
	{
		return m_DrawSets[handle].IsPaused;
	}

	return false;
}

void ManagerImplemented::SetPaused( Handle handle, bool paused )
{
	if( m_DrawSets.count( handle ) > 0 )
	{
		m_DrawSets[handle].IsPaused = paused;
	}
}

void ManagerImplemented::SetPausedToAllEffects(bool paused)
{
	std::map<Handle, DrawSet>::iterator it = m_DrawSets.begin();
	while (it != m_DrawSets.end())
	{
		(*it).second.IsPaused = paused;
		++it;
	}
}

int ManagerImplemented::GetLayer(Handle handle)
{
	if (m_DrawSets.count(handle) > 0)
	{
		return m_DrawSets[handle].Layer;
	}
	return 0;
}

void ManagerImplemented::SetLayer(Handle handle, int32_t layer)
{
	if (m_DrawSets.count(handle) > 0)
	{
		m_DrawSets[handle].Layer = layer;
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
float ManagerImplemented::GetSpeed(Handle handle) const
{
	auto it = m_DrawSets.find(handle);
	if (it == m_DrawSets.end()) return 0.0f;
	return it->second.Speed;
}


void ManagerImplemented::SetSpeed( Handle handle, float speed )
{
	if( m_DrawSets.count( handle ) > 0 )
	{
		m_DrawSets[handle].Speed = speed;
		m_DrawSets[handle].IsParameterChanged = true;
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void ManagerImplemented::SetAutoDrawing( Handle handle, bool autoDraw )
{
	if( m_DrawSets.count( handle ) > 0 )
	{
		m_DrawSets[handle].IsAutoDrawing = autoDraw;
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void ManagerImplemented::Flip()
{
	if( !m_autoFlip )
	{
		m_renderingMutex.lock();
	}

	// execute preupdate
	for (auto& drawSet : m_DrawSets)
	{
		Preupdate(drawSet.second);
	}

	ExecuteEvents();

	// DrawSet削除処理
	GCDrawSet( false );

	m_renderingDrawSets.clear();
	m_renderingDrawSetMaps.clear();

	/* カリング生成 */
	if( cullingNext.SizeX != cullingCurrent.SizeX ||
		cullingNext.SizeY != cullingCurrent.SizeY ||
		cullingNext.SizeZ != cullingCurrent.SizeZ ||
		cullingNext.LayerCount != cullingCurrent.LayerCount)
	{
		Culling3D::SafeRelease(m_cullingWorld);
		
		std::map<Handle,DrawSet>::iterator it = m_DrawSets.begin();
		std::map<Handle,DrawSet>::iterator it_end = m_DrawSets.end();
		while( it != it_end )
		{
			DrawSet& ds = (*it).second;
			Culling3D::SafeRelease(ds.CullingObjectPointer);
			++it;
		}

		m_cullingWorld = Culling3D::World::Create(
			cullingNext.SizeX,
			cullingNext.SizeY,
			cullingNext.SizeZ,
			cullingNext.LayerCount);

		cullingCurrent = cullingNext;
	}

	{
		std::map<Handle,DrawSet>::iterator it = m_DrawSets.begin();
		std::map<Handle,DrawSet>::iterator it_end = m_DrawSets.end();
		
		while( it != it_end )
		{
			DrawSet& ds = (*it).second;
			EffectImplemented* effect = (EffectImplemented*)ds.ParameterPointer;

			if(ds.IsParameterChanged)
			{
				if(m_cullingWorld != NULL)
				{
					if(ds.CullingObjectPointer == NULL)
					{
						ds.CullingObjectPointer = Culling3D::Object::Create();
						if(effect->Culling.Shape == CullingShape::Sphere)
						{
							ds.CullingObjectPointer->ChangeIntoSphere(0.0f);
						}

						if (effect->Culling.Shape == CullingShape::NoneShape)
						{
							ds.CullingObjectPointer->ChangeIntoAll();
						}
					}

					InstanceContainer* pContainer = ds.InstanceContainerPointer;
					Instance* pInstance = pContainer->GetFirstGroup()->GetFirst();

					Vector3D pos(
						ds.CullingObjectPointer->GetPosition().X,
						ds.CullingObjectPointer->GetPosition().Y,
						ds.CullingObjectPointer->GetPosition().Z);

					Matrix43 pos_;
					pos_.Translation(pos.X, pos.Y, pos.Z);

					Matrix43::Multiple(pos_, pos_,  pInstance->m_GlobalMatrix43);

					if(ds.DoUseBaseMatrix)
					{
						Matrix43::Multiple(pos_, pos_,  ds.BaseMatrix);
					}

					Culling3D::Vector3DF position;
					position.X = pos_.Value[3][0];
					position.Y = pos_.Value[3][1];
					position.Z = pos_.Value[3][2];
					ds.CullingObjectPointer->SetPosition(position);

					if(effect->Culling.Shape == CullingShape::Sphere)
					{
						float radius = effect->Culling.Sphere.Radius;

						{
							Vector3D s;
							pInstance->GetGlobalMatrix43().GetScale(s);
						
							radius = radius * sqrt(s.X * s.X + s.Y * s.Y + s.Z * s.Z);
						}

						if(ds.DoUseBaseMatrix)
						{
							Vector3D s;
							ds.BaseMatrix.GetScale(s);
						
							radius = radius * sqrt(s.X * s.X + s.Y * s.Y + s.Z * s.Z);
						}

						ds.CullingObjectPointer->ChangeIntoSphere(radius);
					}

					m_cullingWorld->AddObject(ds.CullingObjectPointer);
				}
				(*it).second.IsParameterChanged = false;
			}

			m_renderingDrawSets.push_back( (*it).second );
			m_renderingDrawSetMaps[(*it).first] = (*it).second;
			++it;
		}

		if(m_cullingWorld != NULL)
		{
			for(size_t i = 0; i < m_renderingDrawSets.size(); i++)
			{
				m_renderingDrawSets[i].CullingObjectPointer->SetUserData(&(m_renderingDrawSets[i]));
			}
		}
	}

	m_culledObjects.clear();
	m_culledObjectSets.clear();
	m_culled = false;

	if( !m_autoFlip )
	{
		m_renderingMutex.unlock();
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void ManagerImplemented::Update( float deltaFrame )
{
	// start to measure time
	int64_t beginTime = ::Effekseer::GetTime();

	BeginUpdate();

	for (auto& chunks : instanceChunks_)
	{
		for (auto chunk : chunks)
		{
			chunk->UpdateInstances(deltaFrame);
		}
	}

	for (auto& drawSet : m_DrawSets)
	{
		UpdateHandle(drawSet.second, deltaFrame);
	}

	EndUpdate();

	// end to measure time
	m_updateTime = (int)(Effekseer::GetTime() - beginTime);
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void ManagerImplemented::BeginUpdate()
{
	m_renderingMutex.lock();
	m_isLockedWithRenderingMutex = true;

	if( m_autoFlip )
	{
		Flip();
	}

	m_sequenceNumber++;

	std::fill(creatableChunkOffsets_.begin(), creatableChunkOffsets_.end(), 0);
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void ManagerImplemented::EndUpdate()
{
	for (auto& chunks : instanceChunks_)
	{
		auto first = chunks.begin();
		auto last = chunks.end();
		while (first != last)
		{
			auto it = std::find_if(first, last, [](const InstanceChunk* chunk) { return chunk->GetAliveCount() == 0; });
			if (it != last)
			{
				pooledChunks_.push(*it);
				if (it != last - 1)
					*it = *(last - 1);
				last--;
			}
			first = it;
		}
		chunks.erase(last, chunks.end());
	}

	m_renderingMutex.unlock();
	m_isLockedWithRenderingMutex = false;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void ManagerImplemented::UpdateHandle( Handle handle, float deltaFrame )
{
	auto it = m_DrawSets.find( handle );
	if( it != m_DrawSets.end() )
	{
		DrawSet& drawSet = it->second;

		for (auto& chunks : instanceChunks_)
		{
			for (auto chunk : chunks)
			{
				chunk->UpdateInstancesByInstanceGlobal(drawSet.GlobalPointer, deltaFrame);
			}
		}

		UpdateHandle( drawSet, deltaFrame );
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void ManagerImplemented::UpdateHandle( DrawSet& drawSet, float deltaFrame )
{
	// calculate dynamic parameters
	auto e = static_cast<EffectImplemented*>(drawSet.ParameterPointer);
	assert(e != nullptr);
	assert(drawSet.GlobalPointer->dynamicEqResults.size() >= e->dynamicEquation.size());

	std::array<float, 1> globals;
	globals[0] = drawSet.GlobalPointer->GetUpdatedFrame() / 60.0f;

	for (size_t i = 0; i < e->dynamicEquation.size(); i++)
	{
		if (e->dynamicEquation[i].GetRunningPhase() != InternalScript::RunningPhaseType::Global)
			continue;

		drawSet.GlobalPointer->dynamicEqResults[i] =
			e->dynamicEquation[i].Execute(
				drawSet.GlobalPointer->dynamicInputParameters,
				globals,
				std::array<float, 5>(),
				InstanceGlobal::Rand, 
				InstanceGlobal::RandSeed, 
				drawSet.GlobalPointer);
	}

	if (!drawSet.IsPreupdated)
	{
		Preupdate(drawSet);
	}

	float df = drawSet.IsPaused ? 0 : deltaFrame * drawSet.Speed;

	drawSet.InstanceContainerPointer->Update( true, df, drawSet.IsShown );

	if( drawSet.DoUseBaseMatrix )
	{
		drawSet.InstanceContainerPointer->SetBaseMatrix( true, drawSet.BaseMatrix );
	}

	drawSet.GlobalPointer->AddUpdatedFrame( df );
}


void ManagerImplemented::Preupdate(DrawSet& drawSet)
{
	if (drawSet.IsPreupdated) return;

	// Create an instance through a container
	InstanceContainer* pContainer = CreateInstanceContainer(drawSet.ParameterPointer->GetRoot(), drawSet.GlobalPointer, true, drawSet.GlobalMatrix, NULL);

	drawSet.InstanceContainerPointer = pContainer;

	for (int32_t frame = 0; frame < drawSet.StartFrame; frame++)
	{
		UpdateHandle(drawSet, 1);
	}

	drawSet.IsPreupdated = true;
}

bool ManagerImplemented::IsClippedWithDepth(DrawSet& drawSet, InstanceContainer* container, const Manager::DrawParameter& drawParameter) {
	
	// don't use this parameter
	if (container->m_pEffectNode->DepthValues.DepthParameter.DepthClipping > FLT_MAX / 10)
		return false;

	Vector3D pos;
	drawSet.GlobalMatrix.GetTranslation(pos);
	auto distance = Vector3D::Dot(drawParameter.CameraPosition - pos, drawParameter.CameraDirection);
	if (container->m_pEffectNode->DepthValues.DepthParameter.DepthClipping < distance)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void ManagerImplemented::Draw(const Manager::DrawParameter& drawParameter)
{
	std::lock_guard<std::mutex> lock(m_renderingMutex);

	// 開始時間を記録
	int64_t beginTime = ::Effekseer::GetTime();

	if(m_culled)
	{
		for( size_t i = 0; i < m_culledObjects.size(); i++ )
		{
			DrawSet& drawSet = *m_culledObjects[i];

			if( drawSet.IsShown && drawSet.IsAutoDrawing && ((drawParameter.CameraCullingMask & (1 << drawSet.Layer)) != 0))
			{
				if (drawSet.GlobalPointer->RenderedInstanceContainers.size() > 0)
				{
					for (auto& c : drawSet.GlobalPointer->RenderedInstanceContainers)
					{
						if (IsClippedWithDepth(drawSet, c, drawParameter))
							continue;

						c->Draw(false);
					}
				}
				else
				{
					drawSet.InstanceContainerPointer->Draw( true );
				}
			}
		}
	}
	else
	{
		for( size_t i = 0; i < m_renderingDrawSets.size(); i++ )
		{
			DrawSet& drawSet = m_renderingDrawSets[i];

			if( drawSet.IsShown && drawSet.IsAutoDrawing && ((drawParameter.CameraCullingMask & (1 << drawSet.Layer)) != 0))
			{
				if (drawSet.GlobalPointer->RenderedInstanceContainers.size() > 0)
				{
					for (auto& c : drawSet.GlobalPointer->RenderedInstanceContainers)
					{
						if (IsClippedWithDepth(drawSet, c, drawParameter))
							continue;

						c->Draw(false);
					}
				}
				else
				{
					drawSet.InstanceContainerPointer->Draw(true);
				}
			}
		}
	}

	// 経過時間を計算
	m_drawTime = (int)(Effekseer::GetTime() - beginTime);
}

void ManagerImplemented::DrawBack(const Manager::DrawParameter& drawParameter)
{
	std::lock_guard<std::mutex> lock(m_renderingMutex);
	
	// 開始時間を記録
	int64_t beginTime = ::Effekseer::GetTime();

	if (m_culled)
	{
		for (size_t i = 0; i < m_culledObjects.size(); i++)
		{
			DrawSet& drawSet = *m_culledObjects[i];

			if (drawSet.IsShown && drawSet.IsAutoDrawing && ((drawParameter.CameraCullingMask & (1 << drawSet.Layer)) != 0))
			{
				auto e = (EffectImplemented*)drawSet.ParameterPointer;
				for (int32_t j = 0; j < e->renderingNodesThreshold; j++)
				{
					if (IsClippedWithDepth(drawSet, drawSet.GlobalPointer->RenderedInstanceContainers[j], drawParameter))
						continue;

					drawSet.GlobalPointer->RenderedInstanceContainers[j]->Draw(false);
				}
			}
		}
	}
	else
	{
		for (size_t i = 0; i < m_renderingDrawSets.size(); i++)
		{
			DrawSet& drawSet = m_renderingDrawSets[i];

			if (drawSet.IsShown && drawSet.IsAutoDrawing && ((drawParameter.CameraCullingMask & (1 << drawSet.Layer)) != 0))
			{
				auto e = (EffectImplemented*)drawSet.ParameterPointer;
				for (int32_t j = 0; j < e->renderingNodesThreshold; j++)
				{
					if (IsClippedWithDepth(drawSet, drawSet.GlobalPointer->RenderedInstanceContainers[j], drawParameter))
						continue;

					drawSet.GlobalPointer->RenderedInstanceContainers[j]->Draw(false);
				}
			}
		}
	}

	// 経過時間を計算
	m_drawTime = (int)(Effekseer::GetTime() - beginTime);
}

void ManagerImplemented::DrawFront(const Manager::DrawParameter& drawParameter)
{
	std::lock_guard<std::mutex> lock(m_renderingMutex);

	// 開始時間を記録
	int64_t beginTime = ::Effekseer::GetTime();

	if (m_culled)
	{
		for (size_t i = 0; i < m_culledObjects.size(); i++)
		{
			DrawSet& drawSet = *m_culledObjects[i];

			if (drawSet.IsShown && drawSet.IsAutoDrawing && ((drawParameter.CameraCullingMask & (1 << drawSet.Layer)) != 0))
			{
				if (drawSet.GlobalPointer->RenderedInstanceContainers.size() > 0)
				{
					auto e = (EffectImplemented*)drawSet.ParameterPointer;
					for (size_t j = e->renderingNodesThreshold; j < drawSet.GlobalPointer->RenderedInstanceContainers.size(); j++)
					{
						if (IsClippedWithDepth(drawSet, drawSet.GlobalPointer->RenderedInstanceContainers[j], drawParameter))
							continue;

						drawSet.GlobalPointer->RenderedInstanceContainers[j]->Draw(false);
					}
				}
				else
				{
					drawSet.InstanceContainerPointer->Draw(true);
				}
			}
		}
	}
	else
	{
		for (size_t i = 0; i < m_renderingDrawSets.size(); i++)
		{
			DrawSet& drawSet = m_renderingDrawSets[i];

			if (drawSet.IsShown && drawSet.IsAutoDrawing && ((drawParameter.CameraCullingMask & (1 << drawSet.Layer)) != 0))
			{
				if (drawSet.GlobalPointer->RenderedInstanceContainers.size() > 0)
				{
					auto e = (EffectImplemented*)drawSet.ParameterPointer;
					for (size_t j = e->renderingNodesThreshold; j < drawSet.GlobalPointer->RenderedInstanceContainers.size(); j++)
					{
						if (IsClippedWithDepth(drawSet, drawSet.GlobalPointer->RenderedInstanceContainers[j], drawParameter))
							continue;

						drawSet.GlobalPointer->RenderedInstanceContainers[j]->Draw(false);
					}
				}
				else
				{
					drawSet.InstanceContainerPointer->Draw(true);
				}
			}
		}
	}

	// 経過時間を計算
	m_drawTime = (int)(Effekseer::GetTime() - beginTime);
}

Handle ManagerImplemented::Play( Effect* effect, float x, float y, float z )
{
	return Play(effect, Vector3D(x, y, z), 0);
}

Handle ManagerImplemented::Play(Effect* effect, const Vector3D& position, int32_t startFrame)
{
	if (effect == nullptr) return -1;

	auto e = (EffectImplemented*)effect;

	// Create root
	InstanceGlobal* pGlobal = new InstanceGlobal();
	
	if (e->m_defaultRandomSeed >= 0)
	{
		pGlobal->SetSeed(e->m_defaultRandomSeed);
	}
	else
	{
		pGlobal->SetSeed(GetRandFunc()());
	}

	pGlobal->dynamicInputParameters = e->defaultDynamicInputs;

	pGlobal->RenderedInstanceContainers.resize(e->renderingNodesCount);
	for (size_t i = 0; i < pGlobal->RenderedInstanceContainers.size(); i++)
	{
		pGlobal->RenderedInstanceContainers[i] = nullptr;
	}

	// create a dateSet without an instance
	// an instance is created in Preupdate because effects need to show instances without update(0 frame)
	Handle handle = AddDrawSet(effect, nullptr, pGlobal);

	auto& drawSet = m_DrawSets[handle];

	drawSet.GlobalMatrix.Indentity();
	drawSet.GlobalMatrix.Value[3][0] = position.X;
	drawSet.GlobalMatrix.Value[3][1] = position.Y;
	drawSet.GlobalMatrix.Value[3][2] = position.Z;

	drawSet.IsParameterChanged = true;
	drawSet.StartFrame = startFrame;

	return handle;
}

int ManagerImplemented::GetCameraCullingMaskToShowAllEffects()
{
	int mask = 0;

	for (auto& ds : m_DrawSets)
	{
		auto layer = 1 << ds.second.Layer;
		mask |= layer;
	}

	return mask;
}

void ManagerImplemented::DrawHandle(Handle handle, const Manager::DrawParameter& drawParameter)
{
	std::lock_guard<std::mutex> lock(m_renderingMutex);

	std::map<Handle,DrawSet>::iterator it = m_renderingDrawSetMaps.find( handle );
	if( it != m_renderingDrawSetMaps.end() )
	{
		DrawSet& drawSet = it->second;

		if(m_culled)
		{
			if(m_culledObjectSets.find(drawSet.Self) !=m_culledObjectSets.end())
			{
				if( drawSet.IsShown )
				{
					if (drawSet.GlobalPointer->RenderedInstanceContainers.size() > 0)
					{
						for (auto& c : drawSet.GlobalPointer->RenderedInstanceContainers)
						{
							if (IsClippedWithDepth(drawSet, c, drawParameter))
								continue;

							c->Draw(false);
						}
					}
					else
					{
						drawSet.InstanceContainerPointer->Draw(true);
					}
				}
			}
		}
		else
		{
			if( drawSet.IsShown )
			{
				if (drawSet.GlobalPointer->RenderedInstanceContainers.size() > 0)
				{
					for (auto& c : drawSet.GlobalPointer->RenderedInstanceContainers)
					{
						if (IsClippedWithDepth(drawSet, c, drawParameter))
							continue;

						c->Draw(false);
					}
				}
				else
				{
					drawSet.InstanceContainerPointer->Draw(true);
				}
			}
		}
	}
}

void ManagerImplemented::DrawHandleBack(Handle handle, const Manager::DrawParameter& drawParameter)
{
	std::lock_guard<std::mutex> lock(m_renderingMutex);

	std::map<Handle, DrawSet>::iterator it = m_renderingDrawSetMaps.find(handle);
	if (it != m_renderingDrawSetMaps.end())
	{
		DrawSet& drawSet = it->second;

		if (m_culled)
		{
			if (m_culledObjectSets.find(drawSet.Self) != m_culledObjectSets.end())
			{
				if (drawSet.IsShown)
				{
					auto e = (EffectImplemented*)drawSet.ParameterPointer;
					for (int32_t i = 0; i < e->renderingNodesThreshold; i++)
					{
						if (IsClippedWithDepth(drawSet, drawSet.GlobalPointer->RenderedInstanceContainers[i], drawParameter))
							continue;

						drawSet.GlobalPointer->RenderedInstanceContainers[i]->Draw(false);
					}
				}
			}
		}
		else
		{
			if (drawSet.IsShown)
			{
				auto e = (EffectImplemented*)drawSet.ParameterPointer;
				for (int32_t i = 0; i < e->renderingNodesThreshold; i++)
				{
					if (IsClippedWithDepth(drawSet, drawSet.GlobalPointer->RenderedInstanceContainers[i], drawParameter))
						continue;

					drawSet.GlobalPointer->RenderedInstanceContainers[i]->Draw(false);
				}
			}
		}
	}
}

void ManagerImplemented::DrawHandleFront(Handle handle, const Manager::DrawParameter& drawParameter)
{
	std::lock_guard<std::mutex> lock(m_renderingMutex);

	std::map<Handle, DrawSet>::iterator it = m_renderingDrawSetMaps.find(handle);
	if (it != m_renderingDrawSetMaps.end())
	{
		DrawSet& drawSet = it->second;

		if (m_culled)
		{
			if (m_culledObjectSets.find(drawSet.Self) != m_culledObjectSets.end())
			{
				if (drawSet.IsShown)
				{
					if (drawSet.GlobalPointer->RenderedInstanceContainers.size() > 0)
					{
						auto e = (EffectImplemented*)drawSet.ParameterPointer;
						for (size_t i = e->renderingNodesThreshold; i < drawSet.GlobalPointer->RenderedInstanceContainers.size(); i++)
						{
							if (IsClippedWithDepth(drawSet, drawSet.GlobalPointer->RenderedInstanceContainers[i], drawParameter))
								continue;

							drawSet.GlobalPointer->RenderedInstanceContainers[i]->Draw(false);
						}
					}
					else
					{
						drawSet.InstanceContainerPointer->Draw(true);
					}
				}
			}
		}
		else
		{
			if (drawSet.IsShown)
			{
				if (drawSet.GlobalPointer->RenderedInstanceContainers.size() > 0)
				{
					auto e = (EffectImplemented*)drawSet.ParameterPointer;
					for (size_t i = e->renderingNodesThreshold; i < drawSet.GlobalPointer->RenderedInstanceContainers.size(); i++)
					{
						if (IsClippedWithDepth(drawSet, drawSet.GlobalPointer->RenderedInstanceContainers[i], drawParameter))
							continue;

						drawSet.GlobalPointer->RenderedInstanceContainers[i]->Draw(false);
					}
				}
				else
				{
					drawSet.InstanceContainerPointer->Draw(true);
				}
			}
		}
	}
}

int ManagerImplemented::GetUpdateTime() const 
{
	return m_updateTime; 
};

int ManagerImplemented::GetDrawTime() const 
{
	return m_drawTime; 
};

int32_t ManagerImplemented::GetRestInstancesCount() const
{
	return static_cast<int32_t>(pooledChunks_.size()) * InstanceChunk::InstancesOfChunk;
}

void ManagerImplemented::BeginReloadEffect( Effect* effect, bool doLockThread)
{
	if (doLockThread)
	{
		m_renderingMutex.lock();
		m_isLockedWithRenderingMutex = true;
	}

	std::map<Handle,DrawSet>::iterator it = m_DrawSets.begin();
	std::map<Handle,DrawSet>::iterator it_end = m_DrawSets.end();

	for(; it != it_end; ++it )
	{
		if( (*it).second.ParameterPointer != effect ) continue;
	
		/* インスタンス削除 */
		(*it).second.InstanceContainerPointer->RemoveForcibly( true );
		ReleaseInstanceContainer( (*it).second.InstanceContainerPointer );
		(*it).second.InstanceContainerPointer = NULL;
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void ManagerImplemented::EndReloadEffect( Effect* effect, bool doLockThread)
{
	std::map<Handle,DrawSet>::iterator it = m_DrawSets.begin();
	std::map<Handle,DrawSet>::iterator it_end = m_DrawSets.end();

	for(; it != it_end; ++it )
	{
		if( (*it).second.ParameterPointer != effect ) continue;

		auto e = (EffectImplemented*)effect;
		auto pGlobal = (*it).second.GlobalPointer;

		// reallocate
		if (e->m_defaultRandomSeed >= 0)
		{
			pGlobal->SetSeed(e->m_defaultRandomSeed);
		}
		else
		{
			pGlobal->SetSeed(GetRandFunc()());
		}

		pGlobal->RenderedInstanceContainers.resize(e->renderingNodesCount);
		for (size_t i = 0; i < pGlobal->RenderedInstanceContainers.size(); i++)
		{
			pGlobal->RenderedInstanceContainers[i] = nullptr;
		}

		// Create an instance through a container
		(*it).second.InstanceContainerPointer = CreateInstanceContainer( ((EffectImplemented*)effect)->GetRoot(), (*it).second.GlobalPointer, true, (*it).second.GlobalMatrix, NULL );
		
		// skip
		for( float f = 0; f < (*it).second.GlobalPointer->GetUpdatedFrame() - 1; f+= 1.0f )
		{
			(*it).second.InstanceContainerPointer->Update( true, 1.0f, false );
		}

		(*it).second.InstanceContainerPointer->Update( true, 1.0f, (*it).second.IsShown );
	}

	if (doLockThread)
	{
		m_renderingMutex.unlock();
		m_isLockedWithRenderingMutex = false;
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void ManagerImplemented::CreateCullingWorld( float xsize, float ysize, float zsize, int32_t layerCount)
{
	cullingNext.SizeX = xsize;
	cullingNext.SizeY = ysize;
	cullingNext.SizeZ = zsize;
	cullingNext.LayerCount = layerCount;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void ManagerImplemented::CalcCulling( const Matrix44& cameraProjMat, bool isOpenGL)
{
	if(m_cullingWorld == NULL) return;

	m_culledObjects.clear();
	m_culledObjectSets.clear();
	
	Matrix44 mat = cameraProjMat;
	mat.Transpose();

	Culling3D::Matrix44* mat_ = (Culling3D::Matrix44*)(&mat);

	m_cullingWorld->Culling(*mat_, isOpenGL);

	for(int32_t i = 0; i < m_cullingWorld->GetObjectCount(); i++)
	{
		Culling3D::Object* o = m_cullingWorld->GetObject(i);
		DrawSet* ds = (DrawSet*)o->GetUserData();

		m_culledObjects.push_back(ds);
		m_culledObjectSets.insert(ds->Self);
	}

	m_culled = true;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void ManagerImplemented::RessignCulling()
{
	if (m_cullingWorld == NULL) return;

	m_culledObjects.clear();
	m_culledObjectSets.clear();

	m_cullingWorld->Reassign();
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------


//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
namespace Effekseer {
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
}
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------


//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------



//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
namespace Effekseer
{

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
InstanceContainer::InstanceContainer(ManagerImplemented* pManager, EffectNode* pEffectNode, InstanceGlobal* pGlobal)
	: m_pManager(pManager)
	, m_pEffectNode((EffectNodeImplemented*)pEffectNode)
	, m_pGlobal(pGlobal)
	, m_headGroups(NULL)
	, m_tailGroups(NULL)

{
	auto en = (EffectNodeImplemented*)pEffectNode;
	if (en->RenderingPriority >= 0)
	{
		pGlobal->RenderedInstanceContainers[en->RenderingPriority] = this;
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
InstanceContainer::~InstanceContainer()
{
	RemoveForcibly(false);

	assert(m_headGroups == NULL);
	assert(m_tailGroups == NULL);

	for( auto child : m_Children )
	{
		m_pManager->ReleaseInstanceContainer( child );
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void InstanceContainer::AddChild(InstanceContainer* pContainter)
{
	m_Children.push_back( pContainter );
}

InstanceContainer* InstanceContainer::GetChild(int index)
{
	auto it = m_Children.begin();
	for( int i = 0; i < index; i++) {
		it++;
	}
	return *it;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void InstanceContainer::RemoveInvalidGroups()
{
	/* 最後に存在する有効なグループ */
	InstanceGroup* tailGroup = NULL;

	for (InstanceGroup* group = m_headGroups; group != NULL; )
	{
		if (!group->IsReferencedFromInstance && group->GetInstanceCount() == 0)
		{
			InstanceGroup* next = group->NextUsedByContainer;
			m_pManager->ReleaseGroup( group );

			if (m_headGroups == group)
			{
				m_headGroups = next;
			}
			group = next;

			if (tailGroup != NULL)
			{
				tailGroup->NextUsedByContainer = next;
			}
		}
		else
		{
			tailGroup = group;
			group = group->NextUsedByContainer;
		}
	}


	m_tailGroups = tailGroup;

	assert(m_tailGroups == NULL || m_tailGroups->NextUsedByContainer == NULL);
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
InstanceGroup* InstanceContainer::CreateInstanceGroup()
{
	InstanceGroup* group = m_pManager->CreateInstanceGroup( m_pEffectNode, this, m_pGlobal );

	if (m_tailGroups != NULL)
	{
		m_tailGroups->NextUsedByContainer = group;
		m_tailGroups = group;
	}
	else
	{
		assert(m_headGroups == NULL);
		m_headGroups = group;
		m_tailGroups = group;
	}

	m_pEffectNode->InitializeRenderedInstanceGroup(*group, m_pManager);

	return group;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
InstanceGroup* InstanceContainer::GetFirstGroup() const
{
	return m_headGroups;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void InstanceContainer::Update(bool recursive, float deltaFrame, bool shown)
{
	// 更新
	for (InstanceGroup* group = m_headGroups; group != NULL; group = group->NextUsedByContainer)
	{
		group->Update(deltaFrame, shown);
	}

	// 破棄
	RemoveInvalidGroups();

	if (recursive)
	{
		for (auto child : m_Children)
		{
			child->Update(recursive, deltaFrame, shown);
		}
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void InstanceContainer::SetBaseMatrix(bool recursive, const Matrix43& mat)
{
	if (m_pEffectNode->GetType() != EFFECT_NODE_TYPE_ROOT)
	{
		for (InstanceGroup* group = m_headGroups; group != NULL; group = group->NextUsedByContainer)
		{
			group->SetBaseMatrix(mat);
		}
	}

	if (recursive)
	{
		for (auto child : m_Children)
		{
			child->SetBaseMatrix(recursive, mat);
		}
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void InstanceContainer::RemoveForcibly(bool recursive)
{
	KillAllInstances(false);


	for (InstanceGroup* group = m_headGroups; group != NULL; group = group->NextUsedByContainer)
	{
		group->RemoveForcibly();
	}
	RemoveInvalidGroups();

	if (recursive)
	{
		for (auto child : m_Children)
		{
			child->RemoveForcibly(recursive);
		}
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void InstanceContainer::Draw(bool recursive)
{
	if (m_pEffectNode->GetType() != EFFECT_NODE_TYPE_ROOT && m_pEffectNode->GetType() != EFFECT_NODE_TYPE_NONE)
	{
		/* 個数計測 */
		int32_t count = 0;
		{
			for (InstanceGroup* group = m_headGroups; group != NULL; group = group->NextUsedByContainer)
			{
				for (auto instance : group->m_instances)
				{
					if (instance->m_State == INSTANCE_STATE_ACTIVE)
					{
						count++;
					}
				}
			}
		}

		if (count > 0)
		{
			/* 描画 */
			m_pEffectNode->BeginRendering(count, m_pManager);

			for (InstanceGroup* group = m_headGroups; group != NULL; group = group->NextUsedByContainer)
			{
				m_pEffectNode->BeginRenderingGroup(group, m_pManager);

				if (m_pEffectNode->RenderingOrder == RenderingOrder_FirstCreatedInstanceIsFirst)
				{
					auto it = group->m_instances.begin();

					while (it != group->m_instances.end())
					{
						if ((*it)->m_State == INSTANCE_STATE_ACTIVE)
						{
							auto it_temp = it;
							it_temp++;

							if (it_temp != group->m_instances.end())
							{
								(*it)->Draw((*it_temp));
							}
							else
							{
								(*it)->Draw(nullptr);
							}
						}

						it++;
					}
				}
				else
				{
					auto it = group->m_instances.rbegin();

					while (it != group->m_instances.rend())
					{
						if ((*it)->m_State == INSTANCE_STATE_ACTIVE)
						{
							auto it_temp = it;
							it_temp++;

							if (it_temp != group->m_instances.rend())
							{
								(*it)->Draw((*it_temp));
							}
							else
							{
								(*it)->Draw(nullptr);
							}
						}
						it++;
					}
				}

				m_pEffectNode->EndRenderingGroup(group, m_pManager);
			}

			m_pEffectNode->EndRendering(m_pManager);
		}
	}

	if (recursive)
	{
		for (auto child : m_Children)
		{
			child->Draw(recursive);
		}
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void InstanceContainer::KillAllInstances(bool recursive)
{
	for (InstanceGroup* group = m_headGroups; group != NULL; group = group->NextUsedByContainer)
	{
		group->KillAllInstances();
	}

	if (recursive)
	{
		for (auto child : m_Children)
		{
			child->KillAllInstances(recursive);
		}
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
InstanceGlobal* InstanceContainer::GetRootInstance()
{
	return m_pGlobal;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------

}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------


//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
namespace Effekseer
{

template<typename T, typename U>
void Instance::ApplyEq(T& dstParam, Effect* e, InstanceGlobal* instg, int dpInd, const U& originalParam)
{
	static_assert(sizeof(T) == sizeof(U), "size is not mismatched");
	const int count = sizeof(T) / 4;

	EFK_ASSERT(e != nullptr);
	EFK_ASSERT(0 <= dpInd && dpInd < static_cast<int>(instg->dynamicEqResults.size()));

	auto dst = reinterpret_cast<float*>(&(dstParam));
	auto src = reinterpret_cast<const float*>(&(originalParam));

	auto eqresult = instg->dynamicEqResults[dpInd];
	std::array<float, 1> globals;
	globals[0] = instg->GetUpdatedFrame() / 60.0f;

	std::array<float, 5> locals;

	for (int i = 0; i < count; i++)
	{
		locals[i] = src[i];
	}

	for (int i = count; i < 4; i++)
	{
		locals[i] = 0.0f;
	}

	locals[4] = m_pParent != nullptr ? m_pParent->m_LivingTime / 60.0f : 0.0f;

	auto e_ = static_cast<EffectImplemented*>(e);
	auto& dp = e_->dynamicEquation[dpInd];

	if (dp.GetRunningPhase() == InternalScript::RunningPhaseType::Local)
	{
		eqresult = dp.Execute(instg->dynamicInputParameters, globals, locals, InstanceGlobal::Rand, InstanceGlobal::RandSeed, instg);
	}

	for (int i = 0; i < count; i++)
	{
		dst[i] = eqresult[i];
	}
}

template <typename S> Vector3D Instance::ApplyEq(const int& dpInd, Vector3D originalParam, const S& scale, const S& scaleInv)
{
	const auto& e = this->m_pEffectNode->m_effect;
	const auto& instg = this->m_pContainer->GetRootInstance();

	if (dpInd >= 0)
	{
		originalParam.X *= scaleInv[0];
		originalParam.Y *= scaleInv[1];
		originalParam.Z *= scaleInv[2];

		ApplyEq(originalParam, e, instg, dpInd, originalParam);

		originalParam.X *= scale[0];
		originalParam.Y *= scale[1];
		originalParam.Z *= scale[2];
	}
	return originalParam;
}

random_float Instance::ApplyEq(const RefMinMax& dpInd, random_float originalParam)
{
	const auto& e = this->m_pEffectNode->m_effect;
	const auto& instg = this->m_pContainer->GetRootInstance();

	if (dpInd.Max >= 0)
	{
		ApplyEq(originalParam.max, e, instg, dpInd.Max, originalParam.max);
	}

	if (dpInd.Min >= 0)
	{
		ApplyEq(originalParam.min, e, instg, dpInd.Min, originalParam.min);
	}

	return originalParam;
}

template <typename S>
random_vector3d Instance::ApplyEq(const RefMinMax& dpInd, random_vector3d originalParam, const S& scale, const S& scaleInv)
{
	const auto& e = this->m_pEffectNode->m_effect;
	const auto& instg = this->m_pContainer->GetRootInstance();

	if (dpInd.Max >= 0)
	{
		originalParam.max.x *= scaleInv[0];
		originalParam.max.y *= scaleInv[1];
		originalParam.max.z *= scaleInv[2];

		ApplyEq(originalParam.max, e, instg, dpInd.Max, originalParam.max);

		originalParam.max.x *= scale[0];
		originalParam.max.y *= scale[1];
		originalParam.max.z *= scale[2];
	}

	if (dpInd.Min >= 0)
	{
		originalParam.min.x *= scaleInv[0];
		originalParam.min.y *= scaleInv[1];
		originalParam.min.z *= scaleInv[2];

		ApplyEq(originalParam.min, e, instg, dpInd.Min, originalParam.min);

		originalParam.min.x *= scale[0];
		originalParam.min.y *= scale[1];
		originalParam.min.z *= scale[2];
	}

	return originalParam;
}

random_int Instance::ApplyEq(const RefMinMax& dpInd, random_int originalParam)
{
	const auto& e = this->m_pEffectNode->m_effect;
	const auto& instg = this->m_pContainer->GetRootInstance();

	if (dpInd.Max >= 0)
	{
		float value = static_cast<float>(originalParam.max);
		ApplyEq(value, e, instg, dpInd.Max, value);
		originalParam.max = static_cast<int32_t>(value);
	}

	if (dpInd.Min >= 0)
	{
		float value = static_cast<float>(originalParam.min);
		ApplyEq(value, e, instg, dpInd.Min, value);
		originalParam.min = static_cast<int32_t>(value);
	}

	return originalParam;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
Instance::Instance(Manager* pManager, EffectNode* pEffectNode, InstanceContainer* pContainer, InstanceGroup* pGroup)
	: m_pManager(pManager)
	, m_pEffectNode((EffectNodeImplemented*)pEffectNode)
	, m_pContainer(pContainer)
	, ownGroup_(pGroup)
	, childrenGroups_(nullptr)
	, m_pParent(nullptr)
	, m_State(INSTANCE_STATE_ACTIVE)
	, m_LivedTime(0)
	, m_LivingTime(0)
	, m_flexibleGeneratedChildrenCount(nullptr)
	, m_flexibleNextGenerationTime(nullptr)
	, m_GlobalMatrix43Calculated(false)
	, m_ParentMatrix43Calculated(false)
	, is_time_step_allowed(false)
	, m_sequenceNumber(0)
{
	m_generatedChildrenCount = m_fixedGeneratedChildrenCount;
	maxGenerationChildrenCount = fixedMaxGenerationChildrenCount_;
	m_nextGenerationTime = m_fixedNextGenerationTime;
	
	ColorInheritance = Color(255, 255, 255, 255);
	ColorParent = Color(255, 255, 255, 255);

	InstanceGroup* group = NULL;

	for( int i = 0; i < m_pEffectNode->GetChildrenCount(); i++ )
	{
		InstanceContainer* childContainer = m_pContainer->GetChild( i );

		if( group != NULL )
		{
			group->NextUsedByInstance = childContainer->CreateInstanceGroup();
			group = group->NextUsedByInstance;
		}
		else
		{
			group = childContainer->CreateInstanceGroup();
			childrenGroups_ = group;
		}
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
Instance::~Instance()
{
	assert( m_State != INSTANCE_STATE_ACTIVE );

	auto parameter = (EffectNodeImplemented*)m_pEffectNode;

	if (m_flexibleGeneratedChildrenCount != nullptr)
	{
		m_pManager->GetFreeFunc()(m_flexibleGeneratedChildrenCount, sizeof(int32_t) * parameter->GetChildrenCount());
	}

	if (flexibleMaxGenerationChildrenCount_ != nullptr)
	{
		m_pManager->GetFreeFunc()(flexibleMaxGenerationChildrenCount_, sizeof(int32_t) * parameter->GetChildrenCount());
	}

	if (m_flexibleNextGenerationTime != nullptr)
	{
		m_pManager->GetFreeFunc()(m_flexibleNextGenerationTime, sizeof(float) * parameter->GetChildrenCount());
	}
}

bool Instance::IsRequiredToCreateChildren(float currentTime)
{
	auto instanceGlobal = this->m_pContainer->GetRootInstance();

	auto parameter = (EffectNodeImplemented*)m_pEffectNode;

	InstanceGroup* group = childrenGroups_;

	for (int32_t i = 0; i < parameter->GetChildrenCount(); i++, group = group->NextUsedByInstance)
	{
		auto node = (EffectNodeImplemented*)parameter->GetChild(i);
		assert(group != NULL);

		// GenerationTimeOffset can be minus value.
		// Minus frame particles is generated simultaniously at frame 0.
		if (maxGenerationChildrenCount[i] > m_generatedChildrenCount[i] && m_nextGenerationTime[i] <= currentTime)
		{
			return true;
		}
	}

	return false;
}

void Instance::GenerateChildrenInRequired(float currentTime)
{
	auto instanceGlobal = this->m_pContainer->GetRootInstance();

	auto parameter = (EffectNodeImplemented*)m_pEffectNode;

	InstanceGroup* group = childrenGroups_;

	for (int32_t i = 0; i < parameter->GetChildrenCount(); i++, group = group->NextUsedByInstance)
	{
		auto node = (EffectNodeImplemented*)parameter->GetChild(i);
		assert(group != NULL);

		while (true)
		{
			// GenerationTimeOffset can be minus value.
			// Minus frame particles is generated simultaniously at frame 0.
			if (maxGenerationChildrenCount[i] > m_generatedChildrenCount[i] &&
				m_nextGenerationTime[i] <= currentTime)
			{
				// Create a particle
				auto newInstance = group->CreateInstance();
				if (newInstance != nullptr)
				{
					Matrix43 rootMatrix;
					rootMatrix.Indentity();

					newInstance->Initialize(this, m_generatedChildrenCount[i], (int32_t)std::max(0.0f, this->m_LivingTime), rootMatrix);
				}

				m_generatedChildrenCount[i]++;

				auto gt = ApplyEq(node->CommonValues.RefEqGenerationTime, node->CommonValues.GenerationTime);
				m_nextGenerationTime[i] += Max(0.0f, gt.getValue(*instanceGlobal));
			}
			else
			{
				break;
			}
		}
	}
}

void Instance::UpdateChildrenGroupMatrix()
{
	for (InstanceGroup* group = childrenGroups_; group != nullptr; group = group->NextUsedByInstance)
	{
		group->SetParentMatrix(m_GlobalMatrix43);
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
InstanceGlobal* Instance::GetInstanceGlobal()
{
	return m_pContainer->GetRootInstance();
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
eInstanceState Instance::GetState() const
{
	return m_State;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
const Matrix43& Instance::GetGlobalMatrix43() const
{
	return m_GlobalMatrix43;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void Instance::Initialize( Instance* parent, int32_t instanceNumber, int32_t parentTime, const Matrix43& globalMatrix)
{
	assert(this->m_pContainer != nullptr);
	
	// Invalidate matrix
	m_GlobalMatrix43Calculated = false;
	m_ParentMatrix43Calculated = false;

	auto instanceGlobal = this->m_pContainer->GetRootInstance();

	auto parameter = (EffectNodeImplemented*) m_pEffectNode;

	// Extend array
	if (parameter->GetChildrenCount() >= ChildrenMax)
	{
		m_flexibleGeneratedChildrenCount = (int32_t*)(m_pManager->GetMallocFunc()(sizeof(int32_t) * parameter->GetChildrenCount()));
		flexibleMaxGenerationChildrenCount_ = (int32_t*)(m_pManager->GetMallocFunc()(sizeof(int32_t) * parameter->GetChildrenCount()));
		m_flexibleNextGenerationTime = (float*)(m_pManager->GetMallocFunc()(sizeof(float) * parameter->GetChildrenCount()));

		m_generatedChildrenCount = m_flexibleGeneratedChildrenCount;
		maxGenerationChildrenCount = flexibleMaxGenerationChildrenCount_;
		m_nextGenerationTime = m_flexibleNextGenerationTime;
	}

	// 親の設定
	m_pParent = parent;

	// initialize children
	for (int32_t i = 0; i < parameter->GetChildrenCount(); i++)
	{
		auto pNode = (EffectNodeImplemented*) parameter->GetChild(i);

		m_generatedChildrenCount[i] = 0;

		auto gt = ApplyEq(pNode->CommonValues.RefEqGenerationTimeOffset, pNode->CommonValues.GenerationTimeOffset);

		m_nextGenerationTime[i] = gt.getValue(*instanceGlobal);

		if (pNode->CommonValues.RefEqMaxGeneration >= 0)
		{
			auto maxGene = static_cast<float>(pNode->CommonValues.MaxGeneration);
			ApplyEq(maxGene,
					this->m_pEffectNode->m_effect,
					this->m_pContainer->GetRootInstance(),
					pNode->CommonValues.RefEqMaxGeneration,
					maxGene);
			maxGenerationChildrenCount[i] = maxGene;
		}
		else
		{
			maxGenerationChildrenCount[i] = pNode->CommonValues.MaxGeneration;
		}
	}

	if( m_pParent == NULL )
	{
		// ROOTの場合

		// 状態の初期化
		m_State = INSTANCE_STATE_ACTIVE;

		// 時間周りの初期化
		m_LivingTime = 0.0f;
		m_LivedTime = FLT_MAX;

		// SRTの初期化
		m_GenerationLocation.Indentity();
		m_GlobalMatrix43 = globalMatrix;
		assert(m_GlobalMatrix43.IsValid());

		// 親の初期化
		m_ParentMatrix.Indentity();

		// Generate zero frame effect

		// for new children
		UpdateChildrenGroupMatrix();

		GenerateChildrenInRequired(0.0f);
		return;
	}
	
	// 状態の初期化
	m_State = INSTANCE_STATE_ACTIVE;

	// initialize about a lifetime
	m_LivingTime = 0.0f;
	{
		auto ri = ApplyEq(parameter->CommonValues.RefEqLife, parameter->CommonValues.life);
		m_LivedTime = (float)ri.getValue(*instanceGlobal);
	}

	// initialize SRT

	// calculate parent matrixt to get matrix
	m_pParent->CalculateMatrix(0);
	
	const Matrix43& parentMatrix = m_pParent->GetGlobalMatrix43();
	parentMatrix.GetTranslation( m_GlobalPosition );
	m_GlobalRevisionLocation = Vector3D(0.0f, 0.0f, 0.0f);
	m_GlobalRevisionVelocity = Vector3D(0.0f, 0.0f, 0.0f);
	m_GenerationLocation.Indentity();
	m_GlobalMatrix43 = globalMatrix;
	assert(m_GlobalMatrix43.IsValid());

	// 親の初期化
	if( parameter->CommonValues.TranslationBindType == BindType::WhenCreating ||
		parameter->CommonValues.RotationBindType == BindType::WhenCreating ||
		parameter->CommonValues.ScalingBindType == BindType::WhenCreating )
	{
		m_ParentMatrix = parentMatrix;
		assert(m_ParentMatrix.IsValid());
	}

	// Initialize parent color
	if (parameter->RendererCommon.ColorBindType == BindType::Always)
	{
		ColorParent = m_pParent->ColorInheritance;
	}
	else if (parameter->RendererCommon.ColorBindType == BindType::WhenCreating)
	{
		ColorParent = m_pParent->ColorInheritance;
	}

	// Translation
	if( m_pEffectNode->TranslationType == ParameterTranslationType_Fixed )
	{
	}
	else if( m_pEffectNode->TranslationType == ParameterTranslationType_PVA )
	{
		auto rvl = ApplyEq(m_pEffectNode->TranslationPVA.RefEqP,
						   m_pEffectNode->TranslationPVA.location,
						   m_pEffectNode->DynamicFactor.Tra,
						   m_pEffectNode->DynamicFactor.TraInv);
		translation_values.random.location = rvl.getValue(*this->m_pContainer->GetRootInstance());

		auto rvv = ApplyEq(m_pEffectNode->TranslationPVA.RefEqV,
						   m_pEffectNode->TranslationPVA.velocity,
						   m_pEffectNode->DynamicFactor.Tra,
						   m_pEffectNode->DynamicFactor.TraInv);
		translation_values.random.velocity = rvv.getValue(*this->m_pContainer->GetRootInstance());

		auto rva = ApplyEq(m_pEffectNode->TranslationPVA.RefEqA,
						   m_pEffectNode->TranslationPVA.acceleration,
						   m_pEffectNode->DynamicFactor.Tra,
						   m_pEffectNode->DynamicFactor.TraInv);
		translation_values.random.acceleration = rva.getValue(*this->m_pContainer->GetRootInstance());

	}
	else if( m_pEffectNode->TranslationType == ParameterTranslationType_Easing )
	{
		auto rvs = ApplyEq(m_pEffectNode->TranslationEasing.RefEqS,
						   m_pEffectNode->TranslationEasing.location.start,
						   m_pEffectNode->DynamicFactor.Tra,
						   m_pEffectNode->DynamicFactor.TraInv);
		auto rve = ApplyEq(m_pEffectNode->TranslationEasing.RefEqE,
						   m_pEffectNode->TranslationEasing.location.end,
						   m_pEffectNode->DynamicFactor.Tra,
						   m_pEffectNode->DynamicFactor.TraInv);

		translation_values.easing.start = rvs.getValue(*this->m_pContainer->GetRootInstance());
		translation_values.easing.end = rve.getValue(*this->m_pContainer->GetRootInstance());
	}
	else if( m_pEffectNode->TranslationType == ParameterTranslationType_FCurve )
	{
		assert( m_pEffectNode->TranslationFCurve != NULL );

		translation_values.fcruve.offset.x = m_pEffectNode->TranslationFCurve->X.GetOffset( *instanceGlobal );
		translation_values.fcruve.offset.y = m_pEffectNode->TranslationFCurve->Y.GetOffset( *instanceGlobal );
		translation_values.fcruve.offset.z = m_pEffectNode->TranslationFCurve->Z.GetOffset( *instanceGlobal );
	}
	
	// Rotation
	if( m_pEffectNode->RotationType == ParameterRotationType_Fixed )
	{
	}
	else if( m_pEffectNode->RotationType == ParameterRotationType_PVA )
	{
		auto rvl = ApplyEq(m_pEffectNode->RotationPVA.RefEqP,
						   m_pEffectNode->RotationPVA.rotation,
						   m_pEffectNode->DynamicFactor.Rot,
						   m_pEffectNode->DynamicFactor.RotInv);
		auto rvv = ApplyEq(m_pEffectNode->RotationPVA.RefEqV,
						   m_pEffectNode->RotationPVA.velocity,
						   m_pEffectNode->DynamicFactor.Rot,
						   m_pEffectNode->DynamicFactor.RotInv);
		auto rva = ApplyEq(m_pEffectNode->RotationPVA.RefEqA,
						   m_pEffectNode->RotationPVA.acceleration,
						   m_pEffectNode->DynamicFactor.Rot,
						   m_pEffectNode->DynamicFactor.RotInv);

		rotation_values.random.rotation = rvl.getValue(*instanceGlobal);
		rotation_values.random.velocity = rvv.getValue(*instanceGlobal);
		rotation_values.random.acceleration = rva.getValue(*instanceGlobal);
	}
	else if( m_pEffectNode->RotationType == ParameterRotationType_Easing )
	{
		auto rvs = ApplyEq(m_pEffectNode->RotationEasing.RefEqS,
						   m_pEffectNode->RotationEasing.rotation.start,
						   m_pEffectNode->DynamicFactor.Rot,
						   m_pEffectNode->DynamicFactor.RotInv);
		auto rve = ApplyEq(m_pEffectNode->RotationEasing.RefEqE,
						   m_pEffectNode->RotationEasing.rotation.end,
						   m_pEffectNode->DynamicFactor.Rot,
						   m_pEffectNode->DynamicFactor.RotInv);

		rotation_values.easing.start = rvs.getValue(*instanceGlobal);
		rotation_values.easing.end = rve.getValue(*instanceGlobal);
	}
	else if( m_pEffectNode->RotationType == ParameterRotationType_AxisPVA )
	{
		rotation_values.axis.random.rotation = m_pEffectNode->RotationAxisPVA.rotation.getValue(*instanceGlobal);
		rotation_values.axis.random.velocity = m_pEffectNode->RotationAxisPVA.velocity.getValue(*instanceGlobal);
		rotation_values.axis.random.acceleration = m_pEffectNode->RotationAxisPVA.acceleration.getValue(*instanceGlobal);
		rotation_values.axis.rotation = rotation_values.axis.random.rotation;
		rotation_values.axis.axis = m_pEffectNode->RotationAxisPVA.axis.getValue(*instanceGlobal);
		rotation_values.axis.axis.normalize();
	}
	else if( m_pEffectNode->RotationType == ParameterRotationType_AxisEasing )
	{
		rotation_values.axis.easing.start = m_pEffectNode->RotationAxisEasing.easing.start.getValue(*instanceGlobal);
		rotation_values.axis.easing.end = m_pEffectNode->RotationAxisEasing.easing.end.getValue(*instanceGlobal);
		rotation_values.axis.rotation = rotation_values.axis.easing.start;
		rotation_values.axis.axis = m_pEffectNode->RotationAxisEasing.axis.getValue(*instanceGlobal);
		rotation_values.axis.axis.normalize();
	}
	else if( m_pEffectNode->RotationType == ParameterRotationType_FCurve )
	{
		assert( m_pEffectNode->RotationFCurve != NULL );

		rotation_values.fcruve.offset.x = m_pEffectNode->RotationFCurve->X.GetOffset( *instanceGlobal );
		rotation_values.fcruve.offset.y = m_pEffectNode->RotationFCurve->Y.GetOffset( *instanceGlobal );
		rotation_values.fcruve.offset.z = m_pEffectNode->RotationFCurve->Z.GetOffset( *instanceGlobal );
	}

	// Scaling
	if( m_pEffectNode->ScalingType == ParameterScalingType_Fixed )
	{
	}
	else if( m_pEffectNode->ScalingType == ParameterScalingType_PVA )
	{
		auto rvl = ApplyEq(m_pEffectNode->ScalingPVA.RefEqP, m_pEffectNode->ScalingPVA.Position, m_pEffectNode->DynamicFactor.Scale, m_pEffectNode->DynamicFactor.ScaleInv);
		auto rvv = ApplyEq(m_pEffectNode->ScalingPVA.RefEqV,
						   m_pEffectNode->ScalingPVA.Velocity,
						   m_pEffectNode->DynamicFactor.Scale,
						   m_pEffectNode->DynamicFactor.ScaleInv);
		auto rva = ApplyEq(m_pEffectNode->ScalingPVA.RefEqA,
						   m_pEffectNode->ScalingPVA.Acceleration,
						   m_pEffectNode->DynamicFactor.Scale,
						   m_pEffectNode->DynamicFactor.ScaleInv);

		scaling_values.random.scale = rvl.getValue(*instanceGlobal);
		scaling_values.random.velocity = rvv.getValue(*instanceGlobal);
		scaling_values.random.acceleration = rva.getValue(*instanceGlobal);
	}
	else if( m_pEffectNode->ScalingType == ParameterScalingType_Easing )
	{
		auto rvs = ApplyEq(m_pEffectNode->ScalingEasing.RefEqS,
						   m_pEffectNode->ScalingEasing.Position.start,
						   m_pEffectNode->DynamicFactor.Scale,
						   m_pEffectNode->DynamicFactor.ScaleInv);
		auto rve = ApplyEq(m_pEffectNode->ScalingEasing.RefEqE,
						   m_pEffectNode->ScalingEasing.Position.end,
						   m_pEffectNode->DynamicFactor.Scale,
						   m_pEffectNode->DynamicFactor.ScaleInv);

		scaling_values.easing.start = rvs.getValue(*instanceGlobal);
		scaling_values.easing.end = rve.getValue(*instanceGlobal);
	}
	else if( m_pEffectNode->ScalingType == ParameterScalingType_SinglePVA )
	{
		scaling_values.single_random.scale = m_pEffectNode->ScalingSinglePVA.Position.getValue(*instanceGlobal);
		scaling_values.single_random.velocity = m_pEffectNode->ScalingSinglePVA.Velocity.getValue(*instanceGlobal);
		scaling_values.single_random.acceleration = m_pEffectNode->ScalingSinglePVA.Acceleration.getValue(*instanceGlobal);
	}
	else if( m_pEffectNode->ScalingType == ParameterScalingType_SingleEasing )
	{
		scaling_values.single_easing.start = m_pEffectNode->ScalingSingleEasing.start.getValue(*instanceGlobal);
		scaling_values.single_easing.end = m_pEffectNode->ScalingSingleEasing.end.getValue(*instanceGlobal);
	}
	else if( m_pEffectNode->ScalingType == ParameterScalingType_FCurve )
	{
		assert( m_pEffectNode->ScalingFCurve != NULL );

		scaling_values.fcruve.offset.x = m_pEffectNode->ScalingFCurve->X.GetOffset( *instanceGlobal );
		scaling_values.fcruve.offset.y = m_pEffectNode->ScalingFCurve->Y.GetOffset( *instanceGlobal );
		scaling_values.fcruve.offset.z = m_pEffectNode->ScalingFCurve->Z.GetOffset( *instanceGlobal );
	}

	// Spawning Method
	if( m_pEffectNode->GenerationLocation.type == ParameterGenerationLocation::TYPE_POINT )
	{
		vector3d p = m_pEffectNode->GenerationLocation.point.location.getValue(*instanceGlobal);
		m_GenerationLocation.Translation( p.x, p.y, p.z );
	}
	else if (m_pEffectNode->GenerationLocation.type == ParameterGenerationLocation::TYPE_LINE)
	{
		vector3d s = m_pEffectNode->GenerationLocation.line.position_start.getValue(*instanceGlobal);
		vector3d e = m_pEffectNode->GenerationLocation.line.position_end.getValue(*instanceGlobal);
		auto noize = m_pEffectNode->GenerationLocation.line.position_noize.getValue(*instanceGlobal);
		auto division = Max(1, m_pEffectNode->GenerationLocation.line.division);

		Vector3D dir;
		(e - s).setValueToArg(dir);

		if (Vector3D::LengthSq(dir) < 0.001)
		{
			m_GenerationLocation.Translation(0 ,0, 0);
		}
		else
		{
			auto len = Vector3D::Length(dir);
			dir /= len;
		
			int32_t target = 0;
			if (m_pEffectNode->GenerationLocation.line.type == ParameterGenerationLocation::LineType::Order)
			{
				target = instanceNumber % division;
			}
			else if (m_pEffectNode->GenerationLocation.line.type == ParameterGenerationLocation::LineType::Random)
			{
				target = (int32_t)((division) * instanceGlobal->GetRand());
				if (target == division) target -= 1;
			}

			auto d = 0.0f;
			if (division > 1)
			{
				d = (len / (float)(division-1)) * target;
			}

			d += noize;
		
			s.x += dir.X * d;
			s.y += dir.Y * d;
			s.z += dir.Z * d;

			Vector3D xdir;
			Vector3D ydir;
			Vector3D zdir;

			if (fabs(dir.Y) > 0.999f)
			{
				xdir = dir;
				Vector3D::Cross(zdir, xdir, Vector3D(-1, 0, 0));
				Vector3D::Normal(zdir, zdir);
				Vector3D::Cross(ydir, zdir, xdir);
				Vector3D::Normal(ydir, ydir);
			}
			else
			{
				xdir = dir;
				Vector3D::Cross(ydir, Vector3D(0, 0, 1), xdir);
				Vector3D::Normal(ydir, ydir);
				Vector3D::Cross(zdir, xdir, ydir);
				Vector3D::Normal(zdir, zdir);
			}

			if (m_pEffectNode->GenerationLocation.EffectsRotation)
			{
				m_GenerationLocation.Value[0][0] = xdir.X;
				m_GenerationLocation.Value[0][1] = xdir.Y;
				m_GenerationLocation.Value[0][2] = xdir.Z;

				m_GenerationLocation.Value[1][0] = ydir.X;
				m_GenerationLocation.Value[1][1] = ydir.Y;
				m_GenerationLocation.Value[1][2] = ydir.Z;

				m_GenerationLocation.Value[2][0] = zdir.X;
				m_GenerationLocation.Value[2][1] = zdir.Y;
				m_GenerationLocation.Value[2][2] = zdir.Z;
			}
			else
			{
				m_GenerationLocation.Indentity();
			}

			m_GenerationLocation.Value[3][0] = s.x;
			m_GenerationLocation.Value[3][1] = s.y;
			m_GenerationLocation.Value[3][2] = s.z;
		}
	}
	else if( m_pEffectNode->GenerationLocation.type == ParameterGenerationLocation::TYPE_SPHERE )
	{
		Matrix43 mat_x, mat_y;
		mat_x.RotationX( m_pEffectNode->GenerationLocation.sphere.rotation_x.getValue( *instanceGlobal ) );
		mat_y.RotationY( m_pEffectNode->GenerationLocation.sphere.rotation_y.getValue( *instanceGlobal ) );
		float r = m_pEffectNode->GenerationLocation.sphere.radius.getValue(*instanceGlobal);
		m_GenerationLocation.Translation( 0, r, 0 );
		Matrix43::Multiple( m_GenerationLocation, m_GenerationLocation, mat_x );
		Matrix43::Multiple( m_GenerationLocation, m_GenerationLocation, mat_y );
	}
	else if( m_pEffectNode->GenerationLocation.type == ParameterGenerationLocation::TYPE_MODEL )
	{
		m_GenerationLocation.Indentity();

		int32_t modelIndex = m_pEffectNode->GenerationLocation.model.index;
		if( modelIndex >= 0 )
		{
			Model* model = (Model*)m_pEffectNode->GetEffect()->GetModel( modelIndex );
			if( model != NULL )
			{
				Model::Emitter emitter;
				
				if( m_pEffectNode->GenerationLocation.model.type == ParameterGenerationLocation::MODELTYPE_RANDOM )
				{
					emitter = model->GetEmitter( 
						instanceGlobal, 
						parentTime,
						m_pManager->GetCoordinateSystem(), 
						((EffectImplemented*)m_pEffectNode->GetEffect())->GetMaginification() );
				}
				else if( m_pEffectNode->GenerationLocation.model.type == ParameterGenerationLocation::MODELTYPE_VERTEX )
				{
					emitter = model->GetEmitterFromVertex( 
						instanceNumber,
						parentTime,
						m_pManager->GetCoordinateSystem(), 
						((EffectImplemented*)m_pEffectNode->GetEffect())->GetMaginification() );
				}
				else if( m_pEffectNode->GenerationLocation.model.type == ParameterGenerationLocation::MODELTYPE_VERTEX_RANDOM )
				{
					emitter = model->GetEmitterFromVertex( 
						instanceGlobal,
						parentTime,
						m_pManager->GetCoordinateSystem(), 
						((EffectImplemented*)m_pEffectNode->GetEffect())->GetMaginification() );
				}
				else if( m_pEffectNode->GenerationLocation.model.type == ParameterGenerationLocation::MODELTYPE_FACE )
				{
					emitter = model->GetEmitterFromFace( 
						instanceNumber,
						parentTime,
						m_pManager->GetCoordinateSystem(), 
						((EffectImplemented*)m_pEffectNode->GetEffect())->GetMaginification() );
				}
				else if( m_pEffectNode->GenerationLocation.model.type == ParameterGenerationLocation::MODELTYPE_FACE_RANDOM )
				{
					emitter = model->GetEmitterFromFace( 
						instanceGlobal,
						parentTime,
						m_pManager->GetCoordinateSystem(), 
						((EffectImplemented*)m_pEffectNode->GetEffect())->GetMaginification() );
				}

				m_GenerationLocation.Translation( 
					emitter.Position.X, 
					emitter.Position.Y,
					emitter.Position.Z );

				if( m_pEffectNode->GenerationLocation.EffectsRotation )
				{
					m_GenerationLocation.Value[0][0] = emitter.Binormal.X;
					m_GenerationLocation.Value[0][1] = emitter.Binormal.Y;
					m_GenerationLocation.Value[0][2] = emitter.Binormal.Z;

					m_GenerationLocation.Value[1][0] = emitter.Tangent.X;
					m_GenerationLocation.Value[1][1] = emitter.Tangent.Y;
					m_GenerationLocation.Value[1][2] = emitter.Tangent.Z;

					m_GenerationLocation.Value[2][0] = emitter.Normal.X;
					m_GenerationLocation.Value[2][1] = emitter.Normal.Y;
					m_GenerationLocation.Value[2][2] = emitter.Normal.Z;
				}
			}
		}
	}
	else if( m_pEffectNode->GenerationLocation.type == ParameterGenerationLocation::TYPE_CIRCLE )
	{
		m_GenerationLocation.Indentity();
		float radius = m_pEffectNode->GenerationLocation.circle.radius.getValue(*instanceGlobal);
		float start = m_pEffectNode->GenerationLocation.circle.angle_start.getValue(*instanceGlobal);
		float end = m_pEffectNode->GenerationLocation.circle.angle_end.getValue(*instanceGlobal);
		int32_t div = Max(m_pEffectNode->GenerationLocation.circle.division, 1);

		int32_t target = 0;
		if(m_pEffectNode->GenerationLocation.circle.type == ParameterGenerationLocation::CIRCLE_TYPE_ORDER)
		{
			target = instanceNumber % div;
		}
		else if(m_pEffectNode->GenerationLocation.circle.type == ParameterGenerationLocation::CIRCLE_TYPE_REVERSE_ORDER)
		{
			target = div - 1 - (instanceNumber % div);
		}
		else if(m_pEffectNode->GenerationLocation.circle.type == ParameterGenerationLocation::CIRCLE_TYPE_RANDOM)
		{
			target = (int32_t)( (div) * instanceGlobal->GetRand() );
			if (target == div) target -= 1;
		}

		float angle = (end - start) * ((float)target / (float)div) + start;

		angle += m_pEffectNode->GenerationLocation.circle.angle_noize.getValue(*instanceGlobal);

		Matrix43 mat;
		if (m_pEffectNode->GenerationLocation.circle.axisDirection == ParameterGenerationLocation::AxisType::X)
		{
			mat.RotationX(angle);
			m_GenerationLocation.Translation(0, 0, radius);
		}
		if (m_pEffectNode->GenerationLocation.circle.axisDirection == ParameterGenerationLocation::AxisType::Y)
		{
			mat.RotationY(angle);
			m_GenerationLocation.Translation(radius, 0, 0);
		}
		if (m_pEffectNode->GenerationLocation.circle.axisDirection == ParameterGenerationLocation::AxisType::Z)
		{
			mat.RotationZ(angle);
			m_GenerationLocation.Translation(0, radius, 0);
		}

		
		Matrix43::Multiple( m_GenerationLocation, m_GenerationLocation, mat );
	}

	if( m_pEffectNode->SoundType == ParameterSoundType_Use )
	{
		soundValues.delay = (int32_t)m_pEffectNode->Sound.Delay.getValue( *instanceGlobal );
	}

	// UV
	if (m_pEffectNode->RendererCommon.UVType == ParameterRendererCommon::UV_ANIMATION)
	{
		uvTimeOffset = (int32_t)m_pEffectNode->RendererCommon.UV.Animation.StartFrame.getValue(*instanceGlobal);
		uvTimeOffset *= m_pEffectNode->RendererCommon.UV.Animation.FrameLength;
	}
	
	if (m_pEffectNode->RendererCommon.UVType == ParameterRendererCommon::UV_SCROLL)
	{
		auto xy = m_pEffectNode->RendererCommon.UV.Scroll.Position.getValue(*instanceGlobal);
		auto zw = m_pEffectNode->RendererCommon.UV.Scroll.Size.getValue(*instanceGlobal);

		uvAreaOffset.X = xy.x;
		uvAreaOffset.Y = xy.y;
		uvAreaOffset.Width = zw.x;
		uvAreaOffset.Height = zw.y;

		m_pEffectNode->RendererCommon.UV.Scroll.Speed.getValue(*instanceGlobal).setValueToArg(uvScrollSpeed);
	}

	if (m_pEffectNode->RendererCommon.UVType == ParameterRendererCommon::UV_FCURVE)
	{
		uvAreaOffset.X = m_pEffectNode->RendererCommon.UV.FCurve.Position->X.GetOffset(*instanceGlobal);
		uvAreaOffset.Y = m_pEffectNode->RendererCommon.UV.FCurve.Position->Y.GetOffset(*instanceGlobal);
		uvAreaOffset.Width = m_pEffectNode->RendererCommon.UV.FCurve.Size->X.GetOffset(*instanceGlobal);
		uvAreaOffset.Height = m_pEffectNode->RendererCommon.UV.FCurve.Size->Y.GetOffset(*instanceGlobal);
	}

	// CustomData
	for (int32_t index = 0; index < 2; index++)
	{
		ParameterCustomData* parameterCustomData = nullptr;
		InstanceCustomData* instanceCustomData = nullptr;

		if (index == 0)
		{
			parameterCustomData = &m_pEffectNode->RendererCommon.CustomData1;
			instanceCustomData = &customDataValues1;
		}
		else if (index == 1)
		{
			parameterCustomData = &m_pEffectNode->RendererCommon.CustomData2;
			instanceCustomData = &customDataValues2;
		}

		if (parameterCustomData->Type == ParameterCustomDataType::Fixed2D)
		{
			// none
		}
		else if (parameterCustomData->Type == ParameterCustomDataType::Easing2D)
		{
			instanceCustomData->easing.start = parameterCustomData->Easing.Values.start.getValue(*instanceGlobal);
			instanceCustomData->easing.end = parameterCustomData->Easing.Values.end.getValue(*instanceGlobal);
		}
		else if (parameterCustomData->Type == ParameterCustomDataType::FCurve2D)
		{
			instanceCustomData->fcruve.offset.x = parameterCustomData->FCurve.Values->X.GetOffset(*instanceGlobal);
			instanceCustomData->fcruve.offset.y = parameterCustomData->FCurve.Values->Y.GetOffset(*instanceGlobal);
		}
		else if (parameterCustomData->Type == ParameterCustomDataType::FCurveColor)
		{
			instanceCustomData->fcurveColor.offset[0] = parameterCustomData->FCurveColor.Values->R.GetOffset(*instanceGlobal);
			instanceCustomData->fcurveColor.offset[1] = parameterCustomData->FCurveColor.Values->G.GetOffset(*instanceGlobal);
			instanceCustomData->fcurveColor.offset[2] = parameterCustomData->FCurveColor.Values->B.GetOffset(*instanceGlobal);
			instanceCustomData->fcurveColor.offset[3] = parameterCustomData->FCurveColor.Values->A.GetOffset(*instanceGlobal);
		}
	}

	m_pEffectNode->InitializeRenderedInstance(*this, m_pManager);

	if (IsRequiredToCreateChildren(0.0f))
	{
		// calculate myself to update children group matrix
		CalculateMatrix(0);

		// for new children
		UpdateChildrenGroupMatrix();

		// Generate zero frame effect
		GenerateChildrenInRequired(0.0f);
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void Instance::Update( float deltaFrame, bool shown )
{
	assert(this->m_pContainer != nullptr);
	
	// Invalidate matrix
	m_GlobalMatrix43Calculated = false;
	m_ParentMatrix43Calculated = false;

	if (is_time_step_allowed && m_pEffectNode->GetType() != EFFECT_NODE_TYPE_ROOT)
	{
		/* 音の更新(現状放置) */
		if (m_pEffectNode->SoundType == ParameterSoundType_Use)
		{
			float living_time = m_LivingTime;
			float living_time_p = living_time + deltaFrame;

			if (living_time <= (float) soundValues.delay && (float) soundValues.delay < living_time_p)
			{
				m_pEffectNode->PlaySound_(*this, m_pContainer->GetRootInstance(), m_pManager);
			}
		}
	}

	float originalTime = m_LivingTime;

	// step time
	// frame 0 - generated time
	// frame 1- now
	if (is_time_step_allowed)
	{
		m_LivingTime += deltaFrame;
	}

	if(shown)
	{
		CalculateMatrix( deltaFrame );
	}
	else if( m_pEffectNode->LocationAbs.type != LocationAbsType::None )
	{
		// If attraction forces are not default, updating is needed in each frame.
		CalculateMatrix( deltaFrame );
	}

	// Get parent color.
	if (m_pParent != NULL)
	{
		if (m_pEffectNode->RendererCommon.ColorBindType == BindType::Always)
		{
			ColorParent = m_pParent->ColorInheritance;
		}
	}

	/* 親の削除処理 */
	if (m_pParent != NULL && m_pParent->GetState() != INSTANCE_STATE_ACTIVE)
	{
		CalculateParentMatrix( deltaFrame );
		m_pParent = nullptr;
	}

	// Create child particles
	if( is_time_step_allowed && (originalTime <= m_LivedTime || !m_pEffectNode->CommonValues.RemoveWhenLifeIsExtinct) )
	{
		GenerateChildrenInRequired(originalTime + deltaFrame);
	}

	UpdateChildrenGroupMatrix();
	/*
	for (InstanceGroup* group = childrenGroups_; group != nullptr; group = group->NextUsedByInstance)
	{
		group->SetParentMatrix(m_GlobalMatrix43);
	}
	*/

	// check whether killed?
	bool killed = false;
	if( m_pEffectNode->GetType() != EFFECT_NODE_TYPE_ROOT )
	{
		// if pass time
		if( m_pEffectNode->CommonValues.RemoveWhenLifeIsExtinct )
		{
			if( m_LivingTime > m_LivedTime )
			{
				killed = true;
			}
		}

		// if remove parent
		if( m_pEffectNode->CommonValues.RemoveWhenParentIsRemoved )
		{
			if( m_pParent == nullptr || m_pParent->GetState() != INSTANCE_STATE_ACTIVE )
			{
				m_pParent = nullptr;
				killed = true;
			}
		}

		// if children are removed and going not to generate a child
		if( !killed && m_pEffectNode->CommonValues.RemoveWhenChildrenIsExtinct )
		{
			int maxcreate_count = 0;
			InstanceGroup* group = childrenGroups_;

			for (int i = 0; i < m_pEffectNode->GetChildrenCount(); i++, group = group->NextUsedByInstance)
			{
				auto child = (EffectNodeImplemented*) m_pEffectNode->GetChild(i);

				if (maxGenerationChildrenCount[i] <= m_generatedChildrenCount[i] && group->GetInstanceCount() == 0)
				{
					maxcreate_count++;
				}
				else
				{
					break;
				}
			}
			
			if( maxcreate_count == m_pEffectNode->GetChildrenCount() )
			{
				killed = true;
			}
		}
	}

	if(killed)
	{
		// if it need to calculate a matrix
		if( m_pEffectNode->GetChildrenCount() > 0)
		{
			// Get parent color.
			if (m_pParent != nullptr)
			{
				if (m_pEffectNode->RendererCommon.ColorBindType == BindType::Always)
				{
					ColorParent = m_pParent->ColorInheritance;
				}
			}
		}

		// Delete this particle with myself.
		Kill();
		return;
	}

	// allow to pass time
	is_time_step_allowed = true;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void Instance::CalculateMatrix( float deltaFrame )
{
	// 計算済なら終了
	if( m_GlobalMatrix43Calculated ) return;
	
	//if( m_sequenceNumber == ((ManagerImplemented*)m_pManager)->GetSequenceNumber() ) return;
	m_sequenceNumber = ((ManagerImplemented*)m_pManager)->GetSequenceNumber();

	assert( m_pEffectNode != NULL );
	assert( m_pContainer != NULL );

	// 親の処理
	if( m_pParent != NULL )
	{
		CalculateParentMatrix( deltaFrame );
	}

	/* 更新処理 */
	if( m_pEffectNode->GetType() != EFFECT_NODE_TYPE_ROOT )
	{
		Vector3D localPosition;
		Vector3D localAngle;
		Vector3D localScaling;

		/* 位置の更新(時間から直接求めれるよう対応済み) */
		if( m_pEffectNode->TranslationType == ParameterTranslationType_None )
		{
			localPosition.X = 0;
			localPosition.Y = 0;
			localPosition.Z = 0;
		}
		else if( m_pEffectNode->TranslationType == ParameterTranslationType_Fixed )
		{
			localPosition = ApplyEq(m_pEffectNode->TranslationFixed.RefEq,
									m_pEffectNode->TranslationFixed.Position,
									m_pEffectNode->DynamicFactor.Tra,
									m_pEffectNode->DynamicFactor.TraInv);
		}
		else if( m_pEffectNode->TranslationType == ParameterTranslationType_PVA )
		{
			/* 現在位置 = 初期座標 + (初期速度 * t) + (初期加速度 * t * t * 0.5)*/
			localPosition.X = translation_values.random.location.x +
				(translation_values.random.velocity.x * m_LivingTime) +
				(translation_values.random.acceleration.x * m_LivingTime * m_LivingTime * 0.5f);

			localPosition.Y = translation_values.random.location.y +
				(translation_values.random.velocity.y * m_LivingTime) +
				(translation_values.random.acceleration.y * m_LivingTime * m_LivingTime * 0.5f);

			localPosition.Z = translation_values.random.location.z +
				(translation_values.random.velocity.z * m_LivingTime) +
				(translation_values.random.acceleration.z * m_LivingTime * m_LivingTime * 0.5f);

		}
		else if( m_pEffectNode->TranslationType == ParameterTranslationType_Easing )
		{
			m_pEffectNode->TranslationEasing.location.setValueToArg(
				localPosition,
				translation_values.easing.start,
				translation_values.easing.end,
				m_LivingTime / m_LivedTime );
		}
		else if( m_pEffectNode->TranslationType == ParameterTranslationType_FCurve )
		{
			assert( m_pEffectNode->TranslationFCurve != NULL );
			auto fcurve = m_pEffectNode->TranslationFCurve->GetValues(m_LivingTime, m_LivedTime);
			localPosition.X = fcurve[0] + translation_values.fcruve.offset.x;
			localPosition.Y = fcurve[1] + translation_values.fcruve.offset.y;
			localPosition.Z = fcurve[2] + translation_values.fcruve.offset.z;
		}

		if( !m_pEffectNode->GenerationLocation.EffectsRotation )
		{
			localPosition.X += m_GenerationLocation.Value[3][0];
			localPosition.Y += m_GenerationLocation.Value[3][1];
			localPosition.Z += m_GenerationLocation.Value[3][2];
		}

		/* 回転の更新(時間から直接求めれるよう対応済み) */
		if( m_pEffectNode->RotationType == ParameterRotationType_None )
		{
			localAngle.X = 0;
			localAngle.Y = 0;
			localAngle.Z = 0;
		}
		else if( m_pEffectNode->RotationType == ParameterRotationType_Fixed )
		{
			localAngle = ApplyEq(m_pEffectNode->RotationFixed.RefEq,
								 m_pEffectNode->RotationFixed.Position,
								 m_pEffectNode->DynamicFactor.Rot,
								 m_pEffectNode->DynamicFactor.RotInv);
		}
		else if( m_pEffectNode->RotationType == ParameterRotationType_PVA )
		{
			/* 現在位置 = 初期座標 + (初期速度 * t) + (初期加速度 * t * t * 0.5)*/
			localAngle.X = rotation_values.random.rotation.x +
				(rotation_values.random.velocity.x * m_LivingTime) +
				(rotation_values.random.acceleration.x * m_LivingTime * m_LivingTime * 0.5f);

			localAngle.Y = rotation_values.random.rotation.y +
				(rotation_values.random.velocity.y * m_LivingTime) +
				(rotation_values.random.acceleration.y * m_LivingTime * m_LivingTime * 0.5f);

			localAngle.Z = rotation_values.random.rotation.z +
				(rotation_values.random.velocity.z * m_LivingTime) +
				(rotation_values.random.acceleration.z * m_LivingTime * m_LivingTime * 0.5f);

		}
		else if( m_pEffectNode->RotationType == ParameterRotationType_Easing )
		{
			m_pEffectNode->RotationEasing.rotation.setValueToArg(
				localAngle,
				rotation_values.easing.start,
				rotation_values.easing.end,
				m_LivingTime / m_LivedTime );
		}
		else if( m_pEffectNode->RotationType == ParameterRotationType_AxisPVA )
		{
			rotation_values.axis.rotation = 
				rotation_values.axis.random.rotation +
				rotation_values.axis.random.velocity * m_LivingTime +
				rotation_values.axis.random.acceleration * m_LivingTime * m_LivingTime * 0.5f;
		}
		else if( m_pEffectNode->RotationType == ParameterRotationType_AxisEasing )
		{
			m_pEffectNode->RotationAxisEasing.easing.setValueToArg(
				rotation_values.axis.rotation,
				rotation_values.axis.easing.start,
				rotation_values.axis.easing.end,
				m_LivingTime / m_LivedTime );
		}
		else if( m_pEffectNode->RotationType == ParameterRotationType_FCurve )
		{
			assert( m_pEffectNode->RotationFCurve != NULL );
			auto fcurve = m_pEffectNode->RotationFCurve->GetValues(m_LivingTime, m_LivedTime);
			localAngle.X = fcurve[0] + rotation_values.fcruve.offset.x;
			localAngle.Y = fcurve[1] + rotation_values.fcruve.offset.y;
			localAngle.Z = fcurve[2] + rotation_values.fcruve.offset.z;
		}

		/* 拡大の更新(時間から直接求めれるよう対応済み) */
		if( m_pEffectNode->ScalingType == ParameterScalingType_None )
		{
			localScaling.X = 1.0f;
			localScaling.Y = 1.0f;
			localScaling.Z = 1.0f;
		}
		else if( m_pEffectNode->ScalingType == ParameterScalingType_Fixed )
		{
			localScaling = ApplyEq(m_pEffectNode->ScalingFixed.RefEq,
								   m_pEffectNode->ScalingFixed.Position,
								   m_pEffectNode->DynamicFactor.Scale,
								   m_pEffectNode->DynamicFactor.ScaleInv);
		}
		else if( m_pEffectNode->ScalingType == ParameterScalingType_PVA )
		{
			/* 現在位置 = 初期座標 + (初期速度 * t) + (初期加速度 * t * t * 0.5)*/
			localScaling.X = scaling_values.random.scale.x +
				(scaling_values.random.velocity.x * m_LivingTime) +
				(scaling_values.random.acceleration.x * m_LivingTime * m_LivingTime * 0.5f);

			localScaling.Y = scaling_values.random.scale.y +
				(scaling_values.random.velocity.y * m_LivingTime) +
				(scaling_values.random.acceleration.y * m_LivingTime * m_LivingTime * 0.5f);

			localScaling.Z = scaling_values.random.scale.z +
				(scaling_values.random.velocity.z * m_LivingTime) +
				(scaling_values.random.acceleration.z * m_LivingTime * m_LivingTime * 0.5f);
		}
		else if( m_pEffectNode->ScalingType == ParameterScalingType_Easing )
		{
			m_pEffectNode->ScalingEasing.Position.setValueToArg(
				localScaling,
				scaling_values.easing.start,
				scaling_values.easing.end,
				m_LivingTime / m_LivedTime );
		}
		else if( m_pEffectNode->ScalingType == ParameterScalingType_SinglePVA )
		{
			float s = scaling_values.single_random.scale +
				scaling_values.single_random.velocity * m_LivingTime +
				scaling_values.single_random.acceleration * m_LivingTime * m_LivingTime * 0.5f;
			localScaling.X = s;
			localScaling.Y = s;
			localScaling.Z = s;
		}
		else if( m_pEffectNode->ScalingType == ParameterScalingType_SingleEasing )
		{
			float scale;
			m_pEffectNode->ScalingSingleEasing.setValueToArg(
				scale,
				scaling_values.single_easing.start,
				scaling_values.single_easing.end,
				m_LivingTime / m_LivedTime );
			localScaling.X = scale;
			localScaling.Y = scale;
			localScaling.Z = scale;
		}
		else if( m_pEffectNode->ScalingType == ParameterScalingType_FCurve )
		{
			assert( m_pEffectNode->ScalingFCurve != NULL );
			auto fcurve = m_pEffectNode->ScalingFCurve->GetValues(m_LivingTime, m_LivedTime);
			localScaling.X = fcurve[0] + scaling_values.fcruve.offset.x;
			localScaling.Y = fcurve[1] + scaling_values.fcruve.offset.y;
			localScaling.Z = fcurve[2] + scaling_values.fcruve.offset.z;
		}

		/* 描画部分の更新 */
		m_pEffectNode->UpdateRenderedInstance( *this, m_pManager );

		// 回転行列の作成
		Matrix43 MatRot;
		if( m_pEffectNode->RotationType == ParameterRotationType_Fixed ||
			m_pEffectNode->RotationType == ParameterRotationType_PVA ||
			m_pEffectNode->RotationType == ParameterRotationType_Easing ||
			m_pEffectNode->RotationType == ParameterRotationType_FCurve )
		{
			MatRot.RotationZXY( localAngle.Z, localAngle.X, localAngle.Y );
		}
		else if( m_pEffectNode->RotationType == ParameterRotationType_AxisPVA ||
				 m_pEffectNode->RotationType == ParameterRotationType_AxisEasing )
		{
			Vector3D axis;
			axis.X = rotation_values.axis.axis.x;
			axis.Y = rotation_values.axis.axis.y;
			axis.Z = rotation_values.axis.axis.z;

			MatRot.RotationAxis( axis, rotation_values.axis.rotation );
		}
		else
		{
			MatRot.Indentity();
		}

		// 行列の更新
		m_GlobalMatrix43.SetSRT( localScaling, MatRot, localPosition );
		assert(m_GlobalMatrix43.IsValid());

		if( m_pEffectNode->GenerationLocation.EffectsRotation )
		{
			Matrix43::Multiple( m_GlobalMatrix43, m_GlobalMatrix43, m_GenerationLocation );
			assert(m_GlobalMatrix43.IsValid());
		}

		Matrix43::Multiple( m_GlobalMatrix43, m_GlobalMatrix43, m_ParentMatrix );
		assert(m_GlobalMatrix43.IsValid());

		if( m_pEffectNode->LocationAbs.type != LocationAbsType::None )
		{
			Vector3D currentPosition;
			m_GlobalMatrix43.GetTranslation( currentPosition );
			assert(m_GlobalMatrix43.IsValid());

			m_GlobalVelocity = currentPosition - m_GlobalPosition;
			m_GlobalPosition = currentPosition;

			ModifyMatrixFromLocationAbs( deltaFrame );
		}
	}

	m_GlobalMatrix43Calculated = true;
}

void Instance::CalculateParentMatrix( float deltaFrame )
{
	// 計算済なら終了
	if( m_ParentMatrix43Calculated ) return;

	// 親の行列を計算
	m_pParent->CalculateMatrix( deltaFrame );

	if (m_pEffectNode->GetType() != EFFECT_NODE_TYPE_ROOT)
	{
		BindType tType = m_pEffectNode->CommonValues.TranslationBindType;
		BindType rType = m_pEffectNode->CommonValues.RotationBindType;
		BindType sType = m_pEffectNode->CommonValues.ScalingBindType;

		if (tType == BindType::WhenCreating && rType == BindType::WhenCreating && sType == BindType::WhenCreating)
		{
			// do not do anything
		}
		else if (tType == BindType::Always && rType == BindType::Always && sType == BindType::Always)
		{
			m_ParentMatrix = ownGroup_->GetParentMatrix();
			assert(m_ParentMatrix.IsValid());
		}
		else
		{
			Vector3D s, t;
			Matrix43 r;

			if (tType == BindType::WhenCreating)
				m_ParentMatrix.GetTranslation(t);
			else
				t = ownGroup_->GetParentTranslation();

			if (rType == BindType::WhenCreating)
				m_ParentMatrix.GetRotation(r);
			else
				r = ownGroup_->GetParentRotation();

			if (sType == BindType::WhenCreating)
				m_ParentMatrix.GetScale(s);
			else
				s = ownGroup_->GetParentScale();

			m_ParentMatrix.SetSRT(s, r, t);
			assert(m_ParentMatrix.IsValid());
		}
	}

	m_ParentMatrix43Calculated = true;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void Instance::ModifyMatrixFromLocationAbs( float deltaFrame )
{
	InstanceGlobal* instanceGlobal = m_pContainer->GetRootInstance();

	// Update attraction forces
	if( m_pEffectNode->LocationAbs.type == LocationAbsType::None )
	{	
	}
	else if( m_pEffectNode->LocationAbs.type == LocationAbsType::Gravity )
	{
		m_GlobalRevisionLocation.X = m_pEffectNode->LocationAbs.gravity.x *
			m_LivingTime * m_LivingTime * 0.5f;
		m_GlobalRevisionLocation.Y = m_pEffectNode->LocationAbs.gravity.y *
			m_LivingTime * m_LivingTime * 0.5f;
		m_GlobalRevisionLocation.Z = m_pEffectNode->LocationAbs.gravity.z *
			m_LivingTime * m_LivingTime * 0.5f;
	}
	else if( m_pEffectNode->LocationAbs.type == LocationAbsType::AttractiveForce )
	{
		float force = m_pEffectNode->LocationAbs.attractiveForce.force;
		float control = m_pEffectNode->LocationAbs.attractiveForce.control;
		float minRange = m_pEffectNode->LocationAbs.attractiveForce.minRange;
		float maxRange = m_pEffectNode->LocationAbs.attractiveForce.maxRange;
		
		Vector3D position = m_GlobalPosition - m_GlobalVelocity + m_GlobalRevisionLocation;

		Vector3D targetDifference = instanceGlobal->GetTargetLocation() - position;
		float targetDistance = Vector3D::Length( targetDifference );
		if( targetDistance > 0.0f )
		{
			Vector3D targetDirection = targetDifference / targetDistance;
		
			if( minRange > 0.0f || maxRange > 0.0f )
			{
				if( targetDistance >= m_pEffectNode->LocationAbs.attractiveForce.maxRange )
				{
					force = 0.0f;
				}
				else if( targetDistance > m_pEffectNode->LocationAbs.attractiveForce.minRange )
				{
					force *= 1.0f - (targetDistance - minRange) / (maxRange - minRange);
				}
			}

			if (deltaFrame > 0)
			{
				float eps = 0.0001f;
				m_GlobalRevisionVelocity += targetDirection * force * deltaFrame;
				float currentVelocity = Vector3D::Length(m_GlobalRevisionVelocity) + eps;
				Vector3D currentDirection = m_GlobalRevisionVelocity / currentVelocity;

				m_GlobalRevisionVelocity = (targetDirection * control + currentDirection * (1.0f - control)) * currentVelocity;
				m_GlobalRevisionLocation += m_GlobalRevisionVelocity * deltaFrame;
			}
		}
	}

	Matrix43 MatTraGlobal;
	MatTraGlobal.Translation( m_GlobalRevisionLocation.X, m_GlobalRevisionLocation.Y, m_GlobalRevisionLocation.Z );
	Matrix43::Multiple( m_GlobalMatrix43, m_GlobalMatrix43, MatTraGlobal );
	assert(m_GlobalMatrix43.IsValid());
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void Instance::Draw(Instance* next)
{
	assert( m_pEffectNode != NULL );

	if( !m_pEffectNode->IsRendered ) return;

	if( m_sequenceNumber != ((ManagerImplemented*)m_pManager)->GetSequenceNumber() )
	{
		CalculateMatrix( 0 );
	}

	m_pEffectNode->Rendering(*this, next, m_pManager);
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void Instance::Kill()
{
	if( m_State == INSTANCE_STATE_ACTIVE )
	{
		for( InstanceGroup* group = childrenGroups_; group != NULL; group = group->NextUsedByInstance )
		{
			group->IsReferencedFromInstance = false;
		}

		m_State = INSTANCE_STATE_REMOVING;
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
RectF Instance::GetUV() const
{
	RectF uv(0.0f, 0.0f, 1.0f, 1.0f);

	if( m_pEffectNode->RendererCommon.UVType == ParameterRendererCommon::UV_DEFAULT )
	{
		return RectF( 0.0f, 0.0f, 1.0f, 1.0f );
	}
	else if( m_pEffectNode->RendererCommon.UVType == ParameterRendererCommon::UV_FIXED )
	{
		uv = RectF(
			m_pEffectNode->RendererCommon.UV.Fixed.Position.x,
			m_pEffectNode->RendererCommon.UV.Fixed.Position.y,
			m_pEffectNode->RendererCommon.UV.Fixed.Position.w,
			m_pEffectNode->RendererCommon.UV.Fixed.Position.h );
	}
	else if( m_pEffectNode->RendererCommon.UVType == ParameterRendererCommon::UV_ANIMATION )
	{
		auto time = m_LivingTime + uvTimeOffset;

		int32_t frameNum = (int32_t)(time / m_pEffectNode->RendererCommon.UV.Animation.FrameLength);
		int32_t frameCount = m_pEffectNode->RendererCommon.UV.Animation.FrameCountX * m_pEffectNode->RendererCommon.UV.Animation.FrameCountY;

		if( m_pEffectNode->RendererCommon.UV.Animation.LoopType == m_pEffectNode->RendererCommon.UV.Animation.LOOPTYPE_ONCE )
		{
			if( frameNum >= frameCount )
			{
				frameNum = frameCount - 1;
			}
		}
		else if ( m_pEffectNode->RendererCommon.UV.Animation.LoopType == m_pEffectNode->RendererCommon.UV.Animation.LOOPTYPE_LOOP )
		{
			frameNum %= frameCount;
		}
		else if ( m_pEffectNode->RendererCommon.UV.Animation.LoopType == m_pEffectNode->RendererCommon.UV.Animation.LOOPTYPE_REVERSELOOP )
		{
			bool rev = (frameNum / frameCount) % 2 == 1;
			frameNum %= frameCount;
			if( rev )
			{
				frameNum = frameCount - 1 - frameNum;
			}
		}

		int32_t frameX = frameNum % m_pEffectNode->RendererCommon.UV.Animation.FrameCountX;
		int32_t frameY = frameNum / m_pEffectNode->RendererCommon.UV.Animation.FrameCountX;

		uv = RectF(
			m_pEffectNode->RendererCommon.UV.Animation.Position.x + m_pEffectNode->RendererCommon.UV.Animation.Position.w * frameX,
			m_pEffectNode->RendererCommon.UV.Animation.Position.y + m_pEffectNode->RendererCommon.UV.Animation.Position.h * frameY,
			m_pEffectNode->RendererCommon.UV.Animation.Position.w,
			m_pEffectNode->RendererCommon.UV.Animation.Position.h );
	}
	else if( m_pEffectNode->RendererCommon.UVType == ParameterRendererCommon::UV_SCROLL )
	{
		auto time = (int32_t)m_LivingTime;

		uv = RectF(
			uvAreaOffset.X + uvScrollSpeed.X * time,
			uvAreaOffset.Y + uvScrollSpeed.Y * time,
			uvAreaOffset.Width,
			uvAreaOffset.Height);
	}
	else if (m_pEffectNode->RendererCommon.UVType == ParameterRendererCommon::UV_FCURVE)
	{
		auto time = (int32_t)m_LivingTime;

		auto fcurvePos = m_pEffectNode->RendererCommon.UV.FCurve.Position->GetValues(m_LivingTime, m_LivedTime);
		auto fcurveSize = m_pEffectNode->RendererCommon.UV.FCurve.Size->GetValues(m_LivingTime, m_LivedTime);

		uv = RectF(uvAreaOffset.X + fcurvePos[0],
					 uvAreaOffset.Y + fcurvePos[1],
					 uvAreaOffset.Width + fcurveSize[0],
					 uvAreaOffset.Height + fcurveSize[1]);
	}

	// For webgl bug (it makes slow if sampling points are too far on WebGL)
	float far = 4.0;

	if (uv.X < -far && uv.X + uv.Width < -far)
	{
		uv.X += far;
	}

	if (uv.X > far && uv.X + uv.Width > far)
	{
		uv.X -= far;
	}

	if (uv.Y < -far && uv.Y + uv.Height < -far)
	{
		uv.Y += far;
	}

	if (uv.Y > far && uv.Y + uv.Height > far)
	{
		uv.Y -= far;
	}

	return uv;
}

std::array<float, 4> Instance::GetCustomData(int32_t index) const
{
	assert(0 <= index && index < 2);

	ParameterCustomData* parameterCustomData = nullptr;
	const InstanceCustomData* instanceCustomData = nullptr;

	if (index == 0)
	{
		parameterCustomData = &m_pEffectNode->RendererCommon.CustomData1;
		instanceCustomData = &customDataValues1;
	}
	else if (index == 1)
	{
		parameterCustomData = &m_pEffectNode->RendererCommon.CustomData2;
		instanceCustomData = &customDataValues2;
	}
	else
	{
		return std::array<float, 4>{0.0f, 0.0f, 0, 0};
	}

	if (parameterCustomData->Type == ParameterCustomDataType::None)
	{
		return std::array<float, 4>{0, 0, 0, 0};
	}
	else if (parameterCustomData->Type == ParameterCustomDataType::Fixed2D)
	{
		auto v = parameterCustomData->Fixed.Values;
		return std::array<float, 4>{v.x, v.y, 0, 0};
	}
	else if (parameterCustomData->Type == ParameterCustomDataType::Easing2D)
	{
		vector2d v;
		parameterCustomData->Easing.Values.setValueToArg(
			v, instanceCustomData->easing.start, instanceCustomData->easing.end, m_LivingTime / m_LivedTime);
		return std::array<float, 4>{v.x, v.y, 0, 0};
	}
	else if (parameterCustomData->Type == ParameterCustomDataType::FCurve2D)
	{
		auto values = parameterCustomData->FCurve.Values->GetValues(m_LivingTime, m_LivedTime);
		return std::array<float, 4>{values[0] + instanceCustomData->fcruve.offset.x, values[1] + instanceCustomData->fcruve.offset.y, 0, 0};
	}
	else if (parameterCustomData->Type == ParameterCustomDataType::Fixed4D)
	{
		return parameterCustomData->Fixed4D;
	}
	else if (parameterCustomData->Type == ParameterCustomDataType::FCurveColor)
	{
		auto values = parameterCustomData->FCurveColor.Values->GetValues(m_LivingTime, m_LivedTime);
		return std::array<float, 4>{(values[0] + instanceCustomData->fcurveColor.offset[0]) / 255.0f,
									(values[1] + instanceCustomData->fcurveColor.offset[1]) / 255.0f,
									(values[2] + instanceCustomData->fcurveColor.offset[2]) / 255.0f,
									(values[3] + instanceCustomData->fcurveColor.offset[3]) / 255.0f};
	}
	else
	{
		assert(false);
	}

	return std::array<float, 4>{0, 0, 0, 0};
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------




namespace Effekseer
{

InstanceChunk::InstanceChunk() { std::fill(instancesAlive_.begin(), instancesAlive_.end(), false); }

InstanceChunk::~InstanceChunk() {}

void InstanceChunk::UpdateInstances(float deltaFrame)
{
	for (int32_t i = 0; i < InstancesOfChunk; i++)
	{
		if (instancesAlive_[i])
		{
			Instance* instance = reinterpret_cast<Instance*>(instances_[i]);

			if (instance->m_State == INSTANCE_STATE_ACTIVE)
			{
				instance->Update(deltaFrame, true);
			}
			else if (instance->m_State == INSTANCE_STATE_REMOVING)
			{
				// start to remove
				instance->m_State = INSTANCE_STATE_REMOVED;
			}
			else if (instance->m_State == INSTANCE_STATE_REMOVED)
			{
				instance->~Instance();
				instancesAlive_[i] = false;
				aliveCount_--;
			}
		}
	}
}
void InstanceChunk::UpdateInstancesByInstanceGlobal(InstanceGlobal* global, float deltaFrame)
{
	for (int32_t i = 0; i < InstancesOfChunk; i++)
	{
		if (instancesAlive_[i])
		{
			Instance* instance = reinterpret_cast<Instance*>(instances_[i]);
			
			if (global != instance->GetInstanceGlobal()) {
				continue;
			}

			if (instance->m_State == INSTANCE_STATE_ACTIVE)
			{
				instance->Update(deltaFrame, true);
			}
			else if (instance->m_State == INSTANCE_STATE_REMOVING)
			{
				// start to remove
				instance->m_State = INSTANCE_STATE_REMOVED;
			}
			else if (instance->m_State == INSTANCE_STATE_REMOVED)
			{
				instance->~Instance();
				instancesAlive_[i] = false;
				aliveCount_--;
			}
		}
	}
}

Instance* InstanceChunk::CreateInstance(Manager* pManager, EffectNode* pEffectNode, InstanceContainer* pContainer, InstanceGroup* pGroup)
{
	for (int32_t i = 0; i < InstancesOfChunk; i++)
	{
		if (!instancesAlive_[i])
		{
			instancesAlive_[i] = true;
			aliveCount_++;
			return new (instances_[i]) Instance(pManager, pEffectNode, pContainer, pGroup);
		}
	}
	return nullptr;
}

} // namespace Effekseer



//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
namespace Effekseer
{
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------

void* InstanceGlobal::operator new(size_t size)
{
	assert(sizeof(InstanceGlobal) == size);
	return GetMallocFunc()(size);
}

void InstanceGlobal::operator delete(void* p) {GetFreeFunc()(p, sizeof(InstanceGlobal)); }

InstanceGlobal::InstanceGlobal()
	: m_instanceCount	( 0 )
	, m_updatedFrame	( 0 )
	, m_rootContainer	( NULL )
{ 
	dynamicInputParameters.fill(0);
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
InstanceGlobal::~InstanceGlobal()
{
	
}

std::array<float, 4> InstanceGlobal::GetDynamicEquationResult(int32_t index) {
	assert(0 <= index && index < dynamicEqResults.size());
	return dynamicEqResults[index];
}

void InstanceGlobal::SetSeed(int64_t seed)
{
	m_seed = seed;
}

float InstanceGlobal::GetRand()
{
	const int a = 1103515245;
	const int c = 12345;
	const int m = 2147483647;
	
	m_seed = (m_seed * a + c) & m;
	auto ret = m_seed % 0x7fff;

	return (float)ret / (float) (0x7fff - 1);
}

float InstanceGlobal::GetRand(float min_, float max_)
{
	return GetRand() * (max_ - min_) + min_;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void InstanceGlobal::AddUpdatedFrame( float frame )
{
	m_updatedFrame += frame;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void InstanceGlobal::IncInstanceCount()
{
	m_instanceCount++;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void InstanceGlobal::DecInstanceCount()
{
	m_instanceCount--;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
int InstanceGlobal::GetInstanceCount()
{
	return m_instanceCount;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
float InstanceGlobal::GetUpdatedFrame()
{
	return m_updatedFrame;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
InstanceContainer* InstanceGlobal::GetRootContainer() const
{
	return m_rootContainer;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void InstanceGlobal::SetRootContainer( InstanceContainer* container )
{
	m_rootContainer = container;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
const Vector3D& InstanceGlobal::GetTargetLocation() const
{
	return m_targetLocation;
}

void InstanceGlobal::SetTargetLocation( const Vector3D& location )
{
	m_targetLocation = location;
}

float InstanceGlobal::Rand(void* userData) { 
	auto g = reinterpret_cast<InstanceGlobal*>(userData);
	return g->GetRand();
}

float InstanceGlobal::RandSeed(void* userData, float randSeed)
{
	auto seed = static_cast<int64_t>(randSeed * 1024 * 8);
	const int a = 1103515245;
	const int c = 12345;
	const int m = 2147483647;

	seed = (seed * a + c) & m;
	auto ret = seed % 0x7fff;

	return (float)ret / (float)(0x7fff - 1);
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------


//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------


//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
namespace Effekseer
{


//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
InstanceGroup::InstanceGroup( Manager* manager, EffectNode* effectNode, InstanceContainer* container, InstanceGlobal* global )
	: m_manager		( (ManagerImplemented*)manager )
	, m_effectNode((EffectNodeImplemented*) effectNode)
	, m_container	( container )
	, m_global		( global )
	, m_time		( 0 )
	, IsReferencedFromInstance	( true )
	, NextUsedByInstance	( NULL )
	, NextUsedByContainer	( NULL )
{
	parentMatrix_.Indentity();
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
InstanceGroup::~InstanceGroup()
{
	RemoveForcibly();
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
Instance* InstanceGroup::CreateInstance()
{
	Instance* instance = NULL;

	instance = m_manager->CreateInstance( m_effectNode, m_container, this );
	
	if( instance )
	{
		m_instances.push_back( instance );
		m_global->IncInstanceCount();
	}
	return instance;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
Instance* InstanceGroup::GetFirst()
{
	if( m_instances.size() > 0 )
	{
		return m_instances.front();
	}
	return NULL;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
int InstanceGroup::GetInstanceCount() const
{
	return (int32_t)m_instances.size();
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void InstanceGroup::Update( float deltaFrame, bool shown )
{
	for (auto it = m_instances.begin(); it != m_instances.end();)
	{
		auto instance = *it;

		if (instance->m_State != INSTANCE_STATE_ACTIVE)
		{
			it = m_instances.erase(it);
			m_global->DecInstanceCount();
		}
		else
		{
			it++;
		}
	}

	m_time++;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void InstanceGroup::SetBaseMatrix( const Matrix43& mat )
{
	for (auto instance : m_instances)
	{
		if (instance->m_State == INSTANCE_STATE_ACTIVE)
		{
			Matrix43::Multiple(instance->m_GlobalMatrix43, instance->m_GlobalMatrix43, mat);
			assert(instance->m_GlobalMatrix43.IsValid());
		}
	}
}

void InstanceGroup::SetParentMatrix(const Matrix43& mat)
{
	BindType tType = m_effectNode->CommonValues.TranslationBindType;
	BindType rType = m_effectNode->CommonValues.RotationBindType;
	BindType sType = m_effectNode->CommonValues.ScalingBindType;

	auto rootGroup = m_global->GetRootContainer()->GetFirstGroup();

	if (tType == BindType::Always && rType == BindType::Always && sType == BindType::Always)
	{
		parentMatrix_ = mat;
	}
	else if (tType == BindType::NotBind_Root && rType == BindType::NotBind_Root && sType == BindType::NotBind_Root)
	{
		parentMatrix_ = rootGroup->GetParentMatrix();
	}
	else if (tType == BindType::WhenCreating && rType == BindType::WhenCreating && sType == BindType::WhenCreating)
	{
		// don't do anything
	}
	else
	{
		Vector3D s, t;
		Matrix43 r;
		mat.GetSRT(s, r, t);

		if (tType == BindType::Always)
		{
			parentTranslation_ = t;
		}
		else if (tType == BindType::NotBind_Root)
		{
			parentTranslation_ = rootGroup->GetParentTranslation();
		}
		else if (tType == BindType::NotBind)
		{
			parentTranslation_ = Vector3D(0.0f, 0.0f, 0.0f);
		}

		if (rType == BindType::Always)
		{
			parentRotation_ = r;
		}
		else if (rType == BindType::NotBind_Root)
		{
			parentRotation_ = rootGroup->GetParentRotation();
		}
		else if (rType == BindType::NotBind)
		{
			parentRotation_.Indentity();
		}

		if (sType == BindType::Always)
		{
			parentScale_ = s;
		}
		else if (sType == BindType::NotBind_Root)
		{
			parentScale_ = rootGroup->GetParentScale();
		}
		else if (sType == BindType::NotBind)
		{
			parentScale_ = Vector3D(1.0f, 1.0f, 1.0f);
		}
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void InstanceGroup::RemoveForcibly()
{
	KillAllInstances();
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void InstanceGroup::KillAllInstances()
{
	while (!m_instances.empty())
	{
		auto instance = m_instances.front();
		m_instances.pop_front();

		if (instance->GetState() == INSTANCE_STATE_ACTIVE)
		{
			instance->Kill();
		}
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------

namespace Effekseer
{

bool InternalScript::IsValidOperator(int value) const
{
	if (0 <= value && value <= 4)
		return true;
	if (11 <= value && value <= 12)
		return true;
	if (21 <= value && value <= 22)
		return true;
	if (31 <= value && value <= 32)
		return true;

	return false;
}

bool InternalScript::IsValidRegister(int index) const
{
	if (index < 0)
		return false;

	if (static_cast<uint32_t>(index) < registers.size())
		return true;

	if (0x1000 + 0 <= index && index <= 0x1000 + 3)
		return true;

	if (0x1000 + 0x100 + 0 <= index && index <= 0x1000 + 0x100 + 0)
		return true;

	if (0x1000 + 0x200 + 0 <= index && index <= 0x1000 + 0x200 + 4)
		return true;

	return false;
}

float InternalScript::GetRegisterValue(int index,
									   const std::array<float, 4>& externals,
									   const std::array<float, 1>& globals,
									   const std::array<float, 5>& locals) const
{
	auto ind = static_cast<uint32_t>(index);
	if (ind < registers.size())
	{
		return registers[ind];
	}
	else if (0x1000 + 0 <= ind && ind <= 0x1000 + 3)
	{
		return externals[ind - 0x1000];
	}
	else if (0x1000 + 0x100 + 0 <= ind && ind <= 0x1000 + 0x100 + 0)
	{
		return globals[ind - 0x1000 - 0x100];
	}
	else if (0x1000 + 0x200 + 0 <= ind && ind <= 0x1000 + 0x200 + 4)
	{
		return locals[ind - 0x1000 - 0x200];
	}

	assert(false);
	return 0.0f;
}

InternalScript::InternalScript() {}

InternalScript ::~InternalScript() {}
bool InternalScript::Load(uint8_t* data, int size)
{
	if (data == nullptr || size <= 0)
		return false;
	BinaryReader<true> reader(data, static_cast<size_t>(size));

	int32_t registerCount = 0;

	reader.Read(version_);
	reader.Read(runningPhase);
	reader.Read(registerCount);
	reader.Read(operatorCount_);

	for (size_t i = 0; i < 4; i++)
		reader.Read(outputRegisters_[i]);

	if (registerCount < 0)
		return false;

	registers.resize(registerCount);

	for (size_t i = 0; i < 4; i++)
	{
		if (!IsValidRegister(outputRegisters_[i]))
		{
			return false;
		}
	}

	reader.Read(operators, size - reader.GetOffset());

	if (reader.GetStatus() == BinaryReaderStatus::Failed)
		return false;

	// check operators
	auto operatorReader = BinaryReader<true>(operators.data(), operators.size());

	for (int i = 0; i < operatorCount_; i++)
	{
		// type
		OperatorType type;
		operatorReader.Read(type);

		if (reader.GetStatus() == BinaryReaderStatus::Failed)
			return false;

		if (!IsValidOperator((int)type))
			return false;

		int32_t inputCount = 0;
		operatorReader.Read(inputCount);

		int32_t outputCount = 0;
		operatorReader.Read(outputCount);

		int32_t attributeCount = 0;
		operatorReader.Read(attributeCount);

		// input
		for (int j = 0; j < inputCount; j++)
		{
			int index = 0;
			operatorReader.Read(index);
			if (!IsValidRegister(index))
			{
				return false;
			}
		}

		// output
		for (int j = 0; j < outputCount; j++)
		{
			int index = 0;
			operatorReader.Read(index);
			if ((index < 0 || index >= static_cast<int32_t>(registers.size())))
			{
				return false;
			}
		}

		// attribute
		for (int j = 0; j < attributeCount; j++)
		{
			int index = 0;
			operatorReader.Read(index);
		}
	}

	if (operatorReader.GetStatus() != BinaryReaderStatus::Complete)
		return false;

	isValid_ = true;

	return true;
}

std::array<float, 4> InternalScript::Execute(const std::array<float, 4>& externals,
											 const std::array<float, 1>& globals,
											 const std::array<float, 5>& locals,
											 RandFuncCallback* randFuncCallback,
											 RandWithSeedFuncCallback* randSeedFuncCallback,
											 void* userData)
{
	std::array<float, 4> ret;
	ret.fill(0.0f);

	if (!isValid_)
	{
		return ret;
	}

	int offset = 0;
	for (int i = 0; i < operatorCount_; i++)
	{
		// type
		OperatorType type;
		memcpy(&type, operators.data() + offset, sizeof(OperatorType));
		offset += sizeof(int);

		int32_t inputCount = 0;
		memcpy(&inputCount, operators.data() + offset, sizeof(int));
		offset += sizeof(int);

		int32_t outputCount = 0;
		memcpy(&outputCount, operators.data() + offset, sizeof(int));
		offset += sizeof(int);

		int32_t attributeCount = 0;
		memcpy(&attributeCount, operators.data() + offset, sizeof(int));
		offset += sizeof(int);

		auto inputOffset = offset;
		auto outputOffset = inputOffset + inputCount * sizeof(int);
		auto attributeOffset = outputOffset + outputCount * sizeof(int);
		offset = attributeOffset + attributeCount * sizeof(int);

		std::array<float, 8> tempInputs;
		if (inputCount > tempInputs.size())
			return ret;

		for (int j = 0; j < inputCount; j++)
		{
			int index = 0;
			memcpy(&index, operators.data() + inputOffset, sizeof(int));
			inputOffset += sizeof(int);

			tempInputs[j] = GetRegisterValue(index, externals, globals, locals);
		}

		for (int j = 0; j < outputCount; j++)
		{
			int index = 0;
			memcpy(&index, operators.data() + outputOffset, sizeof(int));
			outputOffset += sizeof(int);

			if (type == OperatorType::Add)
				registers[index] = tempInputs[0] + tempInputs[1];
			else if (type == OperatorType::Sub)
				registers[index] = tempInputs[0] - tempInputs[1];
			else if (type == OperatorType::Mul)
				registers[index] = tempInputs[0] * tempInputs[1];
			else if (type == OperatorType::Div)
				registers[index] = tempInputs[0] / tempInputs[1];
			else if (type == OperatorType::Sine)
				registers[index] = sin(tempInputs[0]);
			else if (type == OperatorType::Cos)
				registers[index] = cos(tempInputs[0]);
			else if (type == OperatorType::UnaryAdd)
				registers[index] = tempInputs[0];
			else if (type == OperatorType::UnarySub)
				registers[index] = -tempInputs[0];
			else if (type == OperatorType::Rand)
			{
				registers[index] = randFuncCallback(userData);
			}
			else if (type == OperatorType::Rand_WithSeed)
			{
				registers[index] = randSeedFuncCallback(userData, tempInputs[0]);
			}
			else if (type == OperatorType::Constant)
			{
				float att = 0;
				memcpy(&att, operators.data() + attributeOffset, sizeof(int));
				registers[index] = att;
			}
		}
	}

	for (size_t i = 0; i < 4; i++)
	{
		ret[i] = GetRegisterValue(outputRegisters_[i], externals, globals, locals);
	}

	return ret;
}

} // namespace Effekseer

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------





//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
namespace Effekseer {

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
Setting::Setting()
	: m_coordinateSystem(CoordinateSystem::RH)
	, m_effectLoader(NULL)
	, m_textureLoader(NULL)
	, m_soundLoader(NULL)
	, m_modelLoader(NULL)
{
	auto effectFactory = new EffectFactory();
	effectFactories.push_back(effectFactory);

	// this function is for 1.6
	auto efkefcFactory = new EfkEfcFactory();
	effectFactories.push_back(efkefcFactory);
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
Setting::~Setting()
{
	ClearEffectFactory();

	ES_SAFE_DELETE(m_effectLoader);
	ES_SAFE_DELETE(m_textureLoader);
	ES_SAFE_DELETE(m_soundLoader);
	ES_SAFE_DELETE(m_modelLoader);
	ES_SAFE_DELETE(m_materialLoader);
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
Setting* Setting::Create()
{
	return new Setting();
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
CoordinateSystem Setting::GetCoordinateSystem() const
{
	return m_coordinateSystem;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void Setting::SetCoordinateSystem(CoordinateSystem coordinateSystem)
{
	m_coordinateSystem = coordinateSystem;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
EffectLoader* Setting::GetEffectLoader()
{
	return m_effectLoader;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void Setting::SetEffectLoader(EffectLoader* loader)
{
	ES_SAFE_DELETE(m_effectLoader);
	m_effectLoader = loader;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
TextureLoader* Setting::GetTextureLoader()
{
	return m_textureLoader;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void Setting::SetTextureLoader(TextureLoader* loader)
{
	ES_SAFE_DELETE(m_textureLoader);
	m_textureLoader = loader;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
ModelLoader* Setting::GetModelLoader()
{
	return m_modelLoader;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void Setting::SetModelLoader(ModelLoader* loader)
{
	ES_SAFE_DELETE(m_modelLoader);
	m_modelLoader = loader;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
SoundLoader* Setting::GetSoundLoader()
{
	return m_soundLoader;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void Setting::SetSoundLoader(SoundLoader* loader)
{
	ES_SAFE_DELETE(m_soundLoader);
	m_soundLoader = loader;
}

MaterialLoader* Setting::GetMaterialLoader()
{ return m_materialLoader;
}

void Setting::SetMaterialLoader(MaterialLoader* loader)
{
	ES_SAFE_DELETE(m_materialLoader);
	m_materialLoader = loader;
}

void Setting::AddEffectFactory(EffectFactory* effectFactory) { 
	
	if (effectFactory == nullptr)
		return;
	ES_SAFE_ADDREF(effectFactory); 
	effectFactories.push_back(effectFactory);
}

void Setting::ClearEffectFactory()
{
	for (auto& e : effectFactories)
	{
		ES_SAFE_RELEASE(e);
	}
	effectFactories.clear();
}

EffectFactory* Setting::GetEffectFactory(int32_t ind) const
{
	return effectFactories[ind]; 
}

int32_t Setting::GetEffectFactoryCount() const { 
	return effectFactories.size();
}

}
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------

#if PLATFORM_WINDOWS

#ifndef	__EFFEKSEER_SOCKET_H__
#define	__EFFEKSEER_SOCKET_H__

#if !( defined(_PSVITA) || defined(_XBOXONE) )

//----------------------------------------------------------------------------------
// Include
//----------------------------------------------------------------------------------

#if defined(_WIN32) && !defined(_PS4) 
#else

#if !defined(_PS4) 
#endif

#endif

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
namespace Effekseer {
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------

#if defined(_WIN32) && !defined(_PS4) 

typedef SOCKET	EfkSocket;
typedef int		SOCKLEN;
const EfkSocket InvalidSocket = INVALID_SOCKET;
const int32_t SocketError = SOCKET_ERROR;
const int32_t InaddrNone = INADDR_NONE;

#else

typedef int32_t	EfkSocket;
typedef socklen_t SOCKLEN;
const EfkSocket InvalidSocket = -1;
const int32_t SocketError = -1;
const int32_t InaddrNone = -1;

typedef struct hostent HOSTENT;
typedef struct in_addr IN_ADDR;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;

#endif

#if defined(_WIN32) && !defined(_PS4) 
static void Sleep_(int32_t ms)
{
	Sleep(ms);
}
#else
static void Sleep_(int32_t ms)
{
	usleep(1000 * ms);
}
#endif


class Socket
{
private:

public:
	static void Initialize();
	static void Finalize();

	static EfkSocket GenSocket();

	static void Close( EfkSocket s );
	static void Shutsown( EfkSocket s );

	static bool Listen( EfkSocket s, int32_t backlog );
};

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
 } 
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------

#endif	// #if !( defined(_PSVITA) || defined(_XBOXONE) )

#endif	// __EFFEKSEER_SOCKET_H__


#if !( defined(_PSVITA) || defined(_XBOXONE) )

//----------------------------------------------------------------------------------
// Include
//----------------------------------------------------------------------------------
#if defined(_WIN32) && !defined(_PS4) 
#pragma comment( lib, "ws2_32.lib" )
#else
#endif


//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
namespace Effekseer {
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void Socket::Initialize()
{
#if defined(_WIN32) && !defined(_PS4) 
	// Initialize  Winsock
	WSADATA m_WsaData;
	::WSAStartup( MAKEWORD(2,0), &m_WsaData );
#endif
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void Socket::Finalize()
{
#if defined(_WIN32) && !defined(_PS4) 
	// Dispose winsock or decrease a counter
	WSACleanup();
#endif
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
EfkSocket Socket::GenSocket()
{
	return ::socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void Socket::Close( EfkSocket s )
{
#if defined(_WIN32) && !defined(_PS4) 
	::closesocket( s );
#else
	::close( s );
#endif
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void Socket::Shutsown( EfkSocket s )
{
#if defined(_WIN32) && !defined(_PS4) 
	::shutdown( s, SD_BOTH );
#else
	::shutdown( s, SHUT_RDWR );
#endif
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
bool Socket::Listen( EfkSocket s, int32_t backlog )
{
#if defined(_WIN32) && !defined(_PS4) 
	return ::listen( s, backlog ) != SocketError;
#else
	return listen( s, backlog ) >= 0;
#endif
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
 } 
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------

#endif	// #if !( defined(_PSVITA) || defined(_XBOXONE) )



#ifndef	__EFFEKSEER_SERVER_IMPLEMENTED_H__
#define	__EFFEKSEER_SERVER_IMPLEMENTED_H__

#if !( defined(_PSVITA) || defined(_SWITCH) || defined(_XBOXONE) )

//----------------------------------------------------------------------------------
// Include
//----------------------------------------------------------------------------------




//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
namespace Effekseer {
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
class ServerImplemented : public Server
{
private:
	class InternalClient
	{
	public:
		std::thread		m_threadRecv;
		EfkSocket	m_socket;
		ServerImplemented*		m_server;
		bool		m_active;

		std::vector<uint8_t>	m_recvBuffer;

		std::vector<std::vector<uint8_t> >	m_recvBuffers;
		std::mutex						m_ctrlRecvBuffers;

		static void RecvAsync( void* data );

	public:
		InternalClient( EfkSocket socket_, ServerImplemented* server );
		~InternalClient();
		void ShutDown();
	};

private:
	EfkSocket	m_socket;
	uint16_t	m_port;

	std::thread		m_thread;
	std::mutex		m_ctrlClients;

	bool		m_running;

	std::set<InternalClient*>	m_clients;
	std::set<InternalClient*>	m_removedClients;

	std::map<std::u16string,Effect*>	m_effects;

	std::map<std::u16string,std::vector<uint8_t> >	m_data;

	std::vector<EFK_CHAR>	m_materialPath;

	void AddClient( InternalClient* client );
	void RemoveClient( InternalClient* client );
	static void AcceptAsync( void* data );

public:

	ServerImplemented();
	virtual ~ServerImplemented();

	bool Start( uint16_t port ) override;

	void Stop() override;

	void Register(const EFK_CHAR* key, Effect* effect) override;

	void Unregister(Effect* effect) override;

	void Update(Manager** managers, int32_t managerCount, ReloadingThreadType reloadingThreadType) override;

	void SetMaterialPath( const EFK_CHAR* materialPath ) override;

	void Regist(const EFK_CHAR* key, Effect* effect) override;

	void Unregist(Effect* effect) override;

};

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
 } 
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------

#endif	// #if !( defined(_PSVITA) || defined(_SWITCH) || defined(_XBOXONE) )

#endif	// __EFFEKSEER_SERVER_IMPLEMENTED_H__


#if !( defined(_PSVITA) || defined(_XBOXONE) )

//----------------------------------------------------------------------------------
// Include
//----------------------------------------------------------------------------------


#if defined(_WIN32) && !defined(_PS4) 
#else
#endif

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
namespace Effekseer {
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void ServerImplemented::InternalClient::RecvAsync( void* data )
{
	InternalClient* client = (InternalClient*)data;

	while(true)
	{
		client->m_recvBuffer.clear();

		int32_t size = 0;
		int32_t restSize = 0;

		restSize = 4;
		while(restSize > 0)
		{
			auto recvSize = ::recv( client->m_socket, (char*)(&size), restSize, 0 );
			restSize -= recvSize;

			if( recvSize == 0 || recvSize == -1 )
			{
				// Failed
				client->m_server->RemoveClient( client );
				client->ShutDown();
				return;
			}
		}

		restSize = size;
		while(restSize > 0)
		{
			uint8_t buf[256];

			auto recvSize = ::recv( client->m_socket, (char*)(buf), Min(restSize,256), 0 );
			restSize -= recvSize;

			if( recvSize == 0 || recvSize == -1 )
			{
				// Failed
				client->m_server->RemoveClient( client );
				client->ShutDown();
				return;
			}

			for( int32_t i = 0; i < recvSize; i++ )
			{
				client->m_recvBuffer.push_back( buf[i] );
			}
		}

		// recieve buffer
		client->m_ctrlRecvBuffers.lock();
		client->m_recvBuffers.push_back(client->m_recvBuffer);
		client->m_ctrlRecvBuffers.unlock();
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
ServerImplemented::InternalClient::InternalClient( EfkSocket socket_, ServerImplemented* server )
	: m_socket	( socket_ )
	, m_server	( server )
	, m_active	( true )
{
	m_threadRecv = std::thread(
		[this]() 
	{
		RecvAsync(this);
	});
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
ServerImplemented::InternalClient::~InternalClient()
{
	m_threadRecv.join();
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void ServerImplemented::InternalClient::ShutDown()
{
	if ( m_socket != InvalidSocket )
	{
		Socket::Shutsown( m_socket );
		Socket::Close( m_socket );
		m_socket = InvalidSocket;
		m_active = false;
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
ServerImplemented::ServerImplemented()
	: m_running		( false )
{
	Socket::Initialize();
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
ServerImplemented::~ServerImplemented()
{
	Stop();

	Socket::Finalize();
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
Server* Server::Create()
{
	return new ServerImplemented();
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void ServerImplemented::AddClient( InternalClient* client )
{
	std::lock_guard<std::mutex> lock(m_ctrlClients);
	m_clients.insert( client );
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void ServerImplemented::RemoveClient( InternalClient* client )
{
	std::lock_guard<std::mutex> lock(m_ctrlClients);
	if( m_clients.count( client ) > 0 )
	{
		m_clients.erase( client );
		m_removedClients.insert( client );
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void ServerImplemented::AcceptAsync( void* data )
{
	ServerImplemented* server = (ServerImplemented*)data;

	while(true)
	{
		SOCKADDR_IN socketAddrIn;
		int32_t Size = sizeof(SOCKADDR_IN);

		EfkSocket socket_ = ::accept( 
			server->m_socket, 
			(SOCKADDR*)(&socketAddrIn),
			(SOCKLEN*)(&Size) );

		if ( server->m_socket == InvalidSocket || socket_ == InvalidSocket )
		{
			break;
		}

		// Accept and add an internal client
		server->AddClient( new InternalClient( socket_, server ) );

		EffekseerPrintDebug("Server : AcceptClient\n");

	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
bool ServerImplemented::Start( uint16_t port )
{
	if( m_running )
	{
		Stop();
	}

	int32_t returnCode;
	sockaddr_in sockAddr = { AF_INET };

	// Create a socket
	EfkSocket socket_ = Socket::GenSocket();
	if ( socket_ == InvalidSocket )
	{
		return false;
	}

	memset( &sockAddr, 0, sizeof(SOCKADDR_IN));
	sockAddr.sin_family	= AF_INET;
	sockAddr.sin_port	= htons( port );

	returnCode = ::bind( socket_, (sockaddr*)&sockAddr, sizeof(sockaddr_in) );
	if ( returnCode == SocketError )
	{
		if ( socket_ != InvalidSocket )
		{
			Socket::Close( socket_ );
		}
		return false;
	}

	// Connect
	if ( !Socket::Listen( socket_, 30 ) )
	{
		if ( socket_ != InvalidSocket )
		{
			Socket::Close( socket_ );
		}
		return false;
	}

	m_running = true;
	m_socket = socket_;
	m_port = port;

	m_thread = std::thread(
		[this]()
	{
		AcceptAsync(this);
	});

	EffekseerPrintDebug("Server : Start\n");

	return true;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void ServerImplemented::Stop()
{
	if( !m_running ) return;

	Socket::Shutsown( m_socket );
	Socket::Close( m_socket );
	m_socket = InvalidSocket;
	
	m_running = false;

	m_thread.join();

	// Stop clients
	m_ctrlClients.lock();
	for( std::set<InternalClient*>::iterator it = m_clients.begin(); it != m_clients.end(); ++it )
	{
		(*it)->ShutDown();
	}
	m_ctrlClients.unlock();
	

	// Wait clients to be removed
	while(true)
	{
		m_ctrlClients.lock();
		int32_t size = (int32_t)m_clients.size();
		m_ctrlClients.unlock();
	
		if( size == 0 ) break;

		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	// Delete clients
	for( std::set<InternalClient*>::iterator it = m_removedClients.begin(); it != m_removedClients.end(); ++it )
	{
		while( (*it)->m_active )
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
		delete (*it);
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void ServerImplemented::Register( const EFK_CHAR* key, Effect* effect )
{
	if( effect == NULL ) return;

	std::u16string key_( (const char16_t*)key );

	if( m_effects.count( key_ ) > 0 )
	{
		m_effects[key_]->Release();
	}

	m_effects[key_] = effect;
	m_effects[key_]->AddRef();

	if( m_data.count( key_ ) > 0 )
	{
		if( m_materialPath.size() > 1 )
		{
			m_effects[key_]->Reload( &(m_data[key_][0]), (int32_t)m_data.size(), &(m_materialPath[0]) );
		}
		else
		{
			m_effects[key_]->Reload( &(m_data[key_][0]), (int32_t)m_data.size() );
		}
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void ServerImplemented::Unregister( Effect* effect )
{
	if( effect == NULL ) return;

	auto it = m_effects.begin();
	auto it_end = m_effects.end();

	while( it != it_end )
	{
		if( (*it).second == effect )
		{
			(*it).second->Release();
			m_effects.erase( it );
			return;
		}

		it++;
	}
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void ServerImplemented::Update(Manager** managers, int32_t managerCount, ReloadingThreadType reloadingThreadType)
{
	m_ctrlClients.lock();

	for( std::set<InternalClient*>::iterator it = m_removedClients.begin(); it != m_removedClients.end(); ++it )
	{
		while( (*it)->m_active )
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
		delete (*it);
	}
	m_removedClients.clear();

	for( std::set<InternalClient*>::iterator it = m_clients.begin(); it != m_clients.end(); ++it )
	{
		(*it)->m_ctrlRecvBuffers.lock();

		for( size_t i = 0; i < (*it)->m_recvBuffers.size(); i++ )
		{
			std::vector<uint8_t>& buf = (*it)->m_recvBuffers[i];

			uint8_t* p = &(buf[0]);
		
			int32_t keylen = 0;
			memcpy( &keylen, p, sizeof(int32_t) );
			p += sizeof(int32_t);

			std::u16string key;
			for( int32_t k = 0; k < keylen; k++ )
			{
				key.push_back( ((char16_t*)p)[0] );
				p += sizeof(char16_t);
			}

			uint8_t* recv_data = p;
			auto datasize = (int32_t)buf.size() - (p-&(buf[0]));
		
			if( m_data.count( key ) > 0 )
			{
				m_data[key].clear();
			}

			for( int32_t d = 0; d < datasize; d++ )
			{
				m_data[key].push_back( recv_data[d] );
			}

			if( m_effects.count( key ) > 0 )
			{
				if (managers != nullptr)
				{
					auto& data_ = m_data[key];

					if (m_materialPath.size() > 1)
					{
						m_effects[key]->Reload(managers, managerCount, data_.data(), (int32_t)data_.size(), &(m_materialPath[0]), reloadingThreadType);
					}
					else
					{
						m_effects[key]->Reload(managers, managerCount, data_.data(), (int32_t)data_.size(), nullptr, reloadingThreadType);
					}
				}
				else
				{
					auto& data_ = m_data[key];

					if (m_materialPath.size() > 1)
					{
						m_effects[key]->Reload(data_.data(), (int32_t)data_.size(), &(m_materialPath[0]), reloadingThreadType);
					}
					else
					{
						m_effects[key]->Reload(data_.data(), (int32_t)data_.size(), nullptr, reloadingThreadType);
					}
				}
				
			}
		}

		(*it)->m_recvBuffers.clear();
		(*it)->m_ctrlRecvBuffers.unlock();

	}
	m_ctrlClients.unlock();
	
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void ServerImplemented::SetMaterialPath( const EFK_CHAR* materialPath )
{
	m_materialPath.clear();

	int32_t pos = 0;
	while( materialPath[pos] != 0 )
	{
		m_materialPath.push_back( materialPath[pos] );
		pos++;
	}
	m_materialPath.push_back(0);
}

void ServerImplemented::Regist(const EFK_CHAR* key, Effect* effect)
{
	Register(key, effect);
}

void ServerImplemented::Unregist(Effect* effect)
{
	Unregister(effect);
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
} 
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------

#endif	// #if !( defined(_PSVITA) || defined(_XBOXONE) )

#ifndef	__EFFEKSEER_CLIENT_IMPLEMENTED_H__
#define	__EFFEKSEER_CLIENT_IMPLEMENTED_H__

#if !( defined(_PSVITA) || defined(_PS4) || defined(_SWITCH) || defined(_XBOXONE) )

//----------------------------------------------------------------------------------
// Include
//----------------------------------------------------------------------------------


//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
namespace Effekseer {
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
class ClientImplemented : public Client
{
private:
	bool isThreadRunning = false;
	std::thread	m_threadRecv;

	EfkSocket	m_socket;
	uint16_t	m_port;
	std::vector<uint8_t>	m_sendBuffer;

	bool		m_running;
	std::mutex	mutexStop;

	bool GetAddr( const char* host, IN_ADDR* addr);

	static void RecvAsync( void* data );
	void StopInternal();
public:
	ClientImplemented();
	~ClientImplemented();

	bool Start( char* host, uint16_t port );
	void Stop();

	bool Send( void* data, int32_t datasize );

	void Reload( const EFK_CHAR* key, void* data, int32_t size );
	void Reload( Manager* manager, const EFK_CHAR* path, const EFK_CHAR* key );

	bool IsConnected();
};

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
 } 
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------

#endif	// #if !( defined(_PSVITA) || defined(_PS4) || defined(_SWITCH) || defined(_XBOXONE) )

#endif	// __EFFEKSEER_CLIENT_IMPLEMENTED_H__


#if !( defined(_PSVITA) || defined(_PS4) || defined(_SWITCH) || defined(_XBOXONE) )



namespace Effekseer {

void ClientImplemented::RecvAsync( void* data )
{
	auto client = (ClientImplemented*)data;

	while(true)
	{
		int32_t size = 0;
		int32_t restSize = 0;

		restSize = 4;
		while(restSize > 0)
		{
			auto recvSize = ::recv( client->m_socket, (char*)(&size), restSize, 0 );
			restSize -= recvSize;

			if( recvSize == 0 || recvSize == -1 )
			{
				client->StopInternal();
				return;
			}
		}
	}
}

void ClientImplemented::StopInternal()
{
	std::lock_guard<std::mutex> lock(mutexStop);

	if (!m_running) return;
	m_running = false;

	Socket::Shutsown(m_socket);
	Socket::Close(m_socket);

	EffekseerPrintDebug("Client : Stop(Internal)\n");
}

ClientImplemented::ClientImplemented()
	: m_running		( false )
{
	Socket::Initialize();
}

ClientImplemented::~ClientImplemented()
{
	Stop();

	Socket::Finalize();
}

Client* Client::Create()
{
	return new ClientImplemented();
}

/*
HOSTENT* ClientImplemented::GetHostEntry( const char* host )
{
	HOSTENT* hostEntry = nullptr;
	IN_ADDR InAddrHost;

	// check ip adress or DNS
	InAddrHost.s_addr = ::inet_addr( host );
	if ( InAddrHost.s_addr == InaddrNone )
	{
		// DNS
		hostEntry = ::gethostbyname( host );
		if ( hostEntry == nullptr)
		{
			return nullptr;
		}
	}
	else
	{
		// Ip address
		hostEntry = ::gethostbyaddr( (const char*)(&InAddrHost), sizeof(IN_ADDR), AF_INET );
		if ( hostEntry == nullptr)
		{
			return nullptr;
		}
	}

	return hostEntry;
}
*/

bool ClientImplemented::GetAddr(const char* host, IN_ADDR* addr)
{
	HOSTENT* hostEntry = nullptr;

	// check ip adress or DNS
	addr->s_addr = ::inet_addr(host);
	if (addr->s_addr == InaddrNone)
	{
		// DNS
		hostEntry = ::gethostbyname(host);
		if (hostEntry == nullptr)
		{
			return false;
		}

		addr->s_addr = *(unsigned int *)hostEntry->h_addr_list[0];
	}

	return true;
}

bool ClientImplemented::Start( char* host, uint16_t port )
{
	if( m_running ) return false;

	// to stop thread
	Stop();

	SOCKADDR_IN sockAddr;

	// create a socket
	EfkSocket socket_ = Socket::GenSocket();
	if ( socket_ == InvalidSocket )
	{
		return false;
	}

	// get adder
	IN_ADDR addr;
	if (!GetAddr(host, &addr))
	{
		if (socket_ != InvalidSocket) Socket::Close(socket_);
		return false;
	}

	// generate data to connect
	memset( &sockAddr, 0, sizeof(SOCKADDR_IN) );
	sockAddr.sin_family	= AF_INET;
	sockAddr.sin_port	= htons( port );
	sockAddr.sin_addr	= addr;

	// connect
	int32_t ret = ::connect( socket_, (SOCKADDR*)(&sockAddr), sizeof(SOCKADDR_IN) );
	if ( ret == SocketError )
	{
		if ( socket_ != InvalidSocket ) Socket::Close( socket_ );
		return false;
	}

	m_socket = socket_;
	m_port = port;

	m_running = true;

	isThreadRunning = true;
	m_threadRecv = std::thread(
		[this](){
		RecvAsync(this);
	});

	EffekseerPrintDebug("Client : Start\n");

	return true;
}

void ClientImplemented::Stop()
{
	StopInternal();

	if (isThreadRunning)
	{
		m_threadRecv.join();
		isThreadRunning = false;
	}

	EffekseerPrintDebug("Client : Stop\n");
}

bool ClientImplemented::Send( void* data, int32_t datasize )
{
	if( !m_running ) return false;

	m_sendBuffer.clear();
	for( int32_t i = 0; i < sizeof(int32_t); i++ )
	{
		m_sendBuffer.push_back( ((uint8_t*)(&datasize))[i] );
	}

	for( int32_t i = 0; i < datasize; i++ )
	{
		m_sendBuffer.push_back( ((uint8_t*)(data))[i] );
	}

	int32_t size = (int32_t)m_sendBuffer.size();
	while( size > 0 )
	{
		auto ret = ::send( m_socket, (const char*)(&(m_sendBuffer[m_sendBuffer.size()-size])), size, 0 );
		if( ret == 0 || ret < 0 )
		{
			Stop();
			return false;
		}
		size -= ret;
	}

	return true;
}

void ClientImplemented::Reload( const EFK_CHAR* key, void* data, int32_t size )
{
	int32_t keylen = 0;
	for( ; ; keylen++ )
	{
		if(key[keylen] == 0 ) break;
	}

	std::vector<uint8_t> buf;

	for( int32_t i = 0; i < sizeof(int32_t); i++ )
	{
		buf.push_back( ((uint8_t*)(&keylen))[i] );
	}

	for( int32_t i = 0; i < keylen * 2; i++ )
	{
		buf.push_back( ((uint8_t*)(key))[i] );
	}

	for( int32_t i = 0; i < size; i++ )
	{
		buf.push_back( ((uint8_t*)(data))[i] );
	}

	Send( &(buf[0]), (int32_t)buf.size() );
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void ClientImplemented::Reload( Manager* manager, const EFK_CHAR* path, const EFK_CHAR* key )
{
	EffectLoader* loader = manager->GetEffectLoader();
	
	void* data = NULL;
	int32_t size = 0;

	if( !loader->Load( path, data, size ) ) return;

	Reload( key, data, size );

	loader->Unload( data, size );
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
bool ClientImplemented::IsConnected()
{
	return m_running;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
 } 
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------

#endif	// #if !( defined(_PSVITA) || defined(_PS4) || defined(_SWITCH) || defined(_XBOXONE) )

#endif

