package utils

import (
	"fmt"
	"os"
	"regexp"
	"strings"
)

type DebugWriter interface {
	Write(txt string)
	Close()
}

type debugwriter struct {
	f *os.File
}

func stripFilePath(path string) string {
	separator := string(os.PathSeparator)
	path = strings.ReplaceAll(path, separator, "")
	illegalChars := regexp.MustCompile(`[<>:"/\\|?*]`)
	path = illegalChars.ReplaceAllString(path, "_")

	return path
}

func NewDebugWriter(identifier string, filetype string) DebugWriter {
	tmpFile, _ := os.CreateTemp("", fmt.Sprintf("%s_*%s", stripFilePath(identifier), filetype))
	return &debugwriter{
		f: tmpFile,
	}
}

func (d *debugwriter) Write(txt string) {
	d.f.WriteString(txt)
}

func (d *debugwriter) Close() {
	d.f.Close()
}
