// Dependencies
const Err = require("../lib");

// Basic usage
var e1 = new Err("Some nasty stuff happened");
console.log(e1);
// => [Error: Some nasty stuff happened]

// Provide the error code
var e2 = new Err("Some nasty stuff happened", "NASTY_STUFF_HAPPENED");
console.log(e2);
// => {
//   [Error: Some nasty stuff happened]
//   code: 'NASTY_STUFF_HAPPENED'
// }

// Provide the error code and some data
var e3 = new Err("Some nasty stuff happened", "NASTY_STUFF_HAPPENED", {
    additional: "data"
});
console.log(e3);
// => {
//   [Error: Some nasty stuff happened]
//   additional: 'data'
// , code: 'NASTY_STUFF_HAPPENED'
// }

// Provide the error data (including the code as well)
var e3 = new Err("Some nasty stuff happened", {
    additional: "data"
  , code: "NASTY_STUFF_HAPPENED"
});
console.log(e3);
// => {
//   [Error: Some nasty stuff happened]
//   additional: 'data'
// , code: 'NASTY_STUFF_HAPPENED'
// }

// Wrap an existing error
var existingError = new Error("Some nasty stuff happened");
var e4 = new Err(existingError, {
    additional: "data"
  , code: "NASTY_STUFF_HAPPENED"
  , and: "some more data"
});
console.log(e4);
// => {
//   [Error: Some nasty stuff happened]
//   additional: 'data'
// , code: 'NASTY_STUFF_HAPPENED'
// , and: 'some more data'
// }
