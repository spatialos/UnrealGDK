
# pkg.json

 [![PayPal](https://img.shields.io/badge/%24-paypal-f39c12.svg)][paypal-donations] [![AMA](https://img.shields.io/badge/ask%20me-anything-1abc9c.svg)](https://github.com/IonicaBizau/ama) [![Version](https://img.shields.io/npm/v/pkg.json.svg)](https://www.npmjs.com/package/pkg.json) [![Downloads](https://img.shields.io/npm/dt/pkg.json.svg)](https://www.npmjs.com/package/pkg.json) [![Get help on Codementor](https://cdn.codementor.io/badges/get_help_github.svg)](https://www.codementor.io/johnnyb?utm_source=github&utm_medium=button&utm_term=johnnyb&utm_campaign=github)

> Get the package.json content either from npm or from a git repository.

## :cloud: Installation

```sh
$ npm i --save pkg.json
```


## :clipboard: Example



```js
const packageJson = require("pkg.json");

packageJson("git-stats", function (err, data) {
    console.log(err || data);
    // { _id: 'git-stats',
    //   _rev: '34-f8a1ea58b78974bb0c530dd9e366cf24',
    //   name: 'git-stats',
    //   description: 'Local git statistics including GitHub-like contributions calendars.',
    //   'dist-tags': { latest: '2.9.2' },
    //   versions: {...},
    //   readme: '[![git-stats](http://i.imgur.com/Q7TQYHx.png)](#)\n\n# `$ git-stats` [![PayPal](https://img.shields.io/badge/%24-paypal-f39c12.svg)][paypal-donations] [![Version](https://img.shields.io/npm/v/git-stats.svg)](https://www.npmjs.com/package/git-stats) [![Downloads](https://img.shields.io/npm/dt/git-stats.svg)](https://www.npmjs.com/package/git-stats) [![Get help on Codementor](https://cdn.codementor.io/badges/get_help_github.svg)](https://www.codementor.io/johnnyb?utm_source=github&utm_medium=button&utm_term=johnnyb&utm_campaign=github)\n\n> Local git statistics including GitHub-like contributions calendars.\n\nI\'d be curious to see your calendar with all your commits. Ping me on Twitter ([**@IonicaBizau**](https://twitter.com/IonicaBizau)). :smile: Until then, here\'s my calendar:\n\n![](http://i.imgur.com/PpM0i3v.png)\n\n## Contents\n\n - [Installation](#installation)\n - [Usage](#usage)\n    \n     - [Importing and deleting commits](#importing-and-deleting-commits)\n     - [Importing all the commits from GitHub and BitBucket](#importing-all-the-commits-from-github-and-bitbucket)\n     - [What about the GitHub Contributions calendar?](#what-about-the-github-contributions-calendar)\n    \n - [Documentation](#documentation)\n - [How to contribute](#how-to-contribute)\n\n## Installation\n\nYou can install the package globally and use it as command line tool:\n\n```sh\n# Install the package globally\nnpm i -g git-stats\n# Initialize git hooks\n# This is for tracking the new commits\ncurl -s https://raw.githubusercontent.com/IonicaBizau/git-stats/master/scripts/init-git-post-commit | bash\n```\n\nThen, run `git-stats --help` and see what the CLI tool can do.\n\n```sh\n$ git-stats --help\nUsage: git-stats [options]\n\nOptions:\n  -s, --since <date>     Optional start date.                             \n  -u, --until <date>     Optional end date.                               \n  -n, --no-ansi          Forces the tool not to use ANSI styles.          \n  -l, --light            Enables the light theme.                         \n  -a, --authors          Shows a pie chart with the author related        \n                         contributions in the current repository.         \n  -g, --global-activity  Shows global activity calendar in the current    \n                         repository.                                      \n  -d, --data <path>      Sets a custom data store file.                   \n  -f, --first-day <day>  Sets the first day of the week.                  \n  --record <data>        Records a new commit. Don\'t use this unless you  \n                         are a mad scientist. If you are a developer, just\n                         use this option as part of the module.           \n  -r, --raw              Outputs a dump of the raw JSON data.             \n  -h, --help             Displays this help.                              \n  -v, --version          Displays version information.                    \n\nExamples:\n  git-stats # Default behavior (stats in the last year)\n  git-stats -l # Light mode\n  git-stats -s \'1 January 2012\' # All the commits from 1 January 2012 to now\n  git-stats -s \'1 January 2012\' -u \'31 December 2012\' # All the commits from 2012\n\nYour commit history is kept in ~/.git-stats by default. You can create ~/.git-stats-config.json to specify different defaults.\n\nDocumentation can be found at https://github.com/IonicaBizau/git-stats\n```\n\n## Usage\n### Importing and deleting commits\n\nI know it\'s not nice to start your git commit calendar from scratch. That\'s why I created [`git-stats-importer`](https://github.com/IonicaBizau/git-stats-importer)–a tool which imports or deletes the commits from selected repositories.\n\nCheck it out here: https://github.com/IonicaBizau/git-stats-importer\n\nThe usage is simple:\n\n```sh\n# Install the importer tool\n$ npm install -g git-stats-importer\n\n# Go to the repository you want to import\n$ cd path/to/my-repository\n\n# Import the commits\n$ git-stats-importer\n\n# ...or delete them if that\'s a dummy repository\n$ git-stats-importer --delete\n```\n### Importing all the commits from GitHub and BitBucket\n\nYes, that\'s also possible. I [built a tool which downloads and then imports all the commits you have pushed to GitHub and BitBucket](https://github.com/IonicaBizau/repository-downloader)!\n\n```sh\n# Download the repository downloader\n$ git clone https://github.com/IonicaBizau/repository-downloader.git\n\n# Go to repository downloader\n$ cd repository-downloader\n\n# Install the dependencies\n$ npm install\n\n# Start downloading and importing\n$ ./start\n```\n### What about the GitHub Contributions calendar?\n\nIf you want to visualize the calendars that appear on GitHub profiles, you can do that using [`ghcal`](https://github.com/IonicaBizau/ghcal).\n\n```sh\n# Install ghcal\n$ npm install -g ghcal\n\n# Check out @alysonla\'s contributions\n$ ghcal -u alysonla\n```\n\nFor more detailed documentation, check out the repository: https://github.com/IonicaBizau/ghcal.\n\nIf want to get even more GitHub stats in your terminal, you may want to try [`github-stats`](https://github.com/IonicaBizau/github-stats)--this is like `git-stats` but with data taken from GitHub.\n\n## Using the configuration file\n\nYou can tweak the git-stats behavior using a configuration file in your home directory: `~/.git-stats-config.js`.\n\nThis file should export an object, like below (defaults are listed):\n\n```js\nmodule.exports = {\n    // "DARK", "LIGHT" or an object interpreted by IonicaBizau/node-git-stats-colors\n    "theme": "DARK"\n\n    // The file where the commit hashes will be stored\n  , "path": "~/.git-stats"\n\n    // First day of the week\n  , first_day: "Sun"\n\n    // This defaults to *one year ago*\n    // It can be any parsable date\n  , since: undefined\n\n    // This defaults to *now*\n    // It can be any parsable date\n  , until: undefined\n\n    // Don\'t show authors by default\n    // If true, this will enable the authors pie\n  , authors: false\n\n    // No global activity by default\n    // If true, this will enable the global activity calendar in the current project\n  , global_activity: false\n};\n```\n\nSince it\'s a js file, you can `require` any other modules there.\n\n## Cross-platform compatibility\n\n`git-stats` is working fine in terminal emulators supporting ANSI styles. It should work fine on Linux and OS X.\n\nIf you run `git-stats` to display graph on Windows, please use a terminal that can properly display ANSI colors.\n\nCygwin Terminal is known to work, while Windows Command Prompt and Git Bash do not. Improvements are more than welcome! :dizzy:\n\n## Example\n\nHere is an example how to use this package as library. To install it locally, as library, you can do that using `npm`:\n\n```sh\n$ npm i --save git-stats\n```\n\n```js\n// Dependencies\nvar GitStats = require("git-stats");\n\n// Create the GitStats instance\nvar g1 = new GitStats();\n\n// Display the ansi calendar\ng1.ansiCalendar({\n    theme: "DARK"\n}, function (err, data) {\n    console.log(err || data);\n});\n```\n\n## Documentation\n\nFor full API reference, see the [DOCUMENTATION.md][docs] file.\n\n## Press Highlights\n\n - [*A GitHub-like contributions calendar, but locally, with all your git commits*, The Changelog](https://changelog.com/github-like-contributions-calendar-locally-git-commits/)\n\n## How to contribute\nHave an idea? Found a bug? See [how to contribute][contributing].\n\n## Where is this library used?\nIf you are using this library in one of your projects, add it in this list. :sparkles:\n\n - [`git-stats-importer`](https://github.com/IonicaBizau/git-stats-importer)\n\n## License\n\n[MIT][license] © [Ionică Bizău][website]\n\n[paypal-donations]: https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=RVXDDLKKLQRJW\n[donate-now]: http://i.imgur.com/6cMbHOC.png\n\n[license]: http://showalicense.com/?fullname=Ionic%C4%83%20Biz%C4%83u%20%3Cbizauionica%40gmail.com%3E%20(http%3A%2F%2Fionicabizau.net)&year=2015#license-mit\n[website]: http://ionicabizau.net\n[contributing]: /CONTRIBUTING.md\n[docs]: /DOCUMENTATION.md',
    //   maintainers: [ { name: 'ionicabizau', email: 'bizauionica@yahoo.com' } ],
    //   time: {...},
    //   ...
    //   users: { bret: true, despairblue: true, program247365: true },
    //   _attachments: {} }
});

