"use strict";

// Dependencies
var ChildProcess = require("child_process"),
    LimitIt = require("limit-it"),
    Typpy = require("typpy");

/**
 * ExecLimiter
 * Creates a new instance of `ExecLimiter`.
 *
 * @name ExecLimiter
 * @function
 * @param {Number} limit The limit of commands to run same time.
 * @return {ExecLimiter} The `ExecLimiter` instance.
 */
function ExecLimiter(limit) {
    if (Typpy(this) !== "execlimiter") {
        return new ExecLimiter(limit);
    }
    this.limitIt = new LimitIt(limit);
}

/**
 * add
 * Adds a new command to run in the buffer.
 *
 * Usage:
 *
 * ```js
 * el.add(command, fn); // exec
 * el.add(command, args, fn); // spawn
 * el.add(command, options, fn); // exec
 * el.add(command, args, options, fn); // spawn
 * ```
 *
 * @name add
 * @function
 * @param {String} command The command to run as string.
 * @param {Object} args The command arguments as array of strings (optional).
 * @param {Object} options The options passed to the spawn/exec function, but extended with the following fields:
 *
 *  - `ignoreStdout` (Boolean): If `false`, then the stdout output will be stored ant called back.
 *
 * @param {Function} callback The callback function.
 * @return {ExecLimiter} The `ExecLimiter` instance.
 */
ExecLimiter.prototype.add = function (command, args, options, callback) {

    var useExec = false,
        largs = [];

    // add(command, fn);
    if (typeof args === "function") {
        callback = args;
        options = {};
        args = [];
        useExec = true;
    } else if (typeof options === "function") {
        callback = options;
        // add(command, args, fn);
        if (Array.isArray(args)) {
            options = {};
            // add(command, options, fn);
        } else {
            options = args;
            args = [];
            useExec = true;
        }
    }

    // Prepare the arguments for the limit-it call
    if (useExec) {
        largs = [command, options];
    } else {
        largs = [command, args, options];
    }

    /*!
     * spawner
     * Spawns the process.
     *
     * @name spawner
     * @function
     * @param {String} command The command to run.
     * @param {Array} args The options array.
     * @param {Object} options The options passed to the spawn function, but extended with the following fields:
     *
     *  - `ignoreStdout` (Boolean): If `false`, then the stdout output will be stored ant called back.
     *
     * @param {Function} callback The callback function.
     */
    function spawner(command, args, options, callback) {

        var ignoreStdout = options.ignoreStdout;
        delete options.ignoreStdout;

        var child = ChildProcess.spawn(command, args, options),
            err = "",
            out = "";

        child.stderr.on("data", function (data) {
            err += data;
        });

        // By default, we don't store the stdout
        if (ignoreStdout === false) {
            child.stdout.on("data", function (chunk) {
                out += chunk;
            });
        }

        child.on("close", function (code) {
            var error = null;
            if (code) {
                error = new Error(err);
                error.code = code;
            }
            callback(error, out);
        });
    }

    this.limitIt.add(useExec ? ChildProcess.exec : spawner, largs, callback);
    return this;
};

module.exports = ExecLimiter;