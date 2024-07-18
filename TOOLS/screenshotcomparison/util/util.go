package util

func AppendUnique[TSliceType comparable](slice []TSliceType, value TSliceType) []TSliceType {
	for _, v := range slice {
		if v == value {
			return slice
		}
	}

	return append(slice, value)
}
