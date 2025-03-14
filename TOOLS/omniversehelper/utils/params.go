package utils

import (
	"fmt"
	"regexp"
	"strconv"
)

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

func Unquote[T any](potentiallyQuotedStr T) T {
	if inputSlice, ok := any(potentiallyQuotedStr).([]string); ok {
		output := []string{}
		for _, input := range inputSlice {
			output = append(output, Unquote(input))
		}
		return any(output).(T)
	}
	if inputStr, ok := any(potentiallyQuotedStr).(string); ok {
		if s, e := strconv.Unquote(inputStr); e == nil {
			return any(s).(T)
		} else {
			fmt.Printf("failed to unquote path, error %s\r\n", e.Error())
		}
	}

	return potentiallyQuotedStr
}
