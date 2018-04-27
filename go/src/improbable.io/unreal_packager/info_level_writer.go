// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

package main

import (
	"bytes"
	"fmt"
	"io"
	"os"
	"strings"
)

// infoLevelWriter converts each line of input into a format at log_level "info" for `spatial worker build` consumption.
type infoLevelWriter struct {
	buffer    *bytes.Buffer
	unprinted string
}

func newInfoLevelWriter() *infoLevelWriter {
	writer := &infoLevelWriter{
		buffer: &bytes.Buffer{},
	}
	return writer
}

func (w *infoLevelWriter) Write(b []byte) (int, error) {
	n, err := w.buffer.Write(b)
	if err != nil {
		return n, err
	}

	for {
		line, err := w.buffer.ReadString('\n')
		w.unprinted += line
		if err == io.EOF {
			break
		}
		if err != nil {
			return 0, err
		}

		w.unprinted = strings.TrimSuffix(w.unprinted, "\n")
		w.unprinted = strings.TrimSuffix(w.unprinted, "\r")
		fmt.Fprintf(os.Stdout, "level=info%v\n", w.unprinted)
		w.unprinted = ""
	}
	return n, nil
}
