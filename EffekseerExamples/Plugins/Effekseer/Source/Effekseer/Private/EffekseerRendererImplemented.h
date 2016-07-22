
#pragma once

#include "EffekseerRendererNative.h"

namespace EffekseerRendererUE4
{
	class RendererImplemented;
	class RenderState;
	class VertexBuffer;
	class IndexBuffer;
	class Shader;

	struct Vertex
	{
		::Effekseer::Vector3D	Pos;
		::Effekseer::Color	Col;
		float		UV[2];

		void SetColor(const ::Effekseer::Color& color)
		{
			Col = color;
		}
	};

	struct VertexDistortion
	{
		::Effekseer::Vector3D	Pos;
		::Effekseer::Color	Col;
		float		UV[2];
		::Effekseer::Vector3D	Tangent;
		::Effekseer::Vector3D	Binormal;

		void SetColor(const ::Effekseer::Color& color)
		{
			Col = color;
		}
	};

	inline void TransformVertexes(Vertex* vertexes, int32_t count, const ::Effekseer::Matrix43& mat)
	{
#if 1
		__m128 r0 = _mm_loadu_ps(mat.Value[0]);
		__m128 r1 = _mm_loadu_ps(mat.Value[1]);
		__m128 r2 = _mm_loadu_ps(mat.Value[2]);
		__m128 r3 = _mm_loadu_ps(mat.Value[3]);

		float tmp_out[4];
		::Effekseer::Vector3D* inout_prev;

		// �P���[�v��
		{
			::Effekseer::Vector3D* inout_cur = &vertexes[0].Pos;
			__m128 v = _mm_loadu_ps((const float*)inout_cur);

			__m128 x = _mm_shuffle_ps(v, v, _MM_SHUFFLE(0, 0, 0, 0));
			__m128 a0 = _mm_mul_ps(r0, x);
			__m128 y = _mm_shuffle_ps(v, v, _MM_SHUFFLE(1, 1, 1, 1));
			__m128 a1 = _mm_mul_ps(r1, y);
			__m128 z = _mm_shuffle_ps(v, v, _MM_SHUFFLE(2, 2, 2, 2));
			__m128 a2 = _mm_mul_ps(r2, z);

			__m128 a01 = _mm_add_ps(a0, a1);
			__m128 a23 = _mm_add_ps(a2, r3);
			__m128 a = _mm_add_ps(a01, a23);

			// ����̌��ʂ��X�g�A���Ă���
			_mm_storeu_ps(tmp_out, a);
			inout_prev = inout_cur;
		}

		for (int i = 1; i < count; i++)
		{
			::Effekseer::Vector3D* inout_cur = &vertexes[i].Pos;
			__m128 v = _mm_loadu_ps((const float*)inout_cur);

			__m128 x = _mm_shuffle_ps(v, v, _MM_SHUFFLE(0, 0, 0, 0));
			__m128 a0 = _mm_mul_ps(r0, x);
			__m128 y = _mm_shuffle_ps(v, v, _MM_SHUFFLE(1, 1, 1, 1));
			__m128 a1 = _mm_mul_ps(r1, y);
			__m128 z = _mm_shuffle_ps(v, v, _MM_SHUFFLE(2, 2, 2, 2));
			__m128 a2 = _mm_mul_ps(r2, z);

			__m128 a01 = _mm_add_ps(a0, a1);
			__m128 a23 = _mm_add_ps(a2, r3);
			__m128 a = _mm_add_ps(a01, a23);

			// ���O�̃��[�v�̌��ʂ��������݂܂�
			inout_prev->X = tmp_out[0];
			inout_prev->Y = tmp_out[1];
			inout_prev->Z = tmp_out[2];

			// ����̌��ʂ��X�g�A���Ă���
			_mm_storeu_ps(tmp_out, a);
			inout_prev = inout_cur;
		}

		// �Ō�̃��[�v�̌��ʂ���������
		{
			inout_prev->X = tmp_out[0];
			inout_prev->Y = tmp_out[1];
			inout_prev->Z = tmp_out[2];
		}

#else
		for (int i = 0; i < count; i++)
		{
			::Effekseer::Vector3D::Transform(
				vertexes[i].Pos,
				vertexes[i].Pos,
				mat);
		}
#endif
	}

