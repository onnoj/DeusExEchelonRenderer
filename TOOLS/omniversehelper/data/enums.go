package data

import "log"

// Enums
type (
	TextureType uint
)

const (
	TextureType_UNKNOWN   TextureType = iota
	TextureType_Albedo    TextureType = iota
	TextureType_Roughness TextureType = iota
	TextureType_NormalDX  TextureType = iota
	TextureType_NormalOGL TextureType = iota
	TextureType_Metalness TextureType = iota
	TextureType_COUNT     TextureType = iota
)

func TextureTypeToString(t TextureType) string {
	switch t {
	case TextureType_Albedo:
		return "DIFFUSE"
	case TextureType_Roughness:
		return "ROUGHNESS"
	case TextureType_NormalDX:
		return "NORMAL_DX"
	case TextureType_NormalOGL:
		return "NORMAL_OGL"
	case TextureType_Metalness:
		return "METALLIC"
	default:
		log.Fatal("")
	}
	return ""
}

func TextureTypeToShaderInput(t TextureType) string {
	/*
		Pulled by calling:
		http://127.0.0.1:8011/stagecraft/textures/%2FRootNode%2FLooks%2Fmat_414F8DC759E61963%2FShader.inputs%3Areflectionroughness_texture/material/inputs

			* diffuse_texture
		    * reflectionroughness_texture
		    * anisotropy_texture
		    * metallic_texture
		    * emissive_mask_texture
		    * normalmap_texture
		    * height_texture
		    * subsurface_transmittance_texture
		    * subsurface_thickness_texture
		    * subsurface_single_scattering_texture
	*/

	switch t {
	case TextureType_Albedo:
		return "diffuse_texture"
	case TextureType_Roughness:
		return "reflectionroughness_texture"
	case TextureType_NormalDX:
		return "normalmap_texture"
	case TextureType_NormalOGL:
		return "normalmap_texture"
	case TextureType_Metalness:
		return "metallic_texture"
	default:
		log.Fatal("")
	}
	return ""
}

func TextureTypeToFilenameGlob(t TextureType) string {
	/*
		Inspired by.. https://github.com/NVIDIAGameWorks/toolkit-remix/blob/b731a19b8a3e4de7515fc0313191b26ac7342c11/source/extensions/omni.flux.asset_importer.widget/omni/flux/asset_importer/widget/tests/unit/texture_import_list/test_texture_import_list_model.py#L185

			* diffuse_texture
		    * reflectionroughness_texture
		    * anisotropy_texture
		    * metallic_texture
		    * emissive_mask_texture
		    * normalmap_texture
		    * height_texture
		    * subsurface_transmittance_texture
		    * subsurface_thickness_texture
		    * subsurface_single_scattering_texture
	*/

	switch t {
	case TextureType_Albedo:
		return "diffuse"
	case TextureType_Roughness:
		return "roughness"
	case TextureType_NormalDX:
		return "Normal.n"
	case TextureType_NormalOGL:
		return "Normal.n"
	case TextureType_Metalness:
		return "metallic"
	default:
		log.Fatal("")
	}
	return ""
}
