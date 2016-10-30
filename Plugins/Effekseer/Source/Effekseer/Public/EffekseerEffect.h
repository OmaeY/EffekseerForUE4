
#pragma once

#include "Object.h"
#include "EffekseerModel.h"
#include "EffekseerEffect.generated.h"

UENUM()
enum class EEffekseerAlphaBlendType : uint8
{
	/// <summary>
	/// �s����
	/// </summary>
	Opacity = 0,
	/// <summary>
	/// ����
	/// </summary>
	Blend = 1,
	/// <summary>
	/// ���Z
	/// </summary>
	Add = 2,
	/// <summary>
	/// ���Z
	/// </summary>
	Sub = 3,
	/// <summary>
	/// ��Z
	/// </summary>
	Mul = 4,
};


UCLASS()
class EFFEKSEER_API UEffekseerMaterial
	: public UObject
{
public:
	GENERATED_BODY()

	UPROPERTY()
	UTexture2D*		Texture = nullptr;

	UPROPERTY()
	EEffekseerAlphaBlendType	AlphaBlend;

	UPROPERTY()
	bool			IsDepthTestDisabled;

	bool operator == (const UEffekseerMaterial* Other)
	{
		return
			Texture == Other->Texture &&
			AlphaBlend == Other->AlphaBlend &&
			IsDepthTestDisabled == Other->IsDepthTestDisabled;
	}

	friend uint32 GetTypeHash(const UEffekseerMaterial* Other)
	{
		return GetTypeHash(Other->Texture);
	}
};

struct EffekseerMaterial
{
	UTexture2D*		Texture = nullptr;

	EEffekseerAlphaBlendType	AlphaBlend;

	bool			IsDepthTestDisabled;

	bool operator < (const EffekseerMaterial& rhs) const
	{
		if (Texture != rhs.Texture)
		{
			return Texture < rhs.Texture;
		}

		if (AlphaBlend != rhs.AlphaBlend)
		{
			return AlphaBlend < rhs.AlphaBlend;
		}

		if (IsDepthTestDisabled != rhs.IsDepthTestDisabled)
		{
			return IsDepthTestDisabled < rhs.IsDepthTestDisabled;
		}

		return false;
	}
};

UCLASS()
class EFFEKSEER_API UEffekseerEffect : public UObject
{
	GENERATED_BODY()

private:
	void*			effectPtr = nullptr;
	TArray<uint8>	buffer;

	// ���[�h���ɐݒ肳��Ă����X�P�[��
	float			loadedScale = 1.0f;

	void LoadEffect(const uint8_t* data, int32_t size, const TCHAR* path);
	void ReleaseEffect();
public:
	void Load(const uint8_t* data, int32_t size, const TCHAR* path);

	void BeginDestroy() override;

	UPROPERTY(VisibleAnywhere, Transient)
	int32 Version = -1;

	UPROPERTY(EditAnywhere)
	float Scale = 1.0f;

	UPROPERTY(VisibleAnywhere, Transient)
	TArray<UTexture2D*>	ColorTextures;

	UPROPERTY(Transient)
	TArray<UEffekseerMaterial*>	Materials;

	UPROPERTY(VisibleAnywhere, Transient)
	TArray<UEffekseerModel*>	Models;
	
	// TODO Reimport
#if WITH_EDITORONLY_DATA
	UPROPERTY(Category = ImportSettings, VisibleAnywhere)
	class UAssetImportData* AssetImportData;
#endif

	void ReloadIfRequired();
	void AssignResources();

	void* GetNativePtr() { return effectPtr; }

	virtual void Serialize(FArchive& Ar) override;
};