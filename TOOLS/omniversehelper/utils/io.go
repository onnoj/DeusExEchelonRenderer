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

type debugWriter struct {
	f *os.File
}

type nullDebugWriter struct {
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
	return &debugWriter{
		f: tmpFile,
	}
}

func NewDebugWriterDummy(identifier string, filetype string) DebugWriter {
	return &nullDebugWriter{}
}

func (d *debugWriter) Write(txt string) {
	d.f.WriteString(txt)
}

func (d *debugWriter) Close() {
	d.f.Close()
}

func (d *nullDebugWriter) Write(txt string) {
}

func (d *nullDebugWriter) Close() {
}