	inline void TransformVertexes(VertexDistortion* vertexes, int32_t count, const ::Effekseer::Matrix43& mat)
	{
#if 1
		__m128 r0 = _mm_loadu_ps(mat.Value[0]);
		__m128 r1 = _mm_loadu_ps(mat.Value[1]);
		__m128 r2 = _mm_loadu_ps(mat.Value[2]);
		__m128 r3 = _mm_loadu_ps(mat.Value[3]);

		float tmp_out[4];
		::Effekseer::Vector3D* inout_prev;

		// �P���[�v��
		{
			::Effekseer::Vector3D* inout_cur = &vertexes[0].Pos;
			__m128 v = _mm_loadu_ps((const float*)inout_cur);

			__m128 x = _mm_shuffle_ps(v, v, _MM_SHUFFLE(0, 0, 0, 0));
			__m128 a0 = _mm_mul_ps(r0, x);
			__m128 y = _mm_shuffle_ps(v, v, _MM_SHUFFLE(1, 1, 1, 1));
			__m128 a1 = _mm_mul_ps(r1, y);
			__m128 z = _mm_shuffle_ps(v, v, _MM_SHUFFLE(2, 2, 2, 2));
			__m128 a2 = _mm_mul_ps(r2, z);

			__m128 a01 = _mm_add_ps(a0, a1);
			__m128 a23 = _mm_add_ps(a2, r3);
			__m128 a = _mm_add_ps(a01, a23);

			// ����̌��ʂ��X�g�A���Ă���
			_mm_storeu_ps(tmp_out, a);
			inout_prev = inout_cur;
		}

		for (int i = 1; i < count; i++)
		{
			::Effekseer::Vector3D* inout_cur = &vertexes[i].Pos;
			__m128 v = _mm_loadu_ps((const float*)inout_cur);

			__m128 x = _mm_shuffle_ps(v, v, _MM_SHUFFLE(0, 0, 0, 0));
			__m128 a0 = _mm_mul_ps(r0, x);
			__m128 y = _mm_shuffle_ps(v, v, _MM_SHUFFLE(1, 1, 1, 1));
			__m128 a1 = _mm_mul_ps(r1, y);
			__m128 z = _mm_shuffle_ps(v, v, _MM_SHUFFLE(2, 2, 2, 2));
			__m128 a2 = _mm_mul_ps(r2, z);

			__m128 a01 = _mm_add_ps(a0, a1);
			__m128 a23 = _mm_add_ps(a2, r3);
			__m128 a = _mm_add_ps(a01, a23);

			// ���O�̃��[�v�̌��ʂ��������݂܂�
			inout_prev->X = tmp_out[0];
			inout_prev->Y = tmp_out[1];
			inout_prev->Z = tmp_out[2];

			// ����̌��ʂ��X�g�A���Ă���
			_mm_storeu_ps(tmp_out, a);
			inout_prev = inout_cur;
		}

		// �Ō�̃��[�v�̌��ʂ���������
		{
			inout_prev->X = tmp_out[0];
			inout_prev->Y = tmp_out[1];
			inout_prev->Z = tmp_out[2];
		}

#else
		for (int i = 0; i < count; i++)
		{
			::Effekseer::Vector3D::Transform(
				vertexes[i].Pos,
				vertexes[i].Pos,
				mat);
		}
#endif

		for (int i = 0; i < count; i++)
		{
			auto vs = &vertexes[i];

			::Effekseer::Vector3D::Transform(
				vs->Tangent,
				vs->Tangent,
				mat);

			::Effekseer::Vector3D::Transform(
				vs->Binormal,
				vs->Binormal,
				mat);

			Effekseer::Vector3D zero;
			::Effekseer::Vector3D::Transform(
				zero,
				zero,
				mat);

			::Effekseer::Vector3D::Normal(vs->Tangent, vs->Tangent - zero);
			::Effekseer::Vector3D::Normal(vs->Binormal, vs->Binormal - zero);
		}
	}

	typedef ::Effekseer::SpriteRenderer::NodeParameter efkSpriteNodeParam;
	typedef ::Effekseer::SpriteRenderer::InstanceParameter efkSpriteInstanceParam;
	typedef ::Effekseer::Vector3D efkVector3D;

	class SpriteRenderer
		: public ::EffekseerRenderer::SpriteRendererBase
	{
	private:
		RendererImplemented*			m_renderer = nullptr;

		SpriteRenderer(RendererImplemented* renderer);
	public:

		virtual ~SpriteRenderer();

		static SpriteRenderer* Create(RendererImplemented* renderer);

	public:
		void BeginRendering(const efkSpriteNodeParam& parameter, int32_t count, void* userData);

		void Rendering(const efkSpriteNodeParam& parameter, const efkSpriteInstanceParam& instanceParameter, void* userData);

		void EndRendering(const efkSpriteNodeParam& parameter, void* userData);
	};

	typedef ::Effekseer::RibbonRenderer::NodeParameter efkRibbonNodeParam;
	typedef ::Effekseer::RibbonRenderer::InstanceParameter efkRibbonInstanceParam;
	typedef ::Effekseer::Vector3D efkVector3D;

