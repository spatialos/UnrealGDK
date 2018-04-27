package main

import (
	"bufio"
	"flag"
	"fmt"
	"io"
	"os"
	"regexp"
	"strings"
	"time"
)

const (
	TEAMCITY_TIMESTAMP_FORMAT = "2006-01-02T15:04:05.000"
)

type Test struct {
	Start    string
	Name     string
	Output   string
	Details  []string
	Duration time.Duration
	Status   string
	Race     bool
}

var (
	input  = os.Stdin
	output = os.Stdout

	additionalTestName = ""

	run   = regexp.MustCompile("^=== RUN\\s+([a-zA-Z_]\\S*)")
	end   = regexp.MustCompile("^(\\s*)--- (PASS|SKIP|FAIL):\\s+([a-zA-Z_]\\S*) \\((-?[\\.\\ds]+)\\)")
	suite = regexp.MustCompile("^(ok|PASS|FAIL|exit status|Found)")
	race  = regexp.MustCompile("^WARNING: DATA RACE")
)

func init() {
	flag.StringVar(&additionalTestName, "name", "", "Add prefix to test name")
}

func escapeLines(lines []string) string {
	return escape(strings.Join(lines, "\n"))
}

func escape(s string) string {
	s = strings.Replace(s, "|", "||", -1)
	s = strings.Replace(s, "\n", "|n", -1)
	s = strings.Replace(s, "\r", "|n", -1)
	s = strings.Replace(s, "'", "|'", -1)
	s = strings.Replace(s, "]", "|]", -1)
	s = strings.Replace(s, "[", "|[", -1)
	return s
}

func getNow() string {
	return time.Now().Format(TEAMCITY_TIMESTAMP_FORMAT)
}

func outputTest(w io.Writer, test *Test) {
	now := getNow()
	testName := escape(additionalTestName + test.Name)
	fmt.Fprintf(w, "##teamcity[testStarted timestamp='%s' name='%s' captureStandardOutput='true']\n", test.Start, testName)
	fmt.Fprint(w, test.Output)
	if test.Status == "SKIP" {
		fmt.Fprintf(w, "##teamcity[testIgnored timestamp='%s' name='%s']\n", now, testName)
	} else {
		if test.Race {
			fmt.Fprintf(w, "##teamcity[testFailed timestamp='%s' name='%s' message='Race detected!' details='%s']\n",
				now, testName, escapeLines(test.Details))
		} else {
			switch test.Status {
			case "FAIL":
				fmt.Fprintf(w, "##teamcity[testFailed timestamp='%s' name='%s' details='%s']\n",
					now, testName, escapeLines(test.Details))
			case "PASS":
				// ignore
			default:
				fmt.Fprintf(w, "##teamcity[testFailed timestamp='%s' name='%s' message='Test ended in panic.' details='%s']\n",
					now, testName, escapeLines(test.Details))
			}
		}
		fmt.Fprintf(w, "##teamcity[testFinished timestamp='%s' name='%s' duration='%d']\n",
			now, testName, test.Duration/time.Millisecond)
	}
}

func processReader(r *bufio.Reader, w io.Writer) {
	tests := make(map[string]*Test)
	var test *Test
	var final string
	prefix := "\t"
	for {
		line, err := r.ReadString('\n')
		if err != nil {
			break
		}

		runOut := run.FindStringSubmatch(line)
		endOut := end.FindStringSubmatch(line)
		suiteOut := suite.FindStringSubmatch(line)

		if test != nil && test.Status != "" && (runOut != nil || endOut != nil || suiteOut != nil) {
			outputTest(w, test)
			delete(tests, test.Name)
			test = nil
		}

		if runOut != nil {
			test = &Test{
				Name:  runOut[1],
				Start: getNow(),
			}
			tests[test.Name] = test
		} else if endOut != nil {
			test = tests[endOut[3]]
			prefix = endOut[1] + "\t"
			test.Status = endOut[2]
			test.Duration, _ = time.ParseDuration(endOut[4])
		} else if suiteOut != nil {
			final += line
		} else if race.MatchString(line) {
			test.Race = true
		} else if test != nil && test.Status != "" && strings.HasPrefix(line, prefix) {
			line = line[:len(line)-1]
			line = strings.TrimPrefix(line, prefix)
			test.Details = append(test.Details, line)
		} else if test != nil {
			test.Output += line
		} else {
			fmt.Fprint(w, line)
		}
	}
	if test != nil {
		outputTest(w, test)
		delete(tests, test.Name)
	}
	for _, t := range tests {
		outputTest(w, t)
	}

	fmt.Fprint(w, final)
}

func main() {
	flag.Parse()

	if len(additionalTestName) > 0 {
		additionalTestName += " "
	}

	reader := bufio.NewReader(input)

	processReader(reader, output)
}
