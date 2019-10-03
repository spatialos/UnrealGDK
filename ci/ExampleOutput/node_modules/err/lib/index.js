// Dependencies
var typpy = require("typpy");

/**
 * Err
 * Create a custom error object.
 *
 * @name Err
 * @function
 * @param {String|Error} error The error message or an existing `Error` instance.
 * @param {String|Object} code The error code or the data object.
 * @param {Object} data The data object (its fields will be appended to the `Error` object).
 * @return {Error} The custom `Error` instance.
 */
function Err(error, code, data) {

    // Create the error
    if (!typpy(error, Error)) {
        error = new Error(error);
    }

    // Err(message, code, data);
    // Err(message, data);
    if (typpy(data, Object)) {
        data.code = code;
    } else if (typpy(code, Object)) {
        data = code;
        code = undefined;
    } else if (!typpy(code, undefined)) {
        data = { code: code };
    }

    if (data) {
        Object.keys(data).forEach(function (c) {
            error[c] = data[c];
        });
    }

    return error;
}

module.exports = Err;
