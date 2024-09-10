package data

import (
	"github.com/onnoj/DeusExEchelonRenderer/OmniverseHelper/internal"
)

type (
	TextureData struct {
		internal.JSONTextureInfo

		ImportTexturePath [TextureType_COUNT]string
	}

	TextureDataSlice    []TextureData
	TextureDataPtrSlice []*TextureData
)

func FromJSONTextureInfo(info internal.JSONTextureInfo) TextureData {
	returned := TextureData{JSONTextureInfo: info}
	return returned
}
