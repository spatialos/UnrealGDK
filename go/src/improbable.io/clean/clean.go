// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

package main

import (
	"fmt"
	"os"
	"path/filepath"
	"runtime"
	"strings"

	"github.com/spf13/cobra"
)

func main() {
	rootCmd := &cobra.Command{
		Use:   filepath.Base(os.Args[0]),
		Short: "Removes specified directories",
		RunE: func(cmd *cobra.Command, args []string) error {
			errors := []string{}
			for _, dir := range args {
				// Guard against deleting '/'.
				if filepath.IsAbs(dir) {
					errors = append(errors, fmt.Sprintf("paths must be relative: '%v' is absolute", dir))
				}
			}

			if len(errors) > 0 {
				return fmt.Errorf("all paths must be relative:\n%v", strings.Join(errors, "\n"))
			}

			for _, dir := range args {
				absPath, err := filepath.Abs(dir)
				if err != nil {
					return err
				}

				stat, err := os.Stat(absPath)
				if err != nil && !os.IsNotExist(err) {
					fmt.Fprintf(os.Stderr, "Error while retrieving information for '%v': %v\n", dir, err)
				}

				if err == nil && stat.IsDir() {
					// Print the relative path here instead of the absolute.
					fmt.Fprintf(os.Stdout, "Removing %v\n", dir)

					if runtime.GOOS == "windows" {
						// Prepend magic to work around Windows
						// limitation of path length. For details, see
						// https://code.google.com/p/go/issues/detail?id=3358
						// Run filepath.Clean to remove trailing slash.
						// With trailing slash, RemoveAll doesn't remove anything.
						absPath = `\\?\` + filepath.Clean(absPath)
					}

					if err := os.RemoveAll(absPath); err != nil {
						fmt.Fprintf(os.Stderr, "While removing '%v': %v\n", dir, err)
					}
				}
			}
			return nil
		},
	}

	err := rootCmd.Execute()
	if err != nil {
		fmt.Fprintf(os.Stderr, "%v\n", err)
		os.Exit(1)
	}
}
