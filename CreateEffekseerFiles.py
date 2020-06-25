import re
import codecs

class CreateHeader:
	def __init__(self):
		self.lines = []

	def replace(self, s, d):
		temp = []
		for line in self.lines:
			l = line.replace(s,d)
			temp.append(l)
		self.lines = temp

	def addLine(self,line):
		self.lines.append(line + '\n')

	def readLines(self,path):
		f = open(path, 'r', encoding='utf-8_sig')
		line = f.readline()
		while line:
			if re.search('include \"', line) == None and re.search('include <Effekseer.Internal.h>', line) == None and re.search('include <Effekseer/', line) == None:
 	 			self.lines.append(line)
			line = f.readline()
		f.close()
		self.lines.append(line + '\n')

	def output(self,path):
		f = codecs.open(path, 'w','utf-8_sig')
		for line in self.lines:
			f.write(line)
		f.close()

class CreateCPP:
	def __init__(self):
		self.lines = []

	def addLine(self,line):
		self.lines.append(line + '\n')

	def readLines(self,path):
		f = open(path, 'r', encoding='utf-8_sig')
		line = f.readline()
		while line:
			if re.search('include \"', line) == None and re.search('include <', line) == None and re.search('#pragma once', line) == None:
 	 			self.lines.append(line)
			line = f.readline()
		f.close()
		self.lines.append(line + '\n')

	def output(self,path):
		f = codecs.open(path, 'w','utf-8_sig')
		for line in self.lines:
			f.write(line)
		f.close()


rootDir = '../Effekseer/Dev/Cpp/'
rootEDir = rootDir + 'Effekseer/Effekseer/'
rootRDir = rootDir + 'EffekseerRendererCommon/'

effekseerHeader = CreateHeader()

effekseerHeader.addLine('')
effekseerHeader.addLine(r'#pragma once')
effekseerHeader.addLine(r'#include "CoreMinimal.h"')
effekseerHeader.addLine('')
effekseerHeader.addLine('#undef far')

effekseerHeader = CreateHeader()
effekseerHeader.readLines(rootEDir + 'Effekseer.Base.Pre.h')
effekseerHeader.readLines(rootEDir + 'Effekseer.Base.h')

#param
effekseerHeader.readLines(rootEDir + 'Parameter/Effekseer.Parameters.h')

effekseerHeader.readLines(rootEDir + 'Utils/Effekseer.CustomAllocator.h')
effekseerHeader.readLines(rootEDir + 'Utils/Effekseer.BinaryReader.h')

effekseerHeader.readLines(rootEDir + 'Effekseer.Math.h')
effekseerHeader.readLines(rootEDir + 'Effekseer.Vector2D.h')
effekseerHeader.readLines(rootEDir + 'Effekseer.Vector3D.h')
effekseerHeader.readLines(rootEDir + 'Effekseer.Color.h')
effekseerHeader.readLines(rootEDir + 'Effekseer.RectF.h')
effekseerHeader.readLines(rootEDir + 'Effekseer.Matrix43.h')
effekseerHeader.readLines(rootEDir + 'Effekseer.Matrix44.h')
effekseerHeader.readLines(rootEDir + 'Effekseer.File.h')
effekseerHeader.readLines(rootEDir + 'Effekseer.DefaultFile.h')
effekseerHeader.readLines(rootEDir + 'Effekseer.Effect.h')
effekseerHeader.readLines(rootEDir + 'Effekseer.Manager.h')
effekseerHeader.readLines(rootEDir + 'Effekseer.WorkerThread.h')
effekseerHeader.readLines(rootEDir + 'Effekseer.Random.h')
# SIMD

