[![err](http://i.imgur.com/yQF0uDO.png)](#)

# err [![PayPal](https://img.shields.io/badge/%24-paypal-f39c12.svg)][paypal-donations] [![Version](https://img.shields.io/npm/v/err.svg)](https://www.npmjs.com/package/err) [![Downloads](https://img.shields.io/npm/dt/err.svg)](https://www.npmjs.com/package/err) [![Get help on Codementor](https://cdn.codementor.io/badges/get_help_github.svg)](https://www.codementor.io/johnnyb?utm_source=github&utm_medium=button&utm_term=johnnyb&utm_campaign=github)

> A tiny library to create custom errors in JavaScript.

## Installation

```sh
$ npm i --save err
```

## Example

```js
// Dependencies
const Err = require("err");

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
```

## Documentation

### `Err(error, code, data)`
Create a custom error object.

#### Params
- **String|Error** `error`: The error message or an existing `Error` instance.
- **String|Object** `code`: The error code or the data object.
- **Object** `data`: The data object (its fields will be appended to the `Error` object).

#### Return
- **Error** The custom `Error` instance.

## How to contribute
Have an idea? Found a bug? See [how to contribute][contributing].

## Where is this library used?
If you are using this library in one of your projects, add it in this list. :sparkles:

 - [`argon`](http://github.com/TrevorBurnham/argon) by Trevor Burnham

 - [`emoji-logger`](https://github.com/IonicaBizau/emoji-logger#readme)

 - [`engine-comp-crud-errors`](https://github.com/jillix/engine-comp-errors#readme) by jillix

 - [`engine-composition-adapter`](https://github.com/jillix/engine-composition-adapter#readme) by jillix

 - [`showalicense.com`](https://github.com/IonicaBizau/showalicense.com#readme)

## License

[MIT][license] © [Ionică Bizău][website]

[paypal-donations]: https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=RVXDDLKKLQRJW
[donate-now]: http://i.imgur.com/6cMbHOC.png

[license]: http://showalicense.com/?fullname=Ionic%C4%83%20Biz%C4%83u%20%3Cbizauionica%40gmail.com%3E%20(http%3A%2F%2Fionicabizau.net)&year=2015#license-mit
[website]: http://ionicabizau.net
[contributing]: /CONTRIBUTING.md
[docs]: /DOCUMENTATION.md