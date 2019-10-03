"use strict";

var deffy = require("deffy");

/**
 * objDef
 * Easily set default fields in objects.
 *
 * @name objDef
 * @function
 * @param {Object} obj The input object.
 * @param {String} field The field name.
 * @param {Anything} defValue The default value.
 * @param {Object} opts The [`deffy`](https://github.com/IonicaBizau/deffy) options.
 * @return {Anything} The set value.
 */
module.exports = function objDef(obj, field, defValue, opts) {
  return obj[field] = deffy(obj[field], defValue, opts);
};