	class RibbonRenderer
		: public ::EffekseerRenderer::RibbonRendererBase
	{
	private:
		RendererImplemented*	m_renderer;

		RibbonRenderer(RendererImplemented* renderer);

	public:

		virtual ~RibbonRenderer();

		static RibbonRenderer* Create(RendererImplemented* renderer);

	public:
		void BeginRendering(const efkRibbonNodeParam& parameter, int32_t count, void* userData);

		void Rendering(const efkRibbonNodeParam& parameter, const efkRibbonInstanceParam& instanceParameter, void* userData);

		void EndRendering(const efkRibbonNodeParam& parameter, void* userData);
	};

	typedef ::Effekseer::RingRenderer::NodeParameter efkRingNodeParam;
	typedef ::Effekseer::RingRenderer::InstanceParameter efkRingInstanceParam;
	typedef ::Effekseer::Vector3D efkVector3D;

	class RingRenderer
		: public ::EffekseerRenderer::RingRendererBase
	{
	private:
		RendererImplemented*	m_renderer;

		RingRenderer(RendererImplemented* renderer);

	public:

		virtual ~RingRenderer();

		static RingRenderer* Create(RendererImplemented* renderer);

	public:
		void BeginRendering(const efkRingNodeParam& parameter, int32_t count, void* userData);

		void Rendering(const efkRingNodeParam& parameter, const efkRingInstanceParam& instanceParameter, void* userData);

		void EndRendering(const efkRingNodeParam& parameter, void* userData);
	};

	typedef ::Effekseer::TrackRenderer::NodeParameter efkTrackNodeParam;
	typedef ::Effekseer::TrackRenderer::InstanceParameter efkTrackInstanceParam;
	typedef ::Effekseer::Vector3D efkVector3D;

	class TrackRenderer
		: public ::EffekseerRenderer::TrackRendererBase
	{
	private:
		RendererImplemented*	m_renderer;

		TrackRenderer(RendererImplemented* renderer);

	public:

		virtual ~TrackRenderer();

		static TrackRenderer* Create(RendererImplemented* renderer);

	public:
		void BeginRendering(const efkTrackNodeParam& parameter, int32_t count, void* userData);

		void Rendering(const efkTrackNodeParam& parameter, const efkTrackInstanceParam& instanceParameter, void* userData);

		void EndRendering(const efkTrackNodeParam& parameter, void* userData);
	};