effekseerHeader.readLines(rootEDir + 'SIMD/Effekseer.SIMDType.h')
effekseerHeader.readLines(rootEDir + 'SIMD/Effekseer.SIMD4i_Gen.h')
effekseerHeader.readLines(rootEDir + 'SIMD/Effekseer.SIMD4i_NEON.h')
effekseerHeader.readLines(rootEDir + 'SIMD/Effekseer.SIMD4i_SSE.h')
effekseerHeader.readLines(rootEDir + 'SIMD/Effekseer.SIMD4f_Gen.h')
effekseerHeader.readLines(rootEDir + 'SIMD/Effekseer.SIMD4f_NEON.h')
effekseerHeader.readLines(rootEDir + 'SIMD/Effekseer.SIMD4f_SSE.h')
effekseerHeader.readLines(rootEDir + 'SIMD/Effekseer.SIMD4Bridge_Gen.h')
effekseerHeader.readLines(rootEDir + 'SIMD/Effekseer.SIMD4Bridge_NEON.h')
effekseerHeader.readLines(rootEDir + 'SIMD/Effekseer.SIMD4Bridge_SSE.h')
effekseerHeader.readLines(rootEDir + 'SIMD/Effekseer.Vec2f.h')
effekseerHeader.readLines(rootEDir + 'SIMD/Effekseer.Vec3f.h')
effekseerHeader.readLines(rootEDir + 'SIMD/Effekseer.Vec4f.h')
effekseerHeader.readLines(rootEDir + 'SIMD/Effekseer.Mat43f.h')
effekseerHeader.readLines(rootEDir + 'SIMD/Effekseer.Mat44f.h')
effekseerHeader.readLines(rootEDir + 'SIMD/Effekseer.SIMDUtils.h')

effekseerHeader.readLines(rootEDir + 'IO/Effekseer.EfkEfcFactory.h')

#material
effekseerHeader.readLines(rootEDir + 'Material/Effekseer.Material.h')
effekseerHeader.readLines(rootEDir + 'Material/Effekseer.MaterialCompiler.h')
effekseerHeader.readLines(rootEDir + 'Material/Effekseer.CompiledMaterial.h')

effekseerHeader.readLines(rootEDir + 'Renderer/Effekseer.SpriteRenderer.h')
effekseerHeader.readLines(rootEDir + 'Renderer/Effekseer.RibbonRenderer.h')
effekseerHeader.readLines(rootEDir + 'Renderer/Effekseer.RingRenderer.h')
effekseerHeader.readLines(rootEDir + 'Renderer/Effekseer.ModelRenderer.h')
effekseerHeader.readLines(rootEDir + 'Renderer/Effekseer.TrackRenderer.h')

effekseerHeader.readLines(rootEDir + 'Effekseer.EffectLoader.h')
effekseerHeader.readLines(rootEDir + 'Effekseer.TextureLoader.h')
effekseerHeader.readLines(rootEDir + 'Effekseer.ModelLoader.h')
effekseerHeader.readLines(rootEDir + 'Effekseer.MaterialLoader.h')

effekseerHeader.readLines(rootEDir + 'Effekseer.Model.h')

effekseerHeader.readLines(rootEDir + 'Sound/Effekseer.SoundPlayer.h')

effekseerHeader.readLines(rootEDir + 'Effekseer.SoundLoader.h')

effekseerHeader.readLines(rootEDir + 'Effekseer.Setting.h')

effekseerHeader.addLine('')
effekseerHeader.addLine('#if PLATFORM_WINDOWS')
effekseerHeader.readLines(rootEDir + 'Effekseer.Server.h')
effekseerHeader.readLines(rootEDir + 'Effekseer.Client.h')
effekseerHeader.addLine('#endif')


effekseerHeader.readLines(rootEDir + 'Effekseer.Setting.h')


effekseerHeader.output('Plugins/Effekseer/Source/Effekseer/Private/EffekseerNative.h')

effekseerCPP = CreateCPP()
effekseerCPP.addLine('#include "EffekseerNative.h"  // UE4')
effekseerCPP.addLine('#define _WINSOCK_DEPRECATED_NO_WARNINGS')
effekseerCPP.addLine('#define _WINSOCKAPI_')
effekseerCPP.addLine('#define NOMINMAX')
effekseerCPP.addLine('#define __EFFEKSEER_FOR_UE4__ 1')
effekseerCPP.addLine('#include "EffekseerNative.h"')
effekseerCPP.addLine('#include <math.h>')
effekseerCPP.addLine('#include <cmath>')
effekseerCPP.addLine('#include <float.h>')
effekseerCPP.addLine('#include <assert.h>')
effekseerCPP.addLine('#include <string>')
effekseerCPP.addLine('#include <vector>')
effekseerCPP.addLine('#include <map>')
effekseerCPP.addLine('#include <list>')
effekseerCPP.addLine('#include <set>')
effekseerCPP.addLine('#include <queue>')
effekseerCPP.addLine('#include <fstream>')
effekseerCPP.addLine('#include <memory>')
effekseerCPP.addLine('#include <thread>')
effekseerCPP.addLine('#include <mutex>')
effekseerCPP.addLine('#include <iostream>')
effekseerCPP.addLine('#include <array>')
effekseerCPP.addLine('#include <functional>')

