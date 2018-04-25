// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

package main

import (
	"errors"
	"fmt"
	"io"
	"os"
	"path/filepath"

	"github.com/klauspost/compress/zip"
)

const (
	zipOutputOption       = "output"
	zipBasePathOption     = "basePath"
	zipRelativePathOption = "relativePath"
	zipCompression        = "compression"
)

type CompressionType int

const (
	useCompression CompressionType = iota
	noCompression
)

// Zip takes set of file globs and compresses or stores the contents in a zip file.
// The zip file includes directories, and will maintain file modes (e.g. +x.)
// The outputFileName should not include the ".zip" file extension.
// Paths are stored relative to basePath.
// If relativePath is not the empty string, then the contents of the zip file are stored relative to it.
// It is an error if none of the globs match files on the filesystem.
func Zip(outputFileName string, basePath string, relativePath string, compression CompressionType) error {
	inputFiles, err := readDirectory(basePath)
	if err != nil {
		return err
	}

	if err := validateInputFiles(inputFiles); err != nil {
		return err
	}

	outputFileName = fmt.Sprintf("%v.zip", outputFileName)
	if err := createDirectoryIfNotExists(filepath.Dir(outputFileName)); err != nil {
		return err
	}

	fileHandle, err := os.Create(outputFileName)
	if err != nil {
		return err
	}
	defer fileHandle.Close()

	writer := zip.NewWriter(fileHandle)

	for _, fileName := range inputFiles {
		var stat os.FileInfo
		stat, err = os.Stat(fileName)

		if err != nil {
			return err
		}
		if stat.IsDir() {
			// Ensure empty directories are added to the zip.
			var isEmpty bool
			isEmpty, err = isDirEmpty(fileName)
			if err != nil {
				return err
			}

			if isEmpty {
				if err := packageEmptyDirectory(writer, fileName, basePath, relativePath); err != nil {
					return err
				}
			}
			continue
		}
		if err := packageFile(writer, fileName, basePath, relativePath, getCompressionMethod(compression)); err != nil {
			return err
		}
	}

	fmt.Printf("Wrote %v\n", outputFileName)
	return writer.Close()
}

func validateInputFiles(inputFiles []string) error {
	if len(inputFiles) == 0 {
		return errors.New("no input files found")
	}
	return nil
}

func packageFile(writer *zip.Writer, fileName string, basePath string, relativePath string, compressionMethod uint16) error {
	relativeFileName, err := getRelativeFileName(fileName, basePath, relativePath)
	if err != nil {
		return err
	}
	fileInfoHeader, err := createFileHeader(fileName)
	if err != nil {
		return err
	}
	fileInfoHeader.Name = relativeFileName
	fileInfoHeader.Method = compressionMethod

	compressedFile, err := writer.CreateHeader(fileInfoHeader)
	if err != nil {
		return err
	}

	sourceFile, err := os.Open(fileName)
	if err != nil {
		return err
	}

	_, err = io.Copy(compressedFile, sourceFile)
	_ = sourceFile.Close()

	return err
}

func getRelativeFileName(fileName string, basePath string, relativePath string) (string, error) {
	// Zip files only accept relative paths with /'s as the separator
	relativeFileName := fileName
	relativeFileName, err := filepath.Rel(basePath, relativeFileName)
	if err != nil {
		return "", err
	}

	relativeFileName = filepath.Join(relativePath, relativeFileName)
	relativeFileName = filepath.ToSlash(relativeFileName)
	return relativeFileName, nil
}

func createFileHeader(fileName string) (*zip.FileHeader, error) {
	fileInfo, err := os.Stat(fileName)
	if err != nil {
		return nil, err
	}

	header, err := zip.FileInfoHeader(fileInfo)
	if err != nil {
		return nil, err
	}
	return header, nil
}

func packageEmptyDirectory(writer *zip.Writer, dirName string, basePath string, relativePath string) error {
	relativeDirName, err := getRelativeFileName(dirName, basePath, relativePath)
	if err != nil {
		return err
	}

	dirInfoHeader, err := createFileHeader(dirName)
	if err != nil {
		return err
	}

	// NOTE: The trailing '/' is required to indicate to the zip file that this is an empty directory.
	// According to GoLang docs at 'https://golang.org/pkg/archive/zip/': "only forward slashes are allowed"
	dirInfoHeader.Name = relativeDirName + "/"
	_, err = writer.CreateHeader(dirInfoHeader)
	return err
}

func getCompressionMethod(compression CompressionType) uint16 {
	switch compression {
	case useCompression:
		return zip.Deflate
	default:
		return zip.Store
	}
}

// Creates a directory with a given name if it doesn't exist yet.
// Works like os.MkdirAll (and uses that internally), but returns a more meaningful error if the specified path is a file.
func createDirectoryIfNotExists(directoryName string) error {
	var err error
	if stat, err := os.Stat(directoryName); os.IsNotExist(err) {
		return os.MkdirAll(directoryName, 0755)
	} else if err == nil {
		if !stat.IsDir() {
			return fmt.Errorf("failed to create directory '%v': It already exists as a file", directoryName)
		} else {
			// The directory already exists.
			return nil
		}
	}
	return err
}

// isDirEmpty returns true if the directory exists, but contains no directories or files itself.
func isDirEmpty(name string) (bool, error) {
	directory, err := os.Open(name)
	if err != nil {
		return false, err
	}

	_, err = directory.Readdirnames(1)
	_ = directory.Close()

	if err == io.EOF {
		return true, nil
	}
	return false, err
}

func readDirectory(basePath string) ([]string, error) {
	inputFiles := []string{}

	walkErr := filepath.Walk(basePath, func(path string, info os.FileInfo, err error) error {
		if err != nil {
			return err
		}

		if info.IsDir() {
			isEmpty, err := isDirEmpty(path)
			if err != nil {
				return err
			}

			// Only add empty directories.
			if !isEmpty {
				return nil
			}
		}

		inputFiles = append(inputFiles, path)
		return nil
	})

	if walkErr != nil {
		return nil, walkErr
	}
	return inputFiles, nil
}
