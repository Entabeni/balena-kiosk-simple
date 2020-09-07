"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
var CardRead_1 = __importDefault(require("../printers/CardRead"));
var cardReader = new CardRead_1.default('ttyDesktopReader');
cardReader.startScanning();
cardReader.readCardNumber(function (cardNumberHex) {
    console.log('TCL: cardNumberHex', cardNumberHex);
    var last8Digits = cardNumberHex.substr(cardNumberHex.length - 8, cardNumberHex.length);
});
