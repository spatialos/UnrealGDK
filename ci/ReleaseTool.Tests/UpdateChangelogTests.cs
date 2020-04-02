using System.Collections.Generic;
using System.Linq;
using NUnit.Framework;

namespace ReleaseTool.Tests
{
    public class UpdateChangelogTests
    {
        private static PrepCommand.Options OptionsNoPin = new PrepCommand.Options
        {
            Version = "test-version"
        };

        private static PrepCommand.Options OptionsWithPin = new PrepCommand.Options
        {
            Version = "test-version",
            PinnedGdkVersion = "something"
        };

        [Test]
        public void UpdateChangelog_does_nothing_if_header_is_already_there()
        {
            var changelog = new List<string>
            {
                "## Unreleased",
                "",
                $"## `{OptionsNoPin.Version}` - soon :tm:",
                "",
                "### Breaking Changes",
                "",
                "- Made some breaking changes"
            };

            var previousCount = changelog.Count;
            PrepCommand.UpdateChangeLog(changelog, OptionsNoPin);
            Assert.AreEqual(previousCount, changelog.Count);
        }

        [Test]
        public void UpdateChangelog_should_insert_a_heading_after_unreleased()
        {
            var changelog = new List<string>
            {
                "## Unreleased",
                "",
                "### Breaking Changes",
                "",
                "- Made some breaking changes"
            };

            PrepCommand.UpdateChangeLog(changelog, OptionsNoPin);

            Assert.IsTrue(changelog[2].StartsWith($"## `{OptionsNoPin.Version}` - "));
        }

        [Test]
        public void UpdateChangelog_shouldnt_add_a_line_to_changed_section__if_no_gdk_pinned()
        {
            var changelog = new List<string>
            {
                "## Unreleased",
                "",
                "### Breaking Changes",
                "",
                "- Made some breaking changes",
                "",
                "### Changed",
                "",
                "- Made some normal changes"
            };

            var expectedLineCount = changelog.Count + 2; // The release header and an extra newline.

            PrepCommand.UpdateChangeLog(changelog, OptionsNoPin);

            Assert.AreEqual(expectedLineCount, changelog.Count);
        }

        [Test]
        public void UpdateChangelog_should_add_a_line_to_changed_section_if_gdk_pinned()
        {
            var changelog = new List<string>
            {
                "## Unreleased",
                "",
                "### Breaking Changes",
                "",
                "- Made some breaking changes",
                "",
                "### Changed",
                "",
                "- Made some normal changes"
            };

            var expectedLineCount = changelog.Count + 3; // The release header, an extra newline, and the changed entry.
            var expectedLine = string.Format(PrepCommand.ChangeLogUpdateGdkTemplate, OptionsWithPin.Version);

            PrepCommand.UpdateChangeLog(changelog, OptionsWithPin);

            Assert.AreEqual(expectedLineCount, changelog.Count);
            Assert.Contains(expectedLine, changelog);
        }

        [Test]
        public void UpdateChangelog_should_add_a_line_to_the_correct_changed_section_if_gdk_pinned()
        {
            var changelog = new List<string>
            {
                "## Unreleased",
                "",
                "### Breaking Changes",
                "",
                "- Made some breaking changes",
                "",
                "### Changed",
                "",
                "- Made some normal changes",
                "",
                "## A Previous Release",
                "",
                "### Breaking Changes",
                "",
                "- Made some breaking changes",
                "",
                "### Changed",
                "",
                "- Made some normal changes"
            };

            var expectedLine = string.Format(PrepCommand.ChangeLogUpdateGdkTemplate, OptionsWithPin.Version);
            PrepCommand.UpdateChangeLog(changelog, OptionsWithPin);

            var newReleaseSection = changelog.TakeWhile(line => line != "## A Previous Release").ToList();
            Assert.Contains(expectedLine, newReleaseSection);
        }

        [Test]
        public void UpdateChangelog_should_add_a_changed_section_line_if_not_there_if_gdk_pinned()
        {
            var changelog = new List<string>
            {
                "## Unreleased",
                "",
                "### Breaking Changes",
                "",
                "- Made some breaking changes",
                "",
                "## A Previous Release",
                "",
                "### Breaking Changes",
                "",
                "- Made some breaking changes",
                "",
                "### Changed",
                "",
                "- Made some normal changes"
            };

            var expectedChangeHeader = "### Changed";
            var expectedChangeLine = string.Format(PrepCommand.ChangeLogUpdateGdkTemplate, OptionsWithPin.Version);

            PrepCommand.UpdateChangeLog(changelog, OptionsWithPin);

            var newReleaseSection = changelog.TakeWhile(line => line != "## A Previous Release").ToList();
            Assert.Contains(expectedChangeLine, newReleaseSection);
            Assert.Contains(expectedChangeHeader, newReleaseSection);
        }
    }
}