effekseerCPP.addLine('#ifdef _WIN32')
effekseerCPP.addLine('#include "Windows/AllowWindowsPlatformTypes.h"  // UE4')
effekseerCPP.addLine('#include <winsock2.h>')
effekseerCPP.addLine('#include "Windows/HideWindowsPlatformTypes.h"  // UE4')
effekseerCPP.addLine('#pragma comment( lib, "ws2_32.lib" )')
effekseerCPP.addLine('#else')
effekseerCPP.addLine('#include <sys/types.h>')
effekseerCPP.addLine('#include <sys/socket.h>')
effekseerCPP.addLine('#endif')
effekseerCPP.addLine('#undef far')

# noise
effekseerCPP.readLines(rootEDir + 'Noise/PerlinNoise.h')
effekseerCPP.readLines(rootEDir + 'Noise/PerlinNoise.cpp')
effekseerCPP.readLines(rootEDir + 'Noise/CurlNoise.h')
effekseerCPP.readLines(rootEDir + 'Noise/CurlNoise.cpp')

# param
effekseerCPP.readLines(rootEDir + 'Parameter/Effekseer.Parameters.cpp')

effekseerCPP.readLines(rootEDir + 'Utils/Effekseer.CustomAllocator.cpp')

#material
effekseerCPP.readLines(rootEDir + 'Material/Effekseer.Material.cpp')
effekseerCPP.readLines(rootEDir + 'Material/Effekseer.MaterialCompiler.cpp')
effekseerCPP.readLines(rootEDir + 'Material/Effekseer.CompiledMaterial.cpp')

# IO
effekseerCPP.readLines(rootEDir + 'IO/Effekseer.EfkEfcFactory.cpp')

# Culling
effekseerCPP.readLines(rootEDir + 'Culling/Culling3D.h')
effekseerCPP.readLines(rootEDir + 'Culling/Culling3D.ReferenceObject.h')
effekseerCPP.readLines(rootEDir + 'Culling/Culling3D.Grid.h')
effekseerCPP.readLines(rootEDir + 'Culling/Culling3D.Layer.h')
effekseerCPP.readLines(rootEDir + 'Culling/Culling3D.ObjectInternal.h')
effekseerCPP.readLines(rootEDir + 'Culling/Culling3D.WorldInternal.h')

effekseerCPP.readLines(rootEDir + 'Culling/Culling3D.Grid.cpp')
effekseerCPP.readLines(rootEDir + 'Culling/Culling3D.Layer.cpp')
effekseerCPP.readLines(rootEDir + 'Culling/Culling3D.Matrix44.cpp')
effekseerCPP.readLines(rootEDir + 'Culling/Culling3D.ObjectInternal.cpp')
effekseerCPP.readLines(rootEDir + 'Culling/Culling3D.ReferenceObject.cpp')
effekseerCPP.readLines(rootEDir + 'Culling/Culling3D.Vector3DF.cpp')
effekseerCPP.readLines(rootEDir + 'Culling/Culling3D.WorldInternal.cpp')

# Math
effekseerCPP.readLines(rootEDir + 'Effekseer.Vector2D.cpp')
effekseerCPP.readLines(rootEDir + 'Effekseer.Vector3D.cpp')
effekseerCPP.readLines(rootEDir + 'Effekseer.Color.cpp')
effekseerCPP.readLines(rootEDir + 'Effekseer.RectF.cpp')
effekseerCPP.readLines(rootEDir + 'Effekseer.Matrix43.cpp')
effekseerCPP.readLines(rootEDir + 'Effekseer.Matrix44.cpp')

effekseerCPP.readLines(rootEDir + 'Effekseer.InternalStruct.h')
effekseerCPP.readLines(rootEDir + 'Effekseer.InternalScript.h')

