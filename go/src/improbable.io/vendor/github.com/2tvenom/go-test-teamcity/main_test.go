package main

import (
	"bufio"
	"bytes"
	"io/ioutil"
	"os"
	"regexp"
	"strings"
	"testing"
)

var (
	inDir  = "testdata/input/"
	outDir = "testdata/output/"

	timestampRegexp      = regexp.MustCompile(`timestamp='.*?'`)
	timestampReplacement = `timestamp='2017-01-02T04:05:06.789'`
)

func TestProcessReader(t *testing.T) {
	files, err := ioutil.ReadDir(inDir)
	if err != nil {
		t.Error(err)
	}
	for _, file := range files {
		t.Run(file.Name(), func(t *testing.T) {
			inpath := inDir + file.Name()
			f, err := os.Open(inpath)
			if err != nil {
				t.Error(err)
			}
			in := bufio.NewReader(f)

			out := &bytes.Buffer{}
			processReader(in, out)
			actual := out.String()
			actual = timestampRegexp.ReplaceAllString(actual, timestampReplacement)

			outpath := outDir + file.Name()
			t.Logf("input: %s", inpath)
			t.Logf("output: %s", outpath)
			expectedBytes, err := ioutil.ReadFile(outpath)
			if err != nil {
				t.Error(err)
			}
			expected := string(expectedBytes)
			expected = timestampRegexp.ReplaceAllString(expected, timestampReplacement)

			if strings.Compare(expected, actual) != 0 {
				t.Errorf("expected:\n\n%s\nbut got:\n\n%s\n", expected, actual)
			}
		})
	}
}
