"use strict";

var path = require("path"),
    abs = require("abs");

/**
 * packageJsonPath
 * Finds the absolute path to the `package.json` path.
 *
 * @name packageJsonPath
 * @function
 * @param {String} dir The path to the directory.
 * @return {String} The absolute path to the `package.json` file.
 */
module.exports = function packageJsonPath(dir) {
    return path.join(abs(dir), "package.json");
};