effekseerCPP.readLines(rootEDir + 'Effekseer.DefaultEffectLoader.h')
effekseerCPP.readLines(rootEDir + 'Effekseer.DefaultEffectLoader.cpp')

effekseerCPP.readLines(rootEDir + 'Effekseer.DefaultFile.h')
effekseerCPP.readLines(rootEDir + 'Effekseer.DefaultFile.cpp')

effekseerCPP.readLines(rootEDir + 'Effekseer.FCurves.h')
effekseerCPP.readLines(rootEDir + 'Effekseer.EffectNode.h')
effekseerCPP.readLines(rootEDir + 'Effekseer.EffectNodeModel.h')
effekseerCPP.readLines(rootEDir + 'Effekseer.EffectNodeRibbon.h')
effekseerCPP.readLines(rootEDir + 'Effekseer.EffectNodeRing.h')
effekseerCPP.readLines(rootEDir + 'Effekseer.EffectNodeRoot.h')
effekseerCPP.readLines(rootEDir + 'Effekseer.EffectNodeSprite.h')
effekseerCPP.readLines(rootEDir + 'Effekseer.EffectNodeTrack.h')

effekseerCPP.readLines(rootEDir + 'Effekseer.EffectImplemented.h')
effekseerCPP.readLines(rootEDir + 'Effekseer.ManagerImplemented.h')
effekseerCPP.readLines(rootEDir + 'Effekseer.IntrusiveList.h')
effekseerCPP.readLines(rootEDir + 'Effekseer.InstanceContainer.h')
effekseerCPP.readLines(rootEDir + 'Effekseer.Instance.h')
effekseerCPP.readLines(rootEDir + 'Effekseer.InstanceChunk.h')
effekseerCPP.readLines(rootEDir + 'Effekseer.InstanceGlobal.h')
effekseerCPP.readLines(rootEDir + 'Effekseer.InstanceGroup.h')

effekseerCPP.readLines(rootEDir + 'Effekseer.FCurves.cpp')
effekseerCPP.readLines(rootEDir + 'Effekseer.EffectNode.cpp')
effekseerCPP.readLines(rootEDir + 'Effekseer.EffectNodeModel.cpp')
effekseerCPP.readLines(rootEDir + 'Effekseer.EffectNodeRibbon.cpp')
effekseerCPP.readLines(rootEDir + 'Effekseer.EffectNodeRing.cpp')
effekseerCPP.readLines(rootEDir + 'Effekseer.EffectNodeRoot.cpp')
effekseerCPP.readLines(rootEDir + 'Effekseer.EffectNodeSprite.cpp')
effekseerCPP.readLines(rootEDir + 'Effekseer.EffectNodeTrack.cpp')

effekseerCPP.readLines(rootEDir + 'Effekseer.Effect.cpp')
effekseerCPP.readLines(rootEDir + 'Effekseer.Manager.cpp')
effekseerCPP.readLines(rootEDir + 'Effekseer.WorkerThread.cpp')
effekseerCPP.readLines(rootEDir + 'Effekseer.Random.cpp')
effekseerCPP.readLines(rootEDir + 'Effekseer.IntrusiveList.cpp')
effekseerCPP.readLines(rootEDir + 'Effekseer.InstanceContainer.cpp')
effekseerCPP.readLines(rootEDir + 'Effekseer.Instance.cpp')
effekseerCPP.readLines(rootEDir + 'Effekseer.InstanceChunk.cpp')
effekseerCPP.readLines(rootEDir + 'Effekseer.InstanceGlobal.cpp')
effekseerCPP.readLines(rootEDir + 'Effekseer.InstanceGroup.cpp')
effekseerCPP.readLines(rootEDir + 'Effekseer.InternalScript.cpp')

effekseerCPP.readLines(rootEDir + 'Effekseer.Setting.cpp')

effekseerCPP.readLines(rootEDir + 'SIMD/Effekseer.Mat43f.cpp')
effekseerCPP.readLines(rootEDir + 'SIMD/Effekseer.Mat44f.cpp')
effekseerCPP.readLines(rootEDir + 'SIMD/Effekseer.SIMDUtils.cpp')

effekseerCPP.addLine('')
effekseerCPP.addLine('#if PLATFORM_WINDOWS')
effekseerCPP.readLines(rootEDir + 'Effekseer.Socket.h')
effekseerCPP.readLines(rootEDir + 'Effekseer.Socket.cpp')

