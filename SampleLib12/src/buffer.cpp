﻿#include <sl12/buffer.h>

#include <sl12/device.h>
#include <sl12/command_list.h>
#include <sl12/fence.h>


namespace sl12
{
	//----
	bool Buffer::Initialize(Device* pDev, size_t size, size_t stride, BufferUsage::Type type, bool isDynamic, bool isUAV)
	{
		D3D12_HEAP_TYPE heapType = D3D12_HEAP_TYPE_UPLOAD;
		if (!isDynamic)
		{
			if (type != BufferUsage::ConstantBuffer)
			{
				heapType = D3D12_HEAP_TYPE_DEFAULT;
			}
		}
		auto initialState = (heapType == D3D12_HEAP_TYPE_UPLOAD) ? D3D12_RESOURCE_STATE_GENERIC_READ : D3D12_RESOURCE_STATE_COPY_DEST;
		return Initialize(pDev, size, stride, type, initialState, isDynamic, isUAV);
	}

	//----
	bool Buffer::Initialize(Device* pDev, size_t size, size_t stride, BufferUsage::Type type, D3D12_RESOURCE_STATES initialState, bool isDynamic, bool isUAV)
	{
		D3D12_HEAP_TYPE heapType = D3D12_HEAP_TYPE_UPLOAD;
		if (!isDynamic)
		{
			if (type != BufferUsage::ConstantBuffer)
			{
				heapType = D3D12_HEAP_TYPE_DEFAULT;
			}
		}

		size_t allocSize = size;
		if (type == BufferUsage::ConstantBuffer)
		{
			allocSize = (allocSize + 255) / 256 * 256;
		}

		// ByteAddressBufferの場合はR32_TYPELESSに設定する
		DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;

		D3D12_HEAP_PROPERTIES prop{};
		prop.Type = heapType;
		prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		prop.CreationNodeMask = 1;
		prop.VisibleNodeMask = 1;

		D3D12_RESOURCE_DESC desc{};
		desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		desc.Alignment = 0;
		desc.Width = allocSize;
		desc.Height = 1;
		desc.DepthOrArraySize = 1;
		desc.MipLevels = 1;
		desc.Format = format;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		desc.Flags = isUAV ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAG_NONE;

		currentState_ = initialState;

		auto hr = pDev->GetDeviceDep()->CreateCommittedResource(&prop, D3D12_HEAP_FLAG_NONE, &desc, currentState_, nullptr, IID_PPV_ARGS(&pResource_));
		if (FAILED(hr))
		{
			return false;
		}

		resourceDesc_ = desc;
		heapProp_ = prop;
		size_ = size;
		stride_ = stride;
		bufferUsage_ = type;
		isUAV_ = isUAV;
		return true;
	}

	//----
	void Buffer::Destroy()
	{
		SafeRelease(pResource_);
	}

	//----
	void Buffer::UpdateBuffer(Device* pDev, CommandList* pCmdList, const void* pData, size_t size, size_t offset)
	{
		if (!pDev || !pCmdList)
		{
			return;
		}
		if (!pData || !size)
		{
			return;
		}
		if (offset + size > size_)
		{
			return;
		}

		if (heapProp_.Type == D3D12_HEAP_TYPE_UPLOAD)
		{
			u8* p = reinterpret_cast<u8*>(Map(pCmdList));
			memcpy(p + offset, pData, size);
			Unmap();
		}
		else
		{
			Buffer src;
			if (!src.Initialize(pDev, size, stride_, bufferUsage_, true, false))
			{
				return;
			}
			src.UpdateBuffer(pDev, pCmdList, pData, size, 0);

			pCmdList->Reset();
			pCmdList->GetCommandList()->CopyBufferRegion(pResource_, offset, src.pResource_, 0, size);
			pCmdList->Close();
			pCmdList->Execute();

			Fence fence;
			if (!fence.Initialize(pDev))
			{
				return;
			}
			fence.Signal(pCmdList->GetParentQueue(), 1);
			fence.WaitSignal(1);

			fence.Destroy();
			src.Destroy();
		}

	}

	//----
	void* Buffer::Map(CommandList*)
	{
		if (!pResource_)
		{
			return nullptr;
		}

		void* pData{ nullptr };
		auto hr = pResource_->Map(0, nullptr, &pData);
		if (FAILED(hr))
		{
			return nullptr;
		}
		return pData;
	}

	//----
	void Buffer::Unmap()
	{
		if (!pResource_)
		{
			return;
		}

		pResource_->Unmap(0, nullptr);
	}

}	// namespace sl12

//	EOF
