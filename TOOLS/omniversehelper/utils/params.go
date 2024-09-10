package utils

import "regexp"

func NewPtrValue[T any](value T) *T {
	ptr := new(T)
	*ptr = value
	return ptr
}

func MatchAllOfRegex(slice []*regexp.Regexp, value string) bool {
	if len(slice) == 0 {
		return true
	}

	for _, re := range slice {
		if !re.Match([]byte(value)) {
			return false
		}
	}
	return true
}

func MatchAnyOfRegex(slice []*regexp.Regexp, value string) bool {
	if len(slice) == 0 {
		return true
	}

	for _, re := range slice {
		if re.Match([]byte(value)) {
			return true
		}
	}
	return false
}
