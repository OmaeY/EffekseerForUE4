#include "EffekseerPrivatePCH.h"
#include "EffekseerSystemComponent.h"

#include "DynamicMeshBuilder.h"

#include "EffekseerNative.h"
#include "EffekseerRendererImplemented.h"

EffekseerUpdateData::EffekseerUpdateData()
{

}

EffekseerUpdateData::~EffekseerUpdateData()
{
	for (auto p : PlayingEffects)
	{
		auto p_ = (::Effekseer::Effect*)p;
		p_->Release();
	}
}

class FEffekseerSystemSceneProxy : public FPrimitiveSceneProxy
{
private:
	static const int32_t	particleMax = 10000;
	::Effekseer::Manager*	effekseerManager = nullptr;
	::EffekseerRendererUE4::RendererImplemented*	effekseerRenderer = nullptr;

public:
	FEffekseerSystemSceneProxy(const UEffekseerSystemComponent* InComponent)
		: FPrimitiveSceneProxy(InComponent)
		, Material(InComponent->Material)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Start : Construct");
		effekseerManager = ::Effekseer::Manager::Create(particleMax);
		effekseerRenderer = ::EffekseerRendererUE4::RendererImplemented::Create();
		effekseerRenderer->Initialize();

		effekseerManager->SetSpriteRenderer(effekseerRenderer->CreateSpriteRenderer());
		effekseerManager->SetRibbonRenderer(effekseerRenderer->CreateRibbonRenderer());
		effekseerManager->SetRingRenderer(effekseerRenderer->CreateRingRenderer());
		effekseerManager->SetTrackRenderer(effekseerRenderer->CreateTrackRenderer());

		//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "End : Construct");
	}

	virtual ~FEffekseerSystemSceneProxy()
	{
	//	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Start : Destructer");
		if (effekseerManager != nullptr)
		{
			effekseerManager->Destroy();
			effekseerManager = nullptr;
		}

		if (effekseerRenderer != nullptr)
		{
			effekseerRenderer->Destory();
			effekseerRenderer = nullptr;
		}

	//	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "End : Destructer");
	}

	virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override
	{
		// TODO �����ꍂ���ɏ����ł�����@���l����B
		//effekseerCS.Enter();

		//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Start : GetDynamicMeshElements");

		if (!Material)
		{
			return;
		}

		
		for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
		{
			if (!(VisibilityMap & (1 << ViewIndex))) continue;

			FMaterialRenderProxy* matProxy = Material->GetRenderProxy(false);

			effekseerRenderer->SetLocalToWorld(GetLocalToWorld());
			effekseerRenderer->SetMaterialRenderProxy(matProxy);
			effekseerRenderer->SetMeshElementCollector(&Collector);
			effekseerRenderer->SetViewIndex(ViewIndex);

			{
				auto vmat = Views[ViewIndex]->ViewMatrices.ViewMatrix;
				::Effekseer::Matrix44 evmat = *((::Effekseer::Matrix44*)(&vmat));

				std::swap(evmat.Values[1][0], evmat.Values[2][0]);
				std::swap(evmat.Values[1][1], evmat.Values[2][1]);
				std::swap(evmat.Values[1][2], evmat.Values[2][2]);

				effekseerRenderer->SetCameraMatrix(evmat);
				effekseerRenderer->BeginRendering();
				effekseerManager->Draw();
				effekseerRenderer->EndRendering();
			}
		}

		//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "End : GetDynamicMeshElements");

		//effekseerCS.Leave();
	}

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
	{
		bool bVisible = true;
		FPrimitiveViewRelevance Result;
		Result.bDrawRelevance = IsShown(View);
		Result.bDynamicRelevance = true;
		Result.bShadowRelevance = IsShadowCast(View);

		return Result;
	}
	virtual uint32 GetMemoryFootprint() const { return sizeof(*this) + GetAllocatedSize(); }
	uint32 GetAllocatedSize() const { return FPrimitiveSceneProxy::GetAllocatedSize(); }

	void UpdateData_RenderThread(EffekseerUpdateData* updateData)
	{
		//effekseerCS.Enter();

		//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Start : UpdateData");

		// TODO �����ꍂ���ɏ����ł�����@���l����B
		
		for (auto i = 0; i < updateData->PlayingEffects.Num(); i++)
		{
			auto effect = (::Effekseer::Effect*)updateData->PlayingEffects[i];
			auto position = updateData->PlayingEffectPositions[i];

			effekseerManager->Play(effect, position.X, position.Y, position.Z);
			//printf("%f,%f,%f\n", position.X, position.Y, position.Z);
		}

		{
			if (effekseerManager != nullptr)
			{
				effekseerManager->Update();
			}
		}
		
		delete updateData;
		//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "End : UpdateData");
		//effekseerCS.Leave();
	}

	// This function can be called out of renderThread.
	void UpdateData(EffekseerUpdateData* updateData)
	{
		ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
			ParticleUpdateDataCommand,
			FEffekseerSystemSceneProxy*, Proxy, this,
			EffekseerUpdateData*, Data, updateData,
			{
				Proxy->UpdateData_RenderThread(Data);
			}
		);
	}

protected:
	UMaterialInterface *Material;
};

UEffekseerSystemComponent::UEffekseerSystemComponent()
{
	bWantsBeginPlay = true;
	PrimaryComponentTick.bCanEverTick = true;
	currentUpdateData = new EffekseerUpdateData();
}

UEffekseerSystemComponent::~UEffekseerSystemComponent()
{
	ES_SAFE_DELETE(currentUpdateData);
}

void UEffekseerSystemComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UEffekseerSystemComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	auto sp = (FEffekseerSystemSceneProxy*)sceneProxy;

	sp->UpdateData(currentUpdateData);
	currentUpdateData = new EffekseerUpdateData();

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

FPrimitiveSceneProxy* UEffekseerSystemComponent::CreateSceneProxy()
{
	auto sp = new FEffekseerSystemSceneProxy(this);
	sceneProxy = sp;
	return sp;
}

UMaterialInterface* UEffekseerSystemComponent::GetMaterial(int32 ElementIndex) const
{
	if (ElementIndex == 0)
	{
		return Material;
	}
	else
	{
		return nullptr;
	}
}

void UEffekseerSystemComponent::GetUsedMaterials(TArray<UMaterialInterface*>& OutMaterials) const
{
	if (Material)
	{
		OutMaterials.Add(Material);
	}
}

int32 UEffekseerSystemComponent::GetNumMaterials()const
{
	if (Material)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}


FBoxSphereBounds UEffekseerSystemComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	return FBoxSphereBounds(LocalToWorld.GetLocation(), FVector(10, 10, 10), 10);
}

void UEffekseerSystemComponent::Play(UEffekseerEffect* effect, FVector position)
{
	if (effect == nullptr) return;
	if (effect->GetNativePtr() == nullptr) return;

	auto p = (::Effekseer::Effect*)effect->GetNativePtr();

	p->AddRef();
	
	currentUpdateData->PlayingEffects.Add(p);
	currentUpdateData->PlayingEffectPositions.Add(position);
}