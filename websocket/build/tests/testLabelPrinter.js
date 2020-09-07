"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
var pathTestCard = require("path");
var fs = require("fs");
var node_native_printer_1 = __importDefault(require("node-native-printer"));
node_native_printer_1.default.setPrinter("DYMO_LabelWriter_450");
fs.unlink("log.txt", function () {
    function mpTest(relPath) {
        return pathTestCard.resolve(__dirname, relPath);
    }
    var addressJSON = '{"address1":"610 Clematis House Test Cnr Test Road and Smith Street","address2":"Apt 426","state":"FL","zip":"33401","country":"US","city":"West Palm Beach"}';
    var _a = JSON.parse(addressJSON), address1 = _a.address1, address2 = _a.address2, state = _a.state, zip = _a.zip, country = _a.country, city = _a.city;
    var finalString = address1 + ', ' + address2 + "\n" + city + ', ' + state + '\n' + country + '\n' + zip;
    console.log("finalString", finalString);
    fs.appendFile("log.txt", finalString, function (err) {
        if (err) {
            console.log("err", err);
        }
        else {
            console.log("here");
            node_native_printer_1.default.print(mpTest("log.txt"), {
                landscape: true,
                InputSlot: "Left",
                pageSize: "24_mm__1___Label__Auto_",
            });
        }
    });
});