effekseerCPP.readLines(rootEDir + 'Effekseer.ServerImplemented.h')
effekseerCPP.readLines(rootEDir + 'Effekseer.Server.cpp')

effekseerCPP.readLines(rootEDir + 'Effekseer.ClientImplemented.h')
effekseerCPP.readLines(rootEDir + 'Effekseer.Client.cpp')
effekseerCPP.addLine('#endif')
effekseerCPP.addLine('')

effekseerCPP.output('Plugins/Effekseer/Source/Effekseer/Private/EffekseerNative.cpp')

rendererHeader = CreateHeader()

rendererHeader.addLine('')
rendererHeader.addLine(r'#pragma once')
rendererHeader.addLine(r'#include "CoreMinimal.h"')
rendererHeader.addLine('')
rendererHeader.addLine('#undef far')

rendererHeader.readLines(rootRDir + 'EffekseerRenderer.CommonUtils.h')
rendererHeader.readLines(rootRDir + 'EffekseerRenderer.Renderer.h')
rendererHeader.readLines(rootRDir + 'EffekseerRenderer.Renderer_Impl.h')
rendererHeader.readLines(rootRDir + 'EffekseerRenderer.RenderStateBase.h')

rendererHeader.readLines(rootRDir + 'EffekseerRenderer.VertexBufferBase.h')
rendererHeader.readLines(rootRDir + 'EffekseerRenderer.IndexBufferBase.h')

rendererHeader.readLines(rootRDir + 'EffekseerRenderer.StandardRenderer.h')
rendererHeader.readLines(rootRDir + 'EffekseerRenderer.ModelRendererBase.h')

rendererHeader.readLines(rootRDir + 'EffekseerRenderer.ShaderBase.h')

rendererHeader.readLines(rootRDir + 'EffekseerRenderer.RibbonRendererBase.h')
rendererHeader.readLines(rootRDir + 'EffekseerRenderer.RingRendererBase.h')
rendererHeader.readLines(rootRDir + 'EffekseerRenderer.SpriteRendererBase.h')
rendererHeader.readLines(rootRDir + 'EffekseerRenderer.TrackRendererBase.h')
rendererHeader.readLines(rootRDir + 'EffekseerRenderer.VertexBufferBase.h')
rendererHeader.replace('#include <Effekseer.h>','#include "EffekseerNative.h"')

rendererHeader.output('Plugins/Effekseer/Source/Effekseer/Private/EffekseerRendererNative.h')

rendererCPP = CreateCPP()
rendererCPP.addLine('#include "EffekseerRendererNative.h"')
rendererCPP.addLine('#undef far')
rendererCPP.readLines(rootRDir + 'EffekseerRenderer.CommonUtils.cpp')
rendererCPP.readLines(rootRDir + 'EffekseerRenderer.IndexBufferBase.cpp')
rendererCPP.readLines(rootRDir + 'EffekseerRenderer.ModelRendererBase.cpp')
rendererCPP.readLines(rootRDir + 'EffekseerRenderer.Renderer.cpp')
rendererCPP.readLines(rootRDir + 'EffekseerRenderer.Renderer_Impl.cpp')
rendererCPP.readLines(rootRDir + 'EffekseerRenderer.RenderStateBase.cpp')
rendererCPP.readLines(rootRDir + 'EffekseerRenderer.RibbonRendererBase.cpp')
rendererCPP.readLines(rootRDir + 'EffekseerRenderer.RingRendererBase.cpp')
rendererCPP.readLines(rootRDir + 'EffekseerRenderer.SpriteRendererBase.cpp')
rendererCPP.readLines(rootRDir + 'EffekseerRenderer.TrackRendererBase.cpp')
rendererCPP.readLines(rootRDir + 'EffekseerRenderer.VertexBufferBase.cpp')
rendererCPP.output('Plugins/Effekseer/Source/Effekseer/Private/EffekseerRendererNative.cpp')


from distutils.dir_util import copy_tree

copy_tree("../Effekseer/Dev/Cpp/EffekseerMaterial", "./Plugins/Effekseer/Source/EffekseerEd/Private/EffekseerMaterial")

