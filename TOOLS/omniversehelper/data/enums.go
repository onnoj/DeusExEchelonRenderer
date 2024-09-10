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
