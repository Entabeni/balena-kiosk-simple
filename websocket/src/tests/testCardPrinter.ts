const pathTestCard = require("path");
const uuidv4 = require("uuid/v4");
const fs = require("fs");
const https = require("https");
import printer from 'node-native-printer'



/**Ensure this is the correct printer Name on Pi */
printer.setPrinter("Magicard_600");

function mpTest(relPath) {
  return pathTestCard.resolve(__dirname, relPath);
}
const options = { fitplot: true };
console.log("options", options)
console.log("mpTest(`../kevin.pdf`)", mpTest(`../kevin.pdf`))
const printString = printer.print(mpTest(`../kevin.pdf`), options);