packageJson("IonicaBizau/git-stats", function (err, data) {
    console.log(err || data);
    // { name: 'git-stats',
    //   version: '2.9.2',
    //   description: 'Local git statistics including GitHub-like contributions calendars.',
    //   main: 'lib/index.js',
    //   bin: { 'git-stats': './bin/git-stats' },
    //   ...
    //   _id: 'git-stats@2.9.2' }
});

packageJson("git@github.com:IonicaBizau/git-stats.git", function (err, data) {
    console.log(err || data);
    // { name: 'git-stats',
    //   version: '2.9.2',
    //   description: 'Local git statistics including GitHub-like contributions calendars.',
    //   ...
    //   _id: 'git-stats@2.9.2' }
});

packageJson("git@github.com:IonicaBizau/git-stats.git", "2.7.0", function (err, data) {
    console.log(err || data.version);
    // "2.7.0"
});
```

## :memo: Documentation

### `packageJson(input, version, callback)`
Fetches the `package.json` file contents from `npm` or a git repository.

#### Params
- **String** `input`: The package `npm` name or git url. The git urls are friendly, being parsed by [`git-source`](https://github.com/IonicaBizau/git-source).
- **String** `version`: The `npm` package version or a git sha/tag/branch/etc.
- **Function** `callback`: The callback function.



## :yum: How to contribute
Have an idea? Found a bug? See [how to contribute][contributing].


## :scroll: License

[MIT][license] © [Ionică Bizău][website]

[paypal-donations]: https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=RVXDDLKKLQRJW
[donate-now]: http://i.imgur.com/6cMbHOC.png

[license]: http://showalicense.com/?fullname=Ionic%C4%83%20Biz%C4%83u%20%3Cbizauionica%40gmail.com%3E%20(http%3A%2F%2Fionicabizau.net)&year=2016#license-mit
[website]: http://ionicabizau.net
[contributing]: /CONTRIBUTING.md
[docs]: /DOCUMENTATION.md
