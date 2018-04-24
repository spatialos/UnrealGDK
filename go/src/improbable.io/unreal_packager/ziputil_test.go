// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

package main

import (
	"archive/zip"
	"fmt"
	"io/ioutil"
	"os"
	"path/filepath"
	"strings"
	"testing"

	"github.com/stretchr/testify/assert"
)

const (
	testZipOutputName = "testZipOutput"
	testZipFileSuffix = ".zip"
	testRelativePath  = ""
	fileContent       = "beautifulclouds"
)

func TestMain(m *testing.M) {
	os.Exit(m.Run())
}

func TestScenarios(t *testing.T) {
	scenarioBasePath := getTempDir(t)
	defer func() {
		err := os.RemoveAll(scenarioBasePath)
		assert.NoError(t, err, "Should be able to clean up at the end.")
	}()

	scenarios := map[string]*scenarioTester{
		// File in the root.
		"test1": newScenario(zipAndUnzip, &fileCreator{name: "file.txt"}),
		// Nested file.
		"test2": newScenario(zipAndUnzip, &fileCreator{name: "folder/file.txt"}),
		// Empty folder.
		"test3": newScenario(zipAndUnzip, &dirCreator{name: "folder"}),
		// Nested empty folder.
		"test4": newScenario(zipAndUnzip, &dirCreator{name: "folder/folder"}),
		// No files.
		"test5": newScenario(emptyZip),
	}

	for scenario, tester := range scenarios {
		sourcePath := filepath.Join(scenarioBasePath, scenario)
		tester.run(t, sourcePath)
	}
}

// scenarioTesterCallback is invoked with the dynamically-created sorucePath directory structure.
type scenarioTesterCallback func(t *testing.T, sourcePath string)

// scenarioTester manages a set of files/directories used as a source for the Zip command.
// The structure is created dynamically because git doesn't support checking in a directory without a file in it,
// which makes testing empty directory handling impossible.
type scenarioTester struct {
	callback scenarioTesterCallback
	entries  []fileSystemEntry
}

// run creates the required directory structure for the scenario and then invokes the callback.
func (tester *scenarioTester) run(t *testing.T, sourcePath string) {
	for _, entry := range tester.entries {
		err := entry.create(sourcePath)
		assert.NoError(t, err, "Should be able to create entry on disk.")
	}
	tester.callback(t, sourcePath)
}

func newScenario(tester scenarioTesterCallback, entries ...fileSystemEntry) *scenarioTester {
	return &scenarioTester{callback: tester, entries: append([]fileSystemEntry{}, entries...)}
}

// fileSystemEntry is the interface for creating items on the filesystem.
type fileSystemEntry interface {
	create(basePath string) error
}

// fileCreator  will create a text file with some default content.
type fileCreator struct {
	name string // Name of the item on the fileystem. Use forward slashes to create an nested directory structure.
}

func (f *fileCreator) create(basePath string) error {
	return writeTextFile(filepath.Join(basePath, filepath.FromSlash(f.name)), fileContent)
}

// dirCreator will create a directory on the filesystem.
type dirCreator struct {
	name string // Name of the item on the fileystem. Use forward slashes to create an nested directory structure.
}

func (d *dirCreator) create(basePath string) error {
	return createDirectoryIfNotExists(filepath.Join(basePath, filepath.FromSlash(d.name)))
}

// zipAndUnzip loads the zip file into memory and compares its contents to the source on disk
func zipAndUnzip(t *testing.T, sourceFolder string) {
	basePath := getTempDir(t)
	testZipOutputPath := filepath.Join(basePath, testZipOutputName)
	defer func() {
		err := os.RemoveAll(basePath)
		assert.NoError(t, err, "Should be able to clean up at the end.")
	}()

	err := Zip(testZipOutputPath, sourceFolder, testRelativePath, useCompression)
	assert.Nil(t, err, "Zip should complete without errors.")

	// Open the zip file.
	file, err := os.Open(testZipOutputPath + testZipFileSuffix)
	assert.Nil(t, err, "Source file should be valid.")
	defer func() {
		_ = file.Close()
	}()

	stat, err := file.Stat()
	assert.Nil(t, err, "File stat should be valid.")
	reader, err := zip.NewReader(file, stat.Size())
	assert.Nil(t, err, "Zip should be valid.")

	// Read the source directory.
	aFiles, err := readDirectory(sourceFolder)
	assert.Nil(t, err, "Reading directory should succeed.")

	assert.Equal(t, len(aFiles), len(reader.File), "Directories have the same number of elements.")

	// Map relative paths to the on-disk FileInfo.
	diskFiles := map[string]os.FileInfo{}

	for _, fileName := range aFiles {
		relPath, err := filepath.Rel(sourceFolder, fileName)
		assert.Nil(t, err, "Path should be resolvable.")

		// Normalize to match the zip file's forward slashes.
		relPath = strings.Replace(relPath, "\\", "/", -1)

		diskFiles[relPath], err = os.Stat(fileName)
		assert.Nil(t, err, "Source file should exist.")
	}

	// Read the zip and compare its contents to the on-disk source files/directories.
	for _, zipFile := range reader.File {
		zipStat := zipFile.FileInfo()
		zipLookupName := zipFile.Name

		// Empty directories end in a '/'.
		if strings.HasSuffix(zipLookupName, "/") {
			assert.True(t, zipStat.IsDir(), "Zip path ending in '/' must be a directory")

			// Directories on disk don't end in /, so strip it here.
			zipLookupName = strings.TrimSuffix(zipLookupName, "/")
		}

		diskFileInfo, ok := diskFiles[zipLookupName]

		assert.True(t, ok, "Names are identical.")

		assert.Equal(t, diskFileInfo.IsDir(), zipStat.IsDir(), fmt.Sprintf("IsDir: '%v' matches '%v'", diskFileInfo.Name(), zipStat.Name()))
		assert.Equal(t, diskFileInfo.Mode(), zipStat.Mode(), fmt.Sprintf("Mode: '%v' matches '%v'", diskFileInfo.Name(), zipStat.Name()))

		// FileInfo.Size() varies based on OS for directories; avoid test, since .zip will always store as size 0.
		if !diskFileInfo.IsDir() {
			assert.Equal(t, diskFileInfo.Size(), zipStat.Size(), fmt.Sprintf("Size: '%v' matches '%v'", diskFileInfo.Name(), zipStat.Name()))
		}
	}
}

// emptyZip ensures that trying to zip 0 files results in an error.
func emptyZip(t *testing.T, basePath string) {
	testZipOutputPath := filepath.Join(basePath, testZipOutputName)

	err := Zip(testZipOutputPath, basePath, testRelativePath, useCompression)
	assert.NotNil(t, err, "Finding no input files should be an error.")
}

// getTempDir returns a unique temporary path for testing.
// It should be removed by the caller once the test is complete.
func getTempDir(t *testing.T) string {
	basePath, err := ioutil.TempDir("", "ziputil_test_")
	assert.Nil(t, err, "Temporary storage should be available.")

	return basePath
}
