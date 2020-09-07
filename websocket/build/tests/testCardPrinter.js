"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
var pathTestCard = require("path");
var uuidv4 = require("uuid/v4");
var fs = require("fs");
var https = require("https");
var node_native_printer_1 = __importDefault(require("node-native-printer"));
/**Ensure this is the correct printer Name on Pi */
node_native_printer_1.default.setPrinter("Magicard_600");
function mpTest(relPath) {
    return pathTestCard.resolve(__dirname, relPath);
}
var options = { fitplot: true };
console.log("options", options);
console.log("mpTest(`../kevin.pdf`)", mpTest("../kevin.pdf"));
var printString = node_native_printer_1.default.print(mpTest("../kevin.pdf"), options);