	class RendererImplemented
		: public ::EffekseerRenderer::Renderer
	{
	protected:
		/* �Q�ƃJ�E���^ */
		int32_t					m_reference = 1;

		::Effekseer::Vector3D	m_lightDirection;
		::Effekseer::Color		m_lightColor;
		::Effekseer::Color		m_lightAmbient;
		int32_t					m_squareMaxCount;

		::Effekseer::Matrix44	m_proj;
		::Effekseer::Matrix44	m_camera;
		::Effekseer::Matrix44	m_cameraProj;

		VertexBuffer*			m_vertexBuffer = nullptr;
		Shader*					m_stanShader = nullptr;
		RenderState*			m_renderState = nullptr;

		void*					m_textures[16];

		FMatrix					m_localToWorld;
		int32_t					m_viewIndex = 0;
		TMap<UTexture2D*, UMaterialInstanceDynamic*>*	m_materials = nullptr;
		FMeshElementCollector*	m_meshElementCollector = nullptr;

		EffekseerRenderer::StandardRenderer<RendererImplemented, Shader, void*, Vertex, VertexDistortion>*	m_standardRenderer = nullptr;
	public:
		
		static RendererImplemented* Create();

		RendererImplemented();
		virtual ~RendererImplemented();

		void OnLostDevice() override {}
		void OnResetDevice() override {}

		/**
		@brief	������
		*/
		bool Initialize();

		/**
		@brief	�Q�ƃJ�E���^�����Z����B
		@return	���s��̎Q�ƃJ�E���^�̒l
		*/
		int AddRef();

		/**
		@brief	�Q�ƃJ�E���^�����Z����B
		@return	���s��̎Q�ƃJ�E���^�̒l
		*/
		int Release();

		/**
		@brief	���̃C���X�^���X��j������B
		*/
		void Destory() override;

		/**
		@brief	�X�e�[�g�𕜋A���邩�ǂ����̃t���O��ݒ肷��B
		*/
		void SetRestorationOfStatesFlag(bool flag) override;

		/**
		@brief	�`����J�n���鎞�Ɏ��s����B
		*/
		bool BeginRendering() override;

		/**
		@brief	�`����I�����鎞�Ɏ��s����B
		*/
		bool EndRendering() override;

		/**
		@brief	���C�g�̕������擾����B
		*/
		const ::Effekseer::Vector3D& GetLightDirection() const override;

		/**
		@brief	���C�g�̕�����ݒ肷��B
		*/
		void SetLightDirection(::Effekseer::Vector3D& direction) override;

		/**
		@brief	���C�g�̐F���擾����B
		*/
		const ::Effekseer::Color& GetLightColor() const override;

		/**
		@brief	���C�g�̐F��ݒ肷��B
		*/
		void SetLightColor(::Effekseer::Color& color) override;

		/**
		@brief	���C�g�̊����̐F���擾����B
		*/
		const ::Effekseer::Color& GetLightAmbientColor() const override;

		/**
		@brief	���C�g�̊����̐F��ݒ肷��B
		*/
		void SetLightAmbientColor(::Effekseer::Color& color) override;

		/**
		@brief	�ő�`��X�v���C�g�����擾����B
		*/
		int32_t GetSquareMaxCount() const  override;

		/**
		@brief	���e�s����擾����B
		*/
		const ::Effekseer::Matrix44& GetProjectionMatrix() const  override;

		/**
		@brief	���e�s���ݒ肷��B
		*/
		void SetProjectionMatrix(const ::Effekseer::Matrix44& mat)  override;

		/**
		@brief	�J�����s����擾����B
		*/
		const ::Effekseer::Matrix44& GetCameraMatrix() const  override;

		/**
		@brief	�J�����s���ݒ肷��B
		*/
		void SetCameraMatrix(const ::Effekseer::Matrix44& mat)  override;

		/**
		@brief	�J�����v���W�F�N�V�����s����擾����B
		*/
		::Effekseer::Matrix44& GetCameraProjectionMatrix()  override;

		/**
		@brief	�X�v���C�g�����_���[�𐶐�����B
		*/
		::Effekseer::SpriteRenderer* CreateSpriteRenderer() override;

		/**
		@brief	���{�������_���[�𐶐�����B
		*/
		::Effekseer::RibbonRenderer* CreateRibbonRenderer() override;

		/**
		@brief	�����O�����_���[�𐶐�����B
		*/
		::Effekseer::RingRenderer* CreateRingRenderer() override;

		/**
		@brief	���f�������_���[�𐶐�����B
		*/
		::Effekseer::ModelRenderer* CreateModelRenderer() override;

		/**
		@brief	�O�Ճ����_���[�𐶐�����B
		*/
		::Effekseer::TrackRenderer* CreateTrackRenderer() override;

		/**
		@brief	�W���̃e�N�X�`���Ǎ��N���X�𐶐�����B
		*/
		::Effekseer::TextureLoader* CreateTextureLoader(::Effekseer::FileInterface* fileInterface = NULL)  override;

		/**
		@brief	�W���̃��f���Ǎ��N���X�𐶐�����B
		*/
		::Effekseer::ModelLoader* CreateModelLoader(::Effekseer::FileInterface* fileInterface = NULL) override;

		/**
		@brief	�����_�[�X�e�[�g�������I�Ƀ��Z�b�g����B
		*/
		void ResetRenderState() override;

		/**
		@brief	�w�i��c�܂���G�t�F�N�g���`�悳���O�ɌĂ΂��R�[���o�b�N���擾����B
		*/
		::EffekseerRenderer::DistortingCallback* GetDistortingCallback()  override;

		/**
		@brief	�w�i��c�܂���G�t�F�N�g���`�悳���O�ɌĂ΂��R�[���o�b�N��ݒ肷��B
		*/
		void SetDistortingCallback(::EffekseerRenderer::DistortingCallback* callback) override;

		void* GetBackground();

		VertexBuffer* GetVertexBuffer();

		IndexBuffer* GetIndexBuffer();

		EffekseerRenderer::StandardRenderer<RendererImplemented, Shader, void*, Vertex, VertexDistortion>* GetStandardRenderer();

		::EffekseerRenderer::RenderStateBase* GetRenderState();

		void SetVertexBuffer(VertexBuffer* vertexBuffer, int32_t size);
		void SetIndexBuffer(IndexBuffer* indexBuffer);

		void SetLayout(Shader* shader);
		void DrawSprites(int32_t spriteCount, int32_t vertexOffset);
		void DrawPolygon(int32_t vertexCount, int32_t indexCount);

		void BeginShader(Shader* shader);
		void EndShader(Shader* shader);

		void SetTextures(Shader* shader, void** textures, int32_t count);

		void SetLocalToWorld(FMatrix localToWorld);
		void SetViewIndex(int32_t viewIndex);
		void SetMaterials(const TMap<UTexture2D*, UMaterialInstanceDynamic*>* materials);
		void SetMeshElementCollector(FMeshElementCollector* meshElementCollector);
	};

}