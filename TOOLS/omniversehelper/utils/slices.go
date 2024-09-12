package utils

import "strings"

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

func SelectFromSlice[Type any](valueSlice []Type, evalFunc func(eval Type) bool) *Type {
	for _, v := range valueSlice {
		if evalFunc(v) {
			return &v
		}
	}
	return nil
}

func SelectFromMap[KeyType comparable, ValueType any](valueMap map[KeyType]ValueType, evalFunc func(KeyType, ValueType) bool) (*KeyType, *ValueType) {
	for key, value := range valueMap {
		if evalFunc(key, value) {
			return &key, &value
		}
	}

	return nil, nil
}

func StringJoinField[Type any](valueSlice []Type, sep string, evalFunc func(eval Type) *string) string {
	tmp := []string{}
	for _, v := range valueSlice {
		if s := evalFunc(v); s != nil {
			tmp = append(tmp, *s)
		}
	}
	return strings.Join(tmp, sep)
}
