package utils

func AppendUnique[Type comparable](slice []Type, elems ...Type) []Type {
	for _, rh := range elems {
		found := false
		for _, lh := range slice {
			if rh == lh {
				found = true
				break
			}
		}
		if !found {
			slice = append(slice, rh)
		}
	}
	return slice
}
