﻿#include <sl12/texture_view.h>

#include <sl12/device.h>
#include <sl12/texture.h>
#include <sl12/descriptor.h>
#include <sl12/descriptor_heap.h>


namespace sl12
{
	//----
	bool TextureView::Initialize(Device* pDev, Texture* pTex)
	{
		const D3D12_RESOURCE_DESC& resDesc = pTex->GetResourceDesc();
		D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc{};
		viewDesc.Format = resDesc.Format;
		viewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		if (resDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE1D)
		{
			if (resDesc.DepthOrArraySize == 1)
			{
				viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
				viewDesc.Texture1D.MipLevels = resDesc.MipLevels;
				viewDesc.Texture1D.MostDetailedMip = 0;
				viewDesc.Texture1D.ResourceMinLODClamp = 0.0f;
			}
			else
			{
				viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
				viewDesc.Texture1DArray.MipLevels = resDesc.MipLevels;
				viewDesc.Texture1DArray.MostDetailedMip = 0;
				viewDesc.Texture1DArray.ResourceMinLODClamp = 0.0f;
				viewDesc.Texture1DArray.FirstArraySlice = 0;
				viewDesc.Texture1DArray.ArraySize = resDesc.DepthOrArraySize;
			}
		}
		else if (resDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)
		{
			if (resDesc.SampleDesc.Count == 1)
			{
				if (resDesc.DepthOrArraySize == 1)
				{
					viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
					viewDesc.Texture2D.MipLevels = resDesc.MipLevels;
					viewDesc.Texture2D.MostDetailedMip = 0;
					viewDesc.Texture2D.PlaneSlice = 0;
					viewDesc.Texture2D.ResourceMinLODClamp = 0.0f;
				}
				else
				{
					viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
					viewDesc.Texture2DArray.MipLevels = resDesc.MipLevels;
					viewDesc.Texture2DArray.MostDetailedMip = 0;
					viewDesc.Texture2DArray.PlaneSlice = 0;
					viewDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;
					viewDesc.Texture2DArray.FirstArraySlice = 0;
					viewDesc.Texture2DArray.ArraySize = resDesc.DepthOrArraySize;
				}
			}
			else
			{
				if (resDesc.DepthOrArraySize == 1)
				{
					viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
				}
				else
				{
					viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
					viewDesc.Texture2DMSArray.FirstArraySlice = 0;
					viewDesc.Texture2DMSArray.ArraySize = resDesc.DepthOrArraySize;
				}
			}
		}
		else
		{
			viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
			viewDesc.Texture3D.MipLevels = resDesc.MipLevels;
			viewDesc.Texture3D.MostDetailedMip = 0;
			viewDesc.Texture3D.ResourceMinLODClamp = 0.0f;
		}

		pDesc_ = pDev->GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV).CreateDescriptor();
		if (!pDesc_)
		{
			return false;
		}

		pDev->GetDeviceDep()->CreateShaderResourceView(pTex->GetResourceDep(), &viewDesc, pDesc_->GetCpuHandle());

		return true;
	}

	//----
	void TextureView::Destroy()
	{
		SafeRelease(pDesc_);
	}


	//----
	bool RenderTargetView::Initialize(Device* pDev, Texture* pTex, u32 mipSlice, u32 firstArray, u32 arraySize)
	{
		const D3D12_RESOURCE_DESC& resDesc = pTex->GetResourceDesc();
		D3D12_RENDER_TARGET_VIEW_DESC viewDesc{};
		viewDesc.Format = resDesc.Format;
		if (resDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE1D)
		{
			if (resDesc.DepthOrArraySize == 1)
			{
				viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1D;
				viewDesc.Texture1D.MipSlice = mipSlice;
			}
			else
			{
				viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1DARRAY;
				viewDesc.Texture1DArray.MipSlice = mipSlice;
				viewDesc.Texture1DArray.FirstArraySlice = firstArray;
				viewDesc.Texture1DArray.ArraySize = arraySize;
			}
		}
		else if (resDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)
		{
			if (resDesc.SampleDesc.Count == 1)
			{
				if (resDesc.DepthOrArraySize == 1)
				{
					viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
					viewDesc.Texture2D.MipSlice = mipSlice;
					viewDesc.Texture2D.PlaneSlice = 0;
				}
				else
				{
					viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
					viewDesc.Texture2DArray.MipSlice = mipSlice;
					viewDesc.Texture2DArray.PlaneSlice = 0;
					viewDesc.Texture2DArray.FirstArraySlice = firstArray;
					viewDesc.Texture2DArray.ArraySize = arraySize;
				}
			}
			else
			{
				if (resDesc.DepthOrArraySize == 1)
				{
					viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
				}
				else
				{
					viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY;
					viewDesc.Texture2DMSArray.FirstArraySlice = firstArray;
					viewDesc.Texture2DMSArray.ArraySize = arraySize;
				}
			}
		}
		else if (resDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D)
		{
			viewDesc.Texture3D.MipSlice = mipSlice;
			viewDesc.Texture3D.FirstWSlice = firstArray;
			viewDesc.Texture3D.WSize = arraySize;
		}
		else
		{
			return false;
		}

		pDesc_ = pDev->GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV).CreateDescriptor();
		if (!pDesc_)
		{
			return false;
		}

		pDev->GetDeviceDep()->CreateRenderTargetView(pTex->GetResourceDep(), &viewDesc, pDesc_->GetCpuHandle());

		return true;
	}

	//----
	void RenderTargetView::Destroy()
	{
		SafeRelease(pDesc_);
	}


	//----
	bool DepthStencilView::Initialize(Device* pDev, Texture* pTex, u32 mipSlice, u32 firstArray, u32 arraySize)
	{
		const D3D12_RESOURCE_DESC& resDesc = pTex->GetResourceDesc();
		D3D12_DEPTH_STENCIL_VIEW_DESC viewDesc{};
		viewDesc.Format = resDesc.Format;
		if (resDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE1D)
		{
			if (resDesc.DepthOrArraySize == 1)
			{
				viewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1D;
				viewDesc.Texture1D.MipSlice = mipSlice;
			}
			else
			{
				viewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1DARRAY;
				viewDesc.Texture1DArray.MipSlice = mipSlice;
				viewDesc.Texture1DArray.FirstArraySlice = firstArray;
				viewDesc.Texture1DArray.ArraySize = arraySize;
			}
		}
		else if (resDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)
		{
			if (resDesc.SampleDesc.Count == 1)
			{
				if (resDesc.DepthOrArraySize == 1)
				{
					viewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
					viewDesc.Texture2D.MipSlice = mipSlice;
				}
				else
				{
					viewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
					viewDesc.Texture2DArray.MipSlice = mipSlice;
					viewDesc.Texture2DArray.FirstArraySlice = firstArray;
					viewDesc.Texture2DArray.ArraySize = arraySize;
				}
			}
			else
			{
				if (resDesc.DepthOrArraySize == 1)
				{
					viewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
				}
				else
				{
					viewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY;
					viewDesc.Texture2DMSArray.FirstArraySlice = firstArray;
					viewDesc.Texture2DMSArray.ArraySize = arraySize;
				}
			}
		}
		else
		{
			return false;
		}

		pDesc_ = pDev->GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV).CreateDescriptor();
		if (!pDesc_)
		{
			return false;
		}

		pDev->GetDeviceDep()->CreateDepthStencilView(pTex->GetResourceDep(), &viewDesc, pDesc_->GetCpuHandle());

		return true;
	}

	//----
	void DepthStencilView::Destroy()
	{
		SafeRelease(pDesc_);
	}

}	// namespace sl12

//	EOF
