"use strict";

const npmPackageJson = require("package-json")
    , gitPackageJson = require("git-package-json")
    , gitSource = require("git-source")
    ;

/**
 * packageJson
 * Fetches the `package.json` file contents from `npm` or a git repository.
 *
 * @name packageJson
 * @function
 * @param {String} input The package `npm` name or git url. The git urls are
 * friendly, being parsed by [`git-source`](https://github.com/IonicaBizau/git-source).
 * @param {String} version The `npm` package version or a git sha/tag/branch/etc.
 * @param {Function} callback The callback function.
 */
module.exports = function packageJson(input, version, callback) {

    let parsed = gitSource(input);

    if (typeof version === "function") {
        callback = version;
        version = undefined;
    }

    if (parsed.protocol === "file") {
        npmPackageJson(input, version).then(callback.bind(this, null), callback);
    } else {
        gitPackageJson(parsed, version, callback);
    }
};
