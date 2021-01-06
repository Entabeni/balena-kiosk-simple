"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
var tokenService_1 = __importDefault(require("./tokenService"));
var ws_1 = __importDefault(require("./ws"));
// import StateMachine from "./stateMachine";
// import MessageQueue from "./messageQueue";
var CardPrinter_1 = __importDefault(require("./printers/CardPrinter"));
var ReceiptPrinter_1 = __importDefault(require("./printers/ReceiptPrinter"));
var CardScanner_1 = __importDefault(require("./printers/CardScanner"));
// export {};
function main() {
    // const state = new StateMachine();
    var ws = new ws_1.default();
    var ts = new tokenService_1.default();
    var receiptPrintObj = new ReceiptPrinter_1.default(ws, "receiptPrintJobs");
    var cardScanObj = new CardScanner_1.default(ws, "scanJobs");
    var cardPrintPObj = new CardPrinter_1.default(ws, "cardPrintJobs");
    ts.connect(function (token, baseUrl) {
        ws.subscribe(token, baseUrl, receiptPrintObj, cardScanObj, cardPrintPObj);
    });
}
main();
