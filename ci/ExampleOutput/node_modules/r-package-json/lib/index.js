"use strict";

var rJson = require("r-json"),
    packPath = require("package-json-path");

/**
 * readPackageJson
 * Reads the `package.json` content from the provided directory.
 *
 * @name readPackageJson
 * @function
 * @param {String} dir The path to the directory containing the `package.json` file.
 * @param {Function} callback The callback function.
 * @return {Object} The `package.json` content (if a `callback` function was not provided).
 */
module.exports = function readPackageJson(dir, callback) {
    return rJson(packPath(dir), callback);
};