package internal

type (
	JSONTextureInfo struct {
		CacheID       uint     `json:"cacheid,omitempty"`
		Flags         []string `json:"flags,omitempty"`
		Index         uint     `json:"index"`
		Name          string   `json:"name"`
		PackageName   string   `json:"package"`
		Path          string   `json:"path"`
		Realtime      bool     `json:"realtime"`
		RemixHash     uint64   `json:"remixhash"`
		RemixHashText string   `json:"remixhash-hex"`
	}
	JSONTextureInfoSlice []JSONTextureInfo

	JSONTextureObj struct {
		Textures []JSONTextureInfo `json:"Textures"`
	}